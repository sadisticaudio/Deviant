#pragma once
#include "../../Source/SadisticUnlockform.h"
#include "../../Source/SadisticLagrange.h"

namespace sadistic {
    
//    enum { numFX = 10, WAVELENGTH = 128 };
    enum { wetSignal = 0, drySignal = 1, numSignals };
    enum { numFX = 10, WAVELENGTH = MAXTABLELENGTH };
    enum { matrix, dials, staticPad, dynamicPad, numDisplays };
    struct Numbers {
        static const char*   zero_svg, * one_svg, * two_svg, * three_svg, * four_svg, * five_svg, * six_svg, * seven_svg, * eight_svg, * nine_svg, * drive_svg, * saturate_svg;
        static const int     zero_svgSize, one_svgSize, two_svgSize, three_svgSize, four_svgSize, five_svgSize, six_svgSize, seven_svgSize, eight_svgSize, nine_svgSize, drive_svgSize, saturate_svgSize; };
    
    struct EffectInfo { bool defaultEnabled; int defaultRoute; int defaultIndex; float defaultBlend; int numParams; };
    
    static constexpr std::string_view fxName[] {
        "Dynamic Atan",
        "Dynamic Bit Crusher",
        "Dynamic Deviation",
        "Dynamic Waveshaper",
        "Filter A",
        "Filter B",
        "Static Atan",
        "Static Bit Crusher",
        "Static Deviation",
        "Static WaveShaper",
        ""
    };
    
    static constexpr std::string_view fxID[] {
        "dynamicAtan",
        "dynamicBitCrusher",
        "dynamicDeviation",
        "dynamicWaveShaper",
        "filterA",
        "filterB",
        "staticAtan",
        "staticBitCrusher",
        "staticDeviation",
        "staticWaveShaper",
        ""
    };
    
    static constexpr EffectInfo effectInfo[] {
        { true, 1, 0, 0.f, 1 },
        { true, 1, 1, 0.f, 2 },
        { true, 1, 2, 0.f, 3 },
        { true, 1, 3, 0.f, 3 },
        { true, 0, 0, 0.f, 2 },
        { true, 3, 0, 0.f, 2 },
        { true, 2, 0, 0.f, 1 },
        { true, 2, 1, 0.f, 2 },
        { true, 2, 2, 0.f, 3 },
        { true, 2, 3, 0.f, 1 },
        { true, 4, 0, 1.f, 4 }
    };
    
    static constexpr std::string_view paramName[][4] {
        { "Drive" },
        { "Drive", "Floor" },
        { "Drive", "Gate", "Saturation" },
        { "Drive", "Table ID", "Table Position" },
        { "Low Cutoff", "High Cutoff" },
        { "Low Cutoff", "High Cutoff" },
        { "Drive" },
        { "Drive", "Floor" },
        { "Drive", "Gate", "Saturation" },
        { "Drive" },
        { "Blend", "Current Screen", "pre-Blend", "Output Gain" }
    };
    
    static constexpr std::string_view paramID[][4] {
        { "Drive" },
        { "Drive", "Floor" },
        { "Drive", "Gate", "Saturation" },
        { "Drive", "TableID", "TablePosition" },
        { "Low", "High" },
        { "Low", "High" },
        { "Drive" },
        { "Drive", "Floor" },
        { "Drive", "Gate", "Saturation" },
        { "Drive" },
        { "blend", "currentScreen", "preBlend", "outputGain" }
    };
    
