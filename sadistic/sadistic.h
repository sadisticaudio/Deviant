
/*
  sadistic.h


                    .=>>>>>>>=       >>>
                  .=>>>>>>>>>>>      (>>>
                (>>>>>>>>>>>>>>>     )>>>>
              (>>>>>>>>>>>>>>>>>     >>>>>>
            (>>>>>>>>>>>>>>>>>>      >>>>>>>
          )>>>>>>>=   >>>>>>>>>      >>>>>>>
         )>>>>>>       >>>>>>>      )>>>>>>>
       )>>>>>>         \>>>=        )>>>>>>>>
      (>>>>>=                       )>>>  >>>
     >>>>>>                         >>>>  )>>>
    >>>>>>                          >>>>   )>>
   >>>>>>                           >>>>   )>>>
  )>>>>>                            >>>>    >>>
 />>>>>>                           )>>>>    )>>>
 >>>>>>                            )>>>      >>>
(>>>>>>                            )>>>      )>>>
)>>>>>>                            )>>>       >>>
)>>>>>                             >>>>       >>>>
)>>>>>                             >>>>       )>>>
)>>>>>                             >>>>        >>>>
 >>>>>                            )>>>>        >>>>      .=>>>>=
 )>>>>>                           )>>>>        )>>>> .=>>>>>>>>>
  \>>>>                           )>>>         )>>>>>>>>>>>=<>>>
    >>>>.                         )>>>          >>>>>>><
     \>>>>=                       >>>>        .>>>>>>
        \>>>>>>>>===-...          >>>>      .>>>>>>>>
             \\\===>>>>>>>>>>=    >>>>  .=>>>>>>)>>>>
                           )>>>> )>>>>>>>>>>>=   )>>>
                           (>>>> )>>>>>>>=        >>>>
                        .(>>>=   )>>>>>           )>>>
                   .-=>>>><      )>>>>             >>>
            .-=>>>>>>=           )>>>    AUDIO     )>>
      (>>>>>>>>=                 )>>>
      \\\``        SADISTIC

 
  Created by Frye Wilkins on 3/14/20.
  Copyright © 2020 Sadistic. All rights reserved.

*/

#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_product_unlocking/juce_product_unlocking.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_opengl/juce_opengl.h>
#include <SadisticData.h>

using namespace juce;
using namespace dsp;

namespace sadistic {
    
    using namespace juce;
    using namespace dsp;
    
    // some heavily-used constant variable templates
    template<typename F> F zero { static_cast<F>(0) };
    template<typename F> F half { static_cast<F>(0.5) };
    template<typename F> F one { static_cast<F>(1) };
    template<typename F> F two { static_cast<F>(2) };
    template<typename F> F halfPi { MathConstants<F>::halfPi };
    template<typename F> F pi { MathConstants<F>::pi };
    template<typename F> F twoPi { MathConstants<F>::twoPi };
    
    using APVTS = AudioProcessorValueTreeState;
    using ParamList = std::vector<std::reference_wrapper<RangedAudioParameter>>;
    using FloatParamList = std::vector<std::reference_wrapper<AudioParameterFloat>>;
    
    enum { ORDERS = 3, NUMOCTAVES = 6, FFTORDER = 10, FIFOORDER = FFTORDER + 3, FILTERORDER = 128, BUFFERLENGTH = 256, SCOPESIZE = 256, MAXFRAMES = 256, FFTSIZE = 1 << FFTORDER, FIFOSIZE = 1 << FIFOORDER };
    
    struct ParamInfo { enum ParamType { dB = 0, Hz, Pct, Int, ms, NA }; ParamType type { NA }; float min { 0 }; float max { 1.f }; float defaultValue { 1.f }; const char* name; const char* id; };
    
    template <typename Func, typename... Items>
    constexpr void forEach (Func&& func, Items&&... items)
    noexcept (noexcept (std::initializer_list<int> { (func (std::forward<Items> (items)), 0)... }))
    {
        (void) std::initializer_list<int> { ((void) func (std::forward<Items> (items)), 0)... };
    }
    template <typename... Procs> void prepareAll (const dsp::ProcessSpec& spec, Procs&... procs) { forEach ([&] (auto& proc) { proc.prepare (spec); }, procs...); }
    template <typename... Procs> void resetAll (Procs&... procs) { forEach ([] (auto& proc) { proc.reset(); }, procs...); }
    
