#pragma once
#include "WaveShaper.h"

namespace sadistic {
    template <typename FloatType>
    struct DeviantMembers {
        
        static constexpr int bufferLength { BUFFERLENGTH }, maxWaveOrder { 10 }, maxWavelength { 1 << maxWaveOrder }, fifoLength { maxWavelength + bufferLength }, waveLength { WAVELENGTH };
        
        //Constructor for FX: emplaces references of their associated parameters into a vector member variable
        //while adding them to the APVTS, idea stolen from DSPModulePluginDemo. thank you Reuk, Ed, Tom, and Ivan!!!
        DeviantMembers(APVTS::ParameterLayout& layout, TableManager& s) :
        dynamicAtan(createEffect<DynamicAtan<FloatType>>(layout, 0, s)),
        dynamicBitCrusher(createEffect<DynamicBitCrusher<FloatType>>(layout, 1, s)),
        dynamicDeviation(createEffect<DynamicDeviation<FloatType>>(layout, 2, s)),
        dynamicWaveShaper(createEffect<DynamicWaveShaper<FloatType>>(layout, 3, s)),
        filterA(createEffect<SadisticFilter<FloatType>>(layout, 5, s)),
        filterB(createEffect<SadisticFilter<FloatType>>(layout, 4, s)),
        staticAtan(createEffect<Atan<FloatType>>(layout, 6, s)),
        staticBitCrusher(createEffect<BitCrusher<FloatType>>(layout, 7, s)),
        staticDeviation(createEffect<Deviation<FloatType>>(layout, 8, s)),
        staticWaveShaper(createEffect<StaticWaveShaper<FloatType>>(layout, 9, s)),
        params(emplaceParams(layout)) {}
        
        template <typename Param>
        static void add (AudioProcessorParameterGroup& group, std::unique_ptr<Param> param)
        {
            group.addChild (std::move (param));
        }
        
        template <typename Param>
        static void add (AudioProcessorValueTreeState::ParameterLayout& group, std::unique_ptr<Param> param)
        {
            group.add (std::move (param));
        }
        
        template <typename Param, typename Group>
        static Param& addToLayout (Group& layout, std::unique_ptr<Param> param)
        {
//            auto param = std::make_unique<Param> (std::forward<Ts> (ts)...);
            auto& ref = *param;
            add (layout, std::move (param));
            return ref;
        }
        
        
//        template <typename Param, typename Group, typename... Ts>
//        static Param& addToLayout (Group& layout, Ts&&... ts)
//        {
//            auto param = std::make_unique<Param> (std::forward<Ts> (ts)...);
//            auto& ref = *param;
//            add (layout, std::move (param));
//            return ref;
//        }
        
        template <typename Param>
        static Param& addToLayout (APVTS::ParameterLayout& layout, int effectIndex, int paramIndex) {
            ParamInfo info = paramInfo[effectIndex][paramIndex];
            String pID { getParamID(effectIndex, paramIndex) }, pName{ getParamName(effectIndex, paramIndex) };
            std::unique_ptr<Param> param { nullptr };
            switch (info.type) {
                case ParamInfo::dB:
                    param = std::make_unique<AudioParameterFloat>(pID, pName, NormalisableRange<float>(info.min, info.max), info.defaultValue, translate (" dB"), AudioProcessorParameter::genericParameter, [](float value, int) { return String (value, 1) + " dB"; }, [](String text) { return text.dropLastCharacters(3).getFloatValue(); });
                case ParamInfo::Hz:
                    param = std::make_unique<AudioParameterFloat>(pID, pName, NormalisableRange<float>(info.min, info.max), info.defaultValue, translate(" Hz"));
                case ParamInfo::Int:
                {} break;
                case ParamInfo::Pct:{} break;
                default:
                    param = std::make_unique<AudioParameterFloat>(pID, pName, NormalisableRange<float>(info.min, info.max), info.defaultValue);
            }
            auto& ref = *param;
            layout.add(std::move(param));
            return ref;
        }
        
        template <typename Param>
        static Param& addToLayout (APVTS::ParameterLayout& layout, int paramIndex) {
            ParamInfo info = paramInfo[numFX][paramIndex];
            String pID { getParamID(numFX, paramIndex) }, pName{ getParamName(numFX, paramIndex) };
            std::unique_ptr<Param> param { nullptr };
            param = std::make_unique<AudioParameterInt>(pID, pName, static_cast<int>(info.min), static_cast<int>(info.max), static_cast<int>(info.defaultValue), translate(""));
            auto& ref = *param;
            layout.add(std::move(param));
            return ref;
        }
        