    static constexpr ParamInfo paramInfo[][4] {
        { { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB }, { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 2.f, 50.f, 2.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB }, { 0.f, 1.f, 0.f, ParamInfo::dB }, { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 20.f, 20000.f, 20.f, ParamInfo::Hz }, { 20.f, 20000.f, 20000.f, ParamInfo::Hz } },
        { { 20.f, 20000.f, 20.f, ParamInfo::Hz }, { 20.f, 20000.f, 20000.f, ParamInfo::Hz } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB }, { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 2.f, 50.f, 2.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 0.f, 100.f, 100.f, ParamInfo::dB }, { 0.f, 10.f, 1.f, ParamInfo::dB }, { 0.f, 100.f, 100.f, ParamInfo::dB }, { 0.f, 1.f, 1.f, ParamInfo::dB } }
    };
    
    static inline String getFxID(int effectIndex) { return { fxID[effectIndex].data(), fxID[effectIndex].size() }; }
    static inline String getFxName(int effectIndex) { return { fxName[effectIndex].data(), fxName[effectIndex].size() }; }
    static inline String getParamID(int eIndex, int pIndex) { return { getFxID(eIndex) + String(paramID[eIndex][pIndex].data(), paramID[eIndex][pIndex].size()) }; }
    static inline String getParamName(int eIndex, int pIndex) { return { getFxName(eIndex) + " " + String(paramName[eIndex][pIndex].data(), paramName[eIndex][pIndex].size()) }; }
    
    
    
    //        template<typename FloatType> static void getWaveTable(APVTS& state, FloatType* dest, bool = false) {
    //            auto x { state.state.getProperty("waveTable") };
    //            auto array = x.getArray();
    //            auto* b = array->begin();
    //            for(int i { 0 }; i < waveLength; ++i, ++b) dest[i] = static_cast<FloatType>(double(*b));
    //        }
    //        template<typename FloatType> static void setWaveTable(ValueTree& state, UndoManager* uM, const FloatType* src) {
    //            auto tVar { var(Array<var>()) };
    //            auto array = tVar.getArray();
    //            for(int i { 0 }; i < waveLength + 1; ++i) array->add(double(src[i]));
    //            state.setProperty("waveTable", tVar, uM);
    //        }
    //        template<typename FloatType> static void getStaticTable(APVTS& state, FloatType* dest, bool = false) {
    //            auto x { state.state.getProperty("staticTable") };
    //            auto array = x.getArray();
    //            auto* b = array->begin();
    //            for(int i { 0 }; i < waveLength; ++i, ++b) dest[i] = static_cast<FloatType>(double(*b));
    //        }
    //        template<typename FloatType> static void setStaticTable(ValueTree& state, UndoManager* uM, const FloatType* src) {
    //            auto tVar { var(Array<var>()) };
    //            auto array = tVar.getArray();
    //            for(int i { 0 }; i < waveLength + 1; ++i) array->add(double(src[i]));
    //            state.setProperty("staticTable", tVar, uM);
    //        }
    //        template<typename FloatType> static void getDynamicStaticTable(APVTS& state, FloatType* dest, bool = false) {
    //            auto x { state.state.getProperty("dynamicStaticTable") };
    //            auto array = x.getArray();
    //            auto* b = array->begin();
    //            for(int i { 0 }; i < waveLength; ++i, ++b) dest[i] = static_cast<FloatType>(double(*b));
    //        }
    //        template<typename FloatType> static void setDynamicStaticTable(ValueTree& state, UndoManager* uM, const FloatType* src) {
    //            auto tVar { var(Array<var>()) };
    //            auto array = tVar.getArray();
    //            for(int i { 0 }; i < waveLength + 1; ++i) array->add(double(src[i]));
    //            state.setProperty("dynamicStaticTable", tVar, uM);
    //        }
    
    template <typename FloatType>
    typename FIR::Coefficients<FloatType>::Ptr makeBandpass(FloatType lowFrequency, FloatType highFrequency, double sampleRate, size_t order, typename WindowingFunction<FloatType>::WindowingMethod type, FloatType beta = 2.0) {
        jassert (sampleRate > 0);
        jassert (lowFrequency > 0 && lowFrequency <= sampleRate * 0.5);
        jassert (highFrequency > 0 && highFrequency <= sampleRate * 0.5);
        auto result = new FIR::Coefficients<FloatType> (order + 1u);
        auto* c = result->getRawCoefficients();
        auto normalisedLowFrequency = lowFrequency / sampleRate;
        auto normalisedHighFrequency =  highFrequency / sampleRate;
        
        for (size_t i { 0 }; i <= order; ++i) {
            if (i == order / 2) c[i] = static_cast<FloatType> ((normalisedHighFrequency - normalisedLowFrequency) * 2);
            
            else {
                auto indice = MathConstants<double>::pi * (static_cast<double> (i) - 0.5 * static_cast<double> (order));
                c[i] = static_cast<FloatType> ((std::sin (2.0 * indice * normalisedHighFrequency) - std::sin (2.0 * indice * normalisedLowFrequency)) / indice);
            }
        }
        
        WindowingFunction<FloatType> theWindow (order + 1, type, false, beta);
        theWindow.multiplyWithWindowingTable (c, order + 1);
        return result;
    };
    
    struct DeviantEffect {
        DeviantEffect(String eID, ParamList refs, FloatParamList floatRefs, int eIDX) : effectID(eID), effectIndex(eIDX), defaults(refs), params(floatRefs) {}
        DeviantEffect(DeviantEffect& other) : effectID(other.effectID), effectIndex(other.effectIndex), defaults(other.defaults), params(other.params) {}
        virtual ~DeviantEffect() = default;
        bool operator<(const DeviantEffect& other) {
            if (getRoute() < other.getRoute()) return true;
            else if (getRoute() == other.getRoute() && getIndex() < other.getIndex()) return true;
            return false;
        }
        virtual void prepare(const ProcessSpec&) {}
        virtual void processSamples(AudioBuffer<float>&) {}
        virtual void processSamples(AudioBuffer<double>&) {}
        virtual void process(AudioBuffer<float>& buffer) { if (isEnabled()) processSamples(buffer); }
        virtual void process(AudioBuffer<double>& buffer) { if (isEnabled()) processSamples(buffer); }
        virtual void reset() {}
        virtual int getLatency() { return 0; }
        bool isEnabled() const   { return static_cast<AudioParameterBool&>(defaults[0].get()).get(); }
        int getRoute() const     { return static_cast<AudioParameterInt&>(defaults[1].get()).get(); }
        int getIndex() const     { return static_cast<AudioParameterInt&>(defaults[2].get()).get(); }
        float getBlend() const   { return static_cast<AudioParameterFloat&>(defaults[3].get()).get(); }
        
        String effectID;
        int effectIndex;
        ParamList defaults;
        FloatParamList params;
    };

    template<typename FloatType> struct GainTable {
        static constexpr int waveLength { WAVELENGTH };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<FloatType>::pi }, halfPi { MathConstants<FloatType>::halfPi }, twoPi { MathConstants<FloatType>::twoPi };
        FloatType operator[](int idx) const {
            jassert (isPositiveAndBelow (idx, waveLength + 1));
            return table[idx % waveLength]; }
        FloatType operator[](FloatType sample) const {
            FloatType floatIndex { jlimit(-one, one, sample) * FloatType(waveLength)/two + FloatType(waveLength)/two };
            auto i { truncatePositiveToUnsignedInt (floatIndex) };
            auto f { floatIndex - FloatType (i) };
            jassert (isPositiveAndNotGreaterThan (f, one));
            auto x0 { table[i] };
            auto x1 { table[i + 1] };
            return jmap (f, x0, x1);
        }
        FloatType table[waveLength + 1]{};
    };
    
    template<typename FloatType> struct PhaseTable {
        static constexpr int waveLength { WAVELENGTH }, phaseFilterOrder { 8 };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<FloatType>::pi }, halfPi { MathConstants<FloatType>::halfPi }, twoPi { MathConstants<FloatType>::twoPi };
        FloatType operator[](int idx) const {
            jassert (isPositiveAndBelow (idx, waveLength + 1));
            return table[idx % waveLength]; }
        FloatType operator[](FloatType phase) const {
            jassert(isPositiveAndNotGreaterThan(phase, one));
            FloatType floatIndex { jlimit(zero, one, phase) * FloatType(waveLength) };
            auto i { truncatePositiveToUnsignedInt (floatIndex) };
            auto f { floatIndex - FloatType (i) };
            jassert (isPositiveAndBelow (f, one));
            auto x0 { table[i] };
            auto x1 { table[i + 1] };
            return jmap (f, x0, x1);
        }
        FloatType table[waveLength + 1]{};
    };
    
    template<typename F> struct Table {
        static constexpr int max { MAXTABLELENGTH };
        static constexpr F zero { static_cast<F>(0) }, one { static_cast<F>(1) }, two { static_cast<F>(2) }, half { one / two };
        Table(F* t, F w = static_cast<F>(max), F s = one, F i = zero) : table(t), waveLength(w), slope(s), intercept(i) {}
        F operator[](int idx) const { return table[idx]; }
        F operator[](F sample) const {
            F floatIndex { jlimit(zero, one, slope * sample + intercept) * waveLength };
            auto i { truncatePositiveToUnsignedInt (floatIndex) };
            F f { floatIndex - F (i) };
            jassert (isPositiveAndNotGreaterThan (f, one));
            F x0 { table[i] }, x1 { table[i + 1] };
            return jmap (f, x0, x1); }
        F* table{};
        const F waveLength, slope, intercept;
    };
    
    template<typename F> struct SubTable {
        static constexpr int max { MAXTABLELENGTH };
        static constexpr F zero { static_cast<F>(0) }, one { static_cast<F>(1) }, two { static_cast<F>(2) }, half { one / two };
        static constexpr int getOffset(int order) { return static_cast<int>(F(max) * (one - pow(half, order))/half); }
        SubTable(int o) : order(o) { table.assign((max >> o) + 1, {}); }
        F operator[](F sample) const {
            F floatIndex { jlimit(zero, one, sample) * waveLength };
            auto i { truncatePositiveToUnsignedInt (floatIndex) };
            F f { floatIndex - F (i) };
            jassert (isPositiveAndNotGreaterThan (f, one));
            F x0 { table[i] }, x1 { table[i + 1] };
            return jmap (f, x0, x1); }
        std::vector<F> table;
        const int order;
        const F waveLength { static_cast<F>(WAVELENGTH >> order) };
    };
    
    template<typename F> struct WaveTable {
        static constexpr int max { MAXTABLELENGTH }, numOctaves { NUMOCTAVES };
        static constexpr F zero { static_cast<F>(0) }, one { static_cast<F>(1) };
        void loadSubTable(int octave, const F* data) {
            int length { static_cast<int>(subTable[octave].waveLength) };
            for (int i { 0 }; i <= length; ++i) subTable[octave].table[i] = data[i];
            subTable[octave].table[length] = zero;
        }
        const SubTable<F>& getTable(int length) const noexcept { return subTable[jlimit(0, /* numOctaves - */ 1, int(log2((max*2)/length)))]; }
        //        F w[max * 2]{}, w0[max + 1]{}, w1[max/2 + 1]{}, w2[max/4 + 1]{}, w3[max/8 + 1]{}, w4[max/16 + 1]{}, w5[max/32 + 1]{};
        SubTable<F> subTable[numOctaves] { { 0 }, { 1 }, { 2 }, { 3 }, { 4 }, { 5 } };
    };
    
    template<typename F> struct PhaseTableSet {
        static constexpr int max { MAXTABLELENGTH }, numOctaves { NUMOCTAVES };
        static constexpr F zero { static_cast<F>(0) }, one { static_cast<F>(1) };
        
        PhaseTableSet() { *filter.coefficients = *FilterDesign<F>::designFIRLowpassHalfBandEquirippleMethod(F(0.08), F(-80.0));
            filterOrder = int(filter.coefficients->getFilterOrder());
            temp.resize(max + 1);
            srcBuffer.setSize(1, filterOrder + max + max + 1);
            dstBuffer.setSize(1, filterOrder + max + max + 1);
        }
        
        const SubTable<F>& getTable(int length, int frame = 0) noexcept { return frames[frame].getTable(length);}//.getReference(frame).getTable(length); }
        
        int getNumTables() { return int(frames.size()); }
        
        void addFrames(int total) {
            frames.resize(total);
            for(int j { 0 }; j < total; ++j) {
                auto& frame { frames[j] };
                for(int i { 0 }; i < numOctaves; ++i) {
                    for (int k { 0 }; k < max >> i; ++k)
                        frame.subTable[i].table[k] = zero;
                }
            }
        }
        
        void loadTables(const AudioBuffer<float>& buffer){
            int filterDelay { filterOrder/2 };
            
            if (srcBuffer.getNumSamples() <= buffer.getNumSamples() + max + filterOrder) srcBuffer.setSize(1, buffer.getNumSamples() + max + filterOrder);
            if (dstBuffer.getNumSamples() <= buffer.getNumSamples() + max + filterOrder) dstBuffer.setSize(1, buffer.getNumSamples() + max + filterOrder);
            
            //copy the first frame once to allow the filter to warm up
            for (int i { 0 }; i < max; ++i)
                srcBuffer.setSample(0, i, static_cast<F>(buffer.getSample(0, i)));
            
            for (int i { 0 }; i < buffer.getNumSamples(); ++i)
                srcBuffer.setSample(0, max + i, static_cast<F>(buffer.getSample(0, i)));
            
            for (int octave { 0 }, frameLength { max }; octave < numOctaves; ++octave, frameLength /= 2) {
                
                //load the frames starting after the first copy
                for (int f { 0 }; f < int(frames.size()); ++f) {
                    const F* cSrc { srcBuffer.getReadPointer(0, frameLength + f * frameLength) };
                    frames[f].loadSubTable(octave, cSrc);
                }
                
                //filter all frames including the extra copy of the first frame
                AudioBlock<F> srcBlock { srcBuffer }, dstBlock { dstBuffer };
                filter.process(ProcessContextNonReplacing<F>(srcBlock, dstBlock));
                
                //copy, downsampling by 2, the filtered output starting after the filter delay, at the start of the fist copy
                auto* src { srcBuffer.getWritePointer(0) };
                const auto* dst { dstBuffer.getReadPointer(0) };
                for (int i { 0 }; i < (frameLength + frameLength * int(frames.size()))/2; ++i) src[i] = dst[filterDelay + i * 2];
            }
        }
        FIR::Filter<F> filter;
        AudioBuffer<F> srcBuffer, dstBuffer;
        
        int filterOrder;
        std::vector<WaveTable<F>> frames { 1, WaveTable<F>{} };
        std::vector<float> temp { 0.f, max + 1 };
    };
    
    String inline getSadisticFolder() {
        std::vector<String> folders {
            { { File::getSpecialLocation(File::userApplicationDataDirectory).getFullPathName() + "/Application Support/Sadistic Audio" } },
            { { File::getSpecialLocation(File::userMusicDirectory).getFullPathName() + "/.sadisticaudio" } }
        };
        for(size_t i = 0; i < folders.size(); ++i) {
            auto resultFile = File(folders[i] + "/test.xml");
            auto result = resultFile.create();
            if(result.wasOk())
                return folders[i];
        }
        return {};
    }
    
    struct WaveTableData {
        WaveTableData(const String& s, const int n, const bool b) : id(s), numFrames(n), builtIn(b) {}
        File getFile() const {
            return builtIn ? getSadisticFolder() + "/Wave Tables/Stock/" + id : getSadisticFolder() + "/Wave Tables/User/" + id; }
        String id;
        int numFrames;
        bool builtIn;
    };
    
    struct TableManager {
        using UM = UndoManager;
        static constexpr int waveLength { WAVELENGTH };
        
        WaveTableData stockTables[2] {
            { "MATRIXYC64_wav", {}, {} },
            { "cycle2048_wav", {}, {} } };
        
        TableManager(APVTS& a, UM* u) : apvts(a), uM(u) {
//            for (size_t i { 0 }; i < Wave<float>::numWaves; ++i) {
//                String id { String(Wave<float>::waveID[i].data(), Wave<float>::waveID[i].size()) + ".wav" };
//                waveTableFiles.push_back({id, waveLength + 1, true});
//                auto& t { tables.emplace_back(waveLength + 1, 0.f) };
//                Wave<float>::fillTable(t.data(), waveLength, Wave<float>::Type(i), true, true);
//            }
//            
//            array.resize(waveLength + 1);
//            std::copy(tables[1].begin(), tables[1].end(), array.begin());
//            
//            for (size_t i { 0 }; i < sizeof(stockTables)/sizeof(WaveTableData); ++i) {
//                
//                int dataSize { 0 };
//                const auto dataPtr { Data::getNamedResource(stockTables[i].id.toUTF8(), dataSize) };
//                
//                WavAudioFormat format;
//                auto inputStream { MemoryInputStream(dataPtr, size_t(dataSize), true) };
//                auto* reader { format.createReaderFor(&inputStream, true) };
//                if (reader) {
//                    SamplerSound ss { {}, *reader, {}, 10, 10.0, 10.0, 10.0 };
//                    auto* bufferPtr { ss.getAudioData() };
//                    const auto numSamples { bufferPtr->getNumSamples() };
//                    auto& t { tables.emplace_back(size_t(numSamples), 0.f) };
//                    //                    WaveTableData wt;
//                    //                    wt.id =  ;
//                    //                    wt.numFrames = static_cast<int>();
//                    //                    wt.builtIn = true;
//                    waveTableFiles.emplace_back(stockTables[i].id.dropLastCharacters(4), numSamples/waveLength, true);
//                    auto* samples { bufferPtr->getReadPointer(0) };
//                    std::copy(samples, samples + numSamples, t.begin());
//                }
//            }
//            float arr[waveLength + 1];
//            for (int i { 0 }; i <= waveLength; ++i) arr[i] = -1.f + (2.f * float(i))/float(waveLength);
//            setTable("waveTable", arr, waveLength + 1);
//            setTable("gainTable", arr, waveLength + 1);
        }
        
        bool loadTable(File inputFile) {
            String fileName { inputFile.getFileName().removeCharacters(".wav") };
            WavAudioFormat format;
            auto inputStream { FileInputStream(inputFile) };
            auto* reader { format.createReaderFor(&inputStream, true) };
            if (reader) {
                SamplerSound ss { {}, *reader, {}, 10, 10.0, 10.0, 10.0 };
                auto* bufferPtr { ss.getAudioData() };
                const auto numSamples { bufferPtr->getNumSamples() };
                auto& t { tables.emplace_back(size_t(numSamples), 0.f) };
                waveTableFiles.emplace_back(fileName, numSamples/waveLength, true);
                auto* samples { bufferPtr->getReadPointer(0) };
                std::copy(samples, samples + numSamples, t.begin());
                return true;
            }
            return false;
        }
        
        void setTable(Identifier identifier, const float* samples, const int numSamples, UM* u = nullptr) {
            if (identifier.toString() == "waveTable" || identifier.toString() == "gainTable") {
                auto& block { getWriteBlock(identifier) };
                const auto numBytes { sizeof(float) * size_t(numSamples) };
                block.ensureSize(numBytes);
                block.copyFrom(samples, 0, numBytes);
                swapBlock(identifier);
                apvts.state.setProperty(identifier, getReadBlock(identifier), nullptr);
            }
            else {
                array.resize(size_t(numSamples));
                std::copy(samples, samples + numSamples, array.begin());
            }
        }
        
        void getTable(Identifier identifier, float* samples, int numSamples = 0) const {
            if (identifier.toString() == "waveTable" || identifier.toString() == "gainTable") {
                const auto& table { apvts.state.getProperty(identifier) };
                const auto* block { table.getBinaryData() };
                block->copyTo(static_cast<void*>(samples), 0, block->getSize());
            }
            else if (!writing) {
                //                const auto& table { apvts.state.getProperty(identifier) };
                //                const auto* block { table.getBinaryData() };
                if (numSamples == 0) numSamples = array.size();
                std::copy(array.data(), array.data() + numSamples, samples);
                //                const auto& block { getReadBlock(identifier) };
                //                block.copyTo(static_cast<void*>(samples), 0, block.getSize());
                //                const int numSamples { static_cast<int>(block.getSize()/sizeof(float)) };
                //                const float* ptr { reinterpret_cast<const float*>(block.getData()) };
                //                for (int i { 0 }; i < numSamples; ++i) samples[i] = static_cast<F>(ptr[i]);
                //                std::copy(ptr, ptr + size_t(numSamples), samples);
            }
        }
        
        MemoryBlock& getWriteBlock(const Identifier& identifier) const {
            if (identifier.toString().contains("waveTable")) return *waveWriteBlock;
            if (identifier.toString().contains("gainTable")) return *gainWriteBlock;
            return *phaseWriteBlock;
        }
        const MemoryBlock& getReadBlock(const Identifier& identifier) const {
            if (identifier.toString().contains("waveTable")) return *waveReadBlock;
            if (identifier.toString().contains("gainTable")) return *gainReadBlock;
            return *phaseReadBlock;
        }
        void swapBlock(const Identifier& identifier) {
            if (identifier.toString().contains("waveTable")) blocks[0].swapWith(blocks[1]);
            if (identifier.toString().contains("gainTable")) blocks[2].swapWith(blocks[3]);
            else blocks[4].swapWith(blocks[5]);
        }
        
        bool saveTable() {
            const float idxFloat { *apvts.getRawParameterValue("waveTableID") };
            const size_t idx { static_cast<size_t>(idxFloat) };
            auto& table { tables[idx] };
            const int numSamples { static_cast<int>(table.size()) };
            float* mockChannel[1] { table.data() };
            const AudioBuffer<float> buffer { mockChannel, 1, numSamples };
            File file { waveTableFiles[idx].getFile() };
            return saveTable(file, buffer);
        }
        
        bool saveTable(File& outputFile) {
            const int numSamples { static_cast<int>(array.size()) };
            float* mockChannel[1] { array.data() };
            const AudioBuffer<float> buffer { mockChannel, 1, numSamples };
            return saveTable(outputFile, buffer);
        }
        
        bool saveTable(File& outputFile, const AudioBuffer<float>& buffer) {
            StringPairArray metadataValues = WavAudioFormat::createBWAVMetadata ("Custom WaveTable",
                                                                                 "originator",
                                                                                 "originatorRef",
                                                                                 Time::getCurrentTime(),
                                                                                 buffer.getNumChannels(),
                                                                                 "codingHistory");
            std::unique_ptr<juce::FileOutputStream> outStream;
            outStream = outputFile.createOutputStream();
            juce::WavAudioFormat format;
            std::unique_ptr<juce::AudioFormatWriter> writer;
            writer.reset(format.createWriterFor(outStream.get(), 44100.0, uint32(buffer.getNumChannels()), 32, metadataValues, 0));
            
            if (writer != nullptr) {
                outStream.release();
                if (writer->writeFromAudioSampleBuffer (buffer, 0, buffer.getNumSamples())) return true;
            }
            return false;
        }
        
        void selectTable(const int index) {
            size_t idx { static_cast<size_t>(index) };
            writing = true;
            array = tables[idx];
            setTable("phaseTable", tables[idx].data(), int(tables[idx].size()), uM);
            writing = false;
        }
        
        int getTableLength(const Identifier& identifier) {
            if (identifier.toString() == "waveTable" || identifier.toString() == "gainTable") {
                auto tVar { apvts.state.getProperty(identifier, var(MemoryBlock(waveLength))) };
                auto* data = tVar.getBinaryData();
                return static_cast<int>(data->getSize() / sizeof(float));
            }
            else return static_cast<int>(array.size());
        }
        
        
        
        std::vector<float> array;
        APVTS& apvts;
        UM* uM;
        ValueTree props;
        MemoryBlock blocks[6];
        MemoryBlock* waveWriteBlock { &blocks[0] }, * gainWriteBlock { &blocks[2] }, * phaseWriteBlock { &blocks[4] };
        MemoryBlock* waveReadBlock { &blocks[1] }, * gainReadBlock { &blocks[3] }, * phaseReadBlock { &blocks[5] };
        std::atomic<bool> writing { false };
        Value tablesChanged { false };
        std::vector<WaveTableData> waveTableFiles;
        std::vector<std::vector<float>> tables;
    };
    
    ////////////////     GRAPHICS      ////////////////////////////////////////////////////////
    
