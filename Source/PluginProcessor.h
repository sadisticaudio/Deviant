#pragma once
#include "Members.h"
namespace sadistic {
    class Deviant  :  public AudioProcessor {
    public:
        
        Deviant(APVTS::ParameterLayout);
        
        //AudioProcessor overrides - defined in cpp
        bool canApplyBusCountChange (bool isInput, bool isAddingBuses, BusProperties& outNewBusProperties) override;
        bool canAddBus (bool) const override;
        bool canRemoveBus (bool) const override;
        void prepareToPlay (double, int) override;
        void releaseResources() override { if(getProcessingPrecision() == doublePrecision) release(membersD);
            else release(membersF); }
        bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
        void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
        void processBlock (AudioBuffer<double>&, MidiBuffer&) override;
        void processBlockBypassed (AudioBuffer<float>& b, MidiBuffer&) override;
        void processBlockBypassed (AudioBuffer<double>& b, MidiBuffer&) override;
        AudioProcessorEditor* createEditor() override;
        
        //AudioProcessor overrides - defined here
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
        bool hasEditor() const override { return true; }
        void getStateInformation (MemoryBlock& destinationBlockForAPVTS) override {
            MemoryOutputStream stream(destinationBlockForAPVTS, false);
            apvts.state.writeToStream (stream); }
        void setStateInformation (const void* dataFromHost, int size) override {
            auto treeCreatedFromData { ValueTree::readFromData (dataFromHost, static_cast<size_t>(size)) };
            if (treeCreatedFromData.isValid()) apvts.state = treeCreatedFromData; }
        
        //extra function definitions/templates
        APVTS& getAPVTS() { return apvts; }

        void setCurrentScreen(int s) {
            apvts.state.setProperty("mainCurrentScreen", var(int(s)), &undoManager);
        }
        int getCurrentScreen() const { return static_cast<int>(apvts.state.getProperty("mainCurrentScreen")); }
        UndoManager* getUndoManager() { return &undoManager; }
        LongFifo<float>* getOscilloscopeFifo() { return oscilloscopeFifo; }

        template<typename FloatType> void release(DeviantMembers<FloatType>& m) { m.reset(); }
        template<typename FloatType> void prepare(double sampleRate, int samplesPerBlock, DeviantMembers<FloatType>& m) {
            auto channels { jmin(getMainBusNumInputChannels(), 2, getMainBusNumOutputChannels()) };
            dsp::ProcessSpec spec { sampleRate, (uint32) samplesPerBlock, (uint32) channels };
            m.prepare(spec);
            setLatencySamples(m.getLatency());
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
        float coefficients[numFX][maxCoeffs][maxCoeffs];
        std::atomic<int> cIdx[numFX] { 0,0,0,0,0 };
        DeviantMembers<double> membersD;
        DeviantMembers<float> membersF;
        UndoManager undoManager;
        APVTS apvts;
        sadistic::SadisticMarketplaceStatus marketplaceStatus { getPluginCodeString(JucePlugin_PluginCode) };
        LongFifo<float> oscilloscopeFifo[2]{};
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Deviant)
    };
    AudioProcessorEditor* createDeviantEditor(Deviant&);
}
