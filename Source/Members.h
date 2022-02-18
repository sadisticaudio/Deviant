#pragma once
#include "Effects.h"

namespace sadistic {
    
    //represents members of our derived AudioProcessor class which are templated on float or double precision processing
    template <typename FloatType>
    struct DeviantMembers {
        static constexpr int bufferLength { BUFFERLENGTH };
        
        //Constructor for FX: emplaces references of their associated parameters into a vector member variable while
        //also adding them to the APVTS, idea stolen from THE AMAZING DSPModulePluginDemo. thank you Reuk, Ed, Tom, and Ivan!!!
        DeviantMembers(APVTS::ParameterLayout& layout) :
        atan(createEffect<Shaper<Atan, FloatType>>(layout, 0, cIdx[0], coefficients[0])),
        crusher(createEffect<Shaper<Crusher, FloatType>>(layout, 1, cIdx[1], coefficients[1])),
        clipper(createEffect<Shaper<Clipper, FloatType>>(layout, 2, cIdx[2], coefficients[2])),
        deviation(createEffect<Shaper<Deviation, FloatType>>(layout, 3, cIdx[3], coefficients[3])),
        hyperbolic(createEffect<Shaper<Hyperbolic, FloatType>>(layout, 4, cIdx[4], coefficients[4])),
        params(emplaceMainParams(layout)) {}
        
        // like a copy constructor, made to ensure that both double and float DeviantProMember structs receive valid parameter references because hosts will call various double/float methods (prepareToPlay, etc...) during a session
        template<typename F>
        DeviantMembers(DeviantMembers<F>& o) : atan(o.atan), crusher(o.crusher), clipper(o.clipper), deviation(o.deviation), hyperbolic(o.hyperbolic), params(o.params) {}
        
        ParamList emplaceMainParams(APVTS::ParameterLayout& layout) {
            ParamList paramList;
            ParamInfo info =    paramInfo[numFX][0];
            addParameter<AudioParameterFloat>(layout, paramList, getParamID(numFX, 0), getParamName(numFX, 0), NormalisableRange<float>(info.min, info.max), info.defaultValue, getSuffix(info.type));
            return paramList;
        }
        
        int getLatency() { int latency { 0 }; for (auto* effect : effects) latency += effect->getLatency(); return latency; }
        void init() { for (auto* fx : effects) { fx->init(); } }
        void reset() { blendBuffer.clear(); xBuffer.clear(); lastBuffer.clear();
            for (auto* fx : effects) fx->reset();
            resetAll(blendDelay); }
        
        void prepare(const ProcessSpec& spec) {
//            blendDelay.setDelay(getLatency());
            blendBuffer.setSize((int)spec.numChannels, bufferLength);
            xBuffer.setSize((int)spec.numChannels, bufferLength);
            lastBuffer.setSize((int)spec.numChannels, bufferLength);
            for (auto* fx : effects) fx->prepare(spec);
            prepareAll(spec, blendDelay);
            reset();
        }
        
        //this function takes the buffer provided by the main processBlock function and works like a FIFO,
        //providing buffers of length bufferLength to the function that actually does the processing below
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

                    //preProcess copies the buffer and returns whether any parameters need updating
                    auto changing { preProcess(buf) };
                    
                    process(buf, oscilloscope, bypassed, changing);
                    fifoIndex = 0;
                    std::swap(writeFifo, readFifo);
                }
            }
        }
        
        bool paramsAreChanging() {
            bool changing { false };
            for (int i { 0 }; i < numFX; ++i) {
                fxNeedsUpdate[i] = effects[i]->parametersNeedCooking();
                if (fxNeedsUpdate[i])
                    changing = true;
            }
            return changing;
        }
        
        //this function is only here to de-couple these functions from the actual processing - only relevant in PRO version
        bool preProcess(AudioBuffer<FloatType>& buffer) {
            
            //copy what's in our buffer to our blend buffer, delaying the blendBuffer by the total latency
            blendDelay.process(buffer, blendBuffer);
            
            return paramsAreChanging();
        }

        void process(AudioBuffer<FloatType>& buffer, LongFifo<float> (& oscilloscope)[2], bool bypassed, bool changing) {

            if (changing) {
                xBuffer.makeCopyOf(buffer);
                for (int i { 0 }; i < numFX; ++i) {
                    effects[i]->processSamples(xBuffer);
                    effects[i]->processSamples(lastBuffer);
                }
            }
            
            //make a copy of the buffer in order to push it through the fx again if the params have changed and return their delay lines and filters to the state they started at. this not efficient but it is a sure way to keep clicks and pops from creeping into the output buffer no matter what happens... and copying is cheap!
            lastBuffer.makeCopyOf(buffer);
            
            AudioBlock<FloatType> block { buffer }, blendBlock { blendBuffer };

            //the actual waveshaping
            if (!bypassed) {
                for (int i { 0 }; i < numFX; ++i) {
                    effects[i]->process(buffer);
                }
            }
            
            //if params have changed, crossfade between old and new
            if (changing) {
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
        float coefficients[numFX][maxCoeffs][maxCoeffs];
        FloatType fifo1L[bufferLength]{}, fifo1R[bufferLength]{}, fifo2L[bufferLength]{}, fifo2R[bufferLength]{};
        int fifoIndex { 0 };
        FloatType*     fifo1[2] { fifo1L, fifo1R },     *  fifo2[2] { fifo2L, fifo2R };
        FloatType**    writeFifo{ fifo1 },              ** readFifo { fifo2 };
        std::atomic<int> cIdx[numFX] { 0,0,0,0,0 };
        AudioBuffer<FloatType> blendBuffer, xBuffer, lastBuffer;
        SadDelay<FloatType, 20> blendDelay;
        
        //FX Processors
        Shaper<Atan, FloatType> atan;
        Shaper<Crusher, FloatType> crusher;
        Shaper<Clipper, FloatType> clipper;
        Shaper<Deviation, FloatType> deviation;
        Shaper<Hyperbolic, FloatType> hyperbolic;
        
        // Array of base class pointers to the above effects, similar to JUCE's ProcessorBase class,
        // with prepare, reset, and process methods
        DeviantEffect* effects[numFX] { { &atan }, { &crusher }, { &clipper }, { &deviation }, { &hyperbolic } };
        
        // std::vector<reference_wrapper<RangedAudioParameter>> to store parameter references that
        // are not associated with any particular effect
        ParamList params;
        
        bool fxNeedsUpdate[numFX]{};
    };
} // namespace sadistic
