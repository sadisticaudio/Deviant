#pragma once
#include "deviant.h"
namespace sadistic {
    
    struct Dials : Component {
        Dials(APVTS& t, int idx) : apvts(t), button(makeLabel(getFxName(currentEffect = idx))), driveSVG(Data::DRIVE_svg, Colours::grey) {

            button.onClick = [&] { switchEffect(++currentEffect%=numFX); button.label.label.setColour(Label::backgroundColourId, button.colour); };
            
            forEach ([] (auto& knob) {
                knob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
                knob.setDoubleClickReturnValue(true,1.0f,ModifierKeys::altModifier);
                knob.setMouseDragSensitivity (100);
                knob.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
                knob.setScrollWheelEnabled(true);
                knob.setRotaryParameters(degreesToRadians(225.f), degreesToRadians(495.f), true);
                knob.setTextValueSuffix("Hz");
            }, mainBlendKnob, driveKnob, blendKnob, lowKnob, highKnob);
            
            lowKnob.setLookAndFeel(&alaf);
            highKnob.setLookAndFeel(&alaf);
            
            mainBlendKnob.setLookAndFeel(&alaf);
            mainBlendKnob.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
            mainBlendKnob.setTextValueSuffix("%");
            
            driveKnob.setRotaryParameters(degreesToRadians(0.f), 5.81333f, true);
            blendKnob.setRotaryParameters(degreesToRadians(180.f), degreesToRadians(510.f), true);
            blendKnob.setTextValueSuffix("%");

            forEach ([] (auto& label) { label.setJustificationType(Justification::centred);
                label.setColour(juce::Label::textColourId, Colours::lightgrey); }, valueLabel, suffixLabel, lLabel, lSuffix, rLabel, rSuffix);

            driveKnob.onDragStart = [&,this]() {
                hideValue(valueLabel, suffixLabel);
                showEmpiricalValue(driveKnob, valueLabel, driveSVG);
                driveSVG.setRotation(driveKnob.getNormalisedValue() * MathConstants<float>::twoPi);
                repaint(); };
            driveKnob.onValueChange = [&,this]() {
                showEmpiricalValue(driveKnob, valueLabel, driveSVG);
                driveSVG.setRotation(driveKnob.getNormalisedValue() * MathConstants<float>::twoPi);
                repaint(); };
            driveKnob.onDragEnd = [&,this]() { hideValue(valueLabel, driveSVG);};
            
            blendKnob.onDragStart = [&,this]() {
                hideValue(valueLabel, suffixLabel);
                showIntegerValue(blendKnob, valueLabel, suffixLabel);
                repaint(); };
            blendKnob.onValueChange = [&,this]() {
                showIntegerValue(blendKnob, valueLabel, suffixLabel);
                repaint(); };
            blendKnob.onDragEnd = [&,this]() { hideValue(valueLabel, suffixLabel);};
            
            mainBlendKnob.onDragStart = [&,this]() { hideValue(valueLabel, suffixLabel); showIntegerValue(mainBlendKnob, valueLabel, suffixLabel);};
            mainBlendKnob.onValueChange = [&,this]() { showIntegerValue(mainBlendKnob, valueLabel, suffixLabel); };
            mainBlendKnob.onDragEnd = [&,this]() { hideValue(valueLabel, suffixLabel); };
            
            lowKnob.onDragStart = [&,this]() { hideValue(valueLabel, suffixLabel); showHzValue(lowKnob, lLabel, lSuffix);};
            lowKnob.onValueChange = [&,this]() {
                if (highKnob.getValue() < lowKnob.getValue())
                    highKnob.setValue(lowKnob.getValue(), sendNotification);
                showHzValue(lowKnob, lLabel, lSuffix); };
            lowKnob.onDragEnd = [&,this]() { hideValue(lLabel, lSuffix); hideValue(rLabel, rSuffix); };
            
            highKnob.onDragStart = [&,this]() { hideValue(valueLabel, suffixLabel); showHzValue(highKnob, rLabel, rSuffix);};
            highKnob.onValueChange = [&,this]() {
                if (highKnob.getValue() < lowKnob.getValue())
                    lowKnob.setValue(highKnob.getValue(), sendNotification);
                showHzValue(highKnob, rLabel, rSuffix); };
            highKnob.onDragEnd = [&,this]() { hideValue(rLabel, rSuffix); hideValue(lLabel, lSuffix); };
            
            addAllAndMakeVisible(*this, blendKnob, driveKnob, driveSVG, suffixLabel, valueLabel, lLabel, rLabel, button);
            
            addChildComponent(mainBlendKnob);
            addChildComponent(lowKnob);
            addChildComponent(highKnob);
            
            mainBlendAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, "mainBlend", mainBlendKnob);
            
