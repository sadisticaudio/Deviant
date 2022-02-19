
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
  By using JUCE, you agree to the terms of both the Deviant End-User License
  Agreement and Sadistic Audio Privacy Policy (both effective as of the 19th February 2022).
 
  Deviant End User License Agreement: https://github.com/sadisticaudio/Deviant/blob/master/LICENSE.md
  Privacy Policy can be found here: https://shop.sadisticaudio.com/index.php?route=information/information&information_id=5

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
    using APVTS = AudioProcessorValueTreeState;
    using ParamList = std::vector<std::reference_wrapper<RangedAudioParameter>>;
    using FloatParamList = std::vector<std::reference_wrapper<AudioParameterFloat>>;
    
    enum { FIFOORDER = 13, BUFFERLENGTH = 256, SCOPESIZE = 256, FIFOSIZE = 1 << FIFOORDER };
    enum { dB = 0, Hz, Pct, Int };
    
    struct ParamInfo { enum ParamType { dB = 0, Hz, Pct, Int, NA }; float min { 0 }; float max { 1.f }; float defaultValue { 1.f }; ParamType type { dB }; const char* name; const char* id; };
    
    template <typename Func, typename... Items>
    constexpr void forEach (Func&& func, Items&&... items)
    noexcept (noexcept (std::initializer_list<int> { (func (std::forward<Items> (items)), 0)... })) {
        (void) std::initializer_list<int> { ((void) func (std::forward<Items> (items)), 0)... };
    }

    template <typename... Procs>
    void prepareAll (const dsp::ProcessSpec& spec, Procs&... procs) {
        forEach ([&] (auto& proc) { proc.prepare (spec); }, procs...);
    }

    template <typename... Procs>
    void resetAll (Procs&... procs) {
        forEach ([] (auto& proc) { proc.reset(); }, procs...);
    }

    template <typename... Components> void addAllAndMakeVisible (Component& target, Components&... children) {
        forEach ([&] (Component& child) { target.addAndMakeVisible (child); }, children...); }
    template <typename... Components> void addAllChildren (Component& target, Components&... children) {
        forEach ([&] (Component& child) { target.addChildComponent (child); }, children...); }
    
    template<typename T> String getPluginCodeString(const T x) {
        const char* reversed { (const char*) (&x) };
        const char codeCharacters[4] { reversed[3], reversed[2], reversed[1], reversed[0] };
        return { codeCharacters, 4 };
    }

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

    //TODO: though my sanitizers haven't detected anything yet, possibly refactor this class and LongFifo (below) to inheret from juce::AbstractFifo to avoid race conditions.. but, test to see if there is overhead from the virtual function calls
    class ScopeBuffer {
        static constexpr int scopeSize { SCOPESIZE };
    public:
        class FrameWithState {
        public:
            void reset() { memset(frame, 0.f, sizeof(frame)); isReady = false; wasFirstIn = false; }
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
        void copyMostRecentSamples(const FloatType* fifoData, int startIndex) {
            std::copy(fifoData + startIndex, fifoData + fifoSize, editorBuffer);
            if (startIndex != 0) std::copy(fifoData, fifoData + startIndex, editorBuffer + fifoSize - startIndex);
        }
        template <typename F> void pushChannel (const AudioBuffer<F>& newBuffer) {
            for (int i { 0 }; i < newBuffer.getNumSamples(); ++i)
                processorFifo[fifoIndex + i] = static_cast<FloatType>(jlimit(F(-1), F(1), newBuffer.getSample(0, i)));
            
            fifoIndex = (fifoIndex + newBuffer.getNumSamples()) % fifoSize;
            
            if (!bufferReadyForEditor) {
                copyMostRecentSamples(processorFifo, fifoIndex);
                bufferReadyForEditor = true;
            }
        }
        FloatType* getBlock() { if(bufferReadyForEditor) return editorBuffer; else return nullptr; }
        void finishedReading() { bufferReadyForEditor = false; }
        bool isLoaded() { return bufferReadyForEditor; }
        
    private:
        FloatType editorBuffer[fifoSize] {};
        FloatType processorFifo[fifoSize] {};
        int fifoIndex { 0 };
        std::atomic<bool> bufferReadyForEditor { false };
    };
    
    template<typename F> F sadSymmetricSkew(F normVal, F skew /* must be > -1 and < 1 */) {
        const F val { normVal * F(2.0) - F(1.0) };
        const F numerator { val - val * skew };
        const F denominator { skew - abs(val) * F(2.0) * skew + F(1.0) };
        return (numerator / denominator) / F(2.0) + F(0.5);
    };
    
    //I am afraid to use this hand-rolled delay in place of the JUCE version but i seem to get a huge performance penalty with the JUCE version for some reason, even in Release mode, might be my config...
    template<typename FloatType>
    struct DelayBuffer {
        DelayBuffer(int nC = 1, int l = 0) : latency(l), numChannels(nC) {}
        
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
            maxBlockSize = int(spec.maximumBlockSize);
            reset();
        }
        
        void reset() {
            if(numChannels && !dBuffer.hasBeenCleared()) dBuffer.clear();
            dIndex = 0;
        }
        
        void process(AudioBuffer<FloatType>& buffer) {
            if (latency > 0 && dBuffer.getNumSamples() > 0) {
                int channels { jmin(buffer.getNumChannels(), dBuffer.getNumChannels()) }, samples { jmin( maxBlockSize, buffer.getNumSamples()) }, dSamples { dBuffer.getNumSamples() };
                jassert (channels <= dBuffer.getNumChannels());
                
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
        int latency { 0 }, dIndex { 0 }, maxBlockSize;
        const int numChannels;
    };
    
    static std::unique_ptr<Drawable> makeIcon (const char* iconSvgData) {
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
            auto transform = juce::AffineTransform:: rotation (angle, getLocalBounds().getCentreX(), getLocalBounds().getCentreY());
            icon->setTransformToFit (getLocalBounds().toFloat().reduced(getBounds().getHeight()/16.f), RectanglePlacement::centred);
            icon->setTransform(icon->getTransform().followedBy(transform));
        }
        void resized() override { icon->setBounds(getLocalBounds()); }
        void setRotation(float r) { angle = r; }
        std::unique_ptr<Drawable> icon;
        float angle;
    };
} // namespace sadistic
