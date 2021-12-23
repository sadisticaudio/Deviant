#pragma once
#include "../../Source/SadisticUnlockform.h"
#include "../../Source/SadisticLagrange.h"

namespace sadistic {
    
    enum { wetSignal = 0, drySignal, numSignals };
    enum { maxCoeffs = 8 , numFX = 10, WAVELENGTH = 2048, GAINLENGTH = 256 };
    enum { matrix, dials, staticPad, dynamicPad, numDisplays };

    using namespace std::chrono;
    using hi_res = high_resolution_clock;
    
    static inline const Identifier staticIdentifier { "static" }, dynamicIdentifier { "dynamic" }, neitherIdentifier { "n" };
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
        "Main"
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
        "main"
    };
    
    static constexpr EffectInfo effectInfo[] {
        { true, 1, 0, 0.f, 1 },
        { true, 1, 1, 0.f, 2 },
        { true, 1, 2, 0.f, 3 },
        { true, 1, 3, 1.f, 2 },
        { true, 0, 0, 1.f, 2 },
        { true, 3, 0, 1.f, 2 },
        { true, 2, 0, 0.f, 1 },
        { true, 2, 1, 0.f, 2 },
        { true, 2, 2, 0.f, 3 },
        { true, 2, 3, 1.f, 1 },
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
        { "Blend", "CurrentScreen", "WaveTableID", "OutputGain" }
    };
    
    static constexpr ParamInfo paramInfo[][4] {
        { { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB }, { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 2.f, 50.f, 2.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB }, { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 20.f, 20000.f, 20.f, ParamInfo::Hz }, { 20.f, 20000.f, 2000.f, ParamInfo::Hz } },
        { { 20.f, 20000.f, 20.f, ParamInfo::Hz }, { 20.f, 20000.f, 10000.f, ParamInfo::Hz } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB }, { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 2.f, 50.f, 2.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 0.f, 1.f, 1.f, ParamInfo::dB }, { 0.f, 10.f, 1.f, ParamInfo::dB }, { 0.f, 1.f, 1.f, ParamInfo::Int }, { 0.f, 1.f, 1.f, ParamInfo::dB } }
    };
    
    static inline String getFxID(int effectIndex) { return { fxID[effectIndex].data(), fxID[effectIndex].size() }; }
    static inline String getFxName(int effectIndex) { return { fxName[effectIndex].data(), fxName[effectIndex].size() }; }
    static inline String getParamID(int eIndex, int pIndex) { return { getFxID(eIndex) + String(paramID[eIndex][pIndex].data(), paramID[eIndex][pIndex].size()) }; }
    static inline String getParamName(int eIndex, int pIndex) { return { getFxName(eIndex) + " " + String(paramName[eIndex][pIndex].data(), paramName[eIndex][pIndex].size()) }; }
    String inline makeLabel(String name, String label = String()) {
        for (int i { 0 }; i < name.length(); ++i) { label += name[i]; if (i != name.length() - 1) label += " "; }
        return label.toUpperCase(); }
    String inline getSuffix(ParamInfo::ParamType pT) { return pT == dB ? "dB" : pT == Hz ? "Hz" : "%"; }
    
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
    
    template<typename F> inline F rround(F f) { return f > F(0) ? floor (f + F(0.5)) : ceil(f - F(0.5)); }
    template<typename F> inline F fastatan( F x ) { return (F(2)/MathConstants<F>::pi) * atan(x * MathConstants<F>::halfPi); }
    
    template<typename F> inline F processAtan (const F sample, const float (&coeffs)[maxCoeffs]) {
        const auto& [drive, noName1, noName2, noName3, noName4, noName5, attenuation, blend] = coeffs;
        const auto x { static_cast<float>(sample) };
        return fastatan(x * (1.f + drive * 14.f)) * attenuation * blend + x * (1.f - blend);
    }
    
    template<typename F> inline F crushSample (const F sample, const float (&coeffs)[maxCoeffs]) {
        const auto& [drive, floor, max, noName3, noName4, noName5, attenuation, blend] = coeffs;
        const auto x { static_cast<float>(sample) };
        return (rround((x + 1.f) * max) / max - 1.f) * attenuation * blend + x * (1.f - blend);
    }
    
    template<typename F> inline F deviateSample(const F sample, const float (&coeffs)[maxCoeffs]) {
        const auto& [drive, gate, saturation, gateOffset, noName4, noName5, attenuation, blend] = coeffs;
        const auto x { static_cast<float>(sample) };
        return (-1.f - gateOffset + (2.f/(1.f + (1.f / gate) * powf(expf(-saturation * x),(drive))))) * attenuation * blend + x * (1.f - blend);
    }
    
    template<typename Cs, typename F>
    F shapeSample(const Cs& aC, const Cs& cC, const Cs& dC, F sample, F blend1 = 1.f, F blend2 = 1.f, F mag = 1.f) {
        return blend1 * blend2 * mag * deviateSample(processAtan(crushSample(sample/mag, cC), aC), dC) + (F(1) - blend1 * blend2) * sample; }
    
    template<typename Cs, typename F>
    void shapeSamples(const Cs& aC, const Cs& cC, const Cs& dC, F* samples, int len, F blend1 = 1.f, F blend2 = 1.f, F mag = 1.f) {
        for (int i { 0 }; i <= len; ++i)
            samples[i] = blend1 * blend2 * mag * deviateSample(processAtan(crushSample(samples[i]/mag, cC), aC), dC) + (F(1) - blend1 * blend2) * samples[i]; }
    
    template<typename F> struct Table {
        static constexpr F zero { static_cast<F>(0) }, one { static_cast<F>(1) }, two { static_cast<F>(2) }, half { one / two };
        F operator[](int idx) const { return table[idx]; }
        F operator[](F sample) const {
            F floatIndex { jlimit(zero, one, slope * sample + intercept) * waveLength };
            auto i { truncatePositiveToUnsignedInt (floatIndex) };
            F f { floatIndex - F (i) };
            jassert (isPositiveAndNotGreaterThan (f, one));
            F x0 { table[i] }, x1 { table[i + 1] };
            return jmap (f, x0, x1); }
        F* table { nullptr };
        const F waveLength { WAVELENGTH }, slope { one }, intercept { zero };
    };
    
    struct TableManager {
        using UM = UndoManager;
        static constexpr int waveLength { WAVELENGTH }, gainLength { GAINLENGTH };
        struct WaveTableData {
            File getFile() const { return getSadisticFolder() + (builtIn ? "/Wave Tables/Stock/" : "/Wave Tables/User/") + id; }
            String id; int index; int numSamples; bool builtIn; };
        
        TableManager(APVTS& a, std::atomic<int>* cI, float(& cS)[maxCoeffs][maxCoeffs][maxCoeffs], float* pF, double* pD) : apvts(a), ptDouble(pD), ptFloat(pF), coeffIdx(cI), coeffs(cS) {
            for (size_t i { 0 }; i < Wave<float>::numWaves; ++i) {
                String id { String(Wave<float>::waveID[i].data(), Wave<float>::waveID[i].size()) };
                waveTableFiles.push_back({ id, static_cast<int>(i), waveLength + 1, true });
                auto& t { tables.emplace_back(waveLength + 1, 0.f) };
                Wave<float>::fillTable(t.data(), waveLength, Wave<float>::Type(i), true, true);
            }
            selectTable(1);
            
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
                    waveTableFiles.push_back({ stockTables[i].id.dropLastCharacters(4), int(Wave<float>::numWaves + i), int(numSamples/waveLength), true });
                    auto* samples { bufferPtr->getReadPointer(0) };
                    std::copy(samples, samples + numSamples, t.begin());
                }
            }
        }
        
        WaveTableData stockTables[2] {
            { "MATRIXYC64_wav", {}, {}, {} },
            { "cycle2048_wav", {}, {}, {} } };
        
        template <typename F> void makeStaticTable(F* dest) {
            const auto& atanCoeffs { coeffs[3][int(coeffIdx[3])] };
            const auto& crusherCoeffs { coeffs[4][int(coeffIdx[4])] };
            const auto& deviationCoeffs { coeffs[5][int(coeffIdx[5])] };
            float blend { *apvts.getRawParameterValue("staticWaveShaperBlend") };
            makeTable(atanCoeffs, crusherCoeffs, deviationCoeffs, waveTable, blend);
            for (size_t i { 0 }; i <= size_t(gainLength); ++i) dest[i] = static_cast<F>(waveTable[i]);
            newGUIDataHere = true;
        }
        
        template <typename F> void makeDynamicTable() {
            const auto& atanCoeffs { coeffs[0][int(coeffIdx[0])] };
            const auto& crusherCoeffs { coeffs[1][int(coeffIdx[1])] };
            const auto& deviationCoeffs { coeffs[2][int(coeffIdx[2])] };
            float blend { *apvts.getRawParameterValue("dynamicWaveShaperBlend") };
            makeTable(atanCoeffs, crusherCoeffs, deviationCoeffs, gainTable, blend);
//            for (size_t i { 0 }; i <= size_t(gainLength); ++i) dest[i] = static_cast<F>(gainTable[i]);
            newGUIDataHere = true;
        }
        
        template<typename COEFFS, typename F>
        void makeTable(const COEFFS& aC, const COEFFS& cC, const COEFFS& dC, F* table, float shaperBlend) {
            for (int i { 0 }; i <= gainLength; ++i) {
                const auto blend { static_cast<F>(shaperBlend * *apvts.getRawParameterValue("mainBlend")) };
                const auto gainSample { static_cast<F>(-1.f + 2.f * float(i) / float(gainLength)) };
                table[i] = blend * deviateSample(processAtan(crushSample(gainSample, cC), aC), dC) + (F(1) - blend) * gainSample;
            }
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
                const int newIndex { static_cast<int>(tables.size()) };
                auto& t { tables.emplace_back(size_t(numSamples), 0.f) };
                waveTableFiles.push_back({ fileName, newIndex, numSamples/waveLength, true });
                auto* samples { bufferPtr->getReadPointer(0) };
                std::copy(samples, samples + numSamples, t.begin());
                selectTable(size_t(newIndex));
                return true;
            }
            return false;
        }

        template<typename F> void getTable(const Identifier& id, F* samples, int numSamples) const {
            if (id == waveTableID) for (int i { 0 }; i < numSamples; ++i) samples[i] = static_cast<F>(waveTable[i]);
            if (id == gainTableID) for (int i { 0 }; i < numSamples; ++i) samples[i] = static_cast<F>(gainTable[i]);
            else if (id == phaseTableID) {
                for (int i { 0 }; i < numSamples; ++i) samples[i] = F(currentPhaseTable[(size_t) i]);
            }
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
            const int numSamples { static_cast<int>(currentPhaseTable.size()) };
            float* mockChannel[1] { currentPhaseTable.data() };
            const AudioBuffer<float> buffer { mockChannel, 1, numSamples };
            return saveTable(outputFile, buffer);
        }
        
        bool saveTable(File& outputFile, const AudioBuffer<float>& buffer) {
            StringPairArray metadataValues = WavAudioFormat::createBWAVMetadata ("Custom WaveTable", "originator", "originatorRef", Time::getCurrentTime(), buffer.getNumChannels(), "codingHistory");
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

        void selectTable(size_t idx) {
            currentPhaseTable = tables[idx];
            std::copy(currentPhaseTable.begin(), currentPhaseTable.begin() + waveLength + 1, ptFloat);
            for (size_t i { 0 }; i <= size_t(waveLength); ++i) ptDouble[i] = static_cast<double>(currentPhaseTable[i]);
        }
        
        int getTableLength(const Identifier& identifier) {
            if (identifier == waveTableID || identifier == gainTableID)
                return gainLength + 1;
            else return static_cast<int>(currentPhaseTable.size());
        }
        APVTS& apvts;
        static inline const Identifier waveTableID { "waveTable" }, gainTableID { "gainTable" }, phaseTableID { "phaseTable" };
        std::vector<float> currentPhaseTable;
        WaveTableData* currentPhaseTableData;
        float waveTable[gainLength + 1], gainTable[gainLength + 1];
        double* ptDouble;
        float* ptFloat;
        std::atomic<bool> newGUIDataHere { true };
        std::vector<WaveTableData> waveTableFiles;
        std::vector<std::vector<float>> tables;
        std::atomic<int>* coeffIdx;
        float(& coeffs)[maxCoeffs][maxCoeffs][maxCoeffs];
    };
    
    struct DeviantEffect {
        DeviantEffect(String eID, ParamList refs, FloatParamList floatRefs, int eIDX, TableManager& t) : mgmt(t), effectID(eID), shaperType(eID.contains("static") ? staticIdentifier : eID.contains("dynamic") ? dynamicIdentifier : neitherIdentifier), effectIndex(eIDX), defaults(refs), params(floatRefs) {}
        DeviantEffect(DeviantEffect& other) : mgmt(other.mgmt), effectID(other.effectID), shaperType(other.effectID.contains("static") ? staticIdentifier : other.effectID.contains("dynamic") ? dynamicIdentifier : neitherIdentifier), effectIndex(other.effectIndex), defaults(other.defaults), params(other.params) {}
        virtual ~DeviantEffect() {}
        bool operator<(const DeviantEffect& other) {
            if (getRoute() < other.getRoute()) return true;
            else if (getRoute() == other.getRoute() && getIndex() < other.getIndex()) return true;
            return false;
        }
        virtual void prepare(const ProcessSpec&) {}
        virtual void processSamples(AudioBuffer<float>&) = 0;
        virtual void processSamples(AudioBuffer<double>&) = 0;
        void process(AudioBuffer<float>& buffer) { if (isEnabled()) { cookParameters(); processSamples(buffer); } }
        void process(AudioBuffer<double>& buffer) { if (isEnabled()) { cookParameters(); processSamples(buffer); } }
        virtual void reset() {}
        virtual void calculateCoefficients() {}
        virtual int getLatency() { return 0; }
        bool isEnabled() const   { return static_cast<AudioParameterBool&>(defaults[0].get()).get(); }
        int getRoute() const     { return static_cast<AudioParameterInt&>(defaults[1].get()).get(); }
        int getIndex() const     { return static_cast<AudioParameterInt&>(defaults[2].get()).get(); }
        float getBlend() const   { return static_cast<AudioParameterFloat&>(defaults[3].get()).get(); }
        void init() {
            for (size_t i { 0 }; i < params.size(); ++i) coeffs[i] = params[i].get().get();
            coeffs[7] = getBlend();
            calculateCoefficients();
            cookParameters();
        }
        bool cookParameters() {
            bool stillUpdating { false };
            for (size_t i { 0 }; i < params.size(); ++i) {
                const float currentValue { params[i].get().get() };
                const float smoothed { coeffs[i] };
                if (currentValue != smoothed) {
                    const float maxDelta { (params[i].get().getNormalisableRange().end - params[i].get().getNormalisableRange().start) / 20.f };
                    coeffs[i] = jlimit(smoothed - maxDelta, smoothed + maxDelta, currentValue);
                    stillUpdating = true;
                }
            }
            const float currentValue { getBlend() };
            const float smoothed { coeffs[7] };
            if (currentValue != smoothed) {
                const float maxDelta { (defaults[3].get().getNormalisableRange().end - defaults[3].get().getNormalisableRange().start) / 20.f };
                coeffs[7] = jlimit(smoothed - maxDelta, smoothed + maxDelta, currentValue);
                stillUpdating = true;
            }
            if (stillUpdating) calculateCoefficients();
            return stillUpdating;
        }
        TableManager& mgmt;
        String effectID;
        const Identifier& shaperType;
        int effectIndex;
        ParamList defaults;
        FloatParamList params;
        float coeffs[maxCoeffs];
    };
    
    ////////////////     GRAPHICS      ////////////////////////////////////////////////////////

    struct EmpiricalLAF    : public LookAndFeel_V4   {
        static constexpr auto numNumbers { 12 }, numNeedles { 112 };
        void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                               float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override;
        void drawLinearSlider (Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& slider) override; };
    
    struct LeftEmpiricalLAF : EmpiricalLAF { void drawLabel (Graphics& g, Label& label) override; };
    struct RightEmpiricalLAF : EmpiricalLAF { void drawLabel (Graphics& g, Label& label) override; };
    struct EmpiricalSlider : public Slider   {
        static constexpr int numNumbers { EmpiricalLAF::numNumbers }, numNeedles { EmpiricalLAF::numNeedles };
        EmpiricalSlider(bool l = false, bool s = false, bool h = false) : isLeft(l), isSmall(s), isDefaultHigh(h) {}
        float getNormalisedValue() {
            auto range { NormalisableRange<double>(getRange()) };
            return static_cast<float>(range.convertTo0to1(getValue())); }
        bool isLeft, isSmall, isDefaultHigh; public: bool hitTest (int x, int y) override;
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

    struct DeviantScreen : Component {
        static constexpr int gainLength { GAINLENGTH }, scopeSize { SCOPESIZE }, waveLength { WAVELENGTH };
        DeviantScreen(TableManager& h) : mgmt(h) {}
        TableManager& mgmt;
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
            p.startNewSubPath(2,height/2);
            p.addArrow({ 2, height/2, width - 2, height/2 }, 6.f, 12.f, width/2);
            if(isLeft)
                p.applyTransform(AffineTransform::rotation(MathConstants<float>::pi, bounds.getCentreX(), bounds.getCentreY()));
            g.setColour(colour);
            g.strokePath(p,PathStrokeType(2.f));
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
        std::unique_ptr<Drawable> icon;
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
        void paintButton (Graphics&, bool, bool) override {}
        Colour colour { Colours::grey };
        SadLabel label;
        const bool isLeft;
    };
    
    struct SadBox : ComboBox {
        SadBox(String name, TableManager& m) : ComboBox(name), mgmt(m) {
            auto* menu { getRootMenu() };
            size_t i { 1 };
            for (; i < mgmt.waveTableFiles.size(); ++i) {
                PopupMenu::Item item { mgmt.waveTableFiles[i].id.toUTF8() };
                item.itemID = int(i);
                item.action = { [=]{ pickTable(i); } };
                menu->addItem(item);
            }
            PopupMenu::Item itemLoad { "Load Table" };
            itemLoad.itemID = int(i++);
            itemLoad.action = { [&,this](){ loadFile(); } };
            menu->addItem(itemLoad);
            PopupMenu::Item itemSave { "Load Table" };
            itemSave.itemID = int(i++);
            itemSave.action = { [&,this](){ saveFile(); } };
            menu->addItem(itemSave);
            saveCallback = [&,this] (const FileChooser& chooser) {
                auto result { chooser.getResult() };
                if(mgmt.saveTable(result)) juce::AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, "Table Saved...", ""); };
            saveFile = [&,this](){
                fc = std::make_unique<FileChooser>("Save Wave Table", File(), "*.wav");
                fc->launchAsync (FileBrowserComponent::saveMode |
                                 FileBrowserComponent::canSelectFiles |
                                 FileBrowserComponent::warnAboutOverwriting |
                                 FileBrowserComponent::doNotClearFileNameOnRootChange,
                                 saveCallback); };
            loadCallback = [&,this] (const FileChooser& chooser) {
                if (!chooser.getResults().isEmpty()) {
                    if (mgmt.loadTable(chooser.getResult()))
                        juce::AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, "Table Loaded", "!");
                    else juce::AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "File not loaded correctly", "!");
                }
                else juce::AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "File not choosen correctly", "!"); };
            loadFile = [&,this](){
                fc = std::make_unique<FileChooser>("Load Wave Table (.wav)", File(), "*.wav");
                fc->launchAsync (FileBrowserComponent::openMode |
                                 FileBrowserComponent::canSelectFiles,
                                 loadCallback); };
        }
        void pickTable(size_t idx) {
            mgmt.apvts.getParameter("mainWaveTableID")->setValue(int(idx));
            mgmt.selectTable(idx);
            auto* parent { getParentComponent() };
            parent->repaint();
        }
        TableManager& mgmt;
        std::unique_ptr<FileChooser> fc;
        std::function<void()> loadFile, saveFile;
        std::function<void(int)> selectTable;
        std::function<void (const FileChooser&)> loadCallback, saveCallback;
    };
}

