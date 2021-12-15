#pragma once
#include "deviant.h"

namespace sadistic {
    
    template<typename FloatType>
    struct Atan : public DeviantEffect {
        enum { driveIndex = 0 };
        static constexpr int waveLength { WAVELENGTH };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<FloatType>::pi }, halfPi { MathConstants<FloatType>::halfPi }, twoPi { MathConstants<FloatType>::twoPi };
        Atan(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager&) :
        DeviantEffect(eID, refs, floatRefs, eIDX) {}
        template<typename F> Atan(Atan<F>& other) : DeviantEffect(other) {}
        
        void processSamples(AudioBuffer<FloatType>& buffer) override {
            const auto drive { static_cast<FloatType>(params[driveIndex].get().get()) };
            NormalisableRange<FloatType> range { 0.1, 6.0, 0.0, 0.2 }, range2 { 0.7f, 2.0, 0., 1.4 };
            const auto attenuationFactor { range2.convertFrom0to1(drive) };
            const auto blend { static_cast<FloatType>(getBlend()) };

            for (int channel { 0 }; channel < buffer.getNumChannels(); ++channel) {
                FloatType* channelData { buffer.getWritePointer (channel) };
                for (int sample { 0 }; sample < buffer.getNumSamples(); sample++, channelData++)
                    *channelData = (fastatan(*channelData * (one + drive * FloatType(14))) / attenuationFactor) * blend + *channelData * (one - blend);
            }
        }
        inline FloatType fastatan( FloatType x ) { return (two/pi) * atan(x * pi/two); }
    };
    
    template<typename FloatType>
    struct DynamicAtan : public DeviantEffect {
        using Coeffs = CalculatedParamCoefficients<FloatType>;
        enum { drive = 0, attenuation = 6, blend = 7 };
        static constexpr int waveLength { WAVELENGTH };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<FloatType>::pi }, halfPi { MathConstants<FloatType>::halfPi }, twoPi { MathConstants<FloatType>::twoPi };
        DynamicAtan(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager&) :
        DeviantEffect(eID, refs, floatRefs, eIDX) {}
        template<typename F> DynamicAtan(DynamicAtan<F>& other) : DeviantEffect(other) {}
        
        static inline FloatType processAtanSample(FloatType sample, const Coeffs& p) {
            return ((fastatan(sample * (one + p(drive) * FloatType(14))) / p(attenuation)) * p(blend)) + sample * (one - p(blend)); }