    template<typename T> String getPluginCodeString(const T x) {
        const char* reversed { (const char*) (&x) };
        const char codeCharacters[4] { reversed[3], reversed[2], reversed[1], reversed[0] };
        return { codeCharacters, 4 };
    }
    
    template <size_t N>
    struct align_allocator {
        template <typename T>
        struct type {
            using value_type = T;
            static constexpr std::align_val_t alignment{N};
            T* allocate(size_t n) {
                static_cast<T*>(operator new(n*sizeof(T), alignment));
            }
            void deallocate(T* p, size_t) {
                operator delete(p, alignment);
            }
        };
    };
    
    inline String getSadisticFolder() {
        std::vector<String> folders {
            { { File::getSpecialLocation(File::userApplicationDataDirectory).getFullPathName() + "/Application Support/Sadistic Audio" } }
        };
        for(size_t i = 0; i < folders.size(); ++i) {
            auto resultFile = File(folders[i] + "/test.xml");
            auto result = resultFile.create();
            if(result.wasOk())
                return folders[i];
        }
        return {};
    }
    
    template <typename... Components> void addAllAndMakeVisible (Component& target, Components&... children) {
        forEach ([&] (Component& child) { target.addAndMakeVisible (child); }, children...); }
    template <typename... Components> void addAllChildren (Component& target, Components&... children) {
        forEach ([&] (Component& child) { target.addChildComponent (child); }, children...); }
    
    static inline String getSadisticFolderPath() {
        File homePath { File::getSpecialLocation(File::userHomeDirectory) };
        std::vector<String> folders {
            { { File::getSpecialLocation(File::userApplicationDataDirectory).getFullPathName() + "/Application Support/Sadistic Audio" } }
        };
        for(size_t i = 0; i < folders.size(); ++i) {
            auto resultFile = File(folders[i] + "/test.xml");
            auto result = resultFile.create();
            if(result.wasOk())
                return folders[i];
        }
        return {};
    }
    static inline String getSadisticFilePath (String name) { return { getSadisticFolderPath() + "/" + name }; }
    static inline File getSadisticFile (String name) { return { getSadisticFolderPath() + "/" + name }; }
    static inline File getSadisticFile (std::vector<String> names) {
        String finalPath = getSadisticFolderPath();
        for (size_t folderName { 0 }; folderName < names.size() - 1; ++folderName) finalPath += String("/" + names[folderName]);
        return { finalPath + "/" + names[names.size() - 1] }; }
    
    template<typename F>
    struct Wave {
        enum Type { none, sine, cosine, square, sineSaw, sineSquare, sineTriangle, saw, dc, custom, numWaves };
        static constexpr const char* waveID[] {
            "none",
            "sine",
            "cosine",
            "square",
            "sineSaw",
            "sineSquare",
            "sineTriangle",
            "saw",
            "dc",
            "custom",
            "numWaves"
        };
        static inline String getWaveID(int waveIndex) { return { waveID[waveIndex].data(), waveID[waveIndex].size() }; }
        
        static constexpr F zero { static_cast<F>(0) }, one { static_cast<F>(1) }, two { static_cast<F>(2) }, half { one / two }, pi { MathConstants<F>::pi }, halfPi { MathConstants<F>::halfPi }, twoPi { MathConstants<F>::twoPi };
        
        template <typename FloatType> static FloatType getRMS(const FloatType* data, int numSamples) {
            FloatType sum { FloatType(0) };
            for (int i { 0 }; i < numSamples; ++i) sum += data[i] * data[i];
            return static_cast<FloatType> (std::sqrt (sum / numSamples));
        }
        
        template <typename FloatType> static FloatType getMagnitude(const FloatType* data, int numSamples) {
            auto minAndMax { FloatVectorOperations::findMinAndMax(data, numSamples) };
            return jmax(abs(minAndMax.getStart()), abs(minAndMax.getEnd()));
        }

        static void normalize (F* data, int length, bool oneExtraSample = false) {
            auto minAndMax { FloatVectorOperations::findMinAndMax(data, length) };
            auto peak { jmax(abs(minAndMax.getStart()), abs(minAndMax.getEnd())) };
            peak = peak > F(0.01) ? peak : one;
            for (int i { 0 }; i < length; ++i) data[i] /= peak;
            if (oneExtraSample) data[length] = zero;
            data[0] = zero;
        }
        
        static constexpr inline F getSineAmplitudeAtIndex(const int waveLength, const int index) {
            return sin(F(index) * twoPi /F(waveLength)); }

