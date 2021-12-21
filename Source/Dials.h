#pragma once
#include "WavePad.h"
namespace sadistic {
    
    struct Dials : DeviantScreen {
        struct SadImage : Component {
            SadImage(const char* s, const int sS) : svg(s), svgSize(sS) { setInterceptsMouseClicks(false, true); }
            void paint(Graphics&) override {
                Path path;
//                                path = pathFromBinarySVG(Data::SATURATE_svg, Data::SATURATE_svgSize);
//                path.loadPathFromData(svg, size_t(svgSize));
//                g.setColour(Colours::white.darker().darker());
//                auto transform { path.getTransformToScaleToFit(getLocalBounds().toFloat(), true, Justification::centred).rotated(angle, getWidth()/2, getHeight()/2) };
//                path.applyTransform(transform);
//                g.fillPath(path);
            }
            void setRotation(float r) { angle = r; }
            const char* svg;
            const int svgSize;
            float angle { 0.f };
        };
        
        Dials(TableManager& t, int idx = 2) : DeviantScreen(t), button(makeLabel(getFxID(idx))), leftSVG(Data::SATURATE_svg, Data::SATURATE_svgSize), rightSVG(Data::SATURATE_svg, Data::SATURATE_svgSize) {
            
//            setLookAndFeel(&llaf);
            
            button.label.set(makeLabel(getFxID(idx)), Colours::black, Colours::grey.darker());
            
            leftKnob = std::make_unique<sadistic::EmpiricalSlider>(true);
            rightKnob = std::make_unique<sadistic::EmpiricalSlider>();
            blendKnob = std::make_unique<Slider>();
            
            sadistic::setWidgets(*blendKnob, *leftKnob, llaf, *rightKnob, valueLabel, suffixLabel);
            
            leftKnob->onDragStart = [&,this]() {
                showEmpiricalValue(*leftKnob, valueLabel, leftSVG);
                leftSVG.setRotation(std::powf((float)leftKnob->getValue() / 200,2)); repaint(); };
            leftKnob->onValueChange = [&,this]() {
                showEmpiricalValue(*leftKnob, valueLabel, leftSVG);
                leftSVG.setRotation(std::powf((float)leftKnob->getValue() / 200,2)); repaint(); };
            leftKnob->onDragEnd = [&,this]() { hideValue(valueLabel, leftSVG);};
            
            rightKnob->onDragStart = [&,this]() {
                showEmpiricalValue(*rightKnob, valueLabel, rightSVG);
                rightSVG.setRotation((float)rightKnob->getValue() / 10.f - 0.2f); repaint(); };
            rightKnob->onValueChange = [&,this]() {
                showEmpiricalValue(*rightKnob, valueLabel, rightSVG);
                rightSVG.setRotation((float)rightKnob->getValue() / 10.f - 0.2f); repaint(); };
            rightKnob->onDragEnd = [&,this]() { hideValue(valueLabel, rightSVG);};
            
            blendKnob->onDragStart = [&,this]() { showIntegerValue(*blendKnob, valueLabel, suffixLabel);};
            blendKnob->onValueChange = [&,this]() { showIntegerValue(*blendKnob, valueLabel, suffixLabel); };
            blendKnob->onDragEnd = [&,this]() { hideValue(valueLabel, suffixLabel);};
            
            addAllAndMakeVisible(*this, *blendKnob, *rightKnob, *leftKnob, rightSVG, leftSVG, suffixLabel, valueLabel, button);
            
            leftAttachment = std::make_unique<APVTS::SliderAttachment>(mgmt.apvts, getParamID(idx, 0), *leftKnob);
            if (effectInfo[idx].numParams > 1)
                rightAttachment = std::make_unique<APVTS::SliderAttachment>(mgmt.apvts, getParamID(idx, 1), *rightKnob);
            blendAttachment = std::make_unique<APVTS::SliderAttachment>(mgmt.apvts, "mainBlend" /* getFxID(idx) + "Blend" */, *blendKnob);
            
            //            leftKnob->setNormalisableRange({ paramInfo[idx][0].min, paramInfo[idx][0].max, 0.0, paramInfo[idx][0].skew });
            //            rightKnob->setNormalisableRange({ paramInfo[idx][1].min, paramInfo[idx][1].max, 0.0, paramInfo[idx][0].skew });
            
            hideValue(valueLabel, suffixLabel);
            leftSVG.setVisible(false);
            rightSVG.setVisible(false);
            
            rightKnob->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
            leftKnob->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
        }
        
        //        void paint(Graphics&) override {}
        
