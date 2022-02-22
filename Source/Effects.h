#pragma once
#include "deviant.h"

namespace sadistic {
    
    template<typename F> F rround(F f) { return f > F(0) ? floor (f + F(0.5)) : ceil(f - F(0.5)); }
    template<typename F> F fastatan( F x ) { return (F(2)/MathConstants<F>::pi) * atan(x * MathConstants<F>::halfPi); }
    
    struct Atan {
        static void calculateCoefficients(float(& coeffs)[maxCoeffs]) {
            auto& [drive, lo, hi, deviation, mDrive, mag, attenuation, blend] = coeffs;
            mDrive = powf(drive/111.f, 2.f);
            attenuation = jmap(powf(drive/111.f, 0.08f), 1.f, 0.14f);
        }
        template<typename F> static F processSample(const F sample, const float (&coeffs)[maxCoeffs]) {
            const auto& [drive, lo, hi, deviation, mDrive, mag, attenuation, blend] = coeffs;
            const auto x { static_cast<float>(sample) };
            return fastatan(x * (1.f + mDrive * 144.f)) * attenuation * blend + x * (1.f - blend);
        }
    };
    
    struct Crusher {
        static void calculateCoefficients(float(& coeffs)[maxCoeffs]) {
            auto& [drive, lo, hi, deviation, mDrive, mag, attenuation, blend] = coeffs;
            mDrive = powf(((drive * 0.75f)/111.f) * 0.75f - 1.f, 16.f) * 1024.f + 1.f;
            attenuation = 1.f - 0.3f * (drive/111.f);
        }
        template<typename F> static F processSample(const F sample, const float (&coeffs)[maxCoeffs]) {
            const auto& [drive, lo, hi, deviation, mDrive, mag, attenuation, blend] = coeffs;
            const auto x { static_cast<float>(sample) };
            return (rround((x/mag + 1.f) * mDrive) / mDrive - 1.f) * mag * attenuation * blend + x * (1.f - blend);
        }
    };
    
    struct Clipper {
        static void calculateCoefficients(float(& coeffs)[maxCoeffs]) {
            auto& [drive, lo, hi, deviation, mDrive, mag, attenuation, blend] = coeffs;
            mDrive = powf(drive/111.f, 2.f);
            attenuation = jmap(powf(mDrive, 0.04f), 1.f, 0.14f);
        }
        template<typename F> static F processSample(const F sample, const float (&coeffs)[maxCoeffs]) {
            const auto& [drive, lo, hi, deviation, mDrive, mag, attenuation, blend] = coeffs;
            const auto x { static_cast<float>(sample) };
            return jlimit(-1.f, 1.f, x * (1.f + mDrive * 144.f)) * attenuation * blend + x * (1.f - blend);
        }
    };
    
    struct Logistic {
        static void calculateCoefficients(float(& coeffs)[maxCoeffs]) {
            auto& [drive, lo, hi, deviation, mDrive, mag, attenuation, blend] = coeffs;
            mDrive = sadSymmetricSkew(drive/111.f, -0.96f);
            attenuation = jmap(powf(drive/111.f * 0.5f, 0.1f), 1.f, 0.03f);
        }
        template<typename F> static F processSample(const F sample, const float (&coeffs)[maxCoeffs]) {
            const auto& [drive, lo, hi, deviation, mDrive, mag, attenuation, blend] = coeffs;
            const auto x { static_cast<float>(sample) };
            return (-1.f + (2.f/(1.f + 1.f * powf(FastMathApproximations::exp(-2.f * x), 1.f + mDrive * 3000.f)))) * attenuation * blend + x * (1.f - blend);
        }
    };
    
    struct Hyperbolic {
        static void calculateCoefficients(float(& coeffs)[maxCoeffs]) {
            auto& [drive, lo, hi, deviation, mDrive, mag, attenuation, blend] = coeffs;
            mDrive = drive/111.f;
            attenuation = jmap(powf(drive/111.f, 0.08f), 1.f, 0.14f);
        }
        template<typename F> static F processSample(const F sample, const float (&coeffs)[maxCoeffs]) {
            const auto& [drive, lo, hi, deviation, mDrive, mag, attenuation, blend] = coeffs;
            const auto x { static_cast<float>(sample) };
            return FastMathApproximations::tanh(x * (1.f + powf(mDrive, 2.f) * 144.f)) * attenuation * blend + x * (1.f - blend);
        }
    };
    
