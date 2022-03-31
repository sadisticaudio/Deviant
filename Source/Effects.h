#pragma once
#include "deviant.h"

namespace sadistic {
    
    template<typename F> F rround(F f) { return f > F(0) ? floor (f + F(0.5)) : ceil(f - F(0.5)); }
    template<typename F> F fastatan( F x ) { return (F(2)/MathConstants<F>::pi) * atan(x * MathConstants<F>::halfPi); }
    
    static constexpr float attenuationArray[112] { 1.f, 0.969972f, 0.93633f, 0.895768f, 0.840769f, 0.788895f, 0.719035f, 0.650585f, 0.603181f, 0.556584f, 0.50063f, 0.455201f, 0.400208f, 0.355584f, 0.301276f, 0.287243f, 0.26345f, 0.259869f, 0.256477f, 0.253254f, 0.250183f, 0.247251f, 0.244444f, 0.241753f, 0.239167f, 0.236678f, 0.234279f, 0.231964f, 0.229726f, 0.227561f, 0.225463f, 0.223428f, 0.221453f, 0.219534f, 0.217668f, 0.215852f, 0.214083f, 0.212358f, 0.210676f, 0.209034f, 0.20743f, 0.205863f, 0.204331f, 0.202832f, 0.201364f, 0.199927f, 0.198519f, 0.197139f, 0.195785f, 0.194458f, 0.193155f, 0.191876f, 0.190619f, 0.189385f, 0.188172f, 0.186979f, 0.185806f, 0.184653f, 0.183518f, 0.1824f, 0.1813f, 0.180217f, 0.17915f, 0.178098f, 0.177062f, 0.176041f, 0.175034f, 0.174041f, 0.173061f, 0.172095f, 0.171141f, 0.1702f, 0.169271f, 0.168354f, 0.167448f, 0.166554f, 0.16567f, 0.164797f, 0.163935f, 0.163082f, 0.16224f, 0.161407f, 0.160583f, 0.159769f, 0.158963f, 0.158167f, 0.157379f, 0.156599f, 0.155827f, 0.155064f, 0.154308f, 0.15356f, 0.15282f, 0.152087f, 0.151361f, 0.150643f, 0.149931f, 0.149226f, 0.148527f, 0.147835f, 0.14715f, 0.146471f, 0.145798f, 0.145131f, 0.14447f, 0.143815f, 0.143165f, 0.142521f, 0.141883f, 0.14125f, 0.140622f, 0.14f };
    
    struct Atan {
        template<typename F> static F processSample(const F sample, const float (&coeffs)[maxCoeffs]) {
            const auto& [drive, lo, hi, deviation, mDrive, mag, attenuation, blend] = coeffs;
            const auto x { static_cast<float>(sample) };
            return fastatan(x * (1.f + mDrive * 144.f)) * attenuation * blend + x * (1.f - blend);
        }
        static void calculateCoefficients(float(& coeffs)[maxCoeffs]) {
            auto& [drive, lo, hi, deviation, mDrive, mag, attenuation, blend] = coeffs;
            mDrive = powf(drive/111.f, 2.f);
//            attenuation = jmap(powf(drive/111.f, 0.08f), 1.f, 0.14f);
            attenuation = attenuationArray[roundToInt(drive)];
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
            attenuation = attenuationArray[roundToInt(drive)];
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
            attenuation = attenuationArray[roundToInt(drive)];
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
//            if constexpr (std::is_same<F, FloatType>::value) {
            if constexpr (std::is_same_v<F, FloatType>) {
            
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