//        static constexpr Coeffs returnDefaultParams() const {
//            const auto lDrive { static_cast<FloatType>(params[drive].get().get()) };
//            NormalisableRange<FloatType> range { 0.1, 6.0, 0.0, 0.2 }, range2 { 0.7f, 2.0, 0., 1.4 };
//            const auto lAttenuation { range2.convertFrom0to1(drive) };
//            const auto lBlend { static_cast<FloatType>(getBlend()) };
//            Coeffs c{};
//            c[drive] = zero; c[attenuation] = FloatType(0.7); c[blend] = lBlend;
//            return c;
//        }
        Coeffs returnParams() const {
            const auto lDrive { static_cast<FloatType>(params[drive].get().get()) };
            NormalisableRange<FloatType> range { 0.1, 6.0, 0.0, 0.2 }, range2 { 0.7f, 2.0, 0., 1.4 };
            const auto lAttenuation { range2.convertFrom0to1(drive) };
            const auto lBlend { static_cast<FloatType>(getBlend()) };
            Coeffs c{};
            c[drive] = lDrive; c[attenuation] = lAttenuation; c[blend] = lBlend;
            return c;
        }
                    static inline FloatType fastatan( FloatType x )  { return (two/pi) * atan(x * pi/two); }
        
        void processSamples(AudioBuffer<FloatType>& buffer) override {
            const auto p { returnParams() };
            
            for (int channel { 0 }; channel < buffer.getNumChannels(); ++channel) {
                FloatType* channelData { buffer.getWritePointer (channel) };
                for (int sample { 0 }; sample < buffer.getNumSamples(); sample++, channelData++)
                    *channelData = processAtanSample(*channelData, p);
            }
        }
    };
    
    template<typename FloatType>
    struct BitCrusher : public DeviantEffect {
        static constexpr int waveLength { WAVELENGTH };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two };

        BitCrusher(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager&) :
        DeviantEffect(eID, refs, floatRefs, eIDX) {}
        template<typename F> BitCrusher(BitCrusher<F>& other) : DeviantEffect(other) {}

        FloatType rround(FloatType f) { return f > zero ? floor(f + half) : ceil(f - half); }

        void processSamples(AudioBuffer<FloatType>& buffer) {
            enum { driveIndex, floorIndex };
            const int channels { buffer.getNumChannels() }, samples { buffer.getNumSamples() };
            auto drive { params[driveIndex].get().get() };
            auto floor { params[floorIndex].get().get() };

            const FloatType mapped_crush_floor { floor * FloatType(31) + one };
            const FloatType crush_floor { FloatType(32) - static_cast<int>(mapped_crush_floor) + one };
            
            const FloatType mapped_input { drive * (crush_floor - one) + one };
            const int bitDepth { static_cast<int>(crush_floor - mapped_input) + 1 };
            int i { 4 };
            for (; i <= bitDepth; i *= 2) ;
            const auto max { static_cast<FloatType>(i - 1) };
            auto mag { buffer.getMagnitude(0, buffer.getNumSamples()) };
            if (mag == zero) mag = one;
            const auto blend { getBlend() };
            
            for (int channel { 0 }; channel < channels; ++channel) {
                FloatType* channelData { buffer.getWritePointer (channel) };
                for (int sample { 0 }; sample < samples; sample++, channelData++) {
                    jassert(!isnan(*channelData));
                    *channelData = blend * (rround((*channelData / mag + one) * max) / max - one) * mag + (one - blend) * *channelData;
                }
            }
        }
    };
    
    template<typename FloatType>
    struct DynamicBitCrusher : public DeviantEffect {
        using Coeffs = CalculatedParamCoefficients<FloatType>;
        enum { drive = 0, floorParam, max, attenuation = 6, blend = 7 };
        static constexpr int waveLength { WAVELENGTH };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two };
        
        DynamicBitCrusher(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager&) :
        DeviantEffect(eID, refs, floatRefs, eIDX) {}
        template<typename F> DynamicBitCrusher(DynamicBitCrusher<F>& other) : DeviantEffect(other) {}
        
        Coeffs returnParams() {
            auto lDrive { params[drive].get().get() };
            auto lFloor { static_cast<FloatType>(params[floorParam].get().get()) };
            const FloatType mapped_crush_floor { lFloor * FloatType(31) + one };
            const FloatType crush_floor { FloatType(32 - static_cast<int>(mapped_crush_floor)) + one };
            
            const FloatType mapped_input { drive * (crush_floor - one) + one };
            const int bitDepth { static_cast<int>(crush_floor - mapped_input) + 1 };
            int i { 4 };
            for (; i <= bitDepth; i *= 2) ;
//            const auto max { static_cast<FloatType>(i - 1) };
            const auto lMax { static_cast<FloatType>(pow(lFloor - one, 4.)) * 1024 + 1 };
            const auto lAttenuation { one };
            const auto lBlend { getBlend() };
            Coeffs c{};
            c[drive] = lDrive; c[floorParam] = lFloor; c[max] = lMax; c[attenuation] = lAttenuation; c[blend] = lBlend;
            return c;
        }
        static inline FloatType rround(FloatType f) { return f > zero ? floor(f + half) : ceil(f - half); }
        static inline FloatType crushSample(FloatType sample, const Coeffs& p) {
            return p(blend) * (rround((sample + one) * p(max)) / p(max) - one) + (one - p(blend)) * sample; }

        void processSamples(AudioBuffer<FloatType>& buffer) {
            const auto p { returnParams() };
            const int channels { buffer.getNumChannels() }, samples { buffer.getNumSamples() };
            for (int j { 0 }; j < channels; ++j) {
                auto* channelData { buffer.getWritePointer(j) };
                for (int i { 0 }; i < samples; ++i, ++channelData)
                    *channelData = crushSample(*channelData, p);
            }
        }
    };
    
    template<typename FloatType>
    struct Deviation : public DeviantEffect {
        enum { driveIndex, gateIndex, saturationIndex };
        static constexpr int waveLength { WAVELENGTH };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<FloatType>::pi }, halfPi { MathConstants<FloatType>::halfPi }, twoPi { MathConstants<FloatType>::twoPi };
        Deviation(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager&) :
        DeviantEffect(eID, refs, floatRefs, eIDX) {}
        template<typename F> Deviation(Deviation<F>& other) : DeviantEffect(other) {}
        
        void reset() override {}
        void prepare(const ProcessSpec&) override {}
        void processSamples(AudioBuffer<FloatType>& buffer) override {
            auto driveRange { params[driveIndex].get().getNormalisableRange() };
            auto saturationRange { params[saturationIndex].get().getNormalisableRange() };
            
            const float driveNormal { driveRange.convertTo0to1(params[driveIndex].get()) / 8.f };
            const float saturationNormal { saturationRange.convertTo0to1(params[saturationIndex].get()) };
            const float driveFactor { powf(0.1f,powf(driveNormal,0.1f)) };
            const float saturationFactor { powf(0.15f,powf(saturationNormal,0.2f)) };
            const auto attenuationFactor { static_cast<FloatType>(powf(driveFactor,powf(saturationFactor,0.4f)) * powf(saturationFactor,powf(driveFactor,0.4f))) };
            
            const auto drive { static_cast<FloatType>(params[driveIndex].get()) };
            const auto gate { static_cast<FloatType>(params[gateIndex].get()) };
            const auto saturation { static_cast<FloatType>(params[saturationIndex].get()) };
            const auto blend { static_cast<FloatType>(getBlend()) };
            
            const auto gateOffset { (((gate + one) / two) - one) / ((gate + one) / two) };
            
            for (int channel { 0 }; channel < buffer.getNumChannels(); ++channel) {
                FloatType* channelData { buffer.getWritePointer (channel) };
                for (int sample { 0 }; sample < buffer.getNumSamples(); sample++, channelData++) {
                    *channelData = static_cast<FloatType>((-one - gateOffset + (two/(one + (one / gate) * pow(exp(-saturation * *channelData),(drive))))) * attenuationFactor) * blend + *channelData * (one - blend);
                    jassert(!isnan(*channelData));
                }
            }
        }
    };
    
    template<typename FloatType>
    struct DynamicDeviation : public DeviantEffect {
        using Coeffs = CalculatedParamCoefficients<FloatType>;
        enum { drive = 0, gate, saturation, gateOffset, attenuation = 6, blend = 7 };
        static constexpr int waveLength { WAVELENGTH };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<FloatType>::pi }, halfPi { MathConstants<FloatType>::halfPi }, twoPi { MathConstants<FloatType>::twoPi };
        
        DynamicDeviation(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager&) :
        DeviantEffect(eID, refs, floatRefs, eIDX) {}
        template<typename F> DynamicDeviation(DynamicDeviation<F>& other) : DeviantEffect(other) {}
        
        static inline FloatType processSample(FloatType sample, const Coeffs& p) {
            return (-one - p[gateOffset] + (two/(one + (one / p[gate]) * pow(exp(-p[saturation] * sample),(p[drive]))))) * p[attenuation] * p[blend] + sample * (one - p[blend]); }
        
        void reset() override {}
        void prepare(const ProcessSpec&) override {}
        Coeffs returnParams() {
            auto driveRange { params[drive].get().getNormalisableRange() };
            auto saturationRange { params[saturation].get().getNormalisableRange() };
            
            const float driveNormal { driveRange.convertTo0to1(params[drive].get()) / 8.f };
            const float saturationNormal { saturationRange.convertTo0to1(params[saturation].get()) };
            const float driveFactor { powf(0.1f,powf(driveNormal,0.1f)) };
            const float saturationFactor { powf(0.15f,powf(saturationNormal,0.2f)) };
            const auto lAttenuation { static_cast<FloatType>(powf(driveFactor,powf(saturationFactor,0.4f)) * powf(saturationFactor,powf(driveFactor,0.4f))) };
            
            const auto lDrive { static_cast<FloatType>(params[drive].get()) };
            const auto lGate { static_cast<FloatType>(params[gate].get()) };
            const auto lSaturation { static_cast<FloatType>(params[saturation].get()) };
            const auto lBlend { static_cast<FloatType>(getBlend()) };
            
            const auto lGateOffset { (((gate + one) / two) - one) / ((gate + one) / two) };
            Coeffs c{};
            c[drive] = lDrive; c[gate] = lGate; c[saturation] = lSaturation; c[gateOffset] = lGateOffset; c[attenuation] = lAttenuation; c[blend] = lBlend;
            return c;
        }
        static inline FloatType deviateSample(FloatType sample, const Coeffs& p) {
            return (-one - p(gateOffset) + (two/(one + (one / p(gate)) * pow(exp(-p(saturation) * sample),(p(drive)))))) * p(attenuation) * p(blend) + sample * (one - p(blend));
        }
        void processSamples(AudioBuffer<FloatType>& buffer) override {
            const auto p { returnParams() };
            
            for (int channel { 0 }; channel < buffer.getNumChannels(); ++channel) {
                FloatType* channelData { buffer.getWritePointer (channel) };
                for (int sample { 0 }; sample < buffer.getNumSamples(); sample++, channelData++) {
                    deviateSample(*channelData, p);// = static_cast<FloatType>((-one - gateOffset + (two/(one + (one / gate) * pow(exp(-saturation * *channelData),(drive))))) * attenuationFactor) * blend + *channelData * (one - blend);
                    jassert(!isnan(*channelData));
                }
            }
        }
    };
    
    template<typename FloatType>
    struct SadisticFilter : public DeviantEffect {
        enum { lowIndex, highIndex };
        SadisticFilter(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager&) :
        DeviantEffect(eID, refs, floatRefs, eIDX) {}
        template<typename F> SadisticFilter(SadisticFilter<F>& other) : DeviantEffect(other) {}
        
        void reset() override { filter.reset(); }
        void prepare(const ProcessSpec& spec) override {
            lastLow = params[lowIndex].get();
            lastHigh = params[highIndex].get();
            if(spec.sampleRate != 0.0) lastSampleRate = spec.sampleRate;
            update(lastSampleRate, lastLow, lastHigh);
            filter.prepare({ lastSampleRate, spec.maximumBlockSize, spec.numChannels }); }
        void process(AudioBuffer<FloatType>& buffer) override {
            if (isEnabled()) {
                if(params[lowIndex].get() != lastLow || params[highIndex].get() != lastHigh) {
                    lastLow = jlimit(lastLow/maxDelta, lastLow*maxDelta, params[lowIndex].get().get());
                    lastHigh = jlimit(lastHigh/maxDelta, lastHigh*maxDelta, params[highIndex].get().get());
                    update(lastSampleRate, lastLow, lastHigh);
                }
                AudioBlock<FloatType> block { buffer };
                filter.process(ProcessContextReplacing<FloatType>(block));
            }
        }
        void update(double sampleRate, float lowFrequency, float highFrequency, size_t order = 32, typename WindowingFunction<FloatType>::WindowingMethod type = WindowingFunction<FloatType>::kaiser, FloatType beta = 4.0){
            
            *filter.state = *makeBandpass<FloatType>(lowFrequency, highFrequency, sampleRate, order, type, beta);
        }
        int getLatency() override { return static_cast<int>(filter.state->getFilterOrder()/2); }
        double lastSampleRate { 44100.0 };
        float lastLow { 20.f }, lastHigh { 20000.f }, maxDelta { 1.1f };
        ProcessorDuplicator<FIR::Filter<FloatType>, FIR::Coefficients<FloatType>> filter;
    };
    
}
