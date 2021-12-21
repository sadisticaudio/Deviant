#pragma once
#include "Members.h"
namespace sadistic {
    class Deviant  :  public AudioProcessor {
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
        void processBlockBypassed (AudioBuffer<float>& b, MidiBuffer&) override;
        void processBlockBypassed (AudioBuffer<double>& b, MidiBuffer&) override;
        bool supportsDoublePrecisionProcessing() const override { return true; }
        const String getName() const override { return "ddd"; }//JucePlugin_Name; }
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
        bool hasEditor() const override { return true; }
        APVTS& getAPVTS() { return apvts; }
        TableManager& getTableManager() { return mgmt; }
        void setCurrentScreen(int s) { apvts.state.setProperty("currentScreen", s, &undoManager); }
        int getCurrentScreen() { return apvts.state.getProperty("currentScreen", 1); }
        UndoManager* getUndoManager() { return &undoManager; }
        LongFifo<float>* getOscilloscopeFifo() { return oscilloscopeFifo; }

        void getStateInformation (MemoryBlock& destinationBlockForAPVTS) override {
            MemoryOutputStream stream(destinationBlockForAPVTS, false);
            apvts.state.writeToStream (stream); }
        void setStateInformation (const void* dataFromHost, int size) override {
            auto treeCreatedFromData { ValueTree::readFromData (dataFromHost, static_cast<size_t>(size)) };
            if (treeCreatedFromData.isValid()) apvts.state = treeCreatedFromData; }

        template<typename FloatType> void release(DeviantMembers<FloatType>& m) { m.reset(); }
        template<typename FloatType> void prepare(double sampleRate, int samplesPerBlock, DeviantMembers<FloatType>& m) {
            auto channels { jmin(getMainBusNumInputChannels(), 2, getMainBusNumOutputChannels()) };
            dsp::ProcessSpec spec { sampleRate, (uint32) samplesPerBlock, (uint32) channels };
            m.prepare(spec);
            const auto latency { m.filterA.getLatency() + m.filterB.getLatency() + m.dynamicWaveShaper.getLatency() };
            setLatencySamples(latency + BUFFERLENGTH);
            m.blendDelay.setDelay(latency);
            m.spectralInversionDelay1.setDelay(m.filterA.getLatency());
            m.spectralInversionDelay2.setDelay(m.dynamicWaveShaper.getLatency() + m.filterB.getLatency());

        }
        
        template<typename FloatType> void processTheDamnBlock(AudioBuffer<FloatType>& buffer, DeviantMembers<FloatType>& m, bool bypassed = false) {
            const int numIns { getMainBusNumInputChannels() }, numOuts { getMainBusNumOutputChannels() }, channels { jmin(numIns, numOuts, 2) }, samples { buffer.getNumSamples() };
            ScopedNoDenormals noDenormals;
            AudioBuffer<FloatType> mainBuffer { buffer.getArrayOfWritePointers(), channels, samples };
            m.processBlock(mainBuffer, oscilloscopeFifo, bypassed);
        }
        
    protected:
        BusesProperties getDefaultBusesProperties() {
            auto buses = BusesProperties().withInput ("Input", AudioChannelSet::stereo(), true).withOutput ("Output", AudioChannelSet::stereo(), true);
            return buses;
        }
        
    private:
        std::atomic<int> cIdx[6] { 0,0,0,0,0,0 };
        float coefficients[maxCoeffs][maxCoeffs][maxCoeffs];
        DeviantMembers<double> membersD;
        DeviantMembers<float> membersF;
        TableManager mgmt;
        UndoManager undoManager;
        APVTS apvts;
        sadistic::SadisticMarketplaceStatus marketplaceStatus { "hieF" };
        LongFifo<float> oscilloscopeFifo[2]{};
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Deviant)
    };
}
