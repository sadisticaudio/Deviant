#pragma once
#include "Effects.h"
#include "../../Source/SadisticFFT.h"

namespace sadistic {

    template<typename FloatType>
    struct StaticWaveShaper : DeviantEffect, ValueTree::Listener {
        static constexpr int waveLength { WAVELENGTH };
        
        StaticWaveShaper(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager& s) :
        DeviantEffect(eID, refs, floatRefs, eIDX), mgmt(s) {}
        template<typename F> StaticWaveShaper(StaticWaveShaper<F>& other) : DeviantEffect(other), mgmt(other.mgmt) {}
        
        void valueTreePropertyChanged(ValueTree&, const Identifier& identifier) override {
            if(identifier == Identifier("waveTable")) newDataHere = true; }
        
        void processSamples(AudioBuffer<FloatType>& buffer) override {
            if(newDataHere) {
                mgmt.getTable("waveTable", waveTable.table, waveLength + 1);
                newDataHere = false;
            }
            for (int j { 0 }; j < buffer.getNumChannels(); ++j)
                for (int i { 0 }; i < buffer.getNumSamples(); ++i)
                    buffer.setSample(j, i, waveTable[buffer.getSample(j, i)]);
        }
        FloatType wave[waveLength + 1]{};
        Table<FloatType> waveTable{ wave, FloatType(waveLength), FloatType(0.5), FloatType(0.5) };
        bool newDataHere { true };
        TableManager& mgmt;
    };

    template<typename FloatType>
    struct DynamicWaveShaper : DeviantEffect, ValueTree::Listener {

        using Coeffs = CalculatedParamCoefficients<FloatType>;
        
        static constexpr int bufferLength { BUFFERLENGTH }, maxWaveOrder { 10 }, maxWavelength { 1 << maxWaveOrder }, fifoLength { maxWavelength + bufferLength }, waveLength { WAVELENGTH };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<FloatType>::pi }, halfPi { MathConstants<FloatType>::halfPi }, twoPi { MathConstants<FloatType>::twoPi };
        
        DynamicWaveShaper(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager& s) :
        DeviantEffect(eID, refs, floatRefs, eIDX), mgmt(s) {}
        template<typename F> DynamicWaveShaper(DynamicWaveShaper<F>& other) : DeviantEffect(other), mgmt(other.mgmt) {}
        
        static constexpr int distance(const int older, const int newer) { return ((newer-older) + fifoLength) % fifoLength; }
        
        void prepare(const ProcessSpec& spec) override {
            for (auto& w : waveShaper) w.prepare(spec);
            reset();
        }
        
        void reset() override {
            for (auto& w : waveShaper) w.reset();
        }
        
        void valueTreePropertyChanged(ValueTree&, const Identifier& identifier) override {
            if(identifier == Identifier("phaseTable")) newPhaseDataHere = true;
            if(identifier == Identifier("gainTable")) newGainDataHere = true; }
        int getLatency() override { return fifoLength; }// + int(oversampler.getLatencyInSamples()) + interpolator[0].getBaseLatency() * FloatType(dFactor); }
        
        struct WaveShaper {

            using Comparator = bool(WaveShaper::*)(const int);
            using TurnFunction = void(WaveShaper::*)(const int, FloatType);

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

            void process (FloatType* buffer, FloatType blend) {
                for (int i { 0 }; i < bufferLength; ++i) wave[waveIndex + i] = buffer[i];
                for(int i { waveIndex }, end { waveIndex + bufferLength }; i < end; ++i) {
                    if((this->*compare)(i)) {
                        (this->*turn)(leftOf(i), blend);
                        switchDirections();
                    }
                }
                waveIndex = (waveIndex + bufferLength) % fifoLength;
                if (crossing >= waveIndex && crossing < waveIndex + bufferLength) crossing = -1;
                for (int i { 0 }; i < bufferLength; ++i) buffer[i] = wave[waveIndex + i];
            }
            
