#pragma once
#include "../../Source/SadisticUnlockform.h"
#include "../../Source/SadisticLagrange.h"

namespace sadistic {
    
    enum { numFX = 10, WAVELENGTH = 128 };
    
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
        { true, 1, 3, 0.f, 1 },
        { true, 0, 0, 0.f, 2 },
        { true, 3, 0, 0.f, 2 },
        { true, 2, 0, 0.f, 1 },
        { true, 2, 1, 0.f, 2 },
        { true, 2, 2, 0.f, 3 },
        { true, 2, 3, 0.f, 1 }
    };
    
    static constexpr std::string_view paramName[][4] {
        { "Drive" },
        { "Drive", "Floor" },
        { "Drive", "Gate", "Saturation" },
        { "Drive" },
        { "Low Cutoff", "High" },
        { "Low Cutoff", "High" },
        { "Drive" },
        { "Drive", "Floor" },
        { "Drive", "Gate", "Saturation" },
        { "Drive" },
        { "Blend", "Input Gain", "pre-Blend", "Output Gain" }
    };
    
    static constexpr std::string_view paramID[][4] {
        { "Drive" },
        { "Drive", "Floor" },
        { "Drive", "Gate", "Saturation" },
        { "Drive" },
        { "Low", "High" },
        { "Low", "High" },
        { "Drive" },
        { "Drive", "Floor" },
        { "Drive", "Gate", "Saturation" },
        { "Drive" },
        { "blend", "inputGain", "preBlend", "outputGain" }
    };
    
    static constexpr ParamInfo paramInfo[][4] {
        { { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB }, { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 2.f, 50.f, 2.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 20.f, 20000.f, 20.f, ParamInfo::Hz }, { 20.f, 20000.f, 20000.f, ParamInfo::Hz } },
        { { 20.f, 20000.f, 20.f, ParamInfo::Hz }, { 20.f, 20000.f, 20000.f, ParamInfo::Hz } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB }, { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 1.f, 1000.f, 1.f, ParamInfo::dB }, { 2.f, 50.f, 2.f, ParamInfo::dB } },
        { { 0.f, 1.f, 0.f, ParamInfo::dB } },
        { { 0.f, 100.f, 100.f, ParamInfo::dB }, { 0.f, 1.f, 1.f, ParamInfo::dB }, { 0.f, 100.f, 100.f, ParamInfo::dB }, { 0.f, 1.f, 1.f, ParamInfo::dB } }
    };
    
    static inline String getFxID(int effectIndex) { return { fxID[effectIndex].data(), fxID[effectIndex].size() }; }
    static inline String getFxName(int effectIndex) { return { fxName[effectIndex].data(), fxName[effectIndex].size() }; }
    static inline String getParamID(int eIndex, int pIndex) { return { getFxID(eIndex) + String(paramID[eIndex][pIndex].data(), paramID[eIndex][pIndex].size()) }; }
    static inline String getParamName(int eIndex, int pIndex) { return { getFxName(eIndex) + " " + String(paramName[eIndex][pIndex].data(), paramName[eIndex][pIndex].size()) }; }
    
    template <typename Param>
    static inline Param& addToLayout (APVTS::ParameterLayout& layout, int effectIndex, int paramIndex) {
        ParamInfo info = paramInfo[effectIndex][paramIndex];
        String pID { getParamID(effectIndex, paramIndex) }, pName{ getParamName(effectIndex, paramIndex) };
        std::unique_ptr<AudioParameterFloat> param { nullptr };
        switch (info.type) {
            case ParamInfo::dB:
                param = std::make_unique<AudioParameterFloat>(pID, pName, NormalisableRange<float>(info.min, info.max), info.defaultValue, translate (" dB"), AudioProcessorParameter::genericParameter, [](float value, int) { return String (value, 1) + " dB"; }, [](String text) { return text.dropLastCharacters(3).getFloatValue(); });
            case ParamInfo::Hz:
                param = std::make_unique<AudioParameterFloat>(pID, pName, NormalisableRange<float>(info.min, info.max), info.defaultValue, translate(" Hz"));
            default:
                param = std::make_unique<AudioParameterFloat>(pID, pName, NormalisableRange<float>(info.min, info.max), info.defaultValue);
        }
        auto& ref = *param;
        layout.add(std::move(param));
        return ref;
    }
    
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
//        GainTable(FloatType* t) : table(t) {}
        GainTable() = default;
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
        PhaseTable() {
//            table = &t[0];
            filter.coefficients = *makeBandpass<FloatType>(one, 5000, 44100.0, phaseFilterOrder, WindowingFunction<FloatType>::hann);
        }
        FloatType operator[](int idx) const {
            jassert (isPositiveAndBelow (idx, waveLength + 1));
            return table[idx % waveLength]; }
//        FloatType operator[](FloatType phase) const {
//            jassert(isPositiveAndNotGreaterThan(phase, one));
//            FloatType floatIndex { jlimit(zero, one, phase) * FloatType(waveLength-1) };
//            auto i { static_cast<int>(truncatePositiveToUnsignedInt (floatIndex)) };
//            auto f { floatIndex - FloatType (i) };
//            jassert (isPositiveAndBelow (f, one));
//            auto x0 { table[i] };
//            auto x1 { table[i + 1] };
//            return jmap (f, x0, x1);
//        }
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
        FIR::Filter<FloatType> filter;
        FloatType table[waveLength + 1]{};
    };
    
    template<typename FloatType> struct PadGainTable {
        static constexpr int waveLength { WAVELENGTH };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<FloatType>::pi }, halfPi { MathConstants<FloatType>::halfPi }, twoPi { MathConstants<FloatType>::twoPi };
        PadGainTable(FloatType* t) : table(t) {}
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
        FloatType* table;
    };
    
    template<typename FloatType> struct PadPhaseTable {
        static constexpr int waveLength { WAVELENGTH }, phaseFilterOrder { 8 };
        static constexpr FloatType zero { static_cast<FloatType>(0) }, one { static_cast<FloatType>(1) }, two { static_cast<FloatType>(2) }, half { one / two }, quarter { half / two }, pi { MathConstants<FloatType>::pi }, halfPi { MathConstants<FloatType>::halfPi }, twoPi { MathConstants<FloatType>::twoPi };
        PadPhaseTable(FloatType* t) : table(t) {
            table = &t[0];
            filter.coefficients = *makeBandpass<FloatType>(one, 5000, 44100.0, phaseFilterOrder, WindowingFunction<FloatType>::hann);
        }
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
        FIR::Filter<FloatType> filter;
        FloatType* table;
    };
    
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
    

    
    
    ////////////////     GRAPHICS      ////////////////////////////////////////////////////////
    
    struct Numbers {
        static const char*   zero_svg, * one_svg, * two_svg, * three_svg, * four_svg, * five_svg, * six_svg, * seven_svg, * eight_svg, * nine_svg;
        static const int     zero_svgSize, one_svgSize, two_svgSize, three_svgSize, four_svgSize, five_svgSize, six_svgSize, seven_svgSize, eight_svgSize, nine_svgSize;
    };
    
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
    
    struct DeviantScreen : Component {
        static constexpr int waveLength { WAVELENGTH };
        DeviantScreen(APVTS& a, UndoManager* uM) : apvts(a), undoManager(uM) {}
        virtual ~DeviantScreen() = default;
        virtual void updateWaveTable() = 0;
        virtual void init() = 0;
        std::atomic<bool> newDataHere { false };
        float wave[waveLength + 1]{}, pWave[waveLength + 1]{};
        PadGainTable<float> gainTable { wave };
        PadPhaseTable<float> phaseTable { pWave };
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

