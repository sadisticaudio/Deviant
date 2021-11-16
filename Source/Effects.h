#pragma once
#include "deviant.h"

namespace sadistic {
    
    template<typename FloatType>
    struct Atan : public DeviantEffect {
        enum { driveIndex };
        static constexpr int waveLength { WAVELENGTH };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<FloatType>::pi }, halfPi { MathConstants<FloatType>::halfPi }, twoPi { MathConstants<FloatType>::twoPi };
        Atan(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, APVTS&) :
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
        enum { driveIndex };
        static constexpr int waveLength { WAVELENGTH };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<FloatType>::pi }, halfPi { MathConstants<FloatType>::halfPi }, twoPi { MathConstants<FloatType>::twoPi };
        DynamicAtan(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, APVTS&) :
        DeviantEffect(eID, refs, floatRefs, eIDX) {}
        template<typename F> DynamicAtan(DynamicAtan<F>& other) : DeviantEffect(other) {}
        
        struct Params { FloatType drive, attenuationFactor, blend; };
        Params returnParams() {
            const auto drive { static_cast<FloatType>(params[driveIndex].get().get()) };
            NormalisableRange<FloatType> range { 0.1, 6.0, 0.0, 0.2 }, range2 { 0.7f, 2.0, 0., 1.4 };
            const auto attenuationFactor { range2.convertFrom0to1(drive) };
            const auto blend { static_cast<FloatType>(getBlend()) };
            return { drive, attenuationFactor, blend };
        }
        static inline FloatType fastatan( FloatType x ) { return (two/pi) * atan(x * pi/two); }
        static inline FloatType processSample(FloatType sample, const Params& p) {
            return (fastatan(sample * (one + p.drive * FloatType(14))) / p.attenuationFactor) * p.blend + sample * (one - p.blend);
        }
        
        void processSamples(AudioBuffer<FloatType>& buffer) override {
            const auto drive { static_cast<FloatType>(params[driveIndex].get().get()) };
            NormalisableRange<FloatType> range { 0.1, 6.0, 0.0, 0.2 }, range2 { 0.7, 2.0, 0., 1.4 };
            const auto attenuationFactor { range2.convertFrom0to1(drive) };
            const auto blend { static_cast<FloatType>(getBlend()) };
            
            const auto p { returnParams() };
            
            for (int channel { 0 }; channel < buffer.getNumChannels(); ++channel) {
                FloatType* channelData { buffer.getWritePointer (channel) };
                for (int sample { 0 }; sample < buffer.getNumSamples(); sample++, channelData++)
                    *channelData = processSample(*channelData, p);
            }
        }
    };
    
    template<typename FloatType>
    struct BitCrusher : public DeviantEffect {
        static constexpr int waveLength { WAVELENGTH };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two };

        BitCrusher(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, APVTS&) :
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
        enum { driveIndex, floorIndex };
        static constexpr int waveLength { WAVELENGTH };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two };
        
        DynamicBitCrusher(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, APVTS&) :
        DeviantEffect(eID, refs, floatRefs, eIDX) {}
        template<typename F> DynamicBitCrusher(DynamicBitCrusher<F>& other) : DeviantEffect(other) {}
        
        struct Params { FloatType max, attenuationFactor, blend; };
        Params returnParams() {
            auto drive { params[driveIndex].get().get() };
            auto floor { static_cast<FloatType>(params[floorIndex].get().get()) };
            const FloatType mapped_crush_floor { floor * FloatType(31) + one };
            const FloatType crush_floor { FloatType(32) - static_cast<int>(mapped_crush_floor) + one };
            
            const FloatType mapped_input { drive * (crush_floor - one) + one };
            const int bitDepth { static_cast<int>(crush_floor - mapped_input) + 1 };
            int i { 4 };
            for (; i <= bitDepth; i *= 2) ;
//            const auto max { static_cast<FloatType>(i - 1) };
            const auto max { static_cast<FloatType>(powf(floor - one, 4.f)) * 1024 + 1 };
            const auto attenuationFactor { one };
            const auto blend { getBlend() };
            return { max, attenuationFactor, blend };
        }
        static inline FloatType rround(FloatType f) { return f > zero ? floor(f + half) : ceil(f - half); }
        static inline FloatType processSample(FloatType sample, const Params& p) {
            return p.blend * (rround((sample + one) * p.max) / p.max - one) + (one - p.blend) * sample; }

        void processSamples(AudioBuffer<FloatType>& buffer) {
            const auto p { returnParams() };
            const int channels { buffer.getNumChannels() }, samples { buffer.getNumSamples() };
            for (int j { 0 }; j < channels; ++j) {
                auto* channelData { buffer.getWritePointer(j) };
                for (int i { 0 }; i < samples; ++i, ++channelData)
                    *channelData = processSample(*channelData, p);
            }
        }
    };
    
    template<typename FloatType>
    struct Deviation : public DeviantEffect {
        enum { driveIndex, gateIndex, saturationIndex };
        static constexpr int waveLength { WAVELENGTH };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<FloatType>::pi }, halfPi { MathConstants<FloatType>::halfPi }, twoPi { MathConstants<FloatType>::twoPi };
        Deviation(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, APVTS&) :
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
        enum { driveIndex, gateIndex, saturationIndex };
        static constexpr int waveLength { WAVELENGTH };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<FloatType>::pi }, halfPi { MathConstants<FloatType>::halfPi }, twoPi { MathConstants<FloatType>::twoPi };
        
        DynamicDeviation(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, APVTS&) :
        DeviantEffect(eID, refs, floatRefs, eIDX) {}
        template<typename F> DynamicDeviation(DynamicDeviation<F>& other) : DeviantEffect(other) {}
        
        void reset() override {}
        void prepare(const ProcessSpec&) override {}
        struct Params { FloatType gateOffset, gate, saturation, drive, attenuationFactor, blend; };
        Params returnParams() {
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
            return { gateOffset, gate, saturation, drive, attenuationFactor, blend };
        }
        static inline FloatType processSample(FloatType sample, const Params& p) {
            return (-one - p.gateOffset + (two/(one + (one / p.gate) * pow(exp(-p.saturation * sample),(p.drive))))) * p.attenuationFactor * p.blend + sample * (one - p.blend);
        }
        void processSamples(AudioBuffer<FloatType>& buffer) override {
            const auto p { returnParams() };
            
            for (int channel { 0 }; channel < buffer.getNumChannels(); ++channel) {
                FloatType* channelData { buffer.getWritePointer (channel) };
                for (int sample { 0 }; sample < buffer.getNumSamples(); sample++, channelData++) {
                    processSample(*channelData, p);// = static_cast<FloatType>((-one - gateOffset + (two/(one + (one / gate) * pow(exp(-saturation * *channelData),(drive))))) * attenuationFactor) * blend + *channelData * (one - blend);
                    jassert(!isnan(*channelData));
                }
            }
        }
    };
    
    template<typename FloatType>
    struct SadisticFilter : public DeviantEffect {
        enum { lowIndex, highIndex };
        SadisticFilter(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, APVTS&) :
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
