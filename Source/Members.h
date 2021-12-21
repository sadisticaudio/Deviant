#pragma once
#include "WaveShaper.h"

namespace sadistic {
    template <typename FloatType>
    struct DeviantMembers {
//        using Coeffs = CalculatedParamCoefficients<FloatType>;
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, sqrt2 { MathConstants<FloatType>::sqrt2 };
        static constexpr int bufferLength { BUFFERLENGTH }, maxWaveOrder { 10 }, maxWavelength { 1 << maxWaveOrder }, fifoLength { maxWavelength + bufferLength }, waveLength { WAVELENGTH }, gainLength { GAINLENGTH };
        
        //Constructor for FX: emplaces references of their associated parameters into a vector member variable
        //while adding them to the APVTS, idea stolen from DSPModulePluginDemo. thank you Reuk, Ed, Tom, and Ivan!!!
        DeviantMembers(APVTS::ParameterLayout& layout, TableManager& s, std::atomic<int>* cI, float(& cS)[maxCoeffs][maxCoeffs][maxCoeffs]) :
        dynamicAtan(createEffect<Atan<FloatType>>(layout, 0, s, cI[0], cS[0])),
        dynamicBitCrusher(createEffect<BitCrusher<FloatType>>(layout, 1, s, cI[1], cS[1])),
        dynamicDeviation(createEffect<Deviation<FloatType>>(layout, 2, s, cI[2], cS[2])),
        dynamicWaveShaper(createEffect<DynamicWaveShaper<FloatType>>(layout, 3, s, cI, cS)),
        filterA(createEffect<SadisticFilter<FloatType>>(layout, 4, s)),
        filterB(createEffect<SadisticFilter<FloatType>>(layout, 5, s)),
        staticAtan(createEffect<Atan<FloatType>>(layout, 6, s, cI[3], cS[3])),
        staticBitCrusher(createEffect<BitCrusher<FloatType>>(layout, 7, s, cI[4], cS[4])),
        staticDeviation(createEffect<Deviation<FloatType>>(layout, 8, s, cI[5], cS[5])),
        staticWaveShaper(createEffect<StaticWaveShaper<FloatType>>(layout, 9, s)),
        params(emplaceMainParams(layout)), mgmt(s) { }

        template <typename Param, typename ListType, typename ...Ts>
        static void addParameter (APVTS::ParameterLayout& layout, ListType& pList, Ts... ts) {
            std::unique_ptr<Param> param = std::make_unique<Param> (std::forward<Ts> (ts)...);
            auto& ref = *param;
            layout.add(std::move(param));
            pList.emplace_back(ref);
        }
        
        template <typename Effect, typename ...Ts>
        static Effect createEffect (APVTS::ParameterLayout& layout, int effectIndex, TableManager& s, Ts&&... ts) {
            ParamList refs;
            addParameter<AudioParameterBool>(layout, refs, getFxID(effectIndex) + "Enabled", getFxName(effectIndex) + " Enabled", effectInfo[effectIndex].defaultEnabled);
            addParameter<AudioParameterInt>(layout, refs, getFxID(effectIndex) + "Route", getFxName(effectIndex) + " Route", 0, 99, effectInfo[effectIndex].defaultRoute);
            addParameter<AudioParameterInt>(layout, refs, getFxID(effectIndex) + "Index", getFxName(effectIndex) + " Index", 0, 99, effectInfo[effectIndex].defaultIndex);
            addParameter<AudioParameterFloat>(layout, refs, getFxID(effectIndex) + "Blend", getFxName(effectIndex) + " Blend", 0.f, 1.f, effectInfo[effectIndex].defaultBlend);
            FloatParamList floatRefs;
            for(int i { 0 }; !paramID[effectIndex][i].empty() && i < 4; ++i) {
                ParamInfo info = paramInfo[effectIndex][i];
                addParameter<AudioParameterFloat>(layout, floatRefs, getParamID(effectIndex, i), getParamName(effectIndex, i), NormalisableRange<float>(info.min, info.max), info.defaultValue, getSuffix(info.type));
            }
            return Effect(getFxID(effectIndex), refs, floatRefs, effectIndex, s, std::forward<Ts>(ts)...);
        }