        static inline void fillTable(F* table, int length, Type type, bool shouldBeNormalized = true, bool oneExtraSample = false, F numCycles = one, F startPhase = zero) {
            std::fill(table, table + (length + (oneExtraSample ? 1 : 0)), zero);
            auto phase = startPhase, phaseStep { twoPi/length };
            switch (type) {
                case none: {} break;
                case cosine: {
                    for(int i { 0 }; i < length; ++i, phase+=phaseStep)
                        table[i] = cos(phase);
                } break;
                case saw: {
                    F start { one }, step { -two/F(length) };
                    for(int i { 0 }; i < length; ++i, start += step) table[i] = start;
                } break;
                case sine: {
                    for(int i { 0 }; i < length; ++i, phase+=phaseStep)
                        table[i] = sin(phase);
                } break;
                case sineSaw: {
                    for(int harmonic { 1 }; harmonic < 8; ++harmonic)
                        for(int i { 0 }; i < static_cast<int>(length * numCycles); ++i, phase+=phaseStep)
                            table[i] += (one/F(harmonic)) * sin(phase * F(harmonic));
                } break;
                case sineSquare: {
                    for(F harmonic = one; harmonic < F(16); harmonic += two)
                        for(int i { 0 }; i < static_cast<int>(length * numCycles); ++i, phase+=phaseStep)
                            table[i] += (one/harmonic) * sin(phase * harmonic);
                } break;
                case sineTriangle: {
                    for(int iteration { 1 }, harmonic { 1 }; harmonic < 5; iteration *= -1, harmonic += 2)
                        for(int i { 0 }; i < static_cast<int>(length * numCycles); ++i, phase+=phaseStep)
                            table[i] += F(iteration) * (one/F(harmonic * harmonic)) * sin(phase * harmonic);
                } break;
                case square: {
                    for(int i { 0 }; i < length/2; ++i) table[i] = one;
                    for(int i { length/2 }; i < length; ++i) table[i] = -one;
                } break;
                case dc: { for(int i { 0 }; i < length; ++i) table[i] = zero; } break;
                case custom: {
//                    readTableFromData(Data::MATRIXYC64_wav, table);
                } break;
                case numWaves: {} break;
            }
            if (shouldBeNormalized) normalize(table, length, oneExtraSample);
        }
        
        enum StaticType { bypassed, atan, numStaticTables };
        static constexpr inline void fillStaticTable(F* table, int length, StaticType type, bool normalize = true, bool oneExtraSample = false, F startAmp = -one, F endAmp = one) {
            auto waveLength { static_cast<F>(length) }, amp = startAmp, ampStep { (endAmp - startAmp)/waveLength };
            switch (type) {
                case bypassed: {
                    for(int i { 0 }; i < length; ++i, amp+=ampStep)
                        table[i] = amp;
                    if (oneExtraSample) table[length] = one;
                } break;
                case atan: {
                    for(int i { 0 }; i < length; ++i, amp+=ampStep)
                        table[i] = tanh(F(5) * amp) * F(0.05);
                    if (oneExtraSample) table[length] = one;
                } break;
                case numStaticTables: {
                } break;
            }
            
            if(normalize) {
                auto xMax { std::max_element(table, table + length) };
                auto xMin { std::min_element(table, table + length) };
                auto norm { jmax(abs(*xMax),abs(*xMin)) };
                for(int i { 0 }; i < length; ++i) table[i] = table[i] /  norm;
            }
        }
    };

    template <typename Key, typename Value>
    struct unique_ptr_map {
        struct element {
            Value& operator*(){ return *ptr; }
            Value* operator->() { return &ptr.get(); }
            Key key; int id; std::unique_ptr<Value> ptr; };
        unique_ptr_map(std::vector<Key> keys) {
            std::vector<element> tempData;
            for (size_t i = 0; i < keys.size(); ++i)
                tempData.push_back({ keys[i], static_cast<int>(i), nullptr });
            data = std::move(tempData); }
        std::unique_ptr<Value>& operator[](Key key) {
            auto itr { std::find_if(data.begin(), data.end(), [&key](auto &v) { return v.key == key; }) };
            return itr->ptr; }
        int size() { return data.size(); }
        std::vector<element> data;
    };
    
    static inline const Font getSadisticFont() {
        static Font shadowFont (Font (Typeface::createSystemTypefaceFor (Data::shadowsIntoLight_ttf, Data::shadowsIntoLight_ttfSize)));
        return shadowFont;
    }

