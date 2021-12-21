#pragma once
#include "Effects.h"
#include "../../Source/SadisticFFT.h"
#include "../../Source/include/gcem.hpp"

namespace sadistic {

    template<typename F> struct StaticWaveShaper : DeviantEffect {
        static constexpr int waveLength { WAVELENGTH }, gainLength { GAINLENGTH };
        
        StaticWaveShaper(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager& s) :
        DeviantEffect(eID, refs, floatRefs, eIDX, s) {}
        template<typename OtherFloatType> StaticWaveShaper(StaticWaveShaper<OtherFloatType>& other) : DeviantEffect(other) {}
        void calculateCoefficients() override {}
        void reset() override {}
        void prepare(const ProcessSpec&) override {}
        int getLatency() override { return 0; }
        void processSamples(AudioBuffer<float>& buffer) override { processSamples<float>(buffer); }
        void processSamples(AudioBuffer<double>& buffer) override { processSamples<double>(buffer); }
        template<typename FloatType> void processSamples(AudioBuffer<FloatType>& buffer) {
            if constexpr (std::is_same<F, FloatType>::value) {
                const auto mag { jlimit(F(0.1), F(1.0), buffer.getMagnitude(0, buffer.getNumSamples())) };
                for (int j { 0 }; j < buffer.getNumChannels(); ++j) {
                    for (int i { 0 }; i < buffer.getNumSamples(); ++i)
                        buffer.setSample(j, i, mag * waveTable[buffer.getSample(j, i)/mag]);
                }
            }
        }
        F wave[gainLength + 1]{};
        Table<F> waveTable{ wave, F(gainLength), F(0.5), F(0.5) };
    };
    
    template<typename F>
    struct DynamicWaveShaper : DeviantEffect, ValueTree::Listener {

        static constexpr int bufferLength { BUFFERLENGTH }, maxWaveOrder { 12 }, maxWavelength { 1 << maxWaveOrder }, fifoLength { maxWavelength + bufferLength }, waveLength { WAVELENGTH }, gainLength { GAINLENGTH };
        static constexpr F zero { static_cast<F>(0) }, one { static_cast<F>(1) }, two { static_cast<F>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<F>::pi }, halfPi { MathConstants<F>::halfPi }, twoPi { MathConstants<F>::twoPi };
        
        DynamicWaveShaper(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager& s, std::atomic<int>* cI, float(& cS)[maxCoeffs][maxCoeffs][maxCoeffs]) :
        DeviantEffect(eID, refs, floatRefs, eIDX, s), coeffIdx(cI), mCoeffs(cS) {}
        template<typename OtherFloatType> DynamicWaveShaper(DynamicWaveShaper<OtherFloatType>& other) : DeviantEffect(other), coeffIdx(other.coeffIdx), mCoeffs(other.mCoeffs) {}
        
        static constexpr int distance(const int older, const int newer) { return ((newer-older) + fifoLength) % fifoLength; }
        void processSamples(AudioBuffer<float>& buffer) override { processSamples<float>(buffer); }
        void processSamples(AudioBuffer<double>& buffer) override { processSamples<double>(buffer); }
        void prepare(const ProcessSpec&) override { reset(); }
        void calculateCoefficients() override {}
        void reset() override { for (auto& w : waveShaper) w.reset(); }
        int getLatency() override { return fifoLength; }
        struct WaveShaper {
            using Comparator = bool(WaveShaper::*)(const int);
            using TurnFunction = void(WaveShaper::*)(const int);
            
            WaveShaper(DynamicWaveShaper& wS) : shaper(wS) {}
            
            void reset() {
                compare = &WaveShaper::isTrough;
                compareOther = &WaveShaper::isPeak;
                turn = &WaveShaper::turnTrough;
                turnOther = &WaveShaper::turnPeak;
                waveIndex = cross = cross2 = 0;
                peak = trough = crossing = secondCrossing = -1;
                slope = crossingSlope = zero;
                std::fill(wave, wave + fifoLength, zero); }
            