            void processHigh (FloatType* buffer, FloatType* hpBuffer, FloatType blend) {
                for (int i { 0 }; i < bufferLength; ++i) wave[waveIndex + i] = buffer[i];
                for (int i { 0 }; i < bufferLength; ++i) data[waveIndex + i] = hpBuffer[i];
                for(int i { waveIndex }, end { waveIndex + bufferLength }; i < end; ++i) {
                    if((this->*compare)(i)) {
                        (this->*turn)(leftOf(i), blend);
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

            void turnPeak(const int newPeak, FloatType blend) {
                int newCrossing { (cross + distance(cross, cross2)/2) %  fifoLength };
                auto newCrossingSlope { getSlopeAt(newCrossing) };
                if (crossing != -1) convolveOriginal(newCrossing, newPeak, newCrossingSlope, blend);
                crossing = newCrossing;
                crossingSlope = newCrossingSlope;
                peak = newPeak;
                slope = zero;
            }

            void turnTrough(const int newTrough, FloatType) {
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

            void convolveOriginal(int newCrossing, int newPeak, FloatType newCrossingSlope, FloatType blend) {
                const int length1 { distance(crossing, secondCrossing) }, length2 { distance(secondCrossing, newCrossing) };
                if(length1 > 1 && length2 > 1 && distance(peak, trough) > 1 && distance(trough, newPeak) > 1) {
//                    FloatType amp1 { getAmplitudeExiting(crossing, secondCrossing) };
                    FloatType amp1 { getAmpFromSlopeAndLength(crossingSlope, length1) };
                    FloatType amp2 { getAmplitudeEntering(secondCrossing, crossing) };
                    FloatType amp3 { getAmplitudeExiting(secondCrossing, newCrossing) };
//                    FloatType amp4 { getAmplitudeEntering(newCrossing, secondCrossing) };
                    FloatType amp4 { getAmpFromSlopeAndLength(newCrossingSlope, length2) };
                    FloatType diff1 { (amp2 - amp1) / FloatType(length1) }, diff2 { (amp4 - amp3) / FloatType(length2) };
//                    const auto& table { shaper.phaseTableSet.getTable(length1 + length2) };
                    convolve(crossing, secondCrossing, zero, half, amp1, amp2, blend);
                    convolve(secondCrossing, newCrossing, half, one, amp3, amp4, blend);
                }
            }

            FloatType getDC(int i, int length, FloatType startDC, FloatType endDC) {
                return startDC + (endDC - startDC) * (half - std::cos((FloatType(i) * pi)/FloatType(length))/two); }

            void convolve (int start, int end, FloatType startPhase, FloatType endPhase, FloatType startAmp, FloatType endAmp, FloatType blend) {
                auto length { distance(start, end) };
                FloatType phaseStep { (endPhase - startPhase) / FloatType(length) };
                FloatType dcStep { (wave[end] - wave[start]) / FloatType(length) };
                FloatType ampStep { (endAmp - startAmp) / FloatType(length) };
                FloatType phase { startPhase }, dc { wave[start] }, amp { startAmp };
                for (int i { start }, j { 0 }; i != end; ++i%=fifoLength, ++j, phase += phaseStep, amp += ampStep, dc += dcStep) {
                    FloatType phaseAmplitude { shaper.phaseTable[phase] };
                    FloatType atanAmplitude { DynamicAtan<FloatType>::processAtanSample(phaseAmplitude, shaper.atanCoeffs) };
                    FloatType crushedAmplitude { DynamicBitCrusher<FloatType>::crushSample(atanAmplitude, shaper.crusherCoeffs) };
                    FloatType deviationAmplitude { DynamicDeviation<FloatType>::deviateSample(crushedAmplitude, shaper.deviationCoeffs) };
                    FloatType multipliedWithGain { amp * shaper.gainTable[deviationAmplitude] };
                    wave[i] = blend * (getDC(j, length, wave[start], wave[end]) + multipliedWithGain) + (one - blend) * wave[i];
                }
            }
            
            template<int N> void convolve (int start, int end, FloatType startPhase, FloatType endPhase, FloatType startAmp, FloatType endAmp, FloatType blend, const STable<FloatType, N>& table) {
                auto length { distance(start, end) };
                FloatType phaseStep { (endPhase - startPhase) / FloatType(length) };
                FloatType dcStep { (wave[end] - wave[start]) / FloatType(length) };
                FloatType ampStep { (endAmp - startAmp) / FloatType(length) };
                FloatType phase { startPhase }, dc { wave[start] }, amp { startAmp };
                for (int i { start }, j { 0 }; i != end; ++i%=fifoLength, ++j, phase += phaseStep, amp += ampStep, dc += dcStep) {
                    FloatType phaseAmplitude { shaper.phaseTable[phase] };
                    FloatType atanAmplitude { shaper.atan.processSample(phaseAmplitude, shaper.atanParams) };
                    FloatType crushedAmplitude { shaper.crusher.processSample(atanAmplitude, shaper.crusherParams) };
                    FloatType deviationAmplitude { shaper.deviation.processSample(crushedAmplitude, shaper.deviationParams) };
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
        
        void processSamples(AudioBuffer<FloatType>& buffer) override {
            
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
                    const auto blend { getBlend() };

                    for (int j { 0 }; j < buffer.getNumChannels(); ++j)
                        waveShaper[j].process(&writeFifo[j][0], blend);

                    fifoIndex = 0;
                    std::swap(writeFifo, readFifo);
                }
            }
        }

        void bringInNewPhaseData() {
//            FloatType phaseData[waveLength * 2 + phaseFilterOrder]{};
//            sadistic::TableManager::getPhaseTable(mgmt.apvts, phaseData);
//            FloatType firstSample { phaseData[1] };
//            phaseData[0] = zero;
//            std::copy(phaseData, phaseData + waveLength, phaseData + waveLength);
//            FloatType* mockChannel[1] { phaseData };
//            AudioBlock<FloatType> block { mockChannel, 1, waveLength * 2 + phaseFilterOrder };
//            phaseTable.filter.process(ProcessContextReplacing<FloatType>(block));
//            FloatType newFirstSample { phaseData[phaseFilterOrder/2 + 1] };
//            FloatType normalizer { newFirstSample / firstSample };
//            std::transform(phaseData + phaseFilterOrder/2, phaseData + phaseFilterOrder/2 + waveLength, phaseTable.table, [=](FloatType n) { return n * normalizer; });
            
            mgmt.getTable("phaseTable", phaseTable.table, waveLength + 1);
//            phaseTable.table[0] = phaseTable.table[waveLength] = phaseTable.table[waveLength] = zero;
            
            newPhaseDataHere = false;
        }
        void bringInNewGainData() {
            mgmt.getTable("gainTable", gainTable.table, waveLength + 1);
            //            gainTable.table[waveLength] = jlimit(zero, one, gainTable.table[waveLength - 1] + (gainTable.table[waveLength - 1] - gainTable.table[waveLength - 2]));
            newGainDataHere = false;
        }
        
        bool smooth(Coeffs& mCoeffs, const FloatType mDelta, const Coeffs& c) {
            bool needsUpdate { false };
            for (int i { 0 }; i < Coeffs::max; ++i) {
                if (c.data[i] != mCoeffs.data[i]) {
                    mCoeffs.data[i] = jlimit(mCoeffs.data[i]/mDelta, mCoeffs.data[i] * mDelta, c.data[i]);
                    needsUpdate = true;
                }
            }
            return needsUpdate;
        }
        void setParams(const Coeffs& aC, const Coeffs& bC, const Coeffs& dC) {
            
            atanCoeffs = aC; crusherCoeffs = bC; deviationCoeffs = dC;
            
        }
        bool passParams(const Coeffs& aC, const Coeffs& bC, const Coeffs& dC) {
            bool needsUpdate { false };
            if(smooth(atanCoeffs, maxDelta, aC) || smooth(crusherCoeffs, maxDelta, bC) || smooth(deviationCoeffs, maxDelta, dC))
                needsUpdate = true;
        }
        
        FloatType fifo1L[bufferLength]{}, fifo1R[bufferLength]{}, fifo2L[bufferLength]{}, fifo2R[bufferLength]{};
        FloatType wave[waveLength]{}, pWave[waveLength]{};
//        Table<FloatType> gainTable { wave, FloatType(waveLength), half, half };
        GainTable<FloatType> gainTable{};
        STable<FloatType,0> phaseTable{};
        FloatType* fifo1[2]{ fifo1L, fifo1R }, * fifo2[2]{ fifo2L, fifo2R };
        FloatType** writeFifo{ fifo1 }, ** readFifo { fifo2 };
        PhaseTableSet<FloatType> phaseTableSet {};
        int waveIndex { 0 }, peak { -1 }, trough { -1 }, crossing { -1 }, secondCrossing { -1 }, cross { 0 }, cross2 { 0 };
        FloatType slope { zero }, delta { zero }, lastSlope { zero }, crossingSlope { zero };
        const FloatType maxDelta { static_cast<FloatType>(1.05) }, slopeTolerance { static_cast<FloatType>(0.00001) };
        std::atomic<bool> newPhaseDataHere { true }, newGainDataHere { true };
        int fifoIndex { 0 };
        TableManager& mgmt;
        Coeffs atanCoeffs{}, crusherCoeffs{}, deviationCoeffs{};
        WaveShaper waveShaper[2] { { *this }, { *this } };
    };
}
