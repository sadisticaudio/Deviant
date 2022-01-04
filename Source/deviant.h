#pragma once
#if (SADISTIC_PRO == 1)
#include "../../Source/SadisticUnlockform.h"
#else
#include "SadisticUnlockform.h"
#endif


namespace sadistic {
    
    enum { wetSignal = 0, drySignal, numSignals };
    enum { maxCoeffs = 8, numFX = 5 };
    enum { GAINLENGTH = 256 };
    
    static inline const Colour wetSignalColour { Colour::fromFloatRGBA(0.f, 0.9f, 0.9f, 1.f) }, drySignalColour { Colour::fromFloatRGBA(0.8f, 0.9f, 0.1f, 1.f) };

    struct IDs {
        static inline const Identifier currentScreen { "currentScreen" };
        static inline const Identifier staticIdentifier { "static" };
        static inline const Identifier dynamicIdentifier { "dynamic" };
        static inline const Identifier neitherIdentifier { "neither" };
    };
    struct EffectInfo { bool defaultEnabled; int defaultRoute; int defaultIndex; float defaultBlend; int numParams; };
    
    constexpr const char* fxName[] {
        "Static Atan",
        "Static Bit Crusher",
        "Static Clipper",
        "Static Deviation",
        "Static Hyperbolic",
        "Main"
    };
    
    constexpr const char* fxID[] {
        "staticAtan",
        "staticBitCrusher",
        "staticClipper",
        "staticDeviation",
        "staticHyperbolic",
        "main"
    };
    
    constexpr EffectInfo effectInfo[] {
        { true, 2, 0, 1.f, 3 },
        { true, 2, 1, 1.f, 3 },
        { true, 2, 2, 1.f, 3 },
        { true, 2, 3, 1.f, 3 },
        { true, 2, 4, 1.f, 3 },
        { true, 4, 0, 1.f, 1 }
    };
    
    constexpr const char* paramName[][4] {
        { "Drive", "Low Cutoff", "High Cutoff" },
        { "Drive", "Low Cutoff", "High Cutoff" },
        { "Drive", "Low Cutoff", "High Cutoff" },
        { "Drive", "Low Cutoff", "High Cutoff" },
        { "Drive", "Low Cutoff", "High Cutoff" },
        { "Blend" }
    };
    
    constexpr const char* paramID[][4] {
        { "Drive", "Low", "High" },
        { "Drive", "Low", "High" },
        { "Drive", "Low", "High" },
        { "Drive", "Low", "High" },
        { "Drive", "Low", "High" },
        { "Blend" }
    };
    
    constexpr ParamInfo paramInfo[][4] {
        { { 0.f, 111.f, 0.f, ParamInfo::dB }, { 20.f, 20000.f, 20.f, ParamInfo::Hz }, { 20.f, 20000.f, 20000.f, ParamInfo::Hz } },
        { { 0.f, 111.f, 0.f, ParamInfo::dB }, { 20.f, 20000.f, 20.f, ParamInfo::Hz }, { 20.f, 20000.f, 20000.f, ParamInfo::Hz } },
        { { 0.f, 111.f, 0.f, ParamInfo::dB }, { 20.f, 20000.f, 20.f, ParamInfo::Hz }, { 20.f, 20000.f, 20000.f, ParamInfo::Hz } },
        { { 0.f, 111.f, 0.f, ParamInfo::dB }, { 20.f, 20000.f, 20.f, ParamInfo::Hz }, { 20.f, 20000.f, 20000.f, ParamInfo::Hz } },
        { { 0.f, 111.f, 0.f, ParamInfo::dB }, { 20.f, 20000.f, 20.f, ParamInfo::Hz }, { 20.f, 20000.f, 20000.f, ParamInfo::Hz } },
        { { 0.f, 1.f, 1.f, ParamInfo::dB } }
    };
    
    inline String getFxID(int effectIndex) { return { fxID[effectIndex] }; }
    inline String getFxName(int effectIndex) { return { fxName[effectIndex] }; }
    inline String getParamID(int eIndex, int pIndex) { return { getFxID(eIndex) + String(paramID[eIndex][pIndex]) }; }
    inline String getParamName(int eIndex, int pIndex) { return { getFxName(eIndex) + " " + String(paramName[eIndex][pIndex]) }; }
    inline String makeLabel(String name, String label = String()) {
        for (int i { 0 }; i < name.length(); ++i) { label += name[i]; if (i != name.length() - 1) label += " "; }
        return label.toUpperCase(); }
    inline String getSuffix(ParamInfo::ParamType pT) { return pT == dB ? "dB" : pT == Hz ? "Hz" : "%"; }
    