    template <typename Effect, typename ...Ts>
    static Effect createEffect (APVTS::ParameterLayout& layout, int effectIndex, Ts&&... ts) {
        ParamList refs;
        addParameter<AudioParameterBool>(layout, refs, getFxID(effectIndex) + "Enabled", getFxName(effectIndex) + " Enabled", effectInfo[effectIndex].defaultEnabled);
        addParameter<AudioParameterInt>(layout, refs, getFxID(effectIndex) + "Route", getFxName(effectIndex) + " Route", 0, 99, effectInfo[effectIndex].defaultRoute);
        addParameter<AudioParameterInt>(layout, refs, getFxID(effectIndex) + "Index", getFxName(effectIndex) + " Index", 0, 99, effectInfo[effectIndex].defaultIndex);
        addParameter<AudioParameterFloat>(layout, refs, getFxID(effectIndex) + "Blend", getFxName(effectIndex) + " Blend", 0.f, 1.f, effectInfo[effectIndex].defaultBlend);
        FloatParamList floatRefs;
        for(int i { 0 }; i < getNumParamsForEffect(effectIndex); ++i) {
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
        int getLatency() const override { return (int) filter.state->getFilterOrder()/2; }
        void prepare(const ProcessSpec& spec) override {
            mSampleRate = jlimit(44100.0, 192000.0, spec.sampleRate);
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
            //related to Pro Version
            int currentIndex { coeffIdx };
            float* newCoeffSet { coeffArray[++currentIndex%=maxCoeffs] };
            for (int i { 0 }; i < maxCoeffs; ++i) newCoeffSet[i] = coeffs[i];
            coeffIdx = currentIndex;
        }
        template<typename FloatType> void processSamples(AudioBuffer<FloatType>& buffer) {
            if constexpr (std::is_same<F, FloatType>::value) {
                
                mDrive = coeffs[0];

                wanderer.setDeviation(coeffs[3]);
                coeffs[0] = wanderer.getWanderingValue(coeffs[0] / 111.f) * 111.f;
                calculateCoefficients();

                spectralInversionDelay.process(buffer, spectralInversionBuffer);
                
                AudioBlock<FloatType> block { buffer }, spectralInversionBlock { spectralInversionBuffer };
                
                filter.process(ProcessContextReplacing<F>(block));
                //subtract out the filtered signal from the spectralInversionBlock
                spectralInversionBlock -= block;
                setMagnitudeCoefficient(jlimit(0.00001f, 1.f, static_cast<float>(buffer.getMagnitude(0, buffer.getNumSamples()))));
                for (int channel { 0 }; channel < buffer.getNumChannels(); ++channel) {
                    F* channelData { buffer.getWritePointer (channel) };
                    for (int sample { 0 }; sample < buffer.getNumSamples(); sample++, channelData++) {
                        *channelData = ShaperType::processSample(*channelData, coeffs);
                        jassert(!isnan(*channelData));
                    }
                }
                //add back in the spectral inverse of the filter, this part of the signal was not meant to be distorted
                block += spectralInversionBlock;
                
                coeffs[0] = mDrive;
            }
        }

        AudioBuffer<F> spectralInversionBuffer;
        double mSampleRate { 44100.0 };
        float mDrive { 0.f }, mLow { 20.f }, mHigh { 20000.f };
        ProcessorDuplicator<FIR::Filter<F>, FIR::Coefficients<F>> filter;
        SadDelay<F, 4> spectralInversionDelay;
        Wanderer wanderer;
        //related to Pro Version
        std::atomic<int>& coeffIdx;
        float(& coeffArray)[maxCoeffs][maxCoeffs];
    };
} // namespace sadistic
