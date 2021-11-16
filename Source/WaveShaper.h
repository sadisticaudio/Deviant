#pragma once
#include "Effects.h"
#include "../../Source/SadisticFFT.h"

namespace sadistic {

    template<typename FloatType>
    struct StaticWaveShaper : DeviantEffect, ValueTree::Listener {
        static constexpr int waveLength { WAVELENGTH };
        
        StaticWaveShaper(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, APVTS& s) :
        DeviantEffect(eID, refs, floatRefs, eIDX), apvts(s) {}
        template<typename F> StaticWaveShaper(StaticWaveShaper<F>& other) : DeviantEffect(other), apvts(other.apvts) {}
        
        void valueTreePropertyChanged(ValueTree&, const Identifier& identifier) override {
            if(identifier == Identifier("waveTable")) newDataHere = true; }
        
        void processSamples(AudioBuffer<FloatType>& buffer) override {
            if(newDataHere) {
                sadistic::DeviantTree::getWaveTable(apvts, waveTable.table);
                newDataHere = false;
            }
            for (int j { 0 }; j < buffer.getNumChannels(); ++j)
                for (int i { 0 }; i < buffer.getNumSamples(); ++i)
                    buffer.setSample(j, i, waveTable[buffer.getSample(j, i)]);
        }
        GainTable<FloatType> waveTable{};
        bool newDataHere { true };
        APVTS& apvts;
    };

    template<typename FloatType>
    struct DynamicWaveShaper : DeviantEffect, ValueTree::Listener {
        
        using Atan = DynamicAtan<FloatType>;
        using Crusher = DynamicBitCrusher<FloatType>;
        using Deviation = DynamicDeviation<FloatType>;
        using AtanParams = typename Atan::Params;
        using CrusherParams = typename Crusher::Params;
        using DeviationParams = typename Deviation::Params;
        
        static constexpr int bufferLength { BUFFERLENGTH }, maxWaveOrder { 10 }, maxWavelength { 1 << maxWaveOrder }, fifoLength { maxWavelength + bufferLength }, waveLength { WAVELENGTH }, phaseFilterOrder { PhaseTable<FloatType>::phaseFilterOrder }, dOrder { 3 }, dFactor { (1 << dOrder) };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<FloatType>::pi }, halfPi { MathConstants<FloatType>::halfPi }, twoPi { MathConstants<FloatType>::twoPi };
        
        DynamicWaveShaper(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, APVTS& s, DynamicAtan<FloatType>& a, DynamicBitCrusher<FloatType>& c, DynamicDeviation<FloatType>& d) :
        DeviantEffect(eID, refs, floatRefs, eIDX), apvts(s), atan(a), crusher(c), deviation(d) {}
        template<typename F> DynamicWaveShaper(DynamicWaveShaper<F>& other, DynamicAtan<FloatType>& a, DynamicBitCrusher<FloatType>& c, DynamicDeviation<FloatType>& d) : DeviantEffect(other), apvts(other.apvts), atan(a), crusher(c), deviation(d) {}
        
        static constexpr int distance(const int older, const int newer) { return ((newer-older) + fifoLength) % fifoLength; }
        
        void prepare(const ProcessSpec& spec) override {
            for (auto& w : waveShaper) w.prepare(spec);
            oversampler.initProcessing(bufferLength/dFactor);
            reset();
        }
        
        void reset() override {
            for (auto& w : waveShaper) w.reset();
            oversampler.reset();
        }
        
        void valueTreePropertyChanged(ValueTree&, const Identifier& identifier) override {
            if(identifier == Identifier("phaseTable")) newPhaseDataHere = true;
            if(identifier == Identifier("gainTable")) newGainDataHere = true; }
        int getLatency() override { return fifoLength; }// + int(oversampler.getLatencyInSamples()) + interpolator[0].getBaseLatency() * FloatType(dFactor); }
        
        struct WaveShaper {

