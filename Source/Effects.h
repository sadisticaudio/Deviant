#pragma once
#include "deviant.h"

namespace sadistic {
    
    template <typename Effect, typename ...Ts>
    static Effect createEffect (APVTS::ParameterLayout& layout, int effectIndex, Ts&&... ts) {
        ParamList refs;
        addParameter<AudioParameterBool>(layout, refs, getFxID(effectIndex) + "Enabled", getFxName(effectIndex) + " Enabled", effectInfo[effectIndex].defaultEnabled);
        addParameter<AudioParameterInt>(layout, refs, getFxID(effectIndex) + "Route", getFxName(effectIndex) + " Route", 0, 99, effectInfo[effectIndex].defaultRoute);
        addParameter<AudioParameterInt>(layout, refs, getFxID(effectIndex) + "Index", getFxName(effectIndex) + " Index", 0, 99, effectInfo[effectIndex].defaultIndex);
        addParameter<AudioParameterFloat>(layout, refs, getFxID(effectIndex) + "Blend", getFxName(effectIndex) + " Blend", 0.f, 1.f, effectInfo[effectIndex].defaultBlend);
        FloatParamList floatRefs;
        for(int i { 0 }; !String(paramID[effectIndex][i]).isEmpty() && i < 4; ++i) {
            ParamInfo info = paramInfo[effectIndex][i];
            addParameter<AudioParameterFloat>(layout, floatRefs, getParamID(effectIndex, i), getParamName(effectIndex, i), NormalisableRange<float>(info.min, info.max), info.defaultValue, getSuffix(info.type));
        }
        return Effect(getFxID(effectIndex), refs, floatRefs, effectIndex, std::forward<Ts>(ts)...);
    }
    
    template<typename ShaperType, typename F>
    struct Shaper : public DeviantEffect {
        Shaper(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, std::atomic<int>& cI, float(& cS)[maxCoeffs][maxCoeffs]) :
        DeviantEffect(eID, refs, floatRefs, eIDX), coeffIdx(cI), coeffArray(cS) {}
        template<typename OtherFloatType> Shaper(Shaper<ShaperType, OtherFloatType>& other) : DeviantEffect(other), coeffIdx(other.coeffIdx), coeffArray(other.coeffArray) {}
        int getLatency() override { return (int) filter.state->getFilterOrder()/2; }
        void prepare(const ProcessSpec& spec) override {
            mSampleRate = jlimit(44100.0, 192000.0, spec.sampleRate);
            spectralInversionDelay.setDelay(getLatency());
            spectralInversionDelay.prepare(spec);
            spectralInversionBuffer.setSize((int) spec.numChannels, (int) spec.maximumBlockSize);
            spectralInversionBuffer.clear();
            filter.prepare({ mSampleRate, spec.maximumBlockSize, spec.numChannels }); update(); }
        void processSamples(AudioBuffer<float>& buffer) override { processSamples<float>(buffer); }
        void processSamples(AudioBuffer<double>& buffer) override { processSamples<double>(buffer); }
        void update(){ *filter.state = *makeBandpass<F>(mLow, mHigh, mSampleRate, 8, WindowingFunction<F>::kaiser, F(4)); }
        void calculateCoefficients() override {
            ShaperType::calculateCoefficients(coeffs);
            if (coeffs[1] != mLow || coeffs[2] != mHigh) { mLow = coeffs[1]; mHigh = coeffs[2]; update(); }
            int currentIndex { coeffIdx };
            float* newCoeffSet { coeffArray[++currentIndex%=maxCoeffs] };
            for (int i { 0 }; i < maxCoeffs; ++i) newCoeffSet[i] = coeffs[i];
            coeffIdx = currentIndex;
        }
        template<typename FloatType> void processSamples(AudioBuffer<FloatType>& buffer) {
            if constexpr (std::is_same<F, FloatType>::value) {

                spectralInversionDelay.process(buffer, spectralInversionBuffer);
                
                AudioBlock<FloatType> block { buffer }, spectralInversionBlock { spectralInversionBuffer };
                
                filter.process(ProcessContextReplacing<F>(block));
                //subtract out the filtered signal from the spectralInversionBlock
                spectralInversionBlock -= block;
                const float mag { static_cast<float>(buffer.getMagnitude(0, buffer.getNumSamples())) };
                coeffs[5] = jlimit(0.00001f, 1.f, mag);
                for (int channel { 0 }; channel < buffer.getNumChannels(); ++channel) {
                    F* channelData { buffer.getWritePointer (channel) };
                    for (int sample { 0 }; sample < buffer.getNumSamples(); sample++, channelData++) {
                        *channelData = ShaperType::processSample(*channelData, coeffs);
                        jassert(!isnan(*channelData));
                    }
                }
                //add back in the spectral inverse of the filter, this part of the signal was not meant to be distorted
                block += spectralInversionBlock;
            }
        }
        AudioBuffer<F> spectralInversionBuffer;
        std::atomic<int>& coeffIdx;
        float(& coeffArray)[maxCoeffs][maxCoeffs];
        double mSampleRate { 44100.0 };
        F mLow { F(20.0) }, mHigh { F(20000.0) };
        ProcessorDuplicator<FIR::Filter<F>, FIR::Coefficients<F>> filter;
        sadistic::DelayBuffer<F> spectralInversionDelay;
    };
}
