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
        { true, 1, 3, 0.f, 2 },
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
        { "Drive", "Table Position" },
        { "Low Cutoff", "High Cutoff" },
        { "Low Cutoff", "High Cutoff" },
        { "Drive" },
        { "Drive", "Floor" },
        { "Drive", "Gate", "Saturation" },
        { "Drive" },
        { "Blend", "Current Screen", "Wave Table ID", "Output Gain" }
    };
    
    static constexpr std::string_view paramID[][4] {
        { "Drive" },
        { "Drive", "Floor" },
        { "Drive", "Gate", "Saturation" },
        { "Drive", "TablePosition" },
        { "Low", "High" },
        { "Low", "High" },
        { "Drive" },
        { "Drive", "Floor" },
        { "Drive", "Gate", "Saturation" },
        { "Drive" },
        { "blend", "currentScreen", "waveTableID", "outputGain" }
    };
    
    static constexpr ParamInfo paramInfo[][4] {
        { { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB }, { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 2.f, 50.f, 2.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB }, { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 20.f, 20000.f, 20.f, ParamInfo::Hz }, { 20.f, 20000.f, 20000.f, ParamInfo::Hz } },
        { { 20.f, 20000.f, 20.f, ParamInfo::Hz }, { 20.f, 20000.f, 20000.f, ParamInfo::Hz } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB }, { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 2.f, 50.f, 2.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 0.f, 100.f, 100.f, ParamInfo::dB }, { 0.f, 10.f, 1.f, ParamInfo::dB }, { 0.f, 100.f, 1.f, ParamInfo::Int }, { 0.f, 1.f, 1.f, ParamInfo::dB } }
    };
    
    static inline String getFxID(int effectIndex) { return { fxID[effectIndex].data(), fxID[effectIndex].size() }; }
    static inline String getFxName(int effectIndex) { return { fxName[effectIndex].data(), fxName[effectIndex].size() }; }
    static inline String getParamID(int eIndex, int pIndex) { return { getFxID(eIndex) + String(paramID[eIndex][pIndex].data(), paramID[eIndex][pIndex].size()) }; }
    static inline String getParamName(int eIndex, int pIndex) { return { getFxName(eIndex) + " " + String(paramName[eIndex][pIndex].data(), paramName[eIndex][pIndex].size()) }; }
    
    static String makeLabel(String name, String label = String()) {
        for (int i { 0 }; i < name.length(); ++i) { label += name[i]; if (i != name.length() - 1) label += " "; }
        return label.toUpperCase(); }
    //    int getFxIndex(String id, int idx = 0) {
    //        for (; idx < numFX; ++idx) if (id == getFxID(idx)) return idx;
    //        return 666; }
    static String getSuffix(ParamInfo::ParamType pT) { return pT == dB ? "dB" : pT == Hz ? "Hz" : "%"; }
    
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
    
    template<typename F> struct CalculatedParamCoefficients {
        enum { max = 8};
        F& operator[](int i) { return data[i]; }
        const F& operator()(int i) const { return data[i]; }
        F data[max]{}; };
    
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
            FloatType floatIndex { jlimit(zero, one, sample * half + half) * static_cast<FloatType>(waveLength) };
            auto i { truncatePositiveToUnsignedInt (floatIndex) };
            auto f { floatIndex - FloatType (i) };
            jassert (isPositiveAndNotGreaterThan (f, one));
            auto x0 { table[i] };
            auto x1 { table[i + 1] };
            return jmap (f, x0, x1);
        }
        FloatType table[waveLength + 1]{};
    };
