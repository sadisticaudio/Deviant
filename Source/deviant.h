#pragma once
#if (SADISTIC_PRO == 1)
#include "../../Source/SadisticUnlockform.h"
#else
#include "../sadistic/SadisticUnlockform.h"
#endif

namespace sadistic {
    
    enum { wetSignal = 0, drySignal, numSignals };
    enum { maxParams = 5, numFX = 5, maxCoeffs = 8 };
    enum { GAINLENGTH = 256 };
    
    struct EffectInfo { bool defaultEnabled; int defaultRoute; int defaultIndex; float defaultBlend; const char* name; const char* id; int numParams; };

    constexpr EffectInfo effectInfo[] {
        { true, 2, 0, 1.f, "Static Atan", "staticAtan" },
        { true, 2, 1, 1.f, "Static Bit Crusher", "staticBitCrusher" },
        { true, 2, 2, 1.f, "Static Clipper", "staticClipper" },
        { true, 2, 3, 1.f, "Static Logistic", "staticLogistic" },
        { true, 2, 4, 1.f, "Static Hyperbolic", "staticHyperbolic" },
        { true, 4, 0, 1.f, "Main", "main" }
    };
    
    constexpr ParamInfo paramInfo[][maxParams] {
        { { ParamInfo::NA, 0.f, 111.f, 0.f, "Drive", "Drive" },
            { ParamInfo::Hz, 20.f, 20000.f, 20.f, "Low Cutoff", "Low" },
            { ParamInfo::Hz, 20.f, 20000.f, 20000.f, "High Cutoff", "High" },
            { ParamInfo::NA, 0.f, 100.f, 0.f, "Deviation", "Deviation" } },
        { { ParamInfo::NA, 0.f, 111.f, 0.f, "Drive", "Drive" },
            { ParamInfo::Hz, 20.f, 20000.f, 20.f, "Low Cutoff", "Low" },
            { ParamInfo::Hz, 20.f, 20000.f, 20000.f, "High Cutoff", "High" },
            { ParamInfo::NA, 0.f, 100.f, 0.f, "Deviation", "Deviation" } },
        { { ParamInfo::NA, 0.f, 111.f, 0.f, "Drive", "Drive" },
            { ParamInfo::Hz, 20.f, 20000.f, 20.f, "Low Cutoff", "Low" },
            { ParamInfo::Hz, 20.f, 20000.f, 20000.f, "High Cutoff", "High" },
            { ParamInfo::NA, 0.f, 100.f, 0.f, "Deviation", "Deviation" } },
        { { ParamInfo::NA, 0.f, 111.f, 0.f, "Drive", "Drive" },
            { ParamInfo::Hz, 20.f, 20000.f, 20.f, "Low Cutoff", "Low" },
            { ParamInfo::Hz, 20.f, 20000.f, 20000.f, "High Cutoff", "High" },
            { ParamInfo::NA, 0.f, 100.f, 0.f, "Deviation", "Deviation" } },
        { { ParamInfo::NA, 0.f, 111.f, 0.f, "Drive", "Drive" },
            { ParamInfo::Hz, 20.f, 20000.f, 20.f, "Low Cutoff", "Low" },
            { ParamInfo::Hz, 20.f, 20000.f, 20000.f, "High Cutoff", "High" },
            { ParamInfo::NA, 0.f, 100.f, 0.f, "Deviation", "Deviation" } },
        { { ParamInfo::Pct, 0.f, 1.f, 1.f, "Blend", "Blend" } }
    };
    
    constexpr int getNumParamsForEffect(int effectIndex) { return sizeof(paramInfo[effectIndex]) / sizeof(ParamInfo); }
    
    inline String getFxID(int effectIndex) { return { effectInfo[effectIndex].id }; }
    inline String getFxName(int effectIndex) { return { effectInfo[effectIndex].name }; }
    inline String getParamID(int eIndex, int pIndex) { return { getFxID(eIndex) + String(paramInfo[eIndex][pIndex].id) }; }
    inline String getParamName(int eIndex, int pIndex) { return { getFxName(eIndex) + " " + String(paramInfo[eIndex][pIndex].name) }; }
    inline String makeLabel(String name, String label = String()) {
        for (int i { 0 }; i < name.length(); ++i) { label += name[i]; if (i != name.length() - 1) label += " "; }
        return label.toUpperCase(); }
    inline String getSuffix(ParamInfo::ParamType pT) { return pT == ParamInfo::dB ? "dB" : pT == ParamInfo::Hz ? "Hz" : pT == ParamInfo::Pct ? "%" : pT == ParamInfo::NA ? "" : ""; }
    
