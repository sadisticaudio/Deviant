#pragma once
#include <JuceHeader.h>
#include "sadistic.h"
class DeviantProcessor  :  public AudioProcessor,
                            public AudioProcessorValueTreeState::Listener
{
    NormalisableRange<float> filterRange        {20.0f, 1000.f, 0.f, 3.f, true};
    NormalisableRange<float> gainRange          {0.0f, 1.f, 0.f, 0.8f, false};
    NormalisableRange<float> gateRange          {1.f, 1000.f, 0.f, 0.3f, true};
    NormalisableRange<float> saturationRange    {2.f, 50.f, 0.f, 1.f, true};
    NormalisableRange<float> driveRange         {1.f, 1000.f, 0.f, 1.f, true};
    NormalisableRange<float> blendRange         {0.f, 100.f, 0.f, 1.f, true};
    NormalisableRange<float> volumeRange        {0.0f, 1.f, 0.f, 0.2f, false};
    
    Atomic<float>   mFilter     { 20.f };
    Atomic<float>   mGain       { 1.f };
    Atomic<float>   mGate       { 1.f };
    Atomic<float>   mSaturation { 2.f };
    Atomic<float>   mDrive      { 1.f };
    Atomic<float>   mBlend      { 100.f };
    Atomic<float>   mVolume     { 1.f };
    
    dsp::ProcessorDuplicator <dsp::IIR::Filter<float>, dsp::IIR::Coefficients <float>> highPassFilter;
    
    double lastSampleRate;
    float gain, gate, saturation, drive, blend, volume;
    
    UndoManager uM;
    AudioProcessorValueTreeState state;
    sadistic::Queue mBuffer;
    
public:
    DeviantProcessor();
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void parameterChanged (const String &parameterID, float newValue) override;
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif
    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void updateFilter();
    AudioProcessorValueTreeState& getValueTreeState();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeviantProcessor)
};