//    
//    template<typename FloatType> struct PhaseTable {
//        static constexpr int waveLength { WAVELENGTH }, phaseFilterOrder { 8 };
//        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<FloatType>::pi }, halfPi { MathConstants<FloatType>::halfPi }, twoPi { MathConstants<FloatType>::twoPi };
//        FloatType operator[](int idx) const {
//            jassert (isPositiveAndBelow (idx, waveLength + 1));
//            return table[idx % waveLength]; }
//        FloatType operator[](FloatType phase) const {
//            jassert(isPositiveAndNotGreaterThan(phase, one));
//            FloatType floatIndex { jlimit(zero, one, phase) * FloatType(waveLength) };
//            auto i { truncatePositiveToUnsignedInt (floatIndex) };
//            auto f { floatIndex - FloatType (i) };
//            jassert (isPositiveAndBelow (f, one));
//            auto x0 { table[i] };
//            auto x1 { table[i + 1] };
//            return jmap (f, x0, x1);
//        }
//        FloatType table[waveLength + 1]{};
//    };
    
    template<typename F> struct Table {
        static constexpr int max { MAXTABLELENGTH };
        static constexpr F zero { static_cast<F>(0) }, one { static_cast<F>(1) }, two { static_cast<F>(2) }, half { one / two };
        Table(F* t, F w = static_cast<F>(max), F s = one, F i = zero) : table(t), waveLength(w), slope(s), intercept(i) {
            
        }
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
        SubTable<F> subTable[numOctaves] { { 0 }, { 1 }, { 2 }, { 3 }, { 4 }, { 5 } };
    };
    
    template<typename F, int order> struct STable {
        static constexpr int max { MAXTABLELENGTH };
        static constexpr F zero { static_cast<F>(0) }, one { static_cast<F>(1) }, two { static_cast<F>(2) }, half { one / two };
        F operator[](F sample) const {
            F floatIndex { jlimit(zero, one, sample) * waveLength };
            auto i { truncatePositiveToUnsignedInt (floatIndex) };
            F f { floatIndex - F (i) };
            jassert (isPositiveAndNotGreaterThan (f, one));
            F x0 { table[i] }, x1 { table[i + 1] };
            return jmap (f, x0, x1); }
        F table[(max >> order) + 1]{};
        const F waveLength { static_cast<F>(WAVELENGTH >> order) };
    };
    
    template<typename F> struct WTable {
        static constexpr int max { MAXTABLELENGTH }, numOctaves { NUMOCTAVES };
        static constexpr F zero { static_cast<F>(0) }, one { static_cast<F>(1) };
        void loadSubTable(int octave, const F* data) {
            int length { static_cast<int>(subTable[octave].waveLength) };
            for (int i { 0 }; i <= length; ++i) subTable[octave].table[i] = data[i];
            subTable[octave].table[length] = zero;
        }
        const auto& getTable(int length) const noexcept {
            const auto x = jlimit(0, /* numOctaves - */ 1, int(log2((max*2)/length)));
            return get<x>(subTable); }
        std::tuple<STable<F,0>, STable<F,1>, STable<F,2>, STable<F,3>, STable<F,4>, STable<F,5>> subTable{};
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
        
        template<int n> const STable<F,n>& getTable(int length) noexcept { return frames[0].getTable(length);}//.getReference(frame).getTable(length); }
        
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
        std::vector<WTable<F>> frames { 1, WTable<F>{} };
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
        WaveTableData(const String& s, const int idx, const int n, const bool b) : id(s), index(idx), numSamples(n), builtIn(b) {}
        File getFile() const {
            return builtIn ? getSadisticFolder() + "/Wave Tables/Stock/" + id : getSadisticFolder() + "/Wave Tables/User/" + id; }
        String id;
        int index;
        int numSamples;
        bool builtIn;
    };
    
    struct TableManager {
        using UM = UndoManager;
        static constexpr int waveLength { WAVELENGTH };
        
        WaveTableData stockTables[2] {
            { "MATRIXYC64_wav", {}, {}, {} },
            { "cycle2048_wav", {}, {}, {} } };
        
        TableManager(APVTS& a, UM* u) : apvts(a), uM(u) {
            for (size_t i { 0 }; i < Wave<float>::numWaves; ++i) {
                String id { String(Wave<float>::waveID[i].data(), Wave<float>::waveID[i].size()) };
                waveTableFiles.push_back({ id, static_cast<int>(i), waveLength + 1, true });
                auto& t { tables.emplace_back(waveLength + 1, 0.f) };
                Wave<float>::fillTable(t.data(), waveLength, Wave<float>::Type(i), true, true);
            }
            
            currentPhaseTable.resize(waveLength + 1);
            std::copy(tables[1].begin(), tables[1].end(), currentPhaseTable.begin());
            
            for (size_t i { 0 }; i < sizeof(stockTables)/sizeof(WaveTableData); ++i) {
                
                int dataSize { 0 };
                const auto dataPtr { Data::getNamedResource(stockTables[i].id.toUTF8(), dataSize) };
                
                WavAudioFormat format;
                auto inputStream { MemoryInputStream(dataPtr, size_t(dataSize), true) };
                auto* reader { format.createReaderFor(&inputStream, true) };
                if (reader) {
                    SamplerSound ss { {}, *reader, {}, 10, 10.0, 10.0, 10.0 };
                    auto* bufferPtr { ss.getAudioData() };
                    const auto numSamples { bufferPtr->getNumSamples() };
                    auto& t { tables.emplace_back(size_t(numSamples), 0.f) };
                    //                    WaveTableData wt;
                    //                    wt.id =  ;
                    //                    wt.numSamples = static_cast<int>();
                    //                    wt.builtIn = true;
                    waveTableFiles.emplace_back(stockTables[i].id.dropLastCharacters(4), Wave<float>::numWaves + i, numSamples/waveLength, true);
                    auto* samples { bufferPtr->getReadPointer(0) };
                    std::copy(samples, samples + numSamples, t.begin());
                }
            }
            float arr[waveLength + 1];
            for (int i { 0 }; i <= waveLength; ++i) arr[i] = -1.f + (2.f * float(i))/float(waveLength);
            setTable("waveTable", arr, waveLength + 1);
            setTable("gainTable", arr, waveLength + 1);
        }
        
        bool loadTable(File inputFile, float* dest = nullptr) {
            String fileName { inputFile.getFileName().removeCharacters(".wav") };
            WavAudioFormat format;
            auto inputStream { FileInputStream(inputFile) };
            auto* reader { format.createReaderFor(&inputStream, true) };
            if (reader) {
                SamplerSound ss { {}, *reader, {}, 10, 10.0, 10.0, 10.0 };
                auto* bufferPtr { ss.getAudioData() };
                const auto numSamples { bufferPtr->getNumSamples() };
                const int newIndex { static_cast<int>(tables.size()) };
                auto& t { tables.emplace_back(size_t(numSamples), 0.f) };
                waveTableFiles.emplace_back(fileName, newIndex, numSamples/waveLength, true);
                auto* samples { bufferPtr->getReadPointer(0) };
                std::copy(samples, samples + numSamples, t.begin());
                selectTable(newIndex, dest);
                return true;
            }
            return false;
        }
        
        template<typename F> void setTable(const String& identifier, const F* samples, const int numSamples, UM* = nullptr) {
            if (identifier == "waveTable") {
                for (int i { 0 }; i < numSamples; ++i) waveTable[i] = float(samples[i]);
                waveTableChanged = std::rand();
            }
            if (identifier == "gainTable") {
                for (int i { 0 }; i < numSamples; ++i) gainTable[i] = float(samples[i]);
                gainTableChanged = std::rand();
            }
            else if (identifier == "phaseTable") {
                currentPhaseTable.resize(size_t(numSamples));
                std::copy(samples, samples + numSamples, currentPhaseTable.begin());
                phaseTableChanged = std::rand();
            }
        }
        
        template<typename F> void getTable(const String& identifier, F* samples, int numSamples) const {
            if (identifier == "waveTable") for (int i { 0 }; i < numSamples; ++i) samples[i] = F(waveTable[i]);
            if (identifier == "gainTable") for (int i { 0 }; i < numSamples; ++i) samples[i] = F(gainTable[i]);
            else if (identifier == "phaseTable" && !writing) {
                for (int i { 0 }; i < numSamples; ++i) samples[i] = F(currentPhaseTable[i]);
            }
        }
        
//        MemoryBlock& getWriteBlock(const Identifier& identifier) const {
//            if (identifier.toString().contains("waveTable")) return *waveWriteBlock;
//            if (identifier.toString().contains("gainTable")) return *gainWriteBlock;
//            return *phaseWriteBlock;
//        }
//        const MemoryBlock& getReadBlock(const Identifier& identifier) const {
//            if (identifier.toString().contains("waveTable")) return *waveReadBlock;
//            if (identifier.toString().contains("gainTable")) return *gainReadBlock;
//            return *phaseReadBlock;
//        }
//        void swapBlock(const Identifier& identifier) {
//            if (identifier.toString().contains("waveTable")) blocks[0].swapWith(blocks[1]);
//            if (identifier.toString().contains("gainTable")) blocks[2].swapWith(blocks[3]);
//            else blocks[4].swapWith(blocks[5]);
//        }
        
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
            const int numSamples { static_cast<int>(currentPhaseTable.size()) };
            float* mockChannel[1] { currentPhaseTable.data() };
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

        void selectTable(const int index, float* dest = nullptr) {
            size_t idx { static_cast<size_t>(index) };
            writing = true;
            currentPhaseTable = tables[idx];
            if (dest) std::copy(tables[idx].data(), tables[idx].data() + waveLength + 1, dest);
            setTable("phaseTable", tables[idx].data(), int(tables[idx].size()), uM);
            writing = false;
        }
        
        int getTableLength(const Identifier& identifier) {
            if (identifier.toString() == "waveTable" || identifier.toString() == "gainTable") {
                auto tVar { apvts.state.getProperty(identifier, var(MemoryBlock(waveLength))) };
                auto* data = tVar.getBinaryData();
                return static_cast<int>(data->getSize() / sizeof(float));
            }
            else return static_cast<int>(currentPhaseTable.size());
        }
        
        int getCurrentTableIndex() {  }
        
        std::vector<float> currentPhaseTable;
        WaveTableData* currentPhaseTableData;
        float waveTable[waveLength + 1], gainTable[waveLength + 1];
        APVTS& apvts;
        UM* uM;
        ValueTree tree { "XTRA" };
        std::atomic<bool> writing { false };
        Value waveTableChanged { 666 }, gainTableChanged { 666 }, phaseTableChanged { 666 };
        std::vector<WaveTableData> waveTableFiles;
        std::vector<std::vector<float>> tables;
    };
    
    ////////////////     GRAPHICS      ////////////////////////////////////////////////////////
    
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
    void setWidgets(Slider& blendKnob, Slider& driveKnob, sadistic::EmpiricalLAF& llaf, Slider& saturationKnob, Label& valueLabel, Label& suffixLabel);
    
    struct DeviantGUIHub {
        static constexpr int waveLength { WAVELENGTH }, scopeSize { SCOPESIZE };
        DeviantGUIHub(TableManager& m) : mgmt(m) {
            mgmt.getTable("gainTable", gWave, waveLength + 1);
            mgmt.getTable("waveTable", wave, waveLength + 1);
            mgmt.getTable("phaseTable", pWave, waveLength + 1);
        }
        void getData() {
            if (newGainTableHere) { mgmt.getTable("gainTable", gWave, waveLength + 1); newGainTableHere = false; }
            if (newWaveTableHere) { mgmt.getTable("waveTable", wave, waveLength + 1); newWaveTableHere = false; }
            if (newPhaseTableHere) { mgmt.getTable("phaseTable", pWave, waveLength + 1); newPhaseTableHere = false; }
        }
        float lookupGainSample(float norm) { return waveTable[norm]; }
        float lookupPhaseSample(float norm) { return gainTable[phaseTable[norm] * 0.5f + 0.5f]; }
        float wave[waveLength + 1]{}, pWave[waveLength + 1]{}, gWave[waveLength + 1]{};
        Table<float> gainTable { gWave, float(waveLength) }, waveTable { wave, float(waveLength) };
        Table<float> phaseTable { pWave, float(waveLength) };
        std::atomic<bool> newGainTableHere { false }, newWaveTableHere { false }, newPhaseTableHere { false };
        TableManager& mgmt;
    };
    
    
    
    struct DeviantScreen : Component {
        static constexpr int waveLength { WAVELENGTH };
        DeviantScreen(DeviantGUIHub& h) : hub(h) {}
        void paint(Graphics&) override { hub.getData(); }
        
        DeviantGUIHub& hub;
        ValueListener gList { hub.mgmt.gainTableChanged, [&,this]{ hub.newGainTableHere = true; this->repaint(); } };
        ValueListener wList { hub.mgmt.waveTableChanged, [&,this]{ hub.newWaveTableHere = true; this->repaint(); } };
        ValueListener pList { hub.mgmt.phaseTableChanged, [&,this]{ hub.newPhaseTableHere = true; this->repaint(); } };
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