    template <typename Param, typename ListType, typename ...Ts>
    static void addParameter (APVTS::ParameterLayout& layout, ListType& pList, Ts... ts) {
        std::unique_ptr<Param> param = std::make_unique<Param> (std::forward<Ts> (ts)...);
        auto& ref = *param;
        layout.add(std::move(param));
        pList.emplace_back(ref);
    }
    
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
    
    struct Atan {
        static inline void calculateCoefficients(float(& coeffs)[maxCoeffs]) {
            auto& [drive, lo, hi, mDrive, noName4, mag, attenuation, blend] = coeffs;
            mDrive = powf(drive/111.f, 2.f);
            blend *= jlimit(0.f, 1.f, 4.f * drive / 111.f);
            attenuation = jmap(powf(drive/111.f, 0.08f), 1.f, 0.14f); }
        template<typename F> static inline F processSample(const F sample, const float (&coeffs)[maxCoeffs]) {
            const auto& [drive, lo, hi, mDrive, noName4, mag, attenuation, blend] = coeffs;
            const auto x { static_cast<float>(sample) };
            return fastatan(x * (1.f + mDrive * 144.f)) * attenuation * blend + x * (1.f - blend); }
    };
    
    struct Crusher {
        static inline void calculateCoefficients(float(& coeffs)[maxCoeffs]) {
            auto& [drive, lo, hi, mDrive, max, mag, attenuation, blend] = coeffs;
            mDrive = powf(((drive * 0.75f)/111.f) * 0.75f - 1.f, 16.f) * 1024.f + 1.f;
//            blend *= jlimit(0.f, 1.f, 4.f * drive / 111.f);
            max = 1.f;
            mag = 1.f;
            attenuation = 1.f - 0.3f * (drive/111.f); }
        template<typename F> static inline F processSample(const F sample, const float (&coeffs)[maxCoeffs]) {
            const auto& [drive, lo, hi, mDrive, max, mag, attenuation, blend] = coeffs;
            const auto x { static_cast<float>(sample) };
            return (rround((x + 1.f) * mDrive) / mDrive - 1.f) * attenuation * blend + x * (1.f - blend); }
    };
    
    struct Clipper {
        static inline void calculateCoefficients(float(& coeffs)[maxCoeffs]) {
            auto& [drive, lo, hi, mDrive, noName4, mag, attenuation, blend] = coeffs;
            mDrive = powf(drive/111.f, 2.f);
//            blend *= jlimit(0.f, 1.f, 4.f * drive / 111.f);
            attenuation = jmap(powf(mDrive, 0.04f), 1.f, 0.14f); }
        template<typename F> static inline F processSample(const F sample, const float (&coeffs)[maxCoeffs]) {
            const auto& [drive, lo, hi, mDrive, noName4, mag, attenuation, blend] = coeffs;
            const auto x { static_cast<float>(sample) };
            return jlimit(-1.f, 1.f, x * (1.f + mDrive * 144.f)) * attenuation * blend + x * (1.f - blend); }
    };
    
    struct Deviation {
        static inline void calculateCoefficients(float(& coeffs)[maxCoeffs]) {
            auto& [drive, lo, hi, mDrive, noName4, mag, attenuation, blend] = coeffs;
            mDrive = sadSymmetricSkew(drive/111.f, -0.96f);
//            blend *= jlimit(0.f, 1.f, 4.f * drive / 111.f);
            attenuation = jmap(powf(drive/111.f * 0.5f, 0.1f), 1.f, 0.03f); }
        template<typename F> static inline F processSample(const F sample, const float (&coeffs)[maxCoeffs]) {
            const auto& [drive, lo, hi, mDrive, noName4, mag, attenuation, blend] = coeffs;
            const auto x { static_cast<float>(sample) };
            return (-1.f + (2.f/(1.f + 1.f * powf(FastMathApproximations::exp(-2.f * x), 1.f + mDrive * 3000.f)))) * attenuation * blend + x * (1.f - blend); }
    };
    
