#pragma once
#include "deviant.h"
namespace sadistic {
    
    struct Dials : Component {
        Dials(APVTS& t, int idx) : apvts(t), button(makeLabel(getFxName(currentEffect = idx))) {

            button.onClick = [&] { switchEffect(++currentEffect%=numFX); button.label.label.setColour(Label::backgroundColourId, button.colour); };
            
            forEach ([] (auto& knob) {
                knob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
                knob.setDoubleClickReturnValue(true,1.0f,ModifierKeys::altModifier);
                knob.setMouseDragSensitivity (100);
                knob.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
                knob.setScrollWheelEnabled(true);
                knob.setRotaryParameters(degreesToRadians(225.f), degreesToRadians(495.f), true);
                knob.setTextValueSuffix("Hz");
            }, blendKnob, driveKnob, deviateKnob, lowKnob, highKnob);
            
            lowKnob.setLookAndFeel(&alaf);
            highKnob.setLookAndFeel(&alaf);
            
            blendKnob.setLookAndFeel(&alaf);
            blendKnob.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
            blendKnob.setTextValueSuffix("%");
            
            driveKnob.setRotaryParameters(degreesToRadians(0.f), 5.81333f, true);
            deviateKnob.setRotaryParameters(degreesToRadians(180.f), degreesToRadians(510.f), true);

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
            
            deviateKnob.onDragStart = [&,this]() {
                hideValue(valueLabel, suffixLabel);
                showEmpiricalValue(deviateKnob, valueLabel, deviationSVG);
                deviationSVG.setRotation(deviateKnob.getNormalisedValue() * MathConstants<float>::twoPi);
                repaint(); };
            deviateKnob.onValueChange = [&,this]() {
                showEmpiricalValue(deviateKnob, valueLabel, deviationSVG);
                deviationSVG.setRotation(deviateKnob.getNormalisedValue() * MathConstants<float>::twoPi);
                repaint(); };
            deviateKnob.onDragEnd = [&,this]() { hideValue(valueLabel, deviationSVG);};
            
            blendKnob.onDragStart = [&,this]() { hideValue(valueLabel, suffixLabel); showIntegerValue(blendKnob, valueLabel, suffixLabel);};
            blendKnob.onValueChange = [&,this]() { showIntegerValue(blendKnob, valueLabel, suffixLabel); };
            blendKnob.onDragEnd = [&,this]() { hideValue(valueLabel, suffixLabel); };
            
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
            
            addAllAndMakeVisible(*this, deviateKnob, driveKnob, blendKnob, driveSVG, deviationSVG, suffixLabel, valueLabel, lLabel, rLabel, button);
            
            addChildComponent(lowKnob);
            addChildComponent(highKnob);
            
            blendAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, "mainBlend", blendKnob);
            
            setMouseOverLabels(lowKnob, "Low", "Cutoff");
            setMouseOverLabels(highKnob, "High", "Cutoff");
            setMouseOverLabels(blendKnob, "Main", "Blend");
            
            lowKnob.showMouseOver = [&] { if(mouseOverActive) showMouseOverLabels(lowKnob, valueLabel, suffixLabel); repaint(); };
            lowKnob.hideMouseOver = [&] { if(mouseOverActive) hideMouseOverLabels(lowKnob, valueLabel, suffixLabel); repaint(); };
            highKnob.showMouseOver = [&] { if(mouseOverActive) showMouseOverLabels(highKnob, valueLabel, suffixLabel); repaint(); };
            highKnob.hideMouseOver = [&] { if(mouseOverActive) hideMouseOverLabels(highKnob, valueLabel, suffixLabel); repaint(); };
            driveKnob.showMouseOver = [&] { if(mouseOverActive) showMouseOverLabels(driveKnob, valueLabel, suffixLabel); repaint(); };
            driveKnob.hideMouseOver = [&] { if(mouseOverActive) hideMouseOverLabels(driveKnob, valueLabel, suffixLabel); repaint(); };
            deviateKnob.showMouseOver = [&] { if(mouseOverActive) showMouseOverLabels(deviateKnob, valueLabel, suffixLabel); repaint(); };
            deviateKnob.hideMouseOver = [&] { if(mouseOverActive) hideMouseOverLabels(deviateKnob, valueLabel, suffixLabel); repaint(); };
            blendKnob.showMouseOver = [&] { if(mouseOverActive) showMouseOverLabels(blendKnob, valueLabel, suffixLabel); repaint(); };
            blendKnob.hideMouseOver = [&] { if(mouseOverActive) hideMouseOverLabels(blendKnob, valueLabel, suffixLabel); repaint(); };
            
            popupMenu.toggleMouseOverEnabled = [&]{ mouseOverActive = mouseOverActive ? false : true; repaint(); };
            popupMenu.toggleFilterControls = [&]{ filterControlsActive = filterControlsActive ? false : true;
                forEach ([ctrls = filterControlsActive] (auto& knob) { knob.setVisible(ctrls); }, lowKnob, highKnob);
                repaint(); };
            
