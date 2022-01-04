#pragma once
#include "Effects.h"

namespace sadistic {
    
    template <typename FloatType>
    struct DeviantMembers {
        static constexpr int bufferLength { BUFFERLENGTH };
        
        //Constructor for FX: emplaces references of their associated parameters into a vector member variable
        //while adding them to the APVTS, idea stolen from DSPModulePluginDemo. thank you Reuk, Ed, Tom, and Ivan!!!
        DeviantMembers(APVTS::ParameterLayout& layout, std::atomic<int>* cI, float(& cS)[numFX][maxCoeffs][maxCoeffs]) :
        atan(createEffect<Shaper<Atan, FloatType>>(layout, 0, cI[0], cS[0])),
        crusher(createEffect<Shaper<Crusher, FloatType>>(layout, 1, cI[1], cS[1])),
        clipper(createEffect<Shaper<Clipper, FloatType>>(layout, 2, cI[2], cS[2])),
        deviation(createEffect<Shaper<Deviation, FloatType>>(layout, 3, cI[3], cS[3])),
        hyperbolic(createEffect<Shaper<Hyperbolic, FloatType>>(layout, 4, cI[4], cS[4])),
        params(emplaceMainParams(layout)) {}
        
        // like a copy constructor, made to ensure that both double and float DeviantMember structs
        // receive valid parameter references because hosts like to jump back and forth
        template<typename F>
        DeviantMembers(DeviantMembers<F>& o) : atan(o.atan), crusher(o.crusher), clipper(o.clipper), deviation(o.deviation), hyperbolic(o.hyperbolic), params(o.params) {}
        
        ParamList emplaceMainParams(APVTS::ParameterLayout& layout) {
            ParamList paramList;
            ParamInfo info =    paramInfo[numFX][0];
            addParameter<AudioParameterFloat>(layout, paramList, getParamID(numFX, 0), getParamName(numFX, 0), NormalisableRange<float>(info.min, info.max), info.defaultValue, getSuffix(info.type));
//            info =              paramInfo[numFX][1];
//            addParameter<AudioParameterFloat>(layout, paramList, getParamID(numFX, 1), getParamName(numFX, 1), NormalisableRange<float>(info.min, info.max), info.defaultValue, getSuffix(info.type));
//            info =              paramInfo[numFX][2];
//            addParameter<AudioParameterInt>(layout, paramList, getParamID(numFX, 2), getParamName(numFX, 2), int(info.min), int(info.max), int(info.defaultValue), getSuffix(info.type));
//            info =              paramInfo[numFX][3];
//            addParameter<AudioParameterFloat>(layout, paramList, getParamID(numFX, 3), getParamName(numFX, 3), NormalisableRange<float>(info.min, info.max), info.defaultValue, getSuffix(info.type));
            return paramList;
        }
        
        int getLatency() { int latency { 0 }; for (auto* effect : effects) latency += effect->getLatency(); return latency; }
        void init() { for (auto* fx : effects) { fx->init(); } }
        void reset() { blendBuffer.clear(); xBuffer.clear(); lastBuffer.clear();
            for (auto* fx : effects) fx->reset();
            resetAll(blendDelay); }
        
        void prepare(const ProcessSpec& processSpec) {
            print("type of FloatType", typeid(FloatType).name());
            const ProcessSpec spec { processSpec.sampleRate, uint32(bufferLength), processSpec.numChannels };
            blendDelay.setDelay(getLatency());
            blendBuffer.setSize((int)spec.numChannels, bufferLength);
            xBuffer.setSize((int)spec.numChannels, bufferLength);
            lastBuffer.setSize((int)spec.numChannels, bufferLength);
            for (auto* fx : effects) fx->prepare(spec);
            prepareAll(spec, blendDelay);
            reset();
        }
        
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
            
            bool paramsAreChanging { false }, fxNeedsUpdate[numFX]{};
            for (int i { 0 }; i < numFX; ++i) { fxNeedsUpdate[i] = effects[i]->parametersNeedCooking(); if (fxNeedsUpdate[i]) paramsAreChanging = true; }

            if (paramsAreChanging) {
                xBuffer.makeCopyOf(buffer);
                for (int i { 0 }; i < numFX; ++i) { effects[i]->processSamples(xBuffer); effects[i]->processSamples(lastBuffer); }
            }
            
            //make a copy of the buffer in order to push it through the fx again if the params have changed and return their delay lines and filters to the state they started at... not efficient I'm sure but it is a sure way to keep clicks and pops from creeping into the output buffer no matter what happens and copying is cheap!
            lastBuffer.makeCopyOf(buffer);

            //copy what's in our buffer to our blend buffer, delaying the blendBuffer by the total latency
            blendDelay.process(buffer, blendBuffer);
            
            AudioBlock<FloatType> block { buffer }, blendBlock { blendBuffer };

            for (int i { 0 }; i < numFX; ++i) effects[i]->process(buffer);
            
            if (paramsAreChanging) {
                buffer.applyGainRamp(0, buffer.getNumSamples(), FloatType(0.0), FloatType(1.0));
                for (int j { 0 }; j < buffer.getNumChannels(); ++j)
                    buffer.addFromWithRamp(j, 0, xBuffer.getReadPointer(j), buffer.getNumSamples(), FloatType(1.0), FloatType(0.0));
            }
            
            //push the dry and wet signals seperately to the oscilloscope
            if (!bypassed) oscilloscope[wetSignal].pushChannel(buffer);
            oscilloscope[drySignal].pushChannel(blendBuffer);

            //attenuate the signals according to the blend parameter
            const auto blend { static_cast<FloatType>(static_cast<AudioParameterFloat&>(params[0].get()).get()) };
            block *= blend;
            if (!bypassed) blendBlock *= (FloatType(1) - blend);
            
            //output the sum of both signals
            if (bypassed) block = blendBlock;
            else block += blendBlock;
        }

        //Data Members
        AudioBuffer<FloatType> blendBuffer, xBuffer, lastBuffer;
        sadistic::DelayBuffer<FloatType> blendDelay;
        FloatType fifo1L[bufferLength]{}, fifo1R[bufferLength]{}, fifo2L[bufferLength]{}, fifo2R[bufferLength]{};
        FloatType*     fifo1[2] { fifo1L, fifo1R },     *  fifo2[2] { fifo2L, fifo2R };
        FloatType**    writeFifo{ fifo1 },              ** readFifo { fifo2 };
        int fifoIndex { 0 };

        //FX Processors
        Shaper<Atan, FloatType> atan;
        Shaper<Crusher, FloatType> crusher;
        Shaper<Clipper, FloatType> clipper;
        Shaper<Deviation, FloatType> deviation;
        Shaper<Hyperbolic, FloatType> hyperbolic;
//        StaticWaveShaper<FloatType> staticWaveShaper;

        // Array of base class pointers to the above effects, similar to JUCE's ProcessorBase class,
        // with prepare, reset, and process methods
        DeviantEffect* effects[numFX] {
            { &atan }, { &crusher }, { &clipper }, { &deviation }, { &hyperbolic } };
        
        // std::vector<reference_wrapper<AudioParameterFloat>> to store parameter references that
        // are not associated with any particular effect
        ParamList params;
    };
}
