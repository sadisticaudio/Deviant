#pragma once
#include "Members.h"
namespace sadistic {
    class Deviant  :  public AudioProcessor, public APVTS::Listener {
    public:
        
        Deviant(APVTS::ParameterLayout);
        bool canApplyBusCountChange (bool isInput, bool isAddingBuses, BusProperties& outNewBusProperties) override;
        bool canAddBus (bool) const override;
        bool canRemoveBus (bool) const override;
        void prepareToPlay (double, int) override;
        void releaseResources() override { if(getProcessingPrecision() == doublePrecision) release<double>(membersD);
            else release<float>(membersF); }
        bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
        void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
        void processBlock (AudioBuffer<double>&, MidiBuffer&) override;
        bool supportsDoublePrecisionProcessing() const override { return true; }
        const String getName() const override { return JucePlugin_Name; }
        bool acceptsMidi() const override { return false; }
        bool producesMidi() const override { return false; }
        bool isMidiEffect() const override { return false; }
        double getTailLengthSeconds() const override { return 0.0; }
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram (int) override {}
        const String getProgramName (int) override { return {}; }
        void changeProgramName (int, const String&) override {}
        AudioProcessorEditor* createEditor() override;
        APVTS& getAPVTS() { return apvts; }
        bool hasEditor() const override { return true; }

        void getStateInformation (MemoryBlock& destinationBlockForAPVTS) override {
            MemoryOutputStream stream(destinationBlockForAPVTS, false);
            apvts.state.writeToStream (stream);
        }
        
        void setStateInformation (const void* dataFromHost, int size) override {
            auto treeCreatedFromData { ValueTree::readFromData (dataFromHost, static_cast<size_t>(size)) };
            if (treeCreatedFromData.isValid()) apvts.state = treeCreatedFromData;
        }
        
        template<typename FloatType> void setGainTable(DeviantMembers<FloatType>& m) {
            FloatType arr[WAVELENGTH + 1];
            for (int i { 0 }; i < WAVELENGTH + 1; ++i) arr[i] = FloatType(-1) + FloatType(2 * i) / FloatType(WAVELENGTH);
            FloatType* mockChannel[1] = { arr };
            AudioBuffer<FloatType> buf { mockChannel, 1, WAVELENGTH + 1 };
//            m.dynamicBitCrusher.processSamples(buf);
            m.dynamicAtan.processSamples(buf);
            m.dynamicDeviation.processSamples(buf);
            DeviantTree::setGainTable(apvts, &undoManager, arr);
        }
        
        template<typename FloatType> void setWaveTable(DeviantMembers<FloatType>& m) {
            FloatType arr[WAVELENGTH + 1];
            for (int i { 0 }; i < WAVELENGTH + 1; ++i) arr[i] = FloatType(-1) + FloatType(2 * i) / FloatType(WAVELENGTH);
            FloatType* mockChannel[1] = { arr };
            AudioBuffer<FloatType> buf { mockChannel, 1, WAVELENGTH + 1 };
            m.staticAtan.processSamples(buf);
            m.staticBitCrusher.processSamples(buf);
            m.staticDeviation.processSamples(buf);
            DeviantTree::setWaveTable(apvts, &undoManager, arr);
        }
        void setGainTable() { if(getProcessingPrecision() == doublePrecision) setGainTable(membersD); else setGainTable(membersF); }
        void setWaveTable() { if(getProcessingPrecision() == doublePrecision) setWaveTable(membersD); else setWaveTable(membersF); }
        
        void parameterChanged (const String& parameterID, float) override {
            if (parameterID.contains("Index") || parameterID.contains("Route")) needsResorting = true;
            else if (parameterID.contains("static")) setWaveTable();
            else setGainTable();
        }
        
        template<typename FloatType> void release(DeviantMembers<FloatType>& m) { m.reset(); m.oversampler.reset(); }
        template<typename FloatType> void prepare(double sampleRate, int samplesPerBlock, DeviantMembers<FloatType>& m) {
            auto channels { jmin((uint32) getMainBusNumInputChannels(), (uint32) 2, (uint32) getMainBusNumOutputChannels()) };
            dsp::ProcessSpec spec { sampleRate, (uint32) samplesPerBlock, channels };
            m.prepare(spec);
            const auto latency { static_cast<int>(m.lpf.state->getFilterOrder()/2 + m.lpf2.state->getFilterOrder()/2) +
                m.filterA.getLatency() + m.filterB.getLatency() +
                m.dynamicWaveShaper.getLatency() };
            setLatencySamples(latency);
            m.blendDelay1.setDelay(latency);
            m.spectralInversionDelay1.setDelay(static_cast<int>(m.lpf.state->getFilterOrder()/2));
            m.spectralInversionDelay2.setDelay(m.dynamicWaveShaper.getLatency() + m.filterA.getLatency() + m.filterB.getLatency() + static_cast<int>(m.lpf2.state->getFilterOrder()/2));
            m.oversampler.initProcessing(size_t(samplesPerBlock));
        }
        