    template <typename T> void print(String label, T value) { std::cout << label << ": " << value << "\n"; }
    template <typename... T> void print(String label, T... values) { std::cout << label << ": ";
        forEach ([&](auto value){ std::cout << value << ", "; }, values...); std::cout << "\n"; }

    //lock free fifo for pushing buffers and retrieving long chunks of the most recent audio in the editor
    template<typename FloatType>
    class LongFifo {
    public:
        static constexpr int fifoSize { FIFOSIZE };
        void reset() {
            bufferReadyForEditor = false;
            memset(processorFifo, 0.f, sizeof(processorFifo));
            memset(editorBuffer, 0.f, sizeof(editorBuffer));
        }
        void loadChannel(const FloatType* fifoData, int startIndex) {
            std::copy(fifoData + startIndex, fifoData + fifoSize, editorBuffer);
            if (startIndex != 0) std::copy(fifoData, fifoData + startIndex, editorBuffer + fifoSize - startIndex);
        }
        template <typename F> void pushChannel (const AudioBuffer<F>& newBlock) {
            for (int i { 0 }; i < newBlock.getNumSamples(); ++i) {
                const auto sample = static_cast<FloatType>(jlimit(F(-1), F(1), newBlock.getSample(0, i)));
                pushNextSampleIntoFifo(sample);
            }
        }
        FloatType* getBlock() { if(bufferReadyForEditor) return editorBuffer; else return nullptr; }
        void finishedReading() { bufferReadyForEditor = false; }
        void pushNextSampleIntoFifo (FloatType sample) noexcept {
            if (fifoCounter > 1023) {
                fifoCounter = 0;
                if (!bufferReadyForEditor) {
                    loadChannel(processorFifo, fifoIndex);
                    bufferReadyForEditor = true;
                }
                fifoIndex = (fifoIndex + 1024) % fifoSize;
            }
            processorFifo[fifoCounter + fifoIndex] = sample;
            ++fifoCounter;
        }
        bool isLoaded() { return bufferReadyForEditor; }
        
    private:
        FloatType editorBuffer[fifoSize] {};
        FloatType processorFifo[fifoSize] {};
        int fifoCounter { 0 }, fifoIndex { 0 };
        std::atomic<bool> bufferReadyForEditor { false };
    };
    
    class ScopeBuffer {
        static constexpr int scopeSize { SCOPESIZE };
    public:
        class FrameWithState {
        public:
            void reset() { std::fill(frame, frame + sizeof(frame)/sizeof(float), 0.f); isReady = false; wasFirstIn = false; }
            const float* getReadPointer() { return frame; }
            float* getWritePointer() { reset(); return frame; }
            bool isSameAddress(const float* frameToCheck) { if(frameToCheck == frame) return true; else return false; }
            std::atomic<bool> isReady { false }, wasFirstIn { false }, wasLastOut { false };
        private:
            float frame [scopeSize + 1];
        };
        
        void reset() { frameA.reset(); frameB.reset(); }
        float* getBlankFrame() {
            if (!frameA.isReady && !frameB.isReady) return frameA.getWritePointer();
            else if (frameA.isReady && frameB.isReady) {
                if (frameA.wasFirstIn) { frameA.isReady = false; frameA.wasFirstIn = false; return frameA.getWritePointer(); }
                else { frameB.isReady = false; frameB.wasFirstIn = false; return frameB.getWritePointer(); } }
            else if (frameA.isReady) { frameA.wasFirstIn = true; return frameB.getWritePointer(); }
            else  { return nullptr; } }
        void setReadyToRender(float* frame) { if(frameA.isSameAddress(frame)) frameA.isReady = true; else if(frameB.isSameAddress(frame)) frameB.isReady = true; }
        const float* getFrameToRead() {
            if (frameA.isReady && frameB.isReady) {
                if(frameA.wasFirstIn) { frameA.wasFirstIn = false; return frameA.getReadPointer(); }
                else { frameB.wasFirstIn = false; return frameB.getReadPointer(); } }
            else if (frameA.isReady) { frameA.isReady = false; return frameA.getReadPointer(); }
            else if (frameB.isReady) { frameB.isReady = false; return frameB.getReadPointer(); }
            else return nullptr; }
        void finishedRendering(const float* frame) {
            if(frameA.isSameAddress(frame)) { frameA.wasLastOut = true; frameB.wasLastOut = false; }
            else if(frameB.isSameAddress(frame)) { frameB.wasLastOut = true; frameA.wasLastOut = false; } }
        void flush() { reset(); frameA.isReady = true; }
    private:
        FrameWithState frameA, frameB;
    };
    