    static inline const Colour wetSignalColour { Colour::fromFloatRGBA(0.f, 0.9f, 0.9f, 1.f) }, drySignalColour { Colour::fromFloatRGBA(0.8f, 0.9f, 0.1f, 1.f) };
    
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
    
    struct Wanderer {
        void startNewJaunt() {
            jauntLength = std::rand() % (maxLength - minLength) + minLength;
            currentMaxMult = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * (maxMult - minMult) + minMult;
            goingOutward = true;
            goingUp = goingUp ? false : true;
        }
        float getWanderingValue(float val) {
            float returnVal { val };
            if (val != 0.f) {
                if (jauntCounter == 0)
                    startNewJaunt();
                else {
                    if (jauntCounter == jauntLength)
                        goingOutward = false;
                    float mult = powf(currentMaxMult, float(jauntCounter) / float(jauntLength));
                    jassert(mult != 0.f);
                    if (goingUp)
                        returnVal = jlimit(0.f, 1.f, val * mult);
                    else
                        returnVal = jlimit(0.f, 1.f, val / mult);
                }
                jauntCounter += (goingOutward ? 1 : -1);
            }
            return returnVal;
        }
        void setDeviation(float deviation) {
            if (deviation == 0.f) jauntCounter = 0;
            else {
                float normalizedDeviation { deviation / 100.f };
                minLength = 400 - static_cast<int>(395.f * normalizedDeviation);
                maxLength = 600 + static_cast<int>(400.f * normalizedDeviation);
                if (normalizedDeviation > 0.75f)
                    maxLength = 900 - static_cast<int>(890.f * ((normalizedDeviation - 0.75f) * 4.f));
                maxMult = 1.1f + 5.f * normalizedDeviation;
            }
        }
        int minLength { 400 }, maxLength { 600 }, jauntCounter { 0 }, jauntLength { minLength + (minLength + maxLength) / 2 };
        float minMult { 1.01f }, maxMult { 1.1f }, currentMaxMult { minMult + (minMult + maxMult) / 2.f };
        bool isWandering { false }, goingOutward { false }, goingUp { false };
    };
    
    class DeviantEffect {
    public:
        DeviantEffect(String eID, ParamList refs, FloatParamList floatRefs, int eIDX) : effectID(eID), effectIndex(eIDX), defaults(refs), params(floatRefs) {}
        DeviantEffect(DeviantEffect& other) : effectID(other.effectID), effectIndex(other.effectIndex), defaults(other.defaults), params(other.params) {}
        virtual ~DeviantEffect() {}
        virtual void prepare(const ProcessSpec&) {}
        virtual void reset() {}
        virtual void calculateCoefficients() {}
        virtual void processSamples(AudioBuffer<float>&) = 0;
        virtual void processSamples(AudioBuffer<double>&) = 0;
        template<typename F> void process(AudioBuffer<F>& buffer) {
            if (isEnabled()) { if (parametersNeedCooking()) cookParameters(); processSamples(buffer); } }
        virtual int getLatency() const          { return 0; }
        auto& getRange(size_t idx) const        { return params[idx].get().getNormalisableRange(); }
        bool isEnabled() const                  { return static_cast<AudioParameterBool&>(defaults[0].get()).get(); }
        int getRoute() const                    { return static_cast<AudioParameterInt&>(defaults[1].get()).get(); }
        int getIndex() const                    { return static_cast<AudioParameterInt&>(defaults[2].get()).get(); }
        float getBlend() const                  { return static_cast<AudioParameterFloat&>(defaults[3].get()).get(); }
        float getParam(size_t idx) const        { return params[idx].get().get(); }
        float getNormal(size_t idx) const       { return getRange(idx).convertTo0to1(getParam(idx)); }
        void setMagnitudeCoefficient(float mag) { coeffs[5] = mag; }
        void init() {
            for (size_t i { 0 }; i < params.size(); ++i) coeffs[i] = getParam(i);
            coeffs[7] = getBlend();
            calculateCoefficients();
        }
        bool parametersNeedCooking() const {
            bool needsUpdate { false };
            for (size_t i { 0 }; i < params.size(); ++i) if (coeffs[i] != getParam(i)) needsUpdate = true;
            if (coeffs[7] != getBlend()) needsUpdate = true;
            if (coeffs[3] != 0.f) needsUpdate = true;
            return needsUpdate;
        }
        void cookParameters() {
            for (size_t i { 0 }; i < params.size(); ++i) {
                if (getParam(i) != coeffs[i]) {
                    const float maxDelta { (getRange(i).end - getRange(i).start) / maxDeltaDivisor };
                    coeffs[i] = jlimit(coeffs[i] - maxDelta, coeffs[i] + maxDelta, getParam(i));
                }
            }
            if (getBlend() != coeffs[7])
                coeffs[7] = jlimit(coeffs[7] - (1.f / maxDeltaDivisor), coeffs[7] + (1.f / maxDeltaDivisor), getBlend());
            calculateCoefficients();
        }
        float coeffs[maxCoeffs] { 0.f, 20.f, 20000.f, 0.f, 0.f, 1.f, 1.f, 1.f };
    private:
        String effectID;
        static constexpr float maxDeltaDivisor { 50.f };
        int effectIndex;
        ParamList defaults;
        FloatParamList params;
    };
    