    struct Hyperbolic {
        static inline void calculateCoefficients(float(& coeffs)[maxCoeffs]) {
            auto& [drive, lo, hi, mDrive, noName4, mag, attenuation, blend] = coeffs;
            mDrive = drive/111.f;
//            blend *= jlimit(0.f, 1.f, 4.f * drive / 111.f);
            attenuation = jmap(powf(drive/111.f, 0.08f), 1.f, 0.14f); }
        template<typename F> static inline F processSample(const F sample, const float (&coeffs)[maxCoeffs]) {
            const auto& [drive, lo, hi, mDrive, noName4, mag, attenuation, blend] = coeffs;
            const auto x { static_cast<float>(sample) };
            return FastMathApproximations::tanh(x * (1.f + powf(mDrive, 2.f) * 144.f)) * attenuation * blend + x * (1.f - blend); }
    };
    
    template<typename Cs, typename F>
    inline F shapeSample(const Cs& aC, const Cs& bC, const Cs& cC, const Cs& dC, const Cs& hC, F sample, F blend = 1.f) {
        return blend * Hyperbolic::processSample(Clipper::processSample(Deviation::processSample(Atan::processSample(Crusher::processSample(sample, bC), aC), dC), cC), hC) + (F(1) - blend) * sample; }
    
    struct DeviantEffect {
        DeviantEffect(String eID, ParamList refs, FloatParamList floatRefs, int eIDX) : effectID(eID), shaperType(eID.contains("static") ? IDs::staticIdentifier : eID.contains("dynamic") ? IDs::dynamicIdentifier : IDs::neitherIdentifier), effectIndex(eIDX), defaults(refs), params(floatRefs) {}
        DeviantEffect(DeviantEffect& other) : effectID(other.effectID), shaperType(other.effectID.contains("static") ? IDs::staticIdentifier : other.effectID.contains("dynamic") ? IDs::dynamicIdentifier : IDs::neitherIdentifier), effectIndex(other.effectIndex), defaults(other.defaults), params(other.params) {}
        virtual ~DeviantEffect() {}
        bool operator<(const DeviantEffect& other) {
            if (getRoute() < other.getRoute()) return true;
            else if (getRoute() == other.getRoute() && getIndex() < other.getIndex()) return true;
            return false;
        }
        virtual void prepare(const ProcessSpec&) {}
        virtual void processSamples(AudioBuffer<float>&) = 0;
        virtual void processSamples(AudioBuffer<double>&) = 0;
        void process(AudioBuffer<float>& buffer) { if (isEnabled()) { if (parametersNeedCooking()) cookParameters(); processSamples(buffer); } }
        void process(AudioBuffer<double>& buffer) { if (isEnabled()) { if (parametersNeedCooking()) cookParameters(); processSamples(buffer); } }
        virtual void reset() {}
        virtual void calculateCoefficients() {}
        virtual int getLatency() { return 0; }
        bool isEnabled() const   { return static_cast<AudioParameterBool&>(defaults[0].get()).get(); }
        int getRoute() const     { return static_cast<AudioParameterInt&>(defaults[1].get()).get(); }
        int getIndex() const     { return static_cast<AudioParameterInt&>(defaults[2].get()).get(); }
        float getBlend() const   { return static_cast<AudioParameterFloat&>(defaults[3].get()).get(); }
        virtual void init() {
            for (size_t i { 0 }; i < params.size(); ++i) coeffs[i] = params[i].get().get();
            coeffs[4] = 1.f;
            coeffs[5] = 1.f;
            coeffs[7] = getBlend();
            calculateCoefficients();
            cookParameters();
        }
        bool parametersNeedCooking() const {
            bool needsUpdate { false };
            for (size_t i { 0 }; i < params.size(); ++i) if (coeffs[i] != params[i].get().get()) needsUpdate = true;
            if (coeffs[7] != getBlend()) needsUpdate = true;
            return needsUpdate;
        }
        void cookParameters() {
            for (size_t i { 0 }; i < params.size(); ++i) {
                const float currentValue { params[i].get().get() };
                const float smoothed { coeffs[i] };
                if (currentValue != smoothed) {
                    const float maxDelta { (params[i].get().getNormalisableRange().end - params[i].get().getNormalisableRange().start) / maxDeltaDivisor };
                    coeffs[i] = jlimit(smoothed - maxDelta, smoothed + maxDelta, currentValue);
                }
            }
            const float currentValue { getBlend() };
            const float smoothed { coeffs[7] };
            if (currentValue != smoothed) {
                const float maxDelta { (defaults[3].get().getNormalisableRange().end - defaults[3].get().getNormalisableRange().start) / maxDeltaDivisor };
                coeffs[7] = jlimit(smoothed - maxDelta, smoothed + maxDelta, currentValue);
            }
            calculateCoefficients();
        }
        String effectID;
        const Identifier& shaperType;
        const float maxDeltaDivisor { 50.f };
        int effectIndex;
        ParamList defaults;
        FloatParamList params;
        float coeffs[maxCoeffs];
    };
    