    template<typename F> F sadSymmetricSkew(F normVal, F skew /* must be > -1 and < 1 */) {
        const F val { normVal * F(2.0) - F(1.0) };
        const F numerator { val - val * skew };
        const F denominator { skew - abs(val) * F(2.0) * skew + F(1.0) };
        return (numerator / denominator) / F(2.0) + F(0.5);
    };
    
    template<typename FloatType>
    struct Zoomer {
        Zoomer() = default;
        NormalisableRange<float> toFloat() const { return { static_cast<float>(range.start), static_cast<float>(range.end), 0.f, static_cast<float>(range.skew) }; }
        
        static constexpr FloatType makeSkew(FloatType low, FloatType high) {
            return static_cast<FloatType>(jmap(fullRange.convertTo0to1(high) - fullRange.convertTo0to1(low), FloatType(0.05), FloatType(1),  FloatType(1), fullRange.skew)); }
        
        NormalisableRange<FloatType> setAndReturn (FloatType newStart, FloatType newEnd) {
            if (newStart != range.start) {
                range.start = static_cast<FloatType>(jlimit(20.0, 17595.0, (double) newStart));
                range.end = static_cast<FloatType> (jlimit(static_cast<double>(fullRange.convertFrom0to1(fullRange.convertTo0to1(range.start))) + 0.05, 20000.0, (double) newEnd));
            }
            else if (newEnd != range.end) {
                range.end = static_cast<FloatType> (jlimit(31.1692, 20000.0, (double) newEnd));
                range.start = static_cast<FloatType> (jlimit(20.0, static_cast<double>(fullRange.convertFrom0to1(fullRange.convertTo0to1(range.end)) - 0.05), (double) newStart));
            }
            else return range;
            auto min = range.start;
            auto max = range.end;
            auto skew = makeSkew(min, max);
            return range = NormalisableRange<FloatType> ( min, max, 0.0, skew );
        }
        NormalisableRange<FloatType> range { fullRange };
        static inline const NormalisableRange<FloatType> fullRange { FloatType(20), FloatType(20000), FloatType(0), FloatType(0.4) };
    };
    
    template<typename FloatType, int initialDelay = 0>
    struct SadDelay {
        
        SadDelay() : latency(initialDelay) {}
        
        void setDelay(int delay) {
            for (int j { 0 }; j < dBuffer.getNumChannels(); ++j) {
                auto chPtr { dBuffer.getWritePointer(j) };
                std::rotate(chPtr, chPtr + dIndex, chPtr + latency);
            }
            dIndex = 0;
            latency = delay;
        }
        int getDelay() { return latency; }
        int getLatency() { return latency; }
        void prepare(const ProcessSpec& spec) {
            dBuffer.setSize((int) spec.numChannels, latency);
            reset();
        }
        
        void reset() {
            dBuffer.clear();
            dIndex = 0;
        }
        
        void process(AudioBuffer<FloatType>& buffer) {
            if (latency > 0 && dBuffer.getNumSamples() > 0) {
                int channels { jmin(buffer.getNumChannels(), dBuffer.getNumChannels()) }, samples { buffer.getNumSamples() }, dSamples { dBuffer.getNumSamples() };
                jassert (channels == dBuffer.getNumChannels());
                
                auto totalDelayAmount { jmin(latency, samples) };
                auto initialSwapAmount { jmin(dSamples - dIndex, totalDelayAmount) };
                
                for (int j { 0 }; j < channels; ++j) {
                    if (samples > latency) std::rotate(buffer.getWritePointer(j), buffer.getWritePointer(j) + samples - latency, buffer.getWritePointer(j) + samples);
                    std::swap_ranges(buffer.getWritePointer(j), buffer.getWritePointer(j) + initialSwapAmount, dBuffer.getWritePointer(j) + dIndex);
                    if (initialSwapAmount < totalDelayAmount)
                        std::swap_ranges(buffer.getWritePointer(j) + initialSwapAmount, buffer.getWritePointer(j) + totalDelayAmount, dBuffer.getWritePointer(j));
                }
                dIndex = (dIndex + totalDelayAmount) % dSamples;
            }
        }
        
