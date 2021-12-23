#pragma once
#include "deviant.h"

namespace sadistic {
    
    template<typename F>
    struct Atan : public DeviantEffect {
        Atan(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager& m, std::atomic<int>& cI, float(& cS)[maxCoeffs][maxCoeffs]) :
        DeviantEffect(eID, refs, floatRefs, eIDX, m), coeffIdx(cI), coeffArray(cS) {}
        template<typename OtherFloatType> Atan(Atan<OtherFloatType>& other) : DeviantEffect(other), coeffIdx(other.coeffIdx), coeffArray(other.coeffArray) {}
        void processSamples(AudioBuffer<float>& buffer) override { processSamples<float>(buffer); }
        void processSamples(AudioBuffer<double>& buffer) override { processSamples<double>(buffer); }
        void calculateCoefficients() override {
            auto& [drive, noName1, noName2, noName3, noName4, noName5, attenuation, blend] = coeffs;
            attenuation = jmap(powf(drive, 0.5f), 1.f, 0.4f);
            int currentIndex { coeffIdx };
            float* newCoeffSet { coeffArray[++currentIndex%=maxCoeffs] };
            for (int i { 0 }; i < maxCoeffs; ++i) newCoeffSet[i] = coeffs[i];
            coeffIdx = currentIndex;
        }
        template<typename FloatType> void processSamples(AudioBuffer<FloatType>& buffer) {
            if constexpr (std::is_same<F, FloatType>::value) {
                for (int channel { 0 }; channel < buffer.getNumChannels(); ++channel) {
                    F* channelData { buffer.getWritePointer (channel) };
                    for (int sample { 0 }; sample < buffer.getNumSamples(); sample++, channelData++) {
                        *channelData = processAtan(*channelData, coeffs);
                        jassert(!isnan(*channelData));
                    }
                }
            }
        }
        std::atomic<int>& coeffIdx;
        float(& coeffArray)[maxCoeffs][maxCoeffs];
    };
    
    template<typename F>
    struct BitCrusher : public DeviantEffect {
        BitCrusher(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager& m, std::atomic<int>& cI, float(& cS)[maxCoeffs][maxCoeffs]) :
        DeviantEffect(eID, refs, floatRefs, eIDX, m), coeffIdx(cI), coeffArray(cS) {}
        template<typename OtherFloatType> BitCrusher(BitCrusher<OtherFloatType>& other) : DeviantEffect(other), coeffIdx(other.coeffIdx), coeffArray(other.coeffArray) {}
        void processSamples(AudioBuffer<float>& buffer) override { processSamples<float>(buffer); }
        void processSamples(AudioBuffer<double>& buffer) override { processSamples<double>(buffer); }
        void calculateCoefficients() override {
            auto& [drive, floor, max, noName3, noName4, noName5, attenuation, blend] = coeffs;
            const F mapped_crush_floor { floor * F(31) + 1.f };
            const F crush_floor { F(32) - static_cast<int>(mapped_crush_floor) + 1.f };
            
            const F mapped_input { drive * (crush_floor - 1.f) + 1.f };
            const int bitDepth { static_cast<int>(crush_floor - mapped_input) + 1 };
            int i { 4 };
            for (; i <= bitDepth; i *= 2) ;
//            max = static_cast<F>(i - 1);
            max = powf(floor - 1.f, 4.f) * 1024.f + 1.f;
            attenuation = 1.f;
            int currentIndex { coeffIdx };
            float* newCoeffSet { coeffArray[++currentIndex%=maxCoeffs] };
            for (int k { 0 }; k < maxCoeffs; ++k) newCoeffSet[k] = coeffs[k];
            coeffIdx = currentIndex;
        }
        template<typename FloatType> void processSamples(AudioBuffer<FloatType>& buffer) {
            if constexpr (std::is_same<F, FloatType>::value) {
                const int channels { buffer.getNumChannels() }, samples { buffer.getNumSamples() };
                for (int j { 0 }; j < channels; ++j) {
                    auto* channelData { buffer.getWritePointer(j) };
                    for (int i { 0 }; i < samples; ++i, ++channelData) {
                        *channelData = crushSample(*channelData, coeffs);
                        jassert(!isnan(*channelData));
                    }
                }
            }
        }
        std::atomic<int>& coeffIdx;
        float(& coeffArray)[maxCoeffs][maxCoeffs];
    };
    