            using Comparator = bool(WaveShaper::*)(const int);
            using TurnFunction = void(WaveShaper::*)(const int, FloatType, const AtanParams&, const CrusherParams&, const DeviationParams&);

            WaveShaper(DynamicWaveShaper& wS) : shaper(wS) {
                compare = &WaveShaper::isTrough;
                compareOther = &WaveShaper::isPeak;
            }

            void prepare(const ProcessSpec&) { reset(); }
            void reset() {
                compare = &WaveShaper::isTrough;
                compareOther = &WaveShaper::isPeak;
                turn = &WaveShaper::turnTrough;
                turnOther = &WaveShaper::turnPeak;
                peak = -1;
                trough = -1;
                crossing = -1;
                secondCrossing = -1;
                slope = zero;
                std::fill(wave, wave + fifoLength, zero); }

            void process (FloatType* buffer, FloatType blend, const AtanParams& atanParams, const CrusherParams& crusherParams, const DeviationParams& deviationParams) {
                for (int i { 0 }; i < bufferLength; ++i) wave[waveIndex + i] = buffer[i];
                for(int i { waveIndex }, end { waveIndex + bufferLength }; i < end; ++i) {
                    if((this->*compare)(i)) {
                        (this->*turn)(leftOf(i), blend, atanParams, crusherParams, deviationParams);
                        switchDirections();
                    }
                }
                waveIndex = (waveIndex + bufferLength) % fifoLength;
                if (crossing >= waveIndex && crossing < waveIndex + bufferLength) crossing = -1;
                for (int i { 0 }; i < bufferLength; ++i) buffer[i] = wave[waveIndex + i];
            }
            
            void processHigh (FloatType* buffer, FloatType* hpBuffer, FloatType blend, const AtanParams& atanParams, const CrusherParams& crusherParams, const DeviationParams& deviationParams) {
                for (int i { 0 }; i < bufferLength; ++i) wave[waveIndex + i] = buffer[i];
                for (int i { 0 }; i < bufferLength; ++i) data[waveIndex + i] = hpBuffer[i];
                for(int i { waveIndex }, end { waveIndex + bufferLength }; i < end; ++i) {
                    if((this->*compare)(i)) {
                        (this->*turn)(leftOf(i), blend, atanParams, crusherParams, deviationParams);
                        switchDirections();
                    }
                }
                waveIndex = (waveIndex + bufferLength) % fifoLength;
                if (crossing >= waveIndex && crossing < waveIndex + bufferLength) crossing = -1;
                for (int i { 0 }; i < bufferLength; ++i) buffer[i] = wave[waveIndex + i];
                for (int i { 0 }; i < bufferLength; ++i) hpBuffer[i] = data[waveIndex + i];
            }

            int leftOf(int i) { return ((i - 1) + fifoLength) % fifoLength; }
            int rightOf(int i) { return (i + 1) % fifoLength; }

            bool isTrough (const int i) {
                FloatType valLeft { wave[leftOf(i)] }, val { wave[i] };
                FloatType localSlope { valLeft - val };
                if (localSlope - slope > -slopeTolerance) {
                    if (localSlope - slope > slopeTolerance) {
                        cross2 = cross = leftOf(i);
                        slope = localSlope;
                    }
                    else cross2 = leftOf(i);
                }
                return valLeft < val;
            }

            bool isPeak (const int i) {
                FloatType valLeft { wave[leftOf(i)] }, val { wave[i] };
                FloatType localSlope { val - valLeft };
                if (localSlope - slope > -slopeTolerance) {
                    if (localSlope - slope > slopeTolerance) {
                        cross2 = cross = leftOf(i);
                        slope = localSlope;
                    }
                    else cross2 = leftOf(i);
                }
                return val < valLeft;
            }