            setMouseOverLabels(lowKnob, "Low", "Cutoff");
            setMouseOverLabels(highKnob, "High", "Cutoff");
            setMouseOverLabels(mainBlendKnob, "Main", "Blend");
            
            lowKnob.showMouseOver = [&] { if(mouseOverActive) showMouseOverLabels(lowKnob, valueLabel, suffixLabel); repaint(); };
            lowKnob.hideMouseOver = [&] { if(mouseOverActive) hideMouseOverLabels(lowKnob, valueLabel, suffixLabel); repaint(); };
            highKnob.showMouseOver = [&] { if(mouseOverActive) showMouseOverLabels(highKnob, valueLabel, suffixLabel); repaint(); };
            highKnob.hideMouseOver = [&] { if(mouseOverActive) hideMouseOverLabels(highKnob, valueLabel, suffixLabel); repaint(); };
            driveKnob.showMouseOver = [&] { if(mouseOverActive) showMouseOverLabels(driveKnob, valueLabel, suffixLabel); repaint(); };
            driveKnob.hideMouseOver = [&] { if(mouseOverActive) hideMouseOverLabels(driveKnob, valueLabel, suffixLabel); repaint(); };
            blendKnob.showMouseOver = [&] { if(mouseOverActive) showMouseOverLabels(blendKnob, valueLabel, suffixLabel); repaint(); };
            blendKnob.hideMouseOver = [&] { if(mouseOverActive) hideMouseOverLabels(blendKnob, valueLabel, suffixLabel); repaint(); };
            mainBlendKnob.showMouseOver = [&] { if(mouseOverActive) showMouseOverLabels(mainBlendKnob, valueLabel, suffixLabel); repaint(); };
            mainBlendKnob.hideMouseOver = [&] { if(mouseOverActive) hideMouseOverLabels(mainBlendKnob, valueLabel, suffixLabel); repaint(); };
            
            popupMenu.toggleMouseOverEnabled = [&]{ mouseOverActive = mouseOverActive ? false : true; repaint(); };
            popupMenu.toggleControls = [&]{ controlsActive = controlsActive ? false : true;
                forEach ([ctrls = controlsActive] (auto& knob) { knob.setVisible(ctrls); }, mainBlendKnob, lowKnob, highKnob);
                repaint(); };
            