    template<typename F>
    struct DynamicBitCrusher : public DeviantEffect {
        DynamicBitCrusher(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager& m, std::atomic<int>& cI, float(& cS)[maxCoeffs][maxCoeffs]) :
        DeviantEffect(eID, refs, floatRefs, eIDX, m), coeffIdx(cI), coeffArray(cS) {}
        template<typename OtherFloatType> DynamicBitCrusher(DynamicBitCrusher<OtherFloatType>& other) : DeviantEffect(other), coeffIdx(other.coeffIdx), coeffArray(other.coeffArray) {}
        void processSamples(AudioBuffer<float>& buffer) override { processSamples<float>(buffer); }
        void processSamples(AudioBuffer<double>& buffer) override { processSamples<double>(buffer); }
        template<typename FloatType> void processSamples(AudioBuffer<FloatType>& buffer) {
            if constexpr (std::is_same<F, FloatType>::value) {
                const int channels { buffer.getNumChannels() }, samples { buffer.getNumSamples() };
                for (int channel { 0 }; channel < channels; ++channel) {
                    F* channelData { buffer.getWritePointer (channel) };
                    for (int sample { 0 }; sample < samples; sample++, channelData++) {
                        *channelData = crushSample(*channelData, coeffs);
                        jassert(!isnan(*channelData));
                    }
                }
            }
        }
        void calculateCoefficients() override {
            auto& [drive, floor, max, noName3, noName4, noName5, attenuation, blend] = coeffs;
            max = powf(floor - 1.f, 4.f) * 1024.f + 1.f;
            attenuation = 1.f - 0.5f * drive * floor;
            int currentIndex { coeffIdx };
            float* newCoeffSet { coeffArray[++currentIndex%=maxCoeffs] };
            for (int i { 0 }; i < maxCoeffs; ++i) newCoeffSet[i] = coeffs[i];
            coeffIdx = currentIndex;
        }
        std::atomic<int>& coeffIdx;
        float(& coeffArray)[maxCoeffs][maxCoeffs];
    };
    
    template<typename F>
    struct Deviation : public DeviantEffect {
        Deviation(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager& m, std::atomic<int>& cI, float(& cS)[maxCoeffs][maxCoeffs]) :
        DeviantEffect(eID, refs, floatRefs, eIDX, m), coeffIdx(cI), coeffArray(cS) {}
        template<typename OtherFloatType> Deviation(Deviation<OtherFloatType>& other) : DeviantEffect(other), coeffIdx(other.coeffIdx), coeffArray(other.coeffArray) {}
        void processSamples(AudioBuffer<float>& buffer) override { processSamples<float>(buffer); }
        void processSamples(AudioBuffer<double>& buffer) override { processSamples<double>(buffer); }
        void calculateCoefficients() override {
            auto& [drive, gate, saturation, gateOffset, noName4, noName5, attenuation, blend] = coeffs;
            auto driveRange { params[0].get().getNormalisableRange() };
            auto saturationRange { params[2].get().getNormalisableRange() };
            const float driveNormal { driveRange.convertTo0to1(drive) / 8.f };
            const float saturationNormal { saturationRange.convertTo0to1(saturation) };
            const float driveFactor { powf(0.1f,powf(driveNormal,0.1f)) };
            const float saturationFactor { powf(0.15f,powf(saturationNormal,0.2f)) };
            attenuation = powf(driveFactor,powf(saturationFactor,0.4f)) * powf(saturationFactor,powf(driveFactor,0.4f));
            gateOffset = (((gate + 1.f) / 2.f) - 1.f) / ((gate + 1.f) / 2.f);
            int currentIndex { coeffIdx };
            float(& newCoeffSet)[maxCoeffs] { coeffArray[++currentIndex%=maxCoeffs] };
            for (int i { 0 }; i < maxCoeffs; ++i)
                newCoeffSet[i] = coeffs[i];
            coeffIdx = currentIndex;
        }
        template<typename FloatType> void processSamples(AudioBuffer<FloatType>& buffer) {
            if constexpr (std::is_same<F, FloatType>::value) {
                for (int channel { 0 }; channel < buffer.getNumChannels(); ++channel) {
                    F* channelData { buffer.getWritePointer (channel) };
                    for (int sample { 0 }; sample < buffer.getNumSamples(); sample++, channelData++) {
                        *channelData = deviateSample(*channelData, coeffs);
                        jassert(!isnan(*channelData));
                    }
                }
            }
        }
        std::atomic<int>& coeffIdx;
        float(& coeffArray)[maxCoeffs][maxCoeffs];
    };
    
    template<typename F>
    struct SadisticFilter : public DeviantEffect {
        SadisticFilter(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager& m) : DeviantEffect(eID, refs, floatRefs, eIDX, m) {}
        template<typename OtherFloatType> SadisticFilter(SadisticFilter<OtherFloatType>& other) : DeviantEffect(other) {}
        void reset() override { filter.reset(); }
        int getLatency() override { return static_cast<int>(filter.state->getFilterOrder()/2); }
        void prepare(const ProcessSpec& spec) override {
            lastSampleRate = jlimit(44100.0, 192000.0, spec.sampleRate);
            filter.prepare({ lastSampleRate, spec.maximumBlockSize, spec.numChannels }); }
        void processSamples(AudioBuffer<float>& buffer) override { processSamples<float>(buffer); }
        void processSamples(AudioBuffer<double>& buffer) override { processSamples<double>(buffer); }
        template<typename FloatType> void processSamples(AudioBuffer<FloatType>& buffer) {
            if constexpr (std::is_same<F, FloatType>::value) {
                AudioBlock<F> block { buffer };
                filter.process(ProcessContextReplacing<F>(block));
            }
        }
        void calculateCoefficients() override {
            auto& [low, high, noName2, noName3, noName4, noName5, attenuation, blend] = coeffs;
            update(lastSampleRate, low, high); }
        void update(double sR, F lo, F hi){
            *filter.state = *makeBandpass<F>(lo, hi, sR, 8, WindowingFunction<F>::kaiser, F(4)); }
        double lastSampleRate { 44100.0 };
        ProcessorDuplicator<FIR::Filter<F>, FIR::Coefficients<F>> filter;
    };
}