            void turnPeak(const int newPeak, FloatType blend, const AtanParams& atanParams, const CrusherParams& crusherParams, const DeviationParams& deviationParams) {
                int newCrossing { (cross + distance(cross, cross2)/2) %  fifoLength };
                auto newCrossingSlope { getSlopeAt(newCrossing) };
                if (crossing != -1) convolveOriginal(newCrossing, newPeak, newCrossingSlope, blend, atanParams, crusherParams, deviationParams);
                crossing = newCrossing;
                crossingSlope = newCrossingSlope;
                peak = newPeak;
                slope = zero;
            }

            void turnTrough(const int newTrough, FloatType, const AtanParams&, const CrusherParams&, const DeviationParams&) {
                int newCrossing { (cross + distance(cross, cross2)/2) %  fifoLength };
                trough = newTrough;
                secondCrossing = newCrossing;
                slope = zero;
            }
            
            FloatType getSlopeAt(int x) { return (wave[rightOf(x)] - wave[leftOf(x)])/two; }
            FloatType getAmpFromSlopeAndLength(FloatType s, int length) {
                FloatType normalSineAmplitude { Wave<FloatType>::getSineAmplitudeAtIndex(length * 2, 1) };
                return s / normalSineAmplitude;
            }

            FloatType getAmplitudeExiting(int x, int nextCrossing) {
                int length { distance(x, nextCrossing) };
                FloatType normalSineAmplitude { Wave<FloatType>::getSineAmplitudeAtIndex(length * 2, 1) };
                FloatType dcAtAdjacentSample { wave[x] - (wave[x] - wave[nextCrossing])/FloatType(length) };
//                FloatType dcAtAdjacentSample { getDC(1, length, wave[x], wave[nextCrossing]) };
//                return jmin(one, abs((wave[rightOf(x)] - dcAtAdjacentSample) / normalSineAmplitude));
//                return jmin(one, abs((wave[rightOf(x)] - dcAtAdjacentSample / normalSineAmplitude));
                return jmin(one, (abs(wave[rightOf(x)] - wave[leftOf(x)])/two) / normalSineAmplitude);
            }

            FloatType getAmplitudeEntering(int x, int lastCrossing) {
                int length { distance(lastCrossing, x) };
                FloatType normalSineAmplitude { Wave<FloatType>::getSineAmplitudeAtIndex(length * 2, 1) };
                FloatType dcAtAdjacentSample { wave[x] - (wave[x] - wave[lastCrossing])/FloatType(length) };
//                FloatType dcAtAdjacentSample { getDC(length - 1, length, wave[lastCrossing], wave[x]) };
//                return jmin(one, abs((wave[leftOf(x)] - dcAtAdjacentSample) / normalSineAmplitude));
//                return jmin(one, ((abs(wave[leftOf(x)] - wave[x]) + abs(wave[x] - wave[rightOf(x)]))/two) / normalSineAmplitude);
                return jmin(one, (abs(wave[rightOf(x)] - wave[leftOf(x)])/two) / normalSineAmplitude);
            }

            void convolveOriginal(int newCrossing, int newPeak, FloatType newCrossingSlope, FloatType blend, const AtanParams& atanParams, const CrusherParams& crusherParams, const DeviationParams& deviationParams) {
                const int length1 { distance(crossing, secondCrossing) }, length2 { distance(secondCrossing, newCrossing) };
                if(length1 > 1 && length2 > 1 && distance(peak, trough) > 1 && distance(trough, newPeak) > 1) {
//                    FloatType amp1 { getAmplitudeExiting(crossing, secondCrossing) };
                    FloatType amp1 { getAmpFromSlopeAndLength(crossingSlope, length1) };
                    FloatType amp2 { getAmplitudeEntering(secondCrossing, crossing) };
                    FloatType amp3 { getAmplitudeExiting(secondCrossing, newCrossing) };
//                    FloatType amp4 { getAmplitudeEntering(newCrossing, secondCrossing) };
                    FloatType amp4 { getAmpFromSlopeAndLength(newCrossingSlope, length2) };
                    FloatType diff1 { (amp2 - amp1) / FloatType(length1) }, diff2 { (amp4 - amp3) / FloatType(length2) };
                    convolve(crossing, secondCrossing, zero, half, amp1, amp2, blend, atanParams, crusherParams, deviationParams);
                    convolve(secondCrossing, newCrossing, half, one, amp3, amp4, blend, atanParams, crusherParams, deviationParams);
                }
            }

//            void convolve (int start, int length, FloatType phase, FloatType phaseStep, FloatType amp, FloatType ampStep, FloatType dc, FloatType dcStep) {
//
//                for (int i { 0 }; i < length; ++i, phase += phaseStep, amp += ampStep, dc += dcStep) {
//                    FloatType phaseAmplitude { shaper.phaseTable[phase] };
//                    FloatType multipliedWithGain { shaper.gainTable[phaseAmplitude] };
//                    wave[start + i] = dc + amp * multipliedWithGain;
//                }
//                shaper.crusher.processChannel(&wave[start], length, amp, ampStep);
//            }