        void resized() override {
            const auto bounds { getBounds() };
            leftKnob->setMouseDragSensitivity(static_cast<int>((double)bounds.getHeight() * 0.98));
            rightKnob->setMouseDragSensitivity(static_cast<int>((double)bounds.getHeight() * 0.98));
            
            auto h { getHeight() }, w { getWidth() }, diameter { jmax(5 * w / 8, 5 * h / 4) };
            
            leftKnob->setSize(diameter, diameter);
            rightKnob->setSize(diameter, diameter);
            leftKnob->setTopRightPosition(getWidth()/2 - 50, h/2 - diameter/2);
            rightKnob->setTopLeftPosition(w/2 + 50, h/2 - diameter/2);
            Rectangle<int> r { bounds.getX(), bounds.getY(), (leftKnob->getRight() - 40) - bounds.getX(), h };
            r.reduce(0, jmax(r.getHeight()/6, r.getWidth()/6));
            valueLabel.setBounds(r);
            suffixLabel.setBounds(r.withX(rightKnob->getX() + 40));
            leftSVG.setBounds(rightKnob->getBounds().reduced(leftKnob->getWidth()/6));
            rightSVG.setBounds(rightKnob->getBounds().reduced(rightKnob->getWidth()/8));
            auto blendBounds { Rectangle<int>(static_cast<int>((float)(rightKnob->getX() - leftKnob->getRight() + getWidth()/8) * (float)getHeight()/(float)leftKnob->getHeight()), 20) };
            blendKnob->setSize(blendBounds.getWidth(), blendBounds.getHeight());
            blendKnob->setCentrePosition(bounds.getCentre().x, blendBounds.getHeight()/2);
            
            valueLabel.setFont(getSadisticFont(jmin(valueLabel.getHeight() / 2, valueLabel.getWidth() / 2)));
            suffixLabel.setFont(getSadisticFont(jmin(suffixLabel.getHeight() / 2, suffixLabel.getWidth() / 2)));
            
            rightKnob->setMouseDragSensitivity(bounds.getHeight() / 2);
            leftKnob->setMouseDragSensitivity(bounds.getHeight() / 2);
            
            auto toggleWidth { 150 }, toggleHeight { 15 };
            button.setSize(toggleWidth, toggleHeight);
            button.setCentrePosition(getWidth()/2, h - 2 * toggleHeight);
        }
        
        sadistic::EmpiricalLAF llaf;
        SadTextButton button;
        SadImage leftSVG, rightSVG;
        sadistic::TransLabel valueLabel, suffixLabel;
        std::unique_ptr<Slider> leftKnob, rightKnob, blendKnob;
        std::unique_ptr<APVTS::SliderAttachment> leftAttachment, rightAttachment, blendAttachment;
    };
    
