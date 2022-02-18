#pragma once
#include "deviant.h"

namespace sadistic {
    
    class DualScope {
    public:
        static constexpr int fifoSize { FIFOSIZE }, scopeSize { SCOPESIZE };
        DualScope(LongFifo<float>* sf, ScopeBuffer(& wf)[2]) : oscilloscope{ { *this, wf[0], sf[0] }, { *this, wf[1], sf[1] } } {}
        double getSampleRate() const { return lastSampleRate; }
        
        class Oscilloscope : private Timer {
        public:
            
            Oscilloscope(DualScope& dS, ScopeBuffer& sB, LongFifo<float>& lF) : dual(dS), rendererBuffer(sB), scopeFifo(lF) {
                startTimerHz (30);
            }
            
            void prepare (const ProcessSpec&) { reset(); }
            void reset() {
                memset(scopeData, 0.f, sizeof(scopeData));
                rendererBuffer.flush();
            }
            
            void timerCallback() override { if (scopeFifo.isLoaded()) analyze(dual.getSampleRate()); }
            
            void analyze(const double sampleRate) {
                auto aBuf { scopeFifo.getBlock() };
                auto currentWave { rendererBuffer.getBlankFrame() };
                if (currentWave) {
                    float waveLengthInSamples { ((float)sampleRate / 666.f) };
                    float numberOfCycles { 8.f };
                    const int waveDisplayLength { jlimit(10, 6000, static_cast<int>(waveLengthInSamples * numberOfCycles)) };
                    float speedRatio { (float)waveDisplayLength / 258.f };
                    const int indexOfMax { static_cast<int>(std::distance(aBuf,std::max_element(aBuf, aBuf + fifoSize))) };
                    const int indexOfMin { static_cast<int>(std::distance(aBuf,std::min_element(aBuf, aBuf + fifoSize))) };
                    if(abs(aBuf[indexOfMax]) > abs(aBuf[indexOfMin])) extremity = indexOfMax;
                    else extremity = indexOfMin;
                    const float mag { abs(aBuf[extremity]) }, magDB { Decibels::gainToDecibels(mag) }, mult { 1.f - -magDB/100.f };
                    
                    const int startIndex { fifoSize - ((waveDisplayLength * 3) / 2 + 1) };
                    interpolator.process(jlimit(20.f / 256.f, (float)(fifoSize - ((size_t)waveDisplayLength + 1) / 256.f), speedRatio), aBuf + startIndex, scopeData, scopeSize+2);

                    for (int i { 0 }; i < scopeSize; ++i) currentWave[i] = scopeData[i+2] * 2.f * mult;
                    rendererBuffer.setReadyToRender(currentWave);
                }
                scopeFifo.finishedReading();
            }
            
        private:
            float scopeData[scopeSize+2]{};
            DualScope& dual;
            ScopeBuffer& rendererBuffer;
            LongFifo<float>& scopeFifo;
            int extremity { 0 };
            LagrangeInterpolator interpolator;
            
            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Oscilloscope)
        };
        
        void prepare(const ProcessSpec& spec) { oscilloscope[wetSignal].prepare(spec); oscilloscope[drySignal].prepare(spec); if (spec.sampleRate != 0.0) lastSampleRate = spec.sampleRate; }
        void reset() { resetAll(oscilloscope[wetSignal], oscilloscope[drySignal]); }
        
    private:
        Oscilloscope oscilloscope[numSignals];
        double lastSampleRate { 44100.0 };
    };    
} // namespace sadistic