            FloatType getDC(int i, int length, FloatType startDC, FloatType endDC) {
                return startDC + (endDC - startDC) * (half - std::cos((FloatType(i) * pi)/FloatType(length))/two);
            }

            void convolve (int start, int end, FloatType startPhase, FloatType endPhase, FloatType startAmp, FloatType endAmp, FloatType blend, const AtanParams& atanParams, const CrusherParams& crusherParams, const DeviationParams& deviationParams) {
                auto length { distance(start, end) };
                FloatType phaseStep { (endPhase - startPhase) / FloatType(length) };
                FloatType dcStep { (wave[end] - wave[start]) / FloatType(length) };
                FloatType ampStep { (endAmp - startAmp) / FloatType(length) };
                FloatType phase { startPhase }, dc { wave[start] }, amp { startAmp };

//                shaper.crusher.processChannel(&wave[start], length, amp, ampStep);

                for (int i { start }, j { 0 }; i != end; ++i%=fifoLength, ++j, phase += phaseStep, amp += ampStep, dc += dcStep) {
                    FloatType phaseAmplitude { shaper.phaseTable[phase] };
                    FloatType atanAmplitude { shaper.atan.processSample(phaseAmplitude, atanParams) };
                    FloatType crushedAmplitude { shaper.crusher.processSample(atanAmplitude, crusherParams) };
                    FloatType deviationAmplitude { shaper.deviation.processSample(crushedAmplitude, deviationParams) };
                    FloatType multipliedWithGain { amp * shaper.gainTable[phaseAmplitude] };//deviationAmplitude };
                    wave[i] = blend * (getDC(j, length, wave[start], wave[end]) + multipliedWithGain) + (one - blend) * wave[i];
                }
            }

            void switchDirections() { std::swap(compare, compareOther); std::swap(turn, turnOther); }

            FloatType wave[fifoLength]{}, data[fifoLength]{};
            Comparator compare { &WaveShaper::isTrough }, compareOther { &WaveShaper::isPeak };
            TurnFunction turn { &WaveShaper::turnTrough }, turnOther { &WaveShaper::turnPeak };
            int waveIndex { 0 }, peak { -1 }, trough { -1 }, crossing { -1 }, secondCrossing { -1 }, cross { 0 }, cross2 { 0 };
            FloatType slope { zero }, slopeTolerance { static_cast<FloatType>(0.0001) }, crossingSlope { zero };
            DynamicWaveShaper& shaper;
        };

        void loadHighPassBuffer(AudioBuffer<FloatType>& hpBuffer) {  }
        
