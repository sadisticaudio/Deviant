#include "PluginProcessor.h"
#include "PluginEditor.h"
using Parameter = AudioProcessorValueTreeState::Parameter;
DeviantProcessor::DeviantProcessor()  : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  AudioChannelSet::stereo(), true)
#endif
                  .withOutput ("Output", AudioChannelSet::stereo(), true)
#endif
    ), state(*this, &uM, "PARAMETERS", createParameterLayout())
{
    state.addParameterListener ("filter", this);
    state.addParameterListener ("gain", this);
    state.addParameterListener ("gate", this);
    state.addParameterListener ("saturation", this);
    state.addParameterListener ("drive", this);
    state.addParameterListener ("blend", this);
    state.addParameterListener ("volume", this);
}

AudioProcessorValueTreeState::ParameterLayout DeviantProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<RangedAudioParameter>> params;
    params.push_back (std::make_unique<AudioParameterFloat> ("filter", "Filter", filterRange, mFilter.get(), " Hz"));
    params.push_back (std::make_unique<AudioParameterFloat> ("gain", "Gain", gainRange, mGain.get(), " dB"));
    params.push_back (std::make_unique<AudioParameterFloat> ("gate", "Gate", gateRange, mGate.get(), " dB"));
    params.push_back (std::make_unique<AudioParameterFloat> ("saturation", "Saturation", saturationRange, mSaturation.get(), " :)"));
    params.push_back (std::make_unique<AudioParameterFloat> ("drive", "Drive", driveRange, mDrive.get(), " X/"));
    params.push_back (std::make_unique<AudioParameterFloat> ("blend", "Blend", blendRange, mBlend.get(), " %"));
    params.push_back (std::make_unique<AudioParameterFloat> ("volume", "Volume", volumeRange, mVolume.get(), " dB"));
    return { params.begin(), params.end() };
}

void DeviantProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
    lastSampleRate = sampleRate;
    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    highPassFilter.prepare(spec);
    highPassFilter.reset();
    
    mBuffer.prepare(getTotalNumInputChannels(), samplesPerBlock);
    setLatencySamples(mBuffer.getAnchor());
}

void DeviantProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) {
    gain = mGain.get();
    gate = mGate.get();
    saturation = mSaturation.get();
    drive = mDrive.get();
    blend = mBlend.get();
    volume = mVolume.get();
    
    ScopedNoDenormals noDenormals;
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    AudioSampleBuffer preBuffer;
    preBuffer.makeCopyOf(buffer);
    dsp::AudioBlock <float> block (buffer);
    updateFilter();
    highPassFilter.process(dsp::ProcessContextReplacing<float> (block));
    
    mBuffer.process(buffer, *state.getRawParameterValue("saturation"), *state.getRawParameterValue("blend"));
    
//    for (int channel = 0; channel < getTotalNumInputChannels(); ++channel)
//    {
//        const float* cleanData   = preBuffer.getReadPointer (channel);
//        float* channelData = buffer.getWritePointer (channel);
//
//        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
//        {
//            float saturationX = 1;
//            float driveNormal = (drive - driveRange.start) / (driveRange.end - driveRange.start);
//            auto saturationNormal = (saturationX - 2) / (50 - 2);
//
//            float driveFactor = pow(0.1f,pow(driveNormal,0.1f));
//            float saturationFactor = pow(0.15f,pow(saturationNormal,0.2f));
//            float attenuationFactor = pow(driveFactor,pow(saturationFactor,0.4f)) * pow(saturationFactor,pow(driveFactor,0.4f));
//
//            float cleanestSig = *channelData;
//            float cleanerSig = cleanestSig * gain;
//
//            float gateOffset = (((gate + 1) / 2) - 1) / ((gate + 1) / 2);
//
//            float cleanSig = (-1.f -gateOffset + (2.f/(1.f + (1.f / gate) * pow(exp(-saturationX * cleanerSig),(drive))))) * attenuationFactor;
//            float filthySig = cleanSig * (blend/100.f) + (*cleanData * gain) * (1 - (blend/100.f));
//            *channelData = filthySig * volume;
//
//            channelData++;
//            cleanData++;
//        }
//    }
}

void DeviantProcessor::releaseResources() {}
#ifndef JucePlugin_PreferredChannelConfigurations
bool DeviantProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    return true;
#endif
}
#endif
void DeviantProcessor::parameterChanged (const String &parameterID, float newValue) {
    if (parameterID == "filter") {
        mFilter = newValue;
    }
    else if (parameterID == "gain") {
        mGain = newValue;
    }
    else if (parameterID == "gate") {
        mGate = newValue;
    }
    else if (parameterID == "saturation") {
        mSaturation = newValue;
    }
    else if (parameterID == "drive") {
        mDrive = newValue;
    }
    else if (parameterID == "blend") {
        mBlend = newValue;
    }
    else if (parameterID == "volume") {
        mVolume = newValue;
    }
}
void DeviantProcessor::updateFilter() {
    float freq = *state.getRawParameterValue("filter");
    *highPassFilter.state = *dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(lastSampleRate, freq);
}
AudioProcessorValueTreeState& DeviantProcessor::getValueTreeState() {
    return state;
}
const String DeviantProcessor::getName() const {
    return JucePlugin_Name;
}
bool DeviantProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}
bool DeviantProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}
bool DeviantProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}
double DeviantProcessor::getTailLengthSeconds() const { return 0.0; }
int DeviantProcessor::getNumPrograms() { return 1; }
int DeviantProcessor::getCurrentProgram() { return 0; }
void DeviantProcessor::setCurrentProgram (int index) {}
const String DeviantProcessor::getProgramName (int index) { return {}; }
void DeviantProcessor::changeProgramName (int index, const String& newName) {}
bool DeviantProcessor::hasEditor() const {return true;}
AudioProcessorEditor* DeviantProcessor::createEditor() { return new DeviantEditor (*this); }
void DeviantProcessor::getStateInformation (MemoryBlock& destData) {
    MemoryOutputStream stream(destData, false);
    state.state.writeToStream (stream);
}
void DeviantProcessor::setStateInformation (const void* data, int sizeInBytes) {
    ValueTree tree = ValueTree::readFromData (data, sizeInBytes);
    if (tree.isValid()) {
        state.state = tree;
    }
}
AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DeviantProcessor(); }