            void process (F* buffer) {
                for (int i { 0 }; i < bufferLength; ++i) wave[waveIndex + i] = buffer[i];
                for(int i { waveIndex }, end { waveIndex + bufferLength }; i < end; ++i) {
                    if((this->*compare)(i)) {
                        (this->*turn)(leftOf(i));
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
                F valLeft { wave[leftOf(i)] }, val { wave[i] };
                F localSlope { valLeft - val };
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
                F valLeft { wave[leftOf(i)] }, val { wave[i] };
                F localSlope { val - valLeft };
                if (localSlope - slope > -slopeTolerance) {
                    if (localSlope - slope > slopeTolerance) {
                        cross2 = cross = leftOf(i);
                        slope = localSlope;
                    }
                    else cross2 = leftOf(i);
                }
                return val < valLeft;
            }
            
            void turnPeak(const int newPeak) {
                int newCrossing { (cross + distance(cross, cross2)/2) % fifoLength };
                auto newCrossingSlope { getSlopeAt(newCrossing) };
                if (crossing != -1) convolve(newCrossing, newPeak, newCrossingSlope);
                crossing = newCrossing;
                crossingSlope = newCrossingSlope;
                peak = newPeak;
                slope = zero;
            }
            
            void turnTrough(const int newTrough) {
                int newCrossing { (cross + distance(cross, cross2)/2) % fifoLength };
                trough = newTrough;
                secondCrossing = newCrossing;
                slope = zero;
            }
            
            F getAmplitudeExiting(int x, int nextCrossing) {
                int length { distance(x, nextCrossing) };
                F normalSineAmplitude { Wave<F>::getSineAmplitudeAtIndex(length * 2, 1) };
                F dcAtAdjacentSample { wave[x] - (wave[x] - wave[nextCrossing])/F(length) };
                return jmin(one, abs((wave[rightOf(x)] - dcAtAdjacentSample) / normalSineAmplitude));
            }
            
            F getAmplitudeEntering(int x, int lastCrossing) {
                int length { distance(lastCrossing, x) };
                F normalSineAmplitude { Wave<F>::getSineAmplitudeAtIndex(length * 2, 1) };
                F dcAtAdjacentSample { wave[x] - (wave[x] - wave[lastCrossing])/F(length) };
                return jmin(one, abs((wave[leftOf(x)] - dcAtAdjacentSample) / normalSineAmplitude));
            }
            
            F getSlopeAt(int x) { return (wave[rightOf(x)] - wave[leftOf(x)])/two; }
            F getAmpFromSlopeAndLength(F s, int length) {
                F normalSineAmplitude { Wave<F>::getSineAmplitudeAtIndex(length * 2, 1) };
                return abs(s) / normalSineAmplitude;
            }
            
            void convolve(int newCrossing, int newPeak, F newCrossingSlope) {
                const int length1 { distance(crossing, secondCrossing) }, length2 { distance(secondCrossing, newCrossing) };
                if(length1 > 1 && length2 > 1 && distance(peak, trough) > 1 && distance(trough, newPeak) > 1) {
                    if (length1 + length2 > 500) {
                        print("wave is", length1 + length2, String(" samples long!"));
                    }
                    F secondSlope { getSlopeAt(secondCrossing) };
                    F amp1 { getAmpFromSlopeAndLength(crossingSlope, length1) };
                    F amp2 { getAmpFromSlopeAndLength(secondSlope, length1) };
                    F amp3 { getAmpFromSlopeAndLength(secondSlope, length2) };
                    F amp4 { getAmpFromSlopeAndLength(newCrossingSlope, length2) };
//                    F amp1 { getAmplitudeExiting(crossing, secondCrossing) };
//                    F amp2 { getAmplitudeEntering(secondCrossing, crossing) };
//                    F amp3 { getAmplitudeExiting(secondCrossing, newCrossing) };
//                    F amp4 { getAmplitudeEntering(newCrossing, secondCrossing) };
//                    const auto& table { shaper.phaseTableSet->getTable(length1 + length2) };
                    const auto blend { static_cast<F>(shaper.getBlend()) };
                    const auto& table { shaper.phaseTable };
                    const auto& aCoeffs { shaper.mCoeffs[0][int(shaper.coeffIdx[0])] };
                    const auto& bCoeffs { shaper.mCoeffs[1][int(shaper.coeffIdx[1])] };
                    const auto& dCoeffs { shaper.mCoeffs[2][int(shaper.coeffIdx[2])] };
                    convolve(crossing, secondCrossing, zero, half, amp1, amp2, blend, table, aCoeffs, bCoeffs, dCoeffs);
                    convolve(secondCrossing, newCrossing, half, one, amp3, amp4, blend, table, aCoeffs, bCoeffs, dCoeffs);
                }
            }
            
            F getDC(int i, int length, F startDC, F endDC) {
                return startDC + (endDC - startDC) * (half - std::cos((F(i) * pi)/F(length))/two); }
            
            void convolve (int start, int end, F startPhase, F endPhase, F startAmp, F endAmp, F blend, const Table<F>& table, const float (&a)[maxCoeffs], const float (&b)[maxCoeffs], const float (&d)[maxCoeffs]) {
                auto length { distance(start, end) };
                F phaseStep { (endPhase - startPhase) / F(length) };
                F dcStep { (wave[end] - wave[start]) / F(length) };
                F ampStep { (endAmp - startAmp) / F(length) };
                F phase { startPhase }, dc { wave[start] }, amp { startAmp };

                for (int i { start }, j { 0 }; i != end; ++i%=fifoLength, ++j, phase += phaseStep, amp += ampStep, dc += dcStep) {
                    F phaseAmplitude { table[phase] };
                    F multipliedWithGain { amp * shapeSample(a, b, d, phaseAmplitude) };
//                    wave[i] = blend * shapeSample(a, b, d, wave[i]) + (one - blend) * wave[i];
                    wave[i] = blend * (getDC(j, length, wave[start], wave[end]) + multipliedWithGain) + (one - blend) * wave[i];
                }
            }
            
            void switchDirections() { std::swap(compare, compareOther); std::swap(turn, turnOther); }
            
            F wave[fifoLength]{};
            Comparator compare { &WaveShaper::isTrough }, compareOther { &WaveShaper::isPeak };
            TurnFunction turn { &WaveShaper::turnTrough }, turnOther { &WaveShaper::turnPeak };
            int waveIndex { 0 }, peak { -1 }, trough { -1 }, crossing { -1 }, secondCrossing { -1 }, cross { 0 }, cross2 { 0 };
            F slope { zero }, slopeTolerance { static_cast<F>(0.0001) }, crossingSlope { zero };
            DynamicWaveShaper& shaper;
        };
        
        template<typename FloatType> void processSamples(AudioBuffer<FloatType>& buffer) {
            if constexpr (std::is_same<F, FloatType>::value)
                for (int j { 0 }; j < buffer.getNumChannels(); ++j) waveShaper[j].process(buffer.getWritePointer(j));
        }
        
        F wave[gainLength + 1]{}, pWave[waveLength + 1]{};
        Table<F> gainTable { wave, static_cast<F>(gainLength), half, half };
        Table<F> phaseTable { pWave, static_cast<F>(waveLength) };
        WaveShaper waveShaper[2] { { *this }, { *this } };
        std::atomic<int>* coeffIdx;
        float(& mCoeffs)[maxCoeffs][maxCoeffs][maxCoeffs];
    };
}