        void processSamples(AudioBuffer<FloatType>& buffer) override {
            if (isEnabled()) {
                const auto blend { getBlend() };
//                const FloatType crushMax { crusher.getMax() }, crushBlend { crusher.getBlend() };
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
                        
                        if (newPhaseDataHere) bringInNewPhaseData();
                        if (newGainDataHere) bringInNewGainData();
                        
//                        AudioBlock<FloatType> block { temp, static_cast<size_t>(buffer.getNumChannels()), static_cast<size_t>(bufferLength/dFactor) };
//                        for (int j { 0 }; j < buffer.getNumChannels(); ++j) {
//                            for (int i { 0 }; i < bufferLength / dFactor; ++i)
//                                temp[j][i] = writeFifo[j][i * dFactor];
//                        }
//                        auto upBlock { oversampler.processSamplesUp(block) };
                        
                        const auto atanParams { atan.returnParams() };
                        const auto crusherParams { crusher.returnParams() };
                        const auto deviationParams { deviation.returnParams() };
                        
                        for (int j { 0 }; j < buffer.getNumChannels(); ++j)
                            waveShaper[j].process(writeFifo[j], /* upBlock.getChannelPointer(j), */ blend, atanParams, crusherParams, deviationParams);

//                        oversampler.processSamplesDown(upBlock);
//
//                        for (int j { 0 }; j < buffer.getNumChannels(); ++j)
//                            interpolator[j].process(one / FloatType(dFactor), temp[j], writeFifo[j], bufferLength);

//                        for (int i { 0 }; i < buffer.getNumChannels(); ++i)
//                            waveShaper[i].process(writeFifo[i], blend, crushMax, crushBlend);
                        
                        fifoIndex = 0;
                        std::swap(writeFifo, readFifo);
                    }
                }
            }
        }
        
        void processHigh(AudioBuffer<FloatType>& buffer, AudioBuffer<FloatType>& hpBuffer) {
            if (isEnabled()) {
                const auto blend { getBlend() };
                //                const FloatType crushMax { crusher.getMax() }, crushBlend { crusher.getBlend() };
                int bufferIndex { 0 }, numSamples { buffer.getNumSamples() };
                
                while (bufferIndex < numSamples) {
                    int samples { jmin(bufferLength - fifoIndex, numSamples - bufferIndex) };
                    
                    for (int j { 0 }; j < buffer.getNumChannels(); ++j) {
                        for (int i { bufferIndex }, write { fifoIndex }; i < bufferIndex + samples; ++i, ++write)
                            writeData[j][write] = static_cast<FloatType>(hpBuffer.getSample(j,i));
                        for (int i { bufferIndex }, write { fifoIndex }; i < bufferIndex + samples; ++i, ++write)
                            writeFifo[j][write] = static_cast<FloatType>(buffer.getSample(j,i));
                        for (int i { bufferIndex }, read { fifoIndex }; i < bufferIndex + samples; ++i, ++read)
                            hpBuffer.setSample(j, i, static_cast<FloatType>(readData[j][read]));
                        for (int i { bufferIndex }, read { fifoIndex }; i < bufferIndex + samples; ++i, ++read)
                            buffer.setSample(j, i, static_cast<FloatType>(readFifo[j][read]));
                    }
                    
                    fifoIndex += samples;
                    bufferIndex += samples;
                    
                    if (fifoIndex == bufferLength) {
                        
                        if (newPhaseDataHere) bringInNewPhaseData();
                        if (newGainDataHere) bringInNewGainData();
                        
                        //                        AudioBlock<FloatType> block { temp, static_cast<size_t>(buffer.getNumChannels()), static_cast<size_t>(bufferLength/dFactor) };
                        //                        for (int j { 0 }; j < buffer.getNumChannels(); ++j) {
                        //                            for (int i { 0 }; i < bufferLength / dFactor; ++i)
                        //                                temp[j][i] = writeFifo[j][i * dFactor];
                        //                        }
                        //                        auto upBlock { oversampler.processSamplesUp(block) };
                        
                        const auto atanParams { atan.returnParams() };
                        const auto crusherParams { crusher.returnParams() };
                        const auto deviationParams { deviation.returnParams() };
                        
                        for (int j { 0 }; j < buffer.getNumChannels(); ++j)
                            waveShaper[j].process(writeFifo[j], /* upBlock.getChannelPointer(j), */ blend, atanParams, crusherParams, deviationParams);
                        
                        //                        oversampler.processSamplesDown(upBlock);
                        //
                        //                        for (int j { 0 }; j < buffer.getNumChannels(); ++j)
                        //                            interpolator[j].process(one / FloatType(dFactor), temp[j], writeFifo[j], bufferLength);
                        
                        //                        for (int i { 0 }; i < buffer.getNumChannels(); ++i)
                        //                            waveShaper[i].process(writeFifo[i], blend, crushMax, crushBlend);
                        
                        fifoIndex = 0;
                        std::swap(writeFifo, readFifo);
                    }
                }
            }
        }

        void bringInNewPhaseData() {
//            FloatType phaseData[waveLength * 2 + phaseFilterOrder]{};
//            sadistic::DeviantTree::getPhaseTable(apvts, phaseData);
//            FloatType firstSample { phaseData[1] };
//            phaseData[0] = zero;
//            std::copy(phaseData, phaseData + waveLength, phaseData + waveLength);
//            FloatType* mockChannel[1] { phaseData };
//            AudioBlock<FloatType> block { mockChannel, 1, waveLength * 2 + phaseFilterOrder };
//            phaseTable.filter.process(ProcessContextReplacing<FloatType>(block));
//            FloatType newFirstSample { phaseData[phaseFilterOrder/2 + 1] };
//            FloatType normalizer { newFirstSample / firstSample };
//            std::transform(phaseData + phaseFilterOrder/2, phaseData + phaseFilterOrder/2 + waveLength, phaseTable.table, [=](FloatType n) { return n * normalizer; });
            
            sadistic::DeviantTree::getPhaseTable(apvts, phaseTable.table);
            phaseTable.table[0] = phaseTable.table[waveLength] = phaseTable.table[waveLength] = zero;
            
            newPhaseDataHere = false;
        }
        void bringInNewGainData() {
            sadistic::DeviantTree::getGainTable(apvts, gainTable.table);
            //            gainTable.table[waveLength] = jlimit(zero, one, gainTable.table[waveLength - 1] + (gainTable.table[waveLength - 1] - gainTable.table[waveLength - 2]));
            newGainDataHere = false;
        }
        
        FloatType fifo1L[bufferLength]{}, fifo1R[bufferLength]{}, fifo2L[bufferLength]{}, fifo2R[bufferLength]{}, fifo3L[bufferLength]{}, fifo3R[bufferLength]{}, fifo4L[bufferLength]{}, fifo4R[bufferLength]{};
        GainTable<FloatType> gainTable{};
        PhaseTable<FloatType> phaseTable{};
        FloatType* fifo1[2]{ fifo1L, fifo1R }, * fifo2[2]{ fifo2L, fifo2R }, * fifo3[2]{ fifo3L, fifo3R }, * fifo4[2]{ fifo4L, fifo4R };
        FloatType** writeFifo{ fifo1 }, ** readFifo { fifo2 }, ** writeData{ fifo3 }, ** readData { fifo4 };
        FloatType wave[fifoLength]{}, data[fifoLength]{};
        int waveIndex { 0 }, peak { -1 }, trough { -1 }, crossing { -1 }, secondCrossing { -1 }, cross { 0 }, cross2 { 0 };
        FloatType slope { zero }, delta { zero }, lastSlope { zero }, crossingSlope { zero }, slopeTolerance { static_cast<FloatType>(0.00001) };
        std::atomic<bool> newPhaseDataHere { true }, newGainDataHere { true };
        int fifoIndex { 0 };
        Oversampling<FloatType> oversampler { 2, static_cast<size_t>(dOrder), Oversampling<FloatType>::filterHalfBandFIREquiripple };
        LagrangePreciseInterpolator<FloatType> interpolator[2];
        APVTS& apvts;
        DynamicAtan<FloatType>& atan;
        DynamicBitCrusher<FloatType>& crusher;
        DynamicDeviation<FloatType>& deviation;
        WaveShaper waveShaper[2] { { *this }, { *this } };
    };
}