        void process(AudioBuffer<FloatType>& buffer, AudioBuffer<FloatType>& destBuffer) {
            int channels { jmin(buffer.getNumChannels(), destBuffer.getNumChannels()) }, samples { jmin(destBuffer.getNumSamples(), buffer.getNumSamples()) };
            for (int j { 0 }; j < channels; ++j) {
                destBuffer.copyFrom(j, 0, buffer, j, 0, samples);
            }
            process(destBuffer);
        }
        
        AudioBuffer<FloatType> dBuffer;
        int latency { 0 }, dIndex { 0 };
    };
    
    template<typename F, int bufferLength, int filterOrder, int latency> struct FixedSizePreProcessFilterCompensator {
        void setFilterDelay(int fD) { filterDelay.setDelay(fD); }
        void setProcessDelay(int pD) { processDelay.setDelay(pD); }
        void prepare(const ProcessSpec& spec) {
            mBuffer.setSize((int) spec.numChannels, bufferLength);
            filterDelay.prepare(spec);
            processDelay.prepare(spec);
        }
        void reset() {
            filterDelay.reset();
            processDelay.reset();
        }
        void copyBuffer(AudioBuffer<F>& buffer) {
            filterDelay.process(buffer, mBuffer);
        }
        void subtractBuffer(AudioBuffer<F>& buffer) {
            AudioBlock<F> block { buffer }, mBlock { mBuffer };
            mBlock -= block;
        }
        void addBuffer(AudioBuffer<F>& buffer) {
            if (latency > 0) processDelay.process(mBuffer);
            AudioBlock<F> block { buffer }, mBlock { mBuffer };
            block += mBlock;
        }
        AudioBuffer<F> mBuffer;
        SadDelay<F, filterOrder/2> filterDelay;
        SadDelay<F, latency> processDelay;
    };
    
    template<typename F, int bufferLength, int filterOrder, int latency> struct FixedSizePostProcessFilterCompensator {
        void setFilter(typename FIR::Coefficients<F>::Ptr s) {
            *filter.state = *s;
            auto* coeffs { s->getRawCoefficients() };
            auto* mCoeffs { filter.state->getRawCoefficients() };
            jassert(s->getFilterOrder() == filterOrder);
            for (size_t i { 0 }; i <= s->getFilterOrder(); ++i)
                mCoeffs[i] = coeffs[i];// * static_cast<F>(pow(-1, i));
        }
        void setProcessDelay(int pD) { processDelay.setDelay(pD); }
        void prepare(const ProcessSpec& spec) {
            mBuffer.setSize((int) spec.numChannels, bufferLength);
            processDelay.prepare(spec);
            filter.prepare(spec);
        }
        void reset() {
            filter.reset();
            processDelay.reset();
        }
        void copyBuffer(AudioBuffer<F>& buffer) {
            processDelay.process(buffer, mBuffer);
            AudioBlock<F> mBlock { mBuffer };
            filter.process(ProcessContextReplacing<F>(mBlock));
        }
        void addBuffer(AudioBuffer<F>& buffer) {
            AudioBlock<F> block { buffer }, mBlock { mBuffer };
            block += mBlock;
        }
        AudioBuffer<F> mBuffer;
        ProcessorDuplicator<FIR::Filter<F>, FIR::Coefficients<F>> filter;
        SadDelay<F, latency> processDelay;
    };
    
    static std::unique_ptr<Drawable> makeIcon (const char* iconSvgData)
    {
        if (auto svg = XmlDocument::parse (iconSvgData))
            return Drawable::createFromSVG (*svg);
        
        return {};
    }
    struct SadSVG : public Component {
        SadSVG(const char* data, Colour c, float a = 0.f) : icon (makeIcon(data)), angle(a) {
            addAndMakeVisible(*icon);
            setInterceptsMouseClicks(false, false);
            icon->replaceColour(Colours::black, c);
        }
        void paint(Graphics&) override {
            auto transform = juce::AffineTransform:: rotation (angle, getLocalBounds().toFloat().getCentreX(), getLocalBounds().toFloat().getCentreY());
            icon->setTransformToFit (getLocalBounds().toFloat().reduced(0.9f), RectanglePlacement::centred);
            icon->setTransform(icon->getTransform().followedBy(transform));// rotated(angle, getLocalBounds().getCentreX(),
        }
        void resized() override { icon->setBounds(getLocalBounds()); }
        void setRotation(float r) { angle = r; }
        std::unique_ptr<Drawable> icon;
        float angle;
    };
}
