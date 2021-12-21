#pragma once
#include "Renderer.h"
#include "deviant.h"

namespace sadistic {
    
    
    
    struct DualScope {
        static constexpr int fifoSize { FIFOSIZE }, scopeSize { SCOPESIZE };
        DualScope(LongFifo<float>* sf, ScopeBuffer(& wf)[2]) : oscilloscope{ { *this, wf[0], sf[0] }, { *this, wf[1], sf[1] } } {}
        const double getSampleRate() { return lastSampleRate; }
        
        struct Oscilloscope : private Timer {
            
            Oscilloscope(DualScope& dA, ScopeBuffer& wf, LongFifo<float>& sf) : dual(dA), scopeBuffer(wf), scopeFifo(sf) {
                float* mockChannel1[1] { &scopeData1[2] }, * mockChannel2[1] { scopeData2 };
                scopeBuffer1 = { mockChannel1, 1, scopeSize }, scopeBuffer2 = { mockChannel2, 1, scopeSize };
                startTimerHz (30);
            }
            
            void prepare (ProcessSpec) { reset(); }
            void reset() {
                memset(scopeData1, 0.f, sizeof(scopeData1));
                memset(scopeData2, 0.f, sizeof(scopeData2));
                scopeBuffer.flush();
            }
            
            void timerCallback() override { if (scopeFifo.isLoaded()) analyze(dual.getSampleRate()); }
            
            void analyze(const double sampleRate) {
                auto aBuf { scopeFifo.getBlock() };
                if(aBuf[0] != 0.f && aBuf[fifoSize-1] !=0.f) {
                    float* mockChannel[1] { aBuf };
                    auto buffer { AudioBuffer<float>(mockChannel, 1, fifoSize) };
                    float waveLengthInSamples { ((float)sampleRate / lastFreq) };
                    float numberOfCycles { 8.f };
                    const int waveDisplayLength { jlimit(10, 6000, static_cast<int>(waveLengthInSamples * numberOfCycles)) };
                    float speedRatio { (float)waveDisplayLength / 258.f };
                    
                    const int indexOfMax { static_cast<int>(std::distance(aBuf,std::max_element(aBuf, aBuf + fifoSize))) };
                    const int indexOfMin { static_cast<int>(std::distance(aBuf,std::min_element(aBuf, aBuf + fifoSize))) };
                    
                    if(abs(aBuf[indexOfMax]) > abs(aBuf[indexOfMin])) extremity = indexOfMax;
                    else extremity = indexOfMin;
                    if(abs(aBuf[extremity]) > 0.4f) keeps = 0.9f;
                    
                    const int startIndex = fifoSize - ((waveDisplayLength * 3) / 2 + 1);
                    interpolator.process(jlimit(20.f / 256.f, (float)(fifoSize - ((size_t)waveDisplayLength + 1) / 256.f), speedRatio), aBuf + startIndex, scopeData1, scopeSize+2);
                    interpolator.reset();
                    
                    for (int i = 0; i < scopeSize; ++i)
                        scopeData2[i] = scopeData1[i+2];// * 0.8f + scopeData2[i] * keeps;
                    
                    //                    auto rmsOfScopeBuffer2 { scopeBuffer2.getRMSLevel(0, 0, scopeBuffer2.getNumSamples()) };
                    
                    auto currentWave { scopeBuffer.getBlankFrame() };
                    if (currentWave) {
                        for (int i = 0; i < scopeSize; ++i)
                             currentWave[i] = scopeData2[i] * 2.f;
                        
                        //                        scopeBuffer.set("rms", rmsOfScopeBuffer2);
                        scopeBuffer.setReadyToRender(currentWave);
                    }
                    keeps = jmax(0.f, keeps - 0.1f);
                }
                scopeFifo.finishedReading();
            }
            
            float scopeData1[scopeSize+2]{}, scopeData2[scopeSize]{};
            AudioBuffer<float> scopeBuffer1, scopeBuffer2;
            DualScope& dual;
            ScopeBuffer& scopeBuffer;
            LongFifo<float>& scopeFifo;
            int extremity { 0 };
            float keeps { 0.f }, lastFreq { 666.f };
            LagrangeInterpolator interpolator;
            
            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Oscilloscope)
        };
        
        void prepare(ProcessSpec spec) { oscilloscope[wetSignal].prepare(spec); oscilloscope[drySignal].prepare(spec); if (spec.sampleRate != 0.0) lastSampleRate = spec.sampleRate; }
        void reset() { resetAll(oscilloscope[wetSignal], oscilloscope[drySignal]); }
        Oscilloscope oscilloscope[numSignals];
        double lastSampleRate{ 44100.0 };
    };
}