        template <typename Effect>
        static Effect createEffect (APVTS::ParameterLayout& layout, int effectIndex, TableManager& s) {
            ParamList refs;
            refs.emplace_back(addToLayout<AudioParameterBool>(layout, std::make_unique<AudioParameterBool>(getFxID(effectIndex) + "Enabled", getFxName(effectIndex) + " Enabled", effectInfo[effectIndex].defaultEnabled)));
            refs.emplace_back(addToLayout<AudioParameterInt>(layout, std::make_unique<AudioParameterInt>(getFxID(effectIndex) + "Route", getFxName(effectIndex) + " Route", 0, 99, effectInfo[effectIndex].defaultRoute)));
            refs.emplace_back(addToLayout<AudioParameterInt>(layout, std::make_unique<AudioParameterInt>(getFxID(effectIndex) + "Index", getFxName(effectIndex) + " Index", 0, 99, effectInfo[effectIndex].defaultIndex)));
            refs.emplace_back(addToLayout<AudioParameterFloat>(layout, std::make_unique<AudioParameterFloat>(getFxID(effectIndex) + "Blend", getFxName(effectIndex) + " Blend", 0.f, 1.f, effectInfo[effectIndex].defaultBlend)));
            FloatParamList floatRefs;
            for(int i { 0 }; !paramID[effectIndex][i].empty() && i < 4; ++i) {
                floatRefs.emplace_back(addToLayout<AudioParameterFloat>(layout, effectIndex, i));
            }
            return Effect(getFxID(effectIndex), refs, floatRefs, effectIndex, s);
        }
        
//        template <typename Effect>
//        static Effect createEffect (APVTS::ParameterLayout& layout, int effectIndex, APVTS& s, DynamicAtan<FloatType>& a, DynamicBitCrusher<FloatType>& c, DynamicDeviation<FloatType>& d) {
//            ParamList refs;
//            refs.emplace_back(addToLayout(layout, std::make_unique<AudioParameterBool>(getFxID(effectIndex) + "Enabled", getFxName(effectIndex) + " Enabled", effectInfo[effectIndex].defaultEnabled)));
//            refs.emplace_back(addToLayout(layout, std::make_unique<AudioParameterInt>(getFxID(effectIndex) + "Route", getFxName(effectIndex) + " Route", 0, 99, effectInfo[effectIndex].defaultRoute)));
//            refs.emplace_back(addToLayout(layout, std::make_unique<AudioParameterInt>(getFxID(effectIndex) + "Index", getFxName(effectIndex) + " Index", 0, 99, effectInfo[effectIndex].defaultIndex)));
//            refs.emplace_back(addToLayout(layout, std::make_unique<AudioParameterFloat>(getFxID(effectIndex) + "Blend", getFxName(effectIndex) + " Blend", 0.f, 1.f, effectInfo[effectIndex].defaultBlend)));
//            FloatParamList floatRefs;
//            for(int i { 0 }; !paramID[effectIndex][i].empty() && i < 4; ++i) {
//                floatRefs.emplace_back(addToLayout<AudioParameterFloat>(layout, effectIndex, i));
//            }
//            return Effect(getFxID(effectIndex), refs, floatRefs, effectIndex, s, a, c, d);
//        }
        
        ParamList emplaceParams(APVTS::ParameterLayout& layout) {
            ParamList paramList;
            paramList.emplace_back(addToLayout<AudioParameterFloat>(layout, numFX, 0));
            paramList.emplace_back(addToLayout<AudioParameterFloat>(layout, numFX, 1));
            paramList.emplace_back(addToLayout<AudioParameterInt>(layout, 2));
            paramList.emplace_back(addToLayout<AudioParameterFloat>(layout, numFX, 3));
            return paramList;
        }
        
        // like a copy constructor, made to ensure that both double and float DeviantMember structs
        // receive valid parameter references because hosts like to jump back and forth
        template<typename F>
        DeviantMembers(DeviantMembers<F>& o) : dynamicAtan(o.dynamicAtan), dynamicBitCrusher(o.dynamicBitCrusher), dynamicDeviation(o.dynamicDeviation), dynamicWaveShaper(o.dynamicWaveShaper), filterA(o.filterA), filterB(o.filterB), staticAtan(o.staticAtan), staticBitCrusher(o.staticBitCrusher), staticDeviation(o.staticDeviation), staticWaveShaper(o.staticWaveShaper), params(o.params) {}
        
        void reset() { spectralInversionBuffer.clear(); blendBuffer.clear(); resetAll(dynamicWaveShaper, staticWaveShaper, dynamicDeviation, staticDeviation, dynamicBitCrusher, staticBitCrusher, dynamicAtan, staticAtan, lpf, lpf2, spectralInversionDelay1, spectralInversionDelay2, blendDelay1, blendDelay2, interpolator[0], interpolator[1]); }
        