        template<typename FloatType> void processBuffered(AudioBuffer<FloatType>& buffer, DeviantMembers<FloatType>& m) {
            m.process(buffer, [&, this](AudioBuffer<FloatType>& buf) { processTheDamnBlock(buf, m); });
        }

        template<typename FloatType> void processTheDamnBlock(AudioBuffer<FloatType>& buffer, DeviantMembers<FloatType>& m) {
            const int numIns { getMainBusNumInputChannels() }, numOuts { getMainBusNumOutputChannels() }, channels { jmin(numIns, numOuts, 2) }, samples { buffer.getNumSamples() };
            ScopedNoDenormals noDenormals;
            const auto blend { static_cast<FloatType>(m.params[0].get().get()/FloatType(100)) };
            AudioBuffer<FloatType> mainBuffer { buffer.getArrayOfWritePointers(), channels, samples };
            
            //create references to our member buffers
            auto& blendBuffer { m.blendBuffer }, & spectralInversionBuffer { m.spectralInversionBuffer };
            
            //copy what's in our buffer to our blend buffer, delaying it by the total latency
            //this delay will resize our buffer to the current buffer length
            //and retain samples needed in subsequent processBlock calls
            m.blendDelay1.process(mainBuffer, blendBuffer);

            //copy what's in our buffer to our spectral inversion buffer, delaying it by the latency of the pre-filtering
            m.spectralInversionDelay1.process(mainBuffer, spectralInversionBuffer);

            AudioBlock<FloatType> block { mainBuffer }, spectralInversionBlock { spectralInversionBuffer }, blendBlock { blendBuffer };
            m.lpf.process(ProcessContextReplacing<FloatType>(block));
            
            //subtract out the filtered signal from the spectralInversionBlock
            spectralInversionBlock -= block;
            
            //shape the wave
//            auto upBlock { m.oversampler.processSamplesUp(block) };
            
//            for (int j { 0 }; j < buffer.getNumChannels(); ++j)
//                waveShaper[j].process(writeFifo[j], /* upBlock.getChannelPointer(j), */ FloatType(drive), crushMax, crushBlend);
            
//            AudioBuffer<FloatType> upBuffer { upBlock.channels, buffer.getNumChannels(), buffer.getNumSamples() };
            
            m.dynamicWaveShaper.processSamples(mainBuffer);
            
//            m.oversampler.processSamplesDown(block);
//
//            for (int j { 0 }; j < buffer.getNumChannels(); ++j)
//                m.interpolator[j].process(FloatType(1) / FloatType(m.dFactor), upBlock.getChannelPointer(j), block.getChannelPointer(j), m.bufferLength);
            
            m.filterA.process(mainBuffer);
            m.staticWaveShaper.process(mainBuffer);
            m.filterB.process(mainBuffer);
//            m.staticAtan.process(mainBuffer);
//            m.staticDeviation.process(mainBuffer);
//            m.staticBitCrusher.process(mainBuffer);
            
            m.lpf2.process(ProcessContextReplacing<FloatType>(block));
            
            //push the dry and wet signals seperately to the oscilloscope
            oscilloscopeFifo[drySignal].pushChannel(blendBlock);
            oscilloscopeFifo[wetSignal].pushChannel(block);
            
            
            
            //delay the spectralInversionBuffer the rest of the way
            m.spectralInversionDelay2.process(spectralInversionBuffer);
//
//            m.sort();
//
//            for (int i { 0 }; i < m.getFilterAIndex(); ++i) m.effects[i]->process(mainBuffer);
//            spectralInversionBuffer.makeCopyOf(mainBuffer);
//            m.effects[m.getFilterAIndex()]->process(mainBuffer);

//            for (int i { m.getFilterAIndex() + 1 }; i < numFX; ++i) m.effects[i]->process(mainBuffer);
            
            //add back in the spectral inverse of the first filter(s), this part of the signal was not meant to be distorted
//            block += spectralInversionBlock;

            //attenuate the signals according to the blend parameter
            block *= blend;
            blendBlock *= (FloatType(1) - blend);
            
            //output the sum of both signals
            block += blendBlock;
        }
        
        UndoManager* getUndoManager() { return &undoManager; }
        LongFifo<float>* getOscilloscopeFifo() { return oscilloscopeFifo; }
        
    protected:
        BusesProperties getDefaultBusesProperties() {
            auto buses = BusesProperties().withInput ("Input", AudioChannelSet::stereo(), true).withOutput ("Output", AudioChannelSet::stereo(), true);
            return buses;
        }
        
    private:
        enum { wetSignal = 0, drySignal = 1, numSignals };
        DeviantMembers<double> membersD;
        DeviantMembers<float> membersF;
        UndoManager undoManager;
        APVTS apvts;
        DeviantTree deviantTree;
        sadistic::SadisticMarketplaceStatus marketplaceStatus { "hieF" };
        LongFifo<float> oscilloscopeFifo[2]{};
        std::atomic<bool> needsResorting { true };
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Deviant)
    };
}