    struct TableManager {
        using UM = UndoManager;
        static constexpr int gainLength { GAINLENGTH };
        
        TableManager(APVTS& a, std::atomic<int>* cI, float(& cS)[numFX][maxCoeffs][maxCoeffs]) : apvts(a), coeffIdx(cI), coeffs(cS) {
            for (int i { 0 }; i <= gainLength; ++i) inputTable[i] = -1.f + 2.f * float(i) / float(gainLength); }
        
        template <typename F> void makeStaticTable(F* dest = nullptr) {
            const auto& atanCoeffs { coeffs[0][int(coeffIdx[0])] };
            const auto& crusherCoeffs { coeffs[1][int(coeffIdx[1])] };
            const auto& clipperCoeffs { coeffs[2][int(coeffIdx[2])] };
            const auto& deviationCoeffs { coeffs[3][int(coeffIdx[3])] };
            const auto& hyperbolicCoeffs { coeffs[4][int(coeffIdx[4])] };
            makeTable(atanCoeffs, crusherCoeffs, clipperCoeffs, deviationCoeffs, hyperbolicCoeffs, gainTable);
            if (dest) for (size_t i { 0 }; i <= size_t(gainLength); ++i) dest[i] = static_cast<F>(gainTable[i]);
            int extremity;
            const int indexOfMax { static_cast<int>(std::distance(gainTable,std::max_element(gainTable, gainTable + gainLength + 1))) };
            const int indexOfMin { static_cast<int>(std::distance(gainTable,std::min_element(gainTable, gainTable + gainLength + 1))) };
            if(abs(gainTable[indexOfMax]) > abs(gainTable[indexOfMin])) extremity = indexOfMax;
            else extremity = indexOfMin;
            const auto mag { abs(gainTable[extremity]) }, magDB { Decibels::gainToDecibels(mag) }, mult { 1.f - -magDB/100.f };
            waveMult = powf(mult, 7.f);
            newGUIDataHere = true;
        }
        
        template<typename COEFFS, typename F>
        void makeTable(const COEFFS& aC, const COEFFS& bC, const COEFFS& cC, const COEFFS& dC, const COEFFS& hC, F* table, float shaperBlend = 1.f) {
            const auto blend { static_cast<F>(shaperBlend) };
            for (int i { 0 }; i <= gainLength; ++i) {
                const auto gainSample { static_cast<F>(-1.f + 2.f * float(i) / float(gainLength)) };
                table[i] = blend * shapeSample(aC, bC, cC, dC, hC, gainSample) + (F(1.0) - blend) * gainSample;
//                table[i] = blend * Clipper::processSample(Deviation::processSample(Atan::processSample(gainSample, aC), dC), cC) + (F(1.0) - blend) * gainSample;
            }
        }
        
        APVTS& apvts;
        float inputTable[gainLength + 1], gainTable[gainLength + 1];
        float waveMult { 1.f };
        std::atomic<bool> newGUIDataHere { true };
        std::atomic<int>* coeffIdx;
        float(& coeffs)[numFX][maxCoeffs][maxCoeffs];
    };
    
    ////////////////     GRAPHICS      ////////////////////////////////////////////////////////

    struct DeviantSlider : public Slider   {
        void mouseEnter(const MouseEvent&) override { showMouseOver(); repaint(); }
        void mouseExit(const MouseEvent&) override { hideMouseOver(); repaint(); }
        void setMouseOverLabels(String left, String right) { mouseOverLeft = left; mouseOverRight = right; }
        String mouseOverLeft, mouseOverRight;
        std::function<void ()> showMouseOver { []{} }, hideMouseOver { []{} };
    };
    
    struct FilterKnob : public DeviantSlider   {
        FilterKnob(const char* svg) : icon(makeIcon(svg)) {
            icon->replaceColour(Colours::blue, Colours::blue.withAlpha(0.f));
            setColour(Colours::grey.darker());
        }
        void setColour(Colour c) { icon->replaceColour(colour, c); colour = c; }
        void mouseEnter(const MouseEvent& e) override { setColour(Colours::white.darker()); DeviantSlider::mouseEnter(e);; }
        void mouseExit(const MouseEvent& e) override { setColour(Colours::grey.darker()); DeviantSlider::mouseExit(e); }
        void setMouseOverLabels(String left, String right) { mouseOverLeft = left; mouseOverRight = right; }
        std::unique_ptr<Drawable> icon;
        Colour colour { Colours::black };
    };
    