        ParamList emplaceMainParams(APVTS::ParameterLayout& layout) {
            ParamList paramList;
            ParamInfo info =    paramInfo[numFX][0];
            addParameter<AudioParameterFloat>(layout, paramList, getParamID(numFX, 0), getParamName(numFX, 0), NormalisableRange<float>(info.min, info.max), info.defaultValue, getSuffix(info.type));
            info =              paramInfo[numFX][1];
            addParameter<AudioParameterFloat>(layout, paramList, getParamID(numFX, 1), getParamName(numFX, 1), NormalisableRange<float>(info.min, info.max), info.defaultValue, getSuffix(info.type));
            info =              paramInfo[numFX][2];
            addParameter<AudioParameterInt>(layout, paramList, getParamID(numFX, 2), getParamName(numFX, 2), int(info.min), int(info.max), int(info.defaultValue), getSuffix(info.type));
            info =              paramInfo[numFX][3];
            addParameter<AudioParameterFloat>(layout, paramList, getParamID(numFX, 3), getParamName(numFX, 3), NormalisableRange<float>(info.min, info.max), info.defaultValue, getSuffix(info.type));
            return paramList;
        }
        
        // like a copy constructor, made to ensure that both double and float DeviantMember structs
        // receive valid parameter references because hosts like to jump back and forth
        template<typename F>
        DeviantMembers(DeviantMembers<F>& o) : dynamicAtan(o.dynamicAtan), dynamicBitCrusher(o.dynamicBitCrusher), dynamicDeviation(o.dynamicDeviation), dynamicWaveShaper(o.dynamicWaveShaper), filterA(o.filterA), filterB(o.filterB), staticAtan(o.staticAtan), staticBitCrusher(o.staticBitCrusher), staticDeviation(o.staticDeviation), staticWaveShaper(o.staticWaveShaper), params(o.params), mgmt(o.mgmt) {}
        
        void init() { for (auto* effect : effects) { effect->init(); }
            mgmt.makeStaticTable(); mgmt.makeDynamicTable();
            filterA.update(44100.0, 30.0, 2000.0); filterB.update(44100.0, 30.0, 10000.0); }
        
        void reset() { spectralInversionBuffer1.clear(); spectralInversionBuffer2.clear(); blendBuffer.clear(); resetAll(dynamicWaveShaper, staticWaveShaper, dynamicDeviation, staticDeviation, dynamicBitCrusher, staticBitCrusher, dynamicAtan, staticAtan, spectralInversionDelay1, spectralInversionDelay2, blendDelay); }
        
        void prepare(const ProcessSpec& processSpec) {
            const ProcessSpec spec { processSpec.sampleRate, uint32(bufferLength), processSpec.numChannels };
            spectralInversionBuffer1.setSize((int)spec.numChannels, bufferLength);
            spectralInversionBuffer2.setSize((int)spec.numChannels, bufferLength);
            blendBuffer.setSize((int)spec.numChannels, bufferLength);
            spectralInversionBuffer1.clear();
            spectralInversionBuffer2.clear();
            blendBuffer.clear();
            prepareAll(spec, filterA, filterB, dynamicWaveShaper, staticWaveShaper, dynamicDeviation, staticDeviation, dynamicBitCrusher, staticBitCrusher, dynamicAtan, staticAtan, spectralInversionDelay1, spectralInversionDelay2, blendDelay);
        }
        
        void sort () { std::sort(std::begin(effects), std::end(effects), [](DeviantEffect* a, DeviantEffect* b) { return *a < *b; }); }
        
