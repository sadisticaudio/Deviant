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
        FloatType gTable[waveLength + 1]{};
        GainTable<FloatType> waveTable{};
        bool newDataHere { true };
        APVTS& apvts;
    };

    template<typename FloatType>
    struct DynamicWaveShaper : DeviantEffect, ValueTree::Listener {
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
            using TurnFunction = void(WaveShaper::*)(const int, FloatType, FloatType, FloatType);

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

            void process (FloatType* buffer, FloatType drive, FloatType crushMax, FloatType crushBlend) {
                for (int i { 0 }; i < bufferLength; ++i) wave[waveIndex + i] = buffer[i];
                for(int i { waveIndex }, end { waveIndex + bufferLength }; i < end; ++i) {
                    if((this->*compare)(i)) {
                        (this->*turn)(((i - 1) + fifoLength) % fifoLength, drive, crushMax, crushBlend);
                        switchDirections();
                    }
                }
                waveIndex = (waveIndex + bufferLength) % fifoLength;
                if (crossing >= waveIndex && crossing < waveIndex + bufferLength) crossing = -1;
                for (int i { 0 }; i < bufferLength; ++i) buffer[i] = wave[waveIndex + i];
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

            void turnPeak(const int newPeak, FloatType drive, FloatType crushMax, FloatType crushBlend) {
                int newCrossing { (cross + distance(cross, cross2)/2) %  fifoLength };
                if (crossing != -1) convolveOriginal(newCrossing, newPeak, drive, crushMax, crushBlend);
                crossing = newCrossing;
                peak = newPeak;
                slope = zero;
            }

            void turnTrough(const int newTrough, FloatType, FloatType, FloatType) {
                int newCrossing { (cross + distance(cross, cross2)/2) %  fifoLength };
                trough = newTrough;
                secondCrossing = newCrossing;
                slope = zero;
            }

            FloatType getAmplitudeExiting(int x, int nextCrossing) {
                int length { distance(x, nextCrossing) };
                FloatType normalSineAmplitude { Wave<FloatType>::getSineAmplitudeAtIndex(length * 2, 1) };
                FloatType dcAtAdjacentSample { wave[x] - (wave[x] - wave[nextCrossing])/FloatType(length) };
                return jmin(one, abs((wave[rightOf(x)] - dcAtAdjacentSample) / normalSineAmplitude));
            }

            FloatType getAmplitudeEntering(int x, int lastCrossing) {
                int length { distance(lastCrossing, x) };
                FloatType normalSineAmplitude { Wave<FloatType>::getSineAmplitudeAtIndex(length * 2, 1) };
                FloatType dcAtAdjacentSample { wave[x] - (wave[x] - wave[lastCrossing])/FloatType(length) };
                return jmin(one, abs((wave[leftOf(x)] - dcAtAdjacentSample) / normalSineAmplitude));
            }

            void convolveOriginal(int newCrossing, int newPeak, FloatType drive, FloatType crushMax, FloatType crushBlend) {
                const int length1 { distance(crossing, secondCrossing) }, length2 { distance(secondCrossing, newCrossing) };
                if(length1 > 1 && length2 > 1 && distance(peak, trough) > 1 && distance(trough, newPeak) > 1) {
                    FloatType amp1 { getAmplitudeExiting(crossing, secondCrossing) };
                    FloatType amp2 { getAmplitudeEntering(secondCrossing, crossing) };
                    FloatType amp3 { getAmplitudeExiting(secondCrossing, newCrossing) };
                    FloatType amp4 { getAmplitudeEntering(newCrossing, secondCrossing) };
                    FloatType diff1 { (amp2 - amp1) / FloatType(length1) }, diff2 { (amp4 - amp3) / FloatType(length2) };
                    convolve(crossing, secondCrossing, zero, half, amp1 - diff1, amp2 + diff1, drive, crushMax, crushBlend);
                    convolve(secondCrossing, newCrossing, half, one, amp3 - diff2, amp4 + diff2, drive, crushMax, crushBlend);
                }
            }

            void convolve(int newCrossing, int newPeak) {
                const int length1 { distance(crossing, secondCrossing) }, length2 { distance(secondCrossing, newCrossing) };
                if(length1 > 1 && length2 > 1 && distance(peak, trough) > 1 && distance(trough, newPeak) > 1) {
                    FloatType amp1 { getAmplitudeExiting(crossing, secondCrossing) };
                    FloatType amp2 { getAmplitudeEntering(secondCrossing, crossing) };
                    FloatType amp3 { getAmplitudeExiting(secondCrossing, newCrossing) };
                    FloatType amp4 { getAmplitudeEntering(newCrossing, secondCrossing) };
                    FloatType diff1 { (amp2 - amp1) / FloatType(length1) }, diff2 { (amp4 - amp3) / FloatType(length2) };
                    amp1 -= diff1, amp2 += diff1, amp3 -=diff2, amp4 += diff2;
                    FloatType ampStep { (amp2 - amp1) / FloatType(length1) };
                    FloatType phaseStep { half / FloatType(length1) };
                    FloatType dcStep { (wave[secondCrossing] - wave[crossing]) / FloatType(length1) };
                    int length { jmin(distance(crossing, secondCrossing), fifoLength - crossing) };
                    FloatType ratio { FloatType(length)/FloatType(length1) }, endPhase { half * ratio };
                    convolve(crossing, length, zero, phaseStep, amp1, ampStep, wave[crossing], dcStep);
                    if (length != length1)
                        convolve(0, length1 - length, endPhase, phaseStep, amp1 + (amp2 - amp1) * ratio, ampStep, wave[crossing]  + (wave[secondCrossing] - wave[crossing]) * ratio, dcStep);
                    ampStep = { (amp4 - amp3) / FloatType(length2) };
                    phaseStep = { half / FloatType(length2) };
                    dcStep = { (wave[newCrossing] - wave[secondCrossing]) / FloatType(length2) };
                    length = { jmin(distance(secondCrossing, newCrossing), fifoLength - secondCrossing) };
                    ratio = { FloatType(length)/FloatType(length2) }, endPhase = { half + half * ratio };
                    convolve(secondCrossing, length, half, phaseStep, amp3, ampStep, wave[secondCrossing], dcStep);
                    if (length != length2)
                        convolve(0, length2 - length, endPhase, phaseStep, amp3 + (amp4 - amp3) * ratio, ampStep, wave[secondCrossing] + (wave[newCrossing] - wave[secondCrossing]) * ratio, dcStep);
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

            void convolve (int start, int end, FloatType startPhase, FloatType endPhase, FloatType startAmp, FloatType endAmp, FloatType drive, FloatType crushMax, FloatType crushBlend) {
                auto length { distance(start, end) };
                FloatType phaseStep { (endPhase - startPhase) / FloatType(length) };
                FloatType dcStep { (wave[end] - wave[start]) / FloatType(length) };
                FloatType ampStep { (endAmp - startAmp) / FloatType(length) };
                FloatType phase { startPhase }, dc { wave[start] }, amp { startAmp };

//                shaper.crusher.processChannel(&wave[start], length, amp, ampStep);

                for (int i { start }, j { 0 }; i != end; ++i%=fifoLength, ++j, phase += phaseStep, amp += ampStep, dc += dcStep) {
                    FloatType phaseAmplitude { shaper.phaseTable[phase] };
                    FloatType crushedAmplitude { DynamicBitCrusher<FloatType>::processSample(phaseAmplitude, crushMax, crushBlend) };
                    FloatType multipliedWithGain { shaper.gainTable[amp * crushedAmplitude] };
                    wave[i] = drive * (getDC(j, length, wave[start], wave[end]) + multipliedWithGain) + (one - drive) * wave[i];
                }
            }

            void switchDirections() { std::swap(compare, compareOther); std::swap(turn, turnOther); }

            FloatType wave[fifoLength]{};
            Comparator compare { &WaveShaper::isTrough }, compareOther { &WaveShaper::isPeak };
            TurnFunction turn { &WaveShaper::turnTrough }, turnOther { &WaveShaper::turnPeak };
            int waveIndex { 0 }, peak { -1 }, trough { -1 }, crossing { -1 }, secondCrossing { -1 }, cross { 0 }, cross2 { 0 };
            FloatType slope { zero }, slopeTolerance { static_cast<FloatType>(0.0001) };
            DynamicWaveShaper& shaper;
        };
        
        void processSamples(AudioBuffer<FloatType>& buffer) override {
            enum { driveIndex };
            if (isEnabled()) {
                const auto drive { static_cast<FloatType>(params[driveIndex].get().get()) };
                const FloatType crushMax { crusher.getMax() }, crushBlend { crusher.getBlend() };
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
                        
                        for (int j { 0 }; j < buffer.getNumChannels(); ++j)
                            waveShaper[j].process(writeFifo[j], /* upBlock.getChannelPointer(j), */ FloatType(drive), crushMax, crushBlend);

//                        oversampler.processSamplesDown(upBlock);
//
//                        for (int j { 0 }; j < buffer.getNumChannels(); ++j)
//                            interpolator[j].process(one / FloatType(dFactor), temp[j], writeFifo[j], bufferLength);

//                        for (int i { 0 }; i < buffer.getNumChannels(); ++i)
//                            waveShaper[i].process(writeFifo[i], drive, crushMax, crushBlend);
                        
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
        
        FloatType fifo1L[bufferLength]{}, fifo1R[bufferLength]{}, fifo2L[bufferLength]{}, fifo2R[bufferLength]{}, tempL[bufferLength/dFactor]{}, tempR[bufferLength/dFactor]{};
        //        FloatType gTable[waveLength + 1]{}, pTable[waveLength + phaseFilterOrder + 1]{};
        GainTable<FloatType> gainTable{};
        PhaseTable<FloatType> phaseTable{};
        FloatType*     fifo1[2] { fifo1L, fifo1R },     *  fifo2[2] { fifo2L, fifo2R },    *  tempA[2]{ tempL, tempR };
        FloatType**    writeFifo{ fifo1 },              ** readFifo { fifo2 }, ** temp { tempA };
        FloatType wave[fifoLength]{};
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