            switchEffect(idx);
        }
        void switchEffect(int idx) {
            apvts.state.setProperty(Identifier("currentScreen"), var(int(currentEffect)), nullptr);
            
            button.label.set(makeLabel(getFxName(idx)), Colours::black, Colours::grey.darker());
            forEach ([] (auto& attachment) { attachment.reset(); }, driveAttachment, deviationAttachment, lowAttachment, highAttachment);
            driveAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, getFxID(idx) + "Drive", driveKnob);
            deviationAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, getFxID(idx) + "Deviation", deviateKnob);
            lowAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, getFxID(idx) + "Low", lowKnob);
            highAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, getFxID(idx) + "High", highKnob);
            
            lowKnob.setNormalisableRange({ 20.0, 20000.0, 0.0, 0.3 });
            highKnob.setNormalisableRange({ 20.0, 20000.0, 0.0, 0.3 });
            driveKnob.setNormalisableRange({ 0.0, 111.0, 1.0 });
            deviateKnob.setNormalisableRange({ 0.0, 100.0, 1.0 });
            
            setMouseOverLabels(driveKnob, getFxID(idx).trimCharactersAtStart("static"), "Drive");
            setMouseOverLabels(deviateKnob, getFxID(idx).trimCharactersAtStart("static"), "Deviation");
            
            hideValue(valueLabel, suffixLabel);
            hideValue(lLabel, lSuffix);
            hideValue(rLabel, rSuffix);
            driveSVG.setVisible(false);
            deviationSVG.setVisible(false);
        }

        void resized() override {
            const auto bounds { getBounds() };
            driveKnob.setMouseDragSensitivity(static_cast<int>((double)bounds.getHeight() * 0.98));
            deviateKnob.setMouseDragSensitivity(static_cast<int>((double)bounds.getHeight() * 0.98));
            
            auto h { getHeight() }, w { getWidth() }, diameter { jmax(5 * w / 8, 5 * h / 4) };
            
            driveKnob.setSize(diameter, diameter);
            deviateKnob.setSize(diameter, diameter);
            driveKnob.setTopRightPosition(getWidth()/2 - 50, h/2 - diameter/2);
            deviateKnob.setTopLeftPosition(w/2 + 50, h/2 - diameter/2);
            Rectangle<int> r { bounds.getX(), bounds.getY(), (driveKnob.getRight() - 40) - bounds.getX(), h };
            r.reduce(0, jmax(r.getHeight()/6, r.getWidth()/6));
            
            auto leftArea { r }, rightArea { r.withX(deviateKnob.getX() + 40) };
            
            valueLabel.setBounds(leftArea);
            suffixLabel.setBounds(rightArea);
            lLabel.setBounds(leftArea);
            rLabel.setBounds(rightArea);
            
            lSuffix.setSize(lLabel.getWidth()/3, lLabel.getHeight()/3);
            lSuffix.setTopLeftPosition(lLabel.getRight(), (lLabel.getHeight()/4) * 3);
            rSuffix.setSize(rLabel.getWidth()/3, rLabel.getHeight()/3);
            rSuffix.setTopLeftPosition(rLabel.getRight(), (rLabel.getHeight()/4) * 3);
            
            driveSVG.setBounds(deviateKnob.getBounds().reduced(driveKnob.getWidth()/6));
            deviationSVG.setBounds(deviateKnob.getBounds().reduced(driveKnob.getWidth()/6));
            auto blendBounds { Rectangle<int>(static_cast<int>((float)(deviateKnob.getX() - driveKnob.getRight() + getWidth()/8) * (float)getHeight()/(float)driveKnob.getHeight()), 20) };
            blendKnob.setSize(blendBounds.getWidth(), blendBounds.getHeight());
            blendKnob.setCentrePosition(bounds.getCentre().x, blendBounds.getHeight()/2);
            
            forEach ([] (Label& label) { label.setFont(getSadisticFont().withHeight(static_cast<float>(jmin(label.getHeight() * 2 / 3, label.getWidth() * 2 / 3)))); label.setColour(Label::ColourIds::textColourId, Colours::grey); }, valueLabel, suffixLabel, lLabel, lSuffix, rLabel, rSuffix);

            deviateKnob.setMouseDragSensitivity(bounds.getHeight() / 2);
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
        
        struct RightClickMenu : PopupMenu {
            RightClickMenu() {
                PopupMenu::Item itemLoad { "Show Info On Mouse-Over" };
                itemLoad.action = { [&] { toggleMouseOverEnabled(); } };
                addItem(itemLoad);
                PopupMenu::Item itemSave { "Show Filter Controls" };
                itemSave.action = { [&,this] { toggleFilterControls(); } };
                addItem(itemSave);
            }
            void show(Component* comp) {
                PopupMenu::Options options;
                PopupMenu::showMenuAsync(options.withTargetComponent(comp)); }
            std::function<void()> toggleMouseOverEnabled, toggleFilterControls;
        };
        
        FilterLAF alaf;
        APVTS& apvts;
        SadTextButton button;
        SadSVG driveSVG { Data::DRIVE_svg, Colours::grey }, deviationSVG { Data::DEVIATION_svg, Colours::grey };
        TransLabel valueLabel, suffixLabel, lLabel, rLabel, lSuffix, rSuffix;
        int currentEffect;
        EmpiricalSlider driveKnob { true }, deviateKnob;
        FilterKnob lowKnob { Data::apiKnob2_svg }, highKnob { Data::apiKnob2_svg };
        DeviantSlider blendKnob;
        bool mouseOverActive { false }, filterControlsActive { false };
        RightClickMenu popupMenu;
        std::unique_ptr<APVTS::SliderAttachment> driveAttachment, lowAttachment, highAttachment, deviationAttachment, blendAttachment;
    };
} // namespace sadistic