        void prepare(const ProcessSpec& spec) {
            spectralInversionBuffer.setSize((int)spec.numChannels, (int) spec.maximumBlockSize);
            blendBuffer.setSize((int)spec.numChannels, (int) spec.maximumBlockSize);
            *lpf.state = *makeBandpass<FloatType>(40, 800, 44100.0, 64, WindowingFunction<FloatType>::kaiser);
            *lpf2.state = *makeBandpass<FloatType>(30, 15000, 44100.0, 24, WindowingFunction<FloatType>::kaiser);
            prepareAll(spec, filterA, filterB, dynamicWaveShaper, staticWaveShaper, dynamicDeviation, staticDeviation, dynamicBitCrusher, staticBitCrusher, dynamicAtan, staticAtan, lpf, lpf2, spectralInversionDelay1, spectralInversionDelay2, blendDelay1, blendDelay2); reset(); }
        
        void sort () { std::sort(std::begin(effects), std::end(effects), [](DeviantEffect* a, DeviantEffect* b) { return *a < *b; }); }
        
        //must be sorted
        int getFilterAIndex() { int i { 0 }; while(effects[i] != &filterA) i++; return i; }
        int getFilterBIndex() { int i { 0 }; while(effects[i] != &filterB) i++; return i; }
        
        template <typename F> void process(AudioBuffer<FloatType>& buffer, F&& func) {
            int bufferIndex { 0 }, numSamples { buffer.getNumSamples() };
            
            while (bufferIndex < numSamples) {
                int samples { jmin(bufferLength - fifoIndex, numSamples - bufferIndex) };
                
                for (int j { 0 }; j < buffer.getNumChannels(); ++j) {
                    for (int i { bufferIndex }, write { fifoIndex }; i < bufferIndex + samples; ++i, ++write)
                        writeFifo[j][write] = static_cast<FloatType>(buffer.getSample(j,i));
                    for (int i { bufferIndex }, read { fifoIndex }; i < bufferIndex + samples; ++i, ++read)
                        buffer.setSample(j, i, static_cast<FloatType>(readFifo[j][read]));
                }
                
                fifoIndex += samples;
                bufferIndex += samples;
                
                if (fifoIndex == bufferLength) {
                    
                    AudioBuffer<FloatType> buf { writeFifo, buffer.getNumChannels(), bufferLength };
                    
                    func(buf);
                    
                    fifoIndex = 0;
                    std::swap(writeFifo, readFifo);
                }
            }
        }
        
        //Data Members
        
        //FX Processors
        DynamicAtan<FloatType> dynamicAtan;
        DynamicBitCrusher<FloatType> dynamicBitCrusher;
        DynamicDeviation<FloatType> dynamicDeviation;
        DynamicWaveShaper<FloatType> dynamicWaveShaper;
        SadisticFilter<FloatType> filterA;
        SadisticFilter<FloatType> filterB;
        Atan<FloatType> staticAtan;
        BitCrusher<FloatType> staticBitCrusher;
        Deviation<FloatType> staticDeviation;
        StaticWaveShaper<FloatType> staticWaveShaper;
        
        FloatType fifo1L[bufferLength]{}, fifo1R[bufferLength]{}, fifo2L[bufferLength]{}, fifo2R[bufferLength]{};
        FloatType*     fifo1[2] { fifo1L, fifo1R },     *  fifo2[2] { fifo2L, fifo2R };
        FloatType**    writeFifo{ fifo1 },              ** readFifo { fifo2 };
        int fifoIndex { 0 };
        AudioBuffer<FloatType> spectralInversionBuffer, blendBuffer, decimationBuffer;
        sadistic::DelayBuffer<FloatType> spectralInversionDelay1, blendDelay1, spectralInversionDelay2, blendDelay2;
        ProcessorDuplicator<FIR::Filter<FloatType>, FIR::Coefficients<FloatType>> lpf, lpf2;
        Oversampling<FloatType> oversampler { 2, static_cast<size_t>(0), Oversampling<FloatType>::filterHalfBandFIREquiripple };
        LagrangePreciseInterpolator<FloatType> interpolator[2];
        
        // Array of base class pointers to the above effects, similar to JUCE's ProcessorBase class,
        // with prepare, reset, and process methods
        DeviantEffect* effects[numFX] { { &dynamicAtan}, { &dynamicBitCrusher}, { &dynamicDeviation}, { &dynamicWaveShaper}, { &filterA}, { &filterB}, { &staticAtan}, { &staticBitCrusher}, { &staticDeviation}, { &staticWaveShaper } };
        
        // std::vector<reference_wrapper<AudioParameterFloat>> to store parameter references that
        // are not associated with any particular effect
        ParamList params;
    };

}