        //must be sorted
        int getFilterAIndex() { int i { 0 }; while(effects[i] != &filterA) i++; return i; }
        int getFilterBIndex() { int i { 0 }; while(effects[i] != &filterB) i++; return i; }
        
        
        void processBlock(AudioBuffer<FloatType>& buffer, LongFifo<float> (& oscilloscope)[2], bool bypassed = false) {
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
                    process(buf, oscilloscope, bypassed);
                    fifoIndex = 0;
                    std::swap(writeFifo, readFifo);
                }
            }
        }
        
        float getBlend() const   { return static_cast<AudioParameterFloat&>(params[0].get()).get(); }

        void process(AudioBuffer<FloatType>& buffer, LongFifo<float> (& oscilloscope)[2], bool bypassed = false) {

            // cook our parameters and redo the waveshaping lookup tables if anything changed
            //OTHERWISE THESE UNITS WOULDNT UPDATE THEIR PARAMS
            staticBitCrusher.cookParameters();
            staticAtan.cookParameters();
            staticDeviation.cookParameters();
            dynamicAtan.cookParameters();
            dynamicDeviation.cookParameters();
            dynamicBitCrusher.cookParameters();

            //copy what's in our buffer to our blend buffer, delaying the blendBuffer by the total latency
            //and the spectralInversionBuffer by the latency of the pre-filtering
            //the delays will resize our buffers to the current buffer length
            //and retain samples needed in subsequent processBlock calls
            blendDelay.process(buffer, blendBuffer);
            spectralInversionDelay1.process(buffer, spectralInversionBuffer1);
            
            AudioBlock<FloatType> block { buffer }, spectralInversionBlock { spectralInversionBuffer1 }, blendBlock { blendBuffer };
            
            filterA.process(buffer);
            
            //subtract out the filtered signal from the spectralInversionBlock
            spectralInversionBlock -= block;

            dynamicWaveShaper.process(buffer);
            staticWaveShaper.process(buffer);
            filterB.process(buffer);
            
            //delay the spectralInversionBuffer the rest of the way
            spectralInversionDelay2.process(spectralInversionBuffer1);
            
            //add back in the spectral inverse of the first filter(s), this part of the signal was not meant to be distorted
            block += spectralInversionBlock;
            
            //attenuate the signals according to the blend parameter
            const auto blend { static_cast<FloatType>(static_cast<AudioParameterFloat&>(params[0].get()).get()) };
            block *= blend;
            if (!bypassed) blendBlock *= (one - blend);
            
            //push the dry and wet signals seperately to the oscilloscope
            if (!bypassed) oscilloscope[wetSignal].pushChannel(buffer);
            oscilloscope[drySignal].pushChannel(blendBuffer);
            
            //output the sum of both signals
            if (bypassed) block = blendBlock;
            else block += blendBlock;
        }

        //Data Members
        float coeffs[maxCoeffs][maxCoeffs][maxCoeffs];
        Value coeffIdx[6] { Value(var(0)), Value(var(0)), Value(var(0)), Value(var(0)), Value(var(0)), Value(var(0)) };
        
        AudioBuffer<FloatType> spectralInversionBuffer1, spectralInversionBuffer2, blendBuffer;
        sadistic::DelayBuffer<FloatType> spectralInversionDelay1, spectralInversionDelay2, blendDelay;
        FloatType fifo1L[bufferLength]{}, fifo1R[bufferLength]{}, fifo2L[bufferLength]{}, fifo2R[bufferLength]{};
        FloatType*     fifo1[2] { fifo1L, fifo1R },     *  fifo2[2] { fifo2L, fifo2R };
        FloatType**    writeFifo{ fifo1 },              ** readFifo { fifo2 };
        int fifoIndex { 0 };

        //FX Processors
        Atan<FloatType> dynamicAtan;
        BitCrusher<FloatType> dynamicBitCrusher;
        Deviation<FloatType> dynamicDeviation;
        DynamicWaveShaper<FloatType> dynamicWaveShaper;
        SadisticFilter<FloatType> filterA;
        SadisticFilter<FloatType> filterB;
        Atan<FloatType> staticAtan;
        BitCrusher<FloatType> staticBitCrusher;
        Deviation<FloatType> staticDeviation;
        StaticWaveShaper<FloatType> staticWaveShaper;

        // Array of base class pointers to the above effects, similar to JUCE's ProcessorBase class,
        // with prepare, reset, and process methods
        DeviantEffect* effects[numFX] { { &dynamicAtan}, { &dynamicBitCrusher}, { &dynamicDeviation}, { &dynamicWaveShaper}, { &filterA}, { &filterB}, { &staticAtan}, { &staticBitCrusher}, { &staticDeviation}, { &staticWaveShaper } };
        
        // std::vector<reference_wrapper<AudioParameterFloat>> to store parameter references that
        // are not associated with any particular effect
        ParamList params;
        TableManager& mgmt;
    };
}