    template<typename TableType> struct MoreDials : DeviantScreen {
        
        static constexpr int numSliders { 12 }, numPanels { 4 };
        struct Panel : Component {
            Panel(int n, EmpiricalSlider* s, EmpiricalSlider* bS) : num(n) {
                for (int i { 0 }; i < num; ++i) {
                    sliders[i] = &s[i];
                    addAndMakeVisible(sliders[i]);
                }
                sliders[num] = bS;
                addAndMakeVisible(sliders[num]);
            }
            void resized() override {
                const auto localBounds { getLocalBounds() };
                Rectangle<int> bounds[4];
                bounds[0] = getLocalBounds();
                int removal = bounds[0].getHeight()/(num+1);
                for (int i = 1; i <= num; ++i) bounds[i] = bounds[0].removeFromBottom(removal);
                for (int i { 0 }; i <= num; ++i) {
                    if(sliders[i]->isLeft) sliders[i]->setBounds(bounds[i]);
                    else sliders[i]->setBounds(bounds[i]);
                    sliders[i]->setTextBoxStyle(sliders[i]->isLeft ? Slider::TextBoxLeft : Slider::TextBoxRight, false, localBounds.getHeight()/num, localBounds.getHeight()/num);
                }
            }
            int num;
            EmpiricalSlider* sliders[4] { nullptr };
        };
        MoreDials(TableManager& m, float* g, float* p) : DeviantScreen(m), pad(g, p), pWave(p) {
            
            for(int j { 0 }; j < numPanels; ++j) {
                for(int i { 0 }; i <= panel[j].num; ++i) {
                    if(j < 2) panel[j].sliders[i]->setLookAndFeel(&lelaf);
                    else panel[j].sliders[i]->setLookAndFeel(&relaf);
                    panel[j].sliders[i]->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
                    if(j > 1) {
                        if(!panel[j].sliders[i]->isDefaultHigh) panel[j].sliders[i]->setRotaryParameters(0.0f, 5.81333f, true);
                        else panel[j].sliders[i]->setRotaryParameters(5.81333f, 0.0f, true);
                    }
                    else {
                        if(panel[j].sliders[i]->isDefaultHigh) panel[j].sliders[i]->setRotaryParameters(degreesToRadians(513.f), degreesToRadians(180.f), true);
                        else panel[j].sliders[i]->setRotaryParameters(degreesToRadians(180.f), degreesToRadians(513.f), true);
                    }
                    panel[j].sliders[i]->setDoubleClickReturnValue(true, 1.0f, ModifierKeys::altModifier);
                    panel[j].sliders[i]->setMouseDragSensitivity (50);
                    panel[j].sliders[i]->setNumDecimalPlacesToDisplay(2);
                    addAndMakeVisible(panel[j]);
                }
                addAndMakeVisible(panel[j]);
            }
            addAndMakeVisible(frame);
            knobs[1].setNormalisableRange({ knobs[1].getMinimum(), knobs[1].getMaximum(), 0.0, 0.2 });
            knobs[4].setNormalisableRange({ knobs[4].getMinimum(), knobs[4].getMaximum(), 0.0, 0.2 });
            knobs[5].setNormalisableRange({ knobs[5].getMinimum(), knobs[5].getMaximum(), 0.0, 0.6 });
            
            if (pad.table.type.contains("dynamic")) {
                phaseBox.setSelectedId(int(*mgmt.apvts.getRawParameterValue("mainWaveTableID")));
                phaseBox.setText("PHASE TABLES");
                addAndMakeVisible(phaseBox);
            }
            addAndMakeVisible(pad);
            addAllAndMakeVisible(*this, xAxisLabel, yAxisLabel, button);
            button.label.set(pad.table.buttonString, pad.table.textColour, pad.table.bgColour);
        }
        
        void resized() override {
            Rectangle<int> middle { getLocalBounds() };
            Rectangle<int> bounds[4];
            bounds[0] = middle.removeFromLeft(getWidth()/8);
            bounds[1] = bounds[0].removeFromTop(getHeight()/2);
            bounds[2] = middle.removeFromRight(getWidth()/8);
            bounds[3] = bounds[2].removeFromTop(getHeight()/2);
            for(int j { 0 }; j < numPanels; ++j) panel[j].setBounds(bounds[j]);
            frame.setBounds(getLocalBounds().reduced(getLocalBounds().getWidth()/8, 0));
            button.setBounds(middle.getX() + middle.getWidth() - 105, middle.getY() + 5, 95, 15);
            xAxisLabel.setBounds(middle.getX(), middle.getY() + 9 * middle.getHeight()/10, middle.getWidth(), middle.getHeight()/10);
            yAxisLabel.setBounds(middle.getX(), middle.getY(), middle.getHeight()/10, middle.getHeight());
            pad.setBounds(middle.reduced(20));
            phaseBox.setBounds(middle.getX() + 5,middle.getY() + 5, 150, 20);
        }
        
        Frame frame;
        WavePad<TableType> pad;
        LeftEmpiricalLAF lelaf;
        RightEmpiricalLAF relaf;
        Image   volume, filter, in, out, hz, dB;
        sadistic::TransLabel valueLabel, suffixLabel;
        EmpiricalSlider knobs[numSliders] { { true, true }, { true, true }, { false, true }, { false, true }, { true, true }, { true, true, true }, { false, true }, { false, true, true }, { true, true, true }, { false, true, true }, { true, true, true }, { false, true, true } };
        APVTS::SliderAttachment attachments[numSliders] {
            { mgmt.apvts, { pad.table.type +    "AtanDrive" },            knobs[0] },
            { mgmt.apvts, { pad.table.type +    "DeviationGate" },        knobs[1] },
            { mgmt.apvts, { pad.table.type +    "BitCrusherFloor" },      knobs[2] },
            { mgmt.apvts, { pad.table.type +    "BitCrusherDrive" },      knobs[3] },
            { mgmt.apvts, { pad.table.type +    "DeviationDrive" },       knobs[4] },
            { mgmt.apvts, { pad.table.type +    "DeviationSaturation" },  knobs[5] },
            { mgmt.apvts, {                     "filterAHigh" },          knobs[6] },
            { mgmt.apvts, {                     "filterBHigh" },          knobs[7] },
            { mgmt.apvts, { pad.table.type +    "AtanBlend" },            knobs[8] },
            { mgmt.apvts, { pad.table.type +    "BitCrusherBlend" },      knobs[9] },
            { mgmt.apvts, { pad.table.type +    "DeviationBlend" },       knobs[10] },
            { mgmt.apvts, {                     "filterBHigh" },          knobs[11] }
        };
        Panel panel[numPanels] { { 2, &knobs[4], &knobs[10] }, { 2, &knobs[0], &knobs[8] }, { 2, &knobs[6], &knobs[11] }, { 2, &knobs[2], &knobs[9] } };
        
        SadLabel xAxisLabel { pad.table.xString, false, false, 0.f };
        SadLabel yAxisLabel { pad.table.yString, true, false, -0.5f };
        SadTextButton button { pad.table.buttonString };
        SadBox phaseBox { "PHASE TABLES", mgmt };
        APVTS::ComboBoxAttachment boxAttachment { mgmt.apvts, "mainWaveTableID", phaseBox };
        float* pWave { nullptr };
    };
}