            switchEffect(idx);
        }
        void switchEffect(int idx) {
            apvts.state.setProperty(IDs::currentScreen, var(int(currentEffect)), nullptr);
            
            button.label.set(makeLabel(getFxName(idx)), Colours::black, Colours::grey.darker());
            forEach ([] (auto& attachment) { attachment.reset(); }, driveAttachment, blendAttachment, lowAttachment, highAttachment);
            driveAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, getFxID(idx) + "Drive", driveKnob);
            blendAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, getFxID(idx) + "Blend", blendKnob);
            lowAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, getFxID(idx) + "Low", lowKnob);
            highAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, getFxID(idx) + "High", highKnob);
            
            lowKnob.setNormalisableRange({ 20.0, 20000.0, 0.0, 0.3 });
            highKnob.setNormalisableRange({ 20.0, 20000.0, 0.0, 0.3 });
            driveKnob.setNormalisableRange({ 0.0, 111.0, 1.0 });
            blendKnob.setNormalisableRange({ 0.0, 1.0, 0.01 });
            
            setMouseOverLabels(driveKnob, getFxID(idx).trimCharactersAtStart("static"), "Drive");
            setMouseOverLabels(blendKnob, getFxID(idx).trimCharactersAtStart("static"), "Blend");
            
            hideValue(valueLabel, suffixLabel);
            hideValue(lLabel, lSuffix);
            hideValue(rLabel, rSuffix);
            driveSVG.setVisible(false);
        }

        void resized() override {
            const auto bounds { getBounds() };
            driveKnob.setMouseDragSensitivity(static_cast<int>((double)bounds.getHeight() * 0.98));
            blendKnob.setMouseDragSensitivity(static_cast<int>((double)bounds.getHeight() * 0.98));
            
            auto h { getHeight() }, w { getWidth() }, diameter { jmax(5 * w / 8, 5 * h / 4) };
            
            driveKnob.setSize(diameter, diameter);
            blendKnob.setSize(diameter, diameter);
            driveKnob.setTopRightPosition(getWidth()/2 - 50, h/2 - diameter/2);
            blendKnob.setTopLeftPosition(w/2 + 50, h/2 - diameter/2);
            Rectangle<int> r { bounds.getX(), bounds.getY(), (driveKnob.getRight() - 40) - bounds.getX(), h };
            r.reduce(0, jmax(r.getHeight()/6, r.getWidth()/6));
            
            auto leftArea { r }, rightArea { r.withX(blendKnob.getX() + 40) };
            
            valueLabel.setBounds(leftArea);
            suffixLabel.setBounds(rightArea);
            lLabel.setBounds(leftArea);
            rLabel.setBounds(rightArea);
            
            lSuffix.setSize(lLabel.getWidth()/3, lLabel.getHeight()/3);
            lSuffix.setTopLeftPosition(lLabel.getRight(), (lLabel.getHeight()/4) * 3);
            rSuffix.setSize(rLabel.getWidth()/3, rLabel.getHeight()/3);
            rSuffix.setTopLeftPosition(rLabel.getRight(), (rLabel.getHeight()/4) * 3);
            
            driveSVG.setBounds(blendKnob.getBounds().reduced(driveKnob.getWidth()/6));
            auto blendBounds { Rectangle<int>(static_cast<int>((float)(blendKnob.getX() - driveKnob.getRight() + getWidth()/8) * (float)getHeight()/(float)driveKnob.getHeight()), 20) };
            mainBlendKnob.setSize(blendBounds.getWidth(), blendBounds.getHeight());
            mainBlendKnob.setCentrePosition(bounds.getCentre().x, blendBounds.getHeight()/2);
            
            forEach ([] (Label& label) { label.setFont(getSadisticFont().withHeight(jmin(label.getHeight() * 2 / 3, label.getWidth() * 2 / 3))); label.setColour(Label::ColourIds::textColourId, Colours::grey); }, valueLabel, suffixLabel, lLabel, lSuffix, rLabel, rSuffix);

            blendKnob.setMouseDragSensitivity(bounds.getHeight() / 2);
            driveKnob.setMouseDragSensitivity(bounds.getHeight() / 2);
            
            auto toggleWidth { 150 }, toggleHeight { 15 };
            button.setSize(toggleWidth, toggleHeight);
            button.setCentrePosition(getWidth()/2, h - 2 * toggleHeight);
            
            lowKnob.setSize(toggleWidth/3, toggleWidth/3);
            highKnob.setSize(toggleWidth/3, toggleWidth/3);
            auto topCentreOfButton { button.getBounds().getCentre().withY(button.getBounds().getY()) };
            lowKnob.setTopRightPosition(topCentreOfButton.x, topCentreOfButton.y - toggleWidth/3);
            highKnob.setTopLeftPosition(topCentreOfButton.x, topCentreOfButton.y - toggleWidth/3);
        }
        void mouseDown(const MouseEvent& e) override { if (e.mods.isRightButtonDown()) { popupMenu.show(this); } }
        
        FilterLAF alaf;
        APVTS& apvts;
        SadTextButton button;
        SadSVG driveSVG;
        sadistic::TransLabel valueLabel, suffixLabel, lLabel, rLabel, lSuffix, rSuffix;
        int currentEffect;
        EmpiricalSlider driveKnob { true }, blendKnob;
        FilterKnob lowKnob { Data::apiKnob_svg }, highKnob { Data::apiKnob_svg };
        DeviantSlider mainBlendKnob;
        bool mouseOverActive { false }, controlsActive { false };
        RightClickMenu popupMenu;
        std::unique_ptr<APVTS::SliderAttachment> driveAttachment, lowAttachment, highAttachment, blendAttachment, mainBlendAttachment;
    };
    
}