    ////////////////     GRAPHICS      ////////////////////////////////////////////////////////

    struct DeviantSlider : public Slider   {
        void mouseEnter(const MouseEvent&) override { showMouseOver(); repaint(); }
        void mouseExit(const MouseEvent&) override { hideMouseOver(); repaint(); }
        void setMouseOverLabels(String left, String right) { mouseOverLeft = left; mouseOverRight = right; }

        String mouseOverLeft, mouseOverRight;
        std::function<void ()> showMouseOver { []{} }, hideMouseOver { []{} };
    };
    
    struct FilterKnob : public DeviantSlider {
        FilterKnob(const char* svg) : icon(makeIcon(svg)) {
            icon->replaceColour(Colours::blue, Colours::blue.withAlpha(0.f));
            setColour(Colours::grey.darker());
        }
        void setColour(Colour c) { icon->replaceColour(colour, c); colour = c; }
        void mouseEnter(const MouseEvent& e) override { setColour(Colours::white.darker()); DeviantSlider::mouseEnter(e); }
        void mouseExit(const MouseEvent& e) override { setColour(Colours::grey.darker()); DeviantSlider::mouseExit(e); }
        
        Colour colour { Colours::black };
        std::unique_ptr<Drawable> icon;
    };

    template <typename FloatType> String freqString(FloatType val) {
        String returnString = String(val,2);
        if (val >= 1000) {
            int i { static_cast<int>(val) };
            int first =(int)(i / 1000);
            auto remainder = i - first * 1000;
            if (i > 10000 || (int)(i / 1000) == (float) i / 1000.f)
                returnString = String(String(i).dropLastCharacters(3));
            else
                returnString = String(String(i).dropLastCharacters(3) + ((remainder < 100) ? "" : ".") + String(remainder).dropLastCharacters(2));
            return { returnString + "K" };
        }
        else if (val >= 10) return String(val,1).dropLastCharacters(2);
        return returnString;
    }
    struct EmpiricalLAF : public LookAndFeel_V4 {
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
    struct FilterLAF : public LookAndFeel_V4 {
        void drawRotarySlider (Graphics&, int, int, int, int, float, float, float, Slider&) override;
        void drawLinearSlider (Graphics&, int, int, int, int, float, float, float, const Slider::SliderStyle, Slider&) override; };

    struct EmpiricalSlider : public DeviantSlider {
        EmpiricalSlider(bool l = false, bool s = false, bool h = false) : isLeft(l), isSmall(s), isDefaultHigh(h) {}
        bool hitTest (int x, int y) override;
        float getNormalisedValue() {
            auto range { NormalisableRange<double>(getRange()) };
            return static_cast<float>(range.convertTo0to1(getValue())); }
        
        bool isLeft, isSmall, isDefaultHigh;
    };

    class TransLabel : public Label { public: bool hitTest (int x, int y) override; };
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

    class SadButton : public Button {
    public:
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
    private:
        Colour colour { Colours::grey };
        const bool isLeft;
    };
    
    class SadLabel : public Component {
    public:
        SadLabel(String text={}, bool iV={}, float r={}) : isVertical(iV), angle(r) {
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
            if(!isVertical)
                label.setBounds(bounds);
            else {
                label.setBounds(Rectangle<int>(bounds.getHeight(), bounds.getWidth()).withCentre(bounds.getCentre()));
                label.setTransform(AffineTransform::rotation(MathConstants<float>::pi * angle, label.getBounds().getCentreX(), label.getBounds().getCentreY()));
            }
        }
        Label label;
    private:
        const bool isVertical;
        float angle;
        std::unique_ptr<Drawable> icon;
    };
    
    struct SadTextButton : public TextButton {
        SadTextButton(const String& s, bool iV={}, bool iL={}, float r={}) : label(s, iV, r), isLeft(iL) {
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
} // namespace sadistic
