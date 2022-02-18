#pragma once
#include "Members.h"
namespace sadistic {
    class Deviant  :  public AudioProcessor {
    public:
        //defined in cpp
        Deviant(APVTS::ParameterLayout);
        AudioProcessorEditor* createEditor() override;
        
        //AudioProcessor overrides
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
        
        bool isBusesLayoutSupported (const BusesLayout& layouts) const override {
            return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet() &&
            !layouts.getMainInputChannelSet().isDisabled();
        }
        
        void getStateInformation (MemoryBlock& destinationBlockForAPVTS) override {
            MemoryOutputStream stream(destinationBlockForAPVTS, false);
            apvts.state.writeToStream (stream);
        }
        
        void setStateInformation (const void* dataFromHost, int size) override {
            auto treeCreatedFromData { ValueTree::readFromData (dataFromHost, static_cast<size_t>(size)) };
            if (treeCreatedFromData.isValid()) apvts.state = treeCreatedFromData;
        }
        
        void releaseResources() override { usingDouble() ? release(membersD) : release(membersF); }
        void prepareToPlay (double sR, int) override { usingDouble() ? prepare(sR, membersD) : prepare(sR, membersF); }
        void processBlock (AudioBuffer<float>& buffer, MidiBuffer&) override { process(buffer, membersF, false); }
        void processBlock (AudioBuffer<double>& buffer, MidiBuffer&) override { process(buffer, membersD, false); }
        void processBlockBypassed (AudioBuffer<float>& b, MidiBuffer&) override { process(b, membersF, true); }
        void processBlockBypassed (AudioBuffer<double>& b, MidiBuffer&) override { process(b, membersD, true); }
        
        //extra function definitions/templates
        BusesProperties getDefaultBusesProperties() {
            auto buses = BusesProperties().withInput ("Input", AudioChannelSet::stereo(), true).withOutput ("Output", AudioChannelSet::stereo(), true);
            return buses; }
        //this function is used to determine wether to use float or double "members" - see Members.h
        bool usingDouble() const { return getProcessingPrecision() == doublePrecision; }
        APVTS& getAPVTS() { return apvts; }
        UndoManager* getUndoManager() { return &undoManager; }
        LongFifo<float>* getOscilloscopeFifo() { return oscilloscopeFifo; }
        void setCurrentScreen(int s) { apvts.state.setProperty("mainCurrentScreen", var(int(s)), &undoManager); }
        int getCurrentScreen() const { return static_cast<int>(apvts.state.getProperty("mainCurrentScreen")); }

        template<typename FloatType> void release(DeviantMembers<FloatType>& m) { m.reset(); }
        template<typename FloatType> void prepare(double sampleRate, DeviantMembers<FloatType>& m) {
            auto channels { jmin(getMainBusNumInputChannels(), 2, getMainBusNumOutputChannels()) };
            dsp::ProcessSpec spec { sampleRate, (uint32) BUFFERLENGTH, (uint32) channels };
            m.prepare(spec);
            setLatencySamples(m.getLatency());
        }
        
        template<typename FloatType> void process(AudioBuffer<FloatType>& buffer, DeviantMembers<FloatType>& m, bool bypassed) {
            const int numIns { getMainBusNumInputChannels() }, numOuts { getMainBusNumOutputChannels() }, channels { jmin(numIns, numOuts, 2) }, samples { buffer.getNumSamples() };
            ScopedNoDenormals noDenormals;
            AudioBuffer<FloatType> mainBuffer { buffer.getArrayOfWritePointers(), channels, samples };
            m.processBlock(mainBuffer, oscilloscopeFifo, bypassed);
        }

    private:
        DeviantMembers<double> membersD;
        DeviantMembers<float> membersF;
        UndoManager undoManager;
        APVTS apvts;
    public:
        SadisticMarketplaceStatus marketplaceStatus { getPluginCodeString(JucePlugin_PluginCode) };
    private:
        LongFifo<float> oscilloscopeFifo[2]{};
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Deviant)
    };
    
    //Declaration of function that creates the GUI - defined in PluginEditor.cpp
    AudioProcessorEditor* createDeviantEditor(Deviant&);
} // namespace sadistic