    template <typename T> String freqStringInt(T i) {
        if (i > 1001) {
            int first =(int)(i / 1000);
            auto remainder = i - first * 1000;
            if (i > 10000 || (int)(i / 1000) == (float) i / 1000.f) return String(String(i).dropLastCharacters(3)+ "K");
            else return String(String(i).dropLastCharacters(3)+ "." + String(remainder).dropLastCharacters(2) + "K");
        }
        else return String(i);
    }
    
    template <typename FloatType> String freqString(FloatType i) {
        String returnString = String();
        if (i < 10.0) returnString = String(i,2);
        else if (i < 1000.0) returnString = String(i,1).dropLastCharacters(2);
        else returnString = freqStringInt(static_cast<int>(i));
        return returnString;
    }
    struct EmpiricalLAF    : public LookAndFeel_V4   {
        EmpiricalLAF() {
            const char* nums[numNumbers] { Data::zero_svg, Data::one_svg, Data::two_svg, Data::three_svg, Data::four_svg, Data::five_svg, Data::six_svg, Data::seven_svg, Data::eight_svg, Data::nine_svg, Data::zero_svg, Data::one_svg };
            for (auto& one : onesPlaceNums) { one = makeIcon(Data::zero_svg); one->replaceColour(Colours::black, Colours::white); }
            for (int i { 0 }; i < numNumbers; ++i) { tensPlaceNums[i] = makeIcon(nums[i]); tensPlaceNums[i]->replaceColour(Colours::black, Colours::white); }
            hundredsPlaceNums[0] = makeIcon(Data::one_svg); hundredsPlaceNums[0]->replaceColour(Colours::black, Colours::white);
            hundredsPlaceNums[1] = makeIcon(Data::one_svg); hundredsPlaceNums[1]->replaceColour(Colours::black, Colours::white);
        }
        static constexpr auto numNumbers { 12 }, numNeedles { 112 };
        void drawRotarySlider (Graphics&, int, int, int, int, float, float, float, Slider&) override;
        std::unique_ptr<Drawable> onesPlaceNums[numNumbers], tensPlaceNums[numNumbers], hundredsPlaceNums[2];
    };
    struct FilterLAF    : public LookAndFeel_V4   {
        void drawRotarySlider (Graphics&, int, int, int, int, float, float, float, Slider&) override;
        void drawLinearSlider (Graphics&, int, int, int, int, float, float, float, const Slider::SliderStyle, Slider&) override; };

    struct EmpiricalSlider : public DeviantSlider   {
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
    template <typename SliderType> void setMouseOverLabels(SliderType& slider, String left, String right) {
        slider.setMouseOverLabels(left, right); }
    template <typename SliderType> void showMouseOverLabels(SliderType& slider, Label& label1, Label& label2) {
        label1.setText (slider.mouseOverLeft, dontSendNotification);
        label2.setText (slider.mouseOverRight, dontSendNotification); }
    template <typename SliderType> void hideMouseOverLabels(SliderType&, Label& label1, Label& label2) {
        label1.setText ("", dontSendNotification);
        label2.setText ("", dontSendNotification); }
    template <typename SliderType> void showHzValue(SliderType& slider, Label& label1, Label& label2) {
        label1.setText (String(freqString(slider.getValue()) + "Hz"), dontSendNotification);
        label2.setText (slider.getTextValueSuffix(), dontSendNotification);
    }

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
            label.label.setColour(Label::backgroundColourId, colour);
            repaint();
        }
        void mouseExit(const MouseEvent&) override {
            colour = Colours::grey;
            label.label.setColour(Label::backgroundColourId, colour);
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
    
    struct RightClickMenu : PopupMenu {
        RightClickMenu() {
            PopupMenu::Item itemLoad { "Show Info On Mouse-Over" };
            itemLoad.action = { [&] { toggleMouseOverEnabled(); } };
            addItem(itemLoad);
            PopupMenu::Item itemSave { "Show Extra Controls" };
            itemSave.action = { [&,this] { toggleControls(); } };
            addItem(itemSave);
        }
        void show(Component* comp) {
            PopupMenu::Options options;
            PopupMenu::showMenuAsync(options.withTargetComponent(comp)); }
        std::function<void()> toggleMouseOverEnabled, toggleControls;
    };
}