//    struct Numbers {
//        static const char*   zero_svg, * one_svg, * two_svg, * three_svg, * four_svg, * five_svg, * six_svg, * seven_svg, * eight_svg, * nine_svg;
//        static const int     zero_svgSize, one_svgSize, two_svgSize, three_svgSize, four_svgSize, five_svgSize, six_svgSize, seven_svgSize, eight_svgSize, nine_svgSize;
//    };
    
    struct EmpiricalLAF    : public LookAndFeel_V4   {
        static constexpr auto numNumbers { 12 }, numNeedles { 112 };
        void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                               float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override;
        void drawLinearSlider (Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& slider) override;
        void drawComboBox  (Graphics& g, int width, int height, bool, int, int, int, int, ComboBox& box) override;
    };
    
    struct LeftEmpiricalLAF : EmpiricalLAF { void drawLabel (Graphics& g, Label& label) override; };
    struct RightEmpiricalLAF : EmpiricalLAF { void drawLabel (Graphics& g, Label& label) override; };
    
    struct OuterLookAndFeel    : public LookAndFeel_V4 {
        void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override;
    };
    
    struct MiddleLookAndFeel    : public LookAndFeel_V4 {
        void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override;
    };
    
    class OuterSlider : public Slider   {
    public:
        OuterSlider(bool t = false, bool l = false) : top(t), left(l) {}
        bool hitTest (int x, int y) override;
        bool isOnTop() { return top; }
        bool isOnLeft() { return left; }
    private:
        bool top, left;
    };
    class UpperSlider : public Slider   {public: bool hitTest (int x, int y) override;};
    class LowerSlider : public Slider   {public: bool hitTest (int x, int y) override;};
    struct EmpiricalSlider : public Slider   {
        static constexpr int numNumbers { EmpiricalLAF::numNumbers }, numNeedles { EmpiricalLAF::numNeedles };
        EmpiricalSlider(bool l = false, bool s = false, bool h = false) : isLeft(l), isSmall(s), isDefaultHigh(h) {}

        bool isLeft, isSmall, isDefaultHigh; public: bool hitTest (int x, int y) override;
        Path needles[numNeedles], ring;
        Path knob;
        std::unique_ptr<Drawable> dp;
    };
    struct Frame : Component {
        Frame() { setInterceptsMouseClicks(false, false); }
        void paint(Graphics& g) override {
            Path rectangle;
            g.setColour(Colours::white.darker());
            rectangle.addRoundedRectangle(getLocalBounds(), 5);
            g.strokePath(rectangle, PathStrokeType(5.f));
        }
    };
    class TransLabel  : public Label    {public: bool hitTest (int x, int y) override;};
    void showEmpiricalValue(Slider& slider, Label& label1, Component& child);
    void hideValue(Label& valueLabel, Label& suffixLabel);
    void hideValue(Label& valueLabel, Component& child);
    void showIntegerValue(Slider& slider, Label& label1, Label& label2);
    void showLevelValue(Slider& slider, Label& label1, Label& label2);
    void setWidgets(Slider& blendKnob, Slider& driveKnob, sadistic::EmpiricalLAF& llaf, Slider& saturationKnob, Slider& filterKnob, sadistic::OuterLookAndFeel& olaf, Slider& gateKnob, Slider& lpfKnob, Label& valueLabel, Label& suffixLabel, Font& font);
    
    struct DeviantTree {
        static constexpr int waveLength { WAVELENGTH };
        DeviantTree(APVTS& s, UndoManager* uM) {
            auto vts = ValueTree("STATE");
            double def[waveLength + 1];
            Wave<double>::fillStaticTable(def, waveLength, Wave<double>::atan, true, true);
            setWaveTable(s, uM, def);
            Wave<double>::fillStaticTable(def, waveLength, Wave<double>::atan, true, true);
            setGainTable(s, uM, def);
            Wave<double>::fillTable(def, waveLength, Wave<double>::sine, true, true);
            setPhaseTable(s, uM, def);
            s.state = vts;
        }
        
        template<typename FloatType> static void getWaveTable(APVTS& apvts, FloatType* dest, bool = false) {
            auto state = apvts.state;
            getWaveTable(state, dest);
            //            auto x { apvts.state.getProperty("waveTable") };
            //            auto array = x.getArray();
            //            auto* b = array->begin();
            //            for(int i { 0 }; i <= waveLength; ++i, ++b) dest[i] = static_cast<FloatType>(double(*b));
        }
        template<typename FloatType> static void getWaveTable(ValueTree& vts, FloatType* dest, bool = false) {
            auto x { vts.getProperty("waveTable") };
            auto array = x.getArray();
            auto* b = array->begin();
            for(int i { 0 }; i <= waveLength; ++i, ++b) dest[i] = static_cast<FloatType>(double(*b));
        }
        template<typename FloatType> static void setWaveTable(APVTS& apvts, UndoManager* uM, const FloatType* src) {
            auto tVar { var(Array<var>()) };
            auto array = tVar.getArray();
            for(int i { 0 }; i < waveLength + 1; ++i) array->add(double(src[i]));
            apvts.state.setProperty("waveTable", tVar, uM);
        }
        template<typename FloatType> static void getPhaseTable(APVTS& apvts, FloatType* dest, bool = false) {
            auto state = apvts.state;
            getPhaseTable(state, dest);
            //            const auto& x { state.getProperty("phaseTable") };
            //            auto* array = x.getArray();
            //            auto* b = array->begin();
            //            for(int i { 0 }; i <= waveLength; ++i, ++b) dest[i] = static_cast<FloatType>(double(*b));
        }
        template<typename FloatType> static void getPhaseTable(ValueTree& vts, FloatType* dest, bool = false) {
            auto x { vts.getProperty("phaseTable") };
            auto* array = x.getArray();
            auto* b = array->begin();
            for(int i { 0 }; i <= waveLength; ++i, ++b) dest[i] = static_cast<FloatType>(double(*b));
        }
        template<typename FloatType> static void setPhaseTable(APVTS& apvts, UndoManager* uM, const FloatType* src) {
            auto tVar { var(Array<var>()) };
            auto* array = tVar.getArray();
            for(int i { 0 }; i < waveLength + 1; ++i) array->add(double(src[i]));
            apvts.state.setProperty("phaseTable", tVar, uM);
        }
        template<typename FloatType> static void getGainTable(APVTS& apvts, FloatType* dest, bool = false) {
            auto state = apvts.state;
            getGainTable(state, dest);
            //            auto x { apvts.state.getProperty("gainTable") };
            //            auto array = x.getArray();
            //            auto* b = array->begin();
            //            for(int i { 0 }; i <= waveLength; ++i, ++b) dest[i] = static_cast<FloatType>(double(*b));
        }
        template<typename FloatType> static void getGainTable(ValueTree& vts, FloatType* dest, bool = false) {
            auto x { vts.getProperty("gainTable") };
            auto* array = x.getArray();
            auto* b = array->begin();
            for(int i { 0 }; i <= waveLength; ++i, ++b) dest[i] = static_cast<FloatType>(double(*b));
        }
        template<typename FloatType> static void setGainTable(APVTS& apvts, UndoManager* uM, const FloatType* src) {
            auto tVar { var(Array<var>()) };
            auto* array = tVar.getArray();
            for(int i { 0 }; i < waveLength + 1; ++i) array->add(double(src[i]));
            apvts.state.setProperty("gainTable", tVar, uM);
        }
    };
    
    struct DeviantScreen : Component {
        static constexpr int waveLength { WAVELENGTH };
        DeviantScreen(APVTS& a, UndoManager* uM) : apvts(a), undoManager(uM) {}
        virtual ~DeviantScreen() = default;
        virtual void updateWaveTable() = 0;
        virtual void init() = 0;
        std::atomic<bool> newDataHere { false };
        float wave[waveLength + 1]{}, pWave[waveLength + 1]{};
        Table<float> gainTable { wave };
        Table<float> phaseTable { pWave };
        APVTS& apvts;
        UndoManager* undoManager;
    };
    
    struct SadButton : public Button {
        SadButton(bool iL = false) : Button("displays"), isLeft(iL) {}
        void mouseEnter(const MouseEvent&) override {
            colour = Colours::white.darker();
            repaint();
        }
        void mouseExit(const MouseEvent&) override {
            colour = Colours::grey;
            repaint();
        }
        void paintButton (Graphics& g, bool, bool) override {
            auto bounds { getLocalBounds().toFloat() };
            auto width { bounds.getWidth() }, height { bounds.getHeight() };
            g.setColour(Colours::white.darker().darker().darker());
            Path p;
            p.addRoundedRectangle(bounds.reduced(2), 3);
            g.strokePath(p, PathStrokeType(1.f));
            Path p1;
            p1.startNewSubPath(2,height/2);
            p1.addArrow({ 2, height/2, width - 2, height/2 }, 6.f, 12.f, width/2);
            if(isLeft)
                p1.applyTransform(AffineTransform::rotation(MathConstants<float>::pi, bounds.getCentreX(), bounds.getCentreY()));
            g.setColour(colour);
            g.strokePath(p1,PathStrokeType(2.f));
            
        }
        Colour colour { Colours::grey };
        const bool isLeft;
    };
    
    struct SadLabel : public Component {
        SadLabel(String text={}, bool iV={}, bool iL={}, float r={}) : isVertical(iV), isLeft(iL), angle(r) {
            label.setText(text, dontSendNotification);
            label.setJustificationType(Justification::centred);
            addAndMakeVisible(label);
            setInterceptsMouseClicks(false, false);
        }
        void set(const String& s, Colour tCol, Colour bgCol) {
            label.setText(s, dontSendNotification);
            label.setColour(Label::backgroundColourId, bgCol);
            label.setColour(Label::textColourId, tCol);
        }
        void paint (Graphics&) override { }
        void resized() override {
            auto bounds { getLocalBounds() };
            if(!isVertical) label.setBounds(bounds);
            else {
                label.setBounds(Rectangle<int>(bounds.getHeight(), bounds.getWidth()).withCentre(bounds.getCentre()));
                label.setTransform(AffineTransform::rotation(MathConstants<float>::pi * angle, label.getBounds().getCentreX(), label.getBounds().getCentreY()));
            } }
        Label label;
        const bool isVertical, isLeft;
        float angle;
    };
    
    struct SadTextButton : public TextButton {
        SadTextButton(const String& s, bool iV={}, bool iL={}, float r={}) : label(s, iV, iL, r), isLeft(iL) {
            addAndMakeVisible(label);
        }
        void mouseEnter(const MouseEvent&) override {
            colour = Colours::white.darker();
            repaint();
        }
        void mouseExit(const MouseEvent&) override {
            colour = Colours::grey;
            repaint();
        }
        void resized() override {
            auto bounds { getLocalBounds() };
            label.setBounds(bounds);
        }
        void paintButton (Graphics& g, bool, bool) override {
            auto bounds { getLocalBounds().toFloat() };
//            auto width { bounds.getWidth() }, height { bounds.getHeight() };
            g.setColour(Colours::white.darker().darker().darker());
            Path p;
            p.addRoundedRectangle(bounds.reduced(2), 3);
            g.strokePath(p, PathStrokeType(1.f));
        }
        Colour colour { Colours::grey };
        SadLabel label;
        const bool isLeft;
    };
    
    struct SadBox : ComboBox {
        SadBox(String name) : ComboBox(name) {
//            setColour (ComboBox::backgroundColourId, Colours::blue.darker().darker().darker());
//            setColour (ComboBox::outlineColourId, Colours::blue.darker());
//            setColour (ComboBox::arrowColourId, Colours::blue);
//            setColour (ComboBox::focusedOutlineColourId, Colours::blue);
//            setColour (ComboBox::textColourId, Colours::white.darker());
        }
    };
    
}

