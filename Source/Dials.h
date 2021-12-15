#pragma once
#include "WavePad.h"
namespace sadistic {
    
    
    struct Dials : DeviantScreen {
        //        DeviantGUIHub& hub;
        struct SadImage : Component {
            SadImage(const char* s, const int sS) : svg(s), svgSize(sS) { setInterceptsMouseClicks(false, true); }
            void paint(Graphics& g) override {
                Path path;
                //                path = pathFromBinarySVG(Data::SATURATE_svg, Data::SATURATE_svgSize);
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
        
        Dials(DeviantGUIHub& h, int idx = 2) : DeviantScreen(h), button(makeLabel(getFxID(idx))), leftSVG(Data::SATURATE_svg, Data::SATURATE_svgSize), rightSVG(Data::SATURATE_svg, Data::SATURATE_svgSize) {
            
            setLookAndFeel(&llaf);
            
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
            
            leftAttachment = std::make_unique<APVTS::SliderAttachment>(hub.mgmt.apvts, getParamID(idx, 0), *leftKnob);
            if (effectInfo[idx].numParams > 1)
                rightAttachment = std::make_unique<APVTS::SliderAttachment>(hub.mgmt.apvts, getParamID(idx, 1), *rightKnob);
            blendAttachment = std::make_unique<APVTS::SliderAttachment>(hub.mgmt.apvts, "blend" /* getFxID(idx) + "Blend" */, *blendKnob);
            
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
    
    template<typename WavePadType> struct MoreDials : DeviantScreen {
        
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
        MoreDials(DeviantGUIHub& h) : DeviantScreen(h), pad(h) {
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
            
            if (pad.wavePadType.contains("dynamic")) {
                for (size_t i { 0 }; i < hub.mgmt.waveTableFiles.size(); ++i) phaseBox.addItem(hub.mgmt.waveTableFiles[i].id.toUTF8(), (int) i + 1);
                phaseBox.addItem("Load Table", int(hub.mgmt.waveTableFiles.size() + 1));
                phaseBox.addItem("Save Table", int(hub.mgmt.waveTableFiles.size() + 2));
                phaseBox.setText("PHASE TABLES");
                phaseBox.setSelectedItemIndex(int(*hub.mgmt.apvts.getRawParameterValue("waveTableID")), dontSendNotification);
                phaseBox.onChange = [&,this] {
                    const int newID { phaseBox.getSelectedItemIndex() };
                    const int tableID { int(*hub.mgmt.apvts.getRawParameterValue("waveTableID")) };
                    if (newID != tableID) {
                        const auto tableCount { int(hub.mgmt.waveTableFiles.size()) };
                        if (newID == tableCount) loadFile();
                        if (newID == tableCount + 1) saveFile();
                        hub.mgmt.selectTable(newID, hub.pWave); repaint();
                    }
                };

                saveCallback = [&,this] (const FileChooser& chooser) {
                    auto result { chooser.getResult() };
                        if(hub.mgmt.saveTable(result)) juce::AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, "Table Saved...", "Good");
                };

                saveFile = [&,this](){
                    fc = std::make_unique<FileChooser>("Save Wave Table", File(), "*.wav");
                    fc->launchAsync (FileBrowserComponent::saveMode |
                                     FileBrowserComponent::canSelectFiles |
                                     FileBrowserComponent::warnAboutOverwriting |
                                     FileBrowserComponent::doNotClearFileNameOnRootChange,
                                     saveCallback);
                };
                loadCallback = [&,this] (const FileChooser& chooser) {
                    if (!chooser.getResults().isEmpty()) {
                        if (hub.mgmt.loadTable(chooser.getResult()), hub.pWave)
                            juce::AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, "Table Loaded", "!");
                        else juce::AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "File not loaded correctly", "!");
                    }
                    else juce::AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "File not choosen correctly", "!");
                };

                loadFile = [&,this](){
                    fc = std::make_unique<FileChooser>("Load Wave Table (.wav)", File(), "*.wav");
                    fc->launchAsync (FileBrowserComponent::openMode |
                                     FileBrowserComponent::canSelectFiles,
                                     loadCallback);
                };
                addAndMakeVisible(phaseBox);
            }
            addAndMakeVisible(pad);
            addAllAndMakeVisible(*this, xAxisLabel, yAxisLabel, button);
            button.label.set(pad.buttonString, pad.textColour, pad.bgColour);
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
        WavePadType pad;
        LeftEmpiricalLAF lelaf;
        RightEmpiricalLAF relaf;
        EmpiricalLAF elaf;
        Image   volume, filter, in, out, hz, dB;
        sadistic::TransLabel valueLabel, suffixLabel;
        EmpiricalSlider knobs[numSliders] { { true, true }, { true, true }, { false, true }, { false, true }, { true, true }, { true, true, true }, { false, true }, { false, true, true }, { true, true, true }, { false, true, true }, { true, true, true }, { false, true, true } };
        APVTS::SliderAttachment attachments[numSliders] {
            { hub.mgmt.apvts, { pad.wavePadType + "AtanDrive" },            knobs[0] },
            { hub.mgmt.apvts, { pad.wavePadType + "DeviationGate" },        knobs[1] },
            { hub.mgmt.apvts, { pad.wavePadType + "WaveShaperBlend" },      knobs[2] },
            { hub.mgmt.apvts, { pad.wavePadType + "BitCrusherFloor" },      knobs[3] },
            { hub.mgmt.apvts, { pad.wavePadType + "DeviationDrive" },       knobs[4] },
            { hub.mgmt.apvts, { pad.wavePadType + "DeviationSaturation" },  knobs[5] },
            { hub.mgmt.apvts,                   { "filterAHigh" },          knobs[6] },
            { hub.mgmt.apvts,                   { "filterBHigh" },          knobs[7] },
            { hub.mgmt.apvts, { pad.wavePadType + "AtanBlend" },            knobs[8] },
            { hub.mgmt.apvts, { pad.wavePadType + "BitCrusherBlend" },      knobs[9] },
            { hub.mgmt.apvts, { pad.wavePadType + "DeviationBlend" },       knobs[10] },
            { hub.mgmt.apvts,                   { "filterBHigh" },          knobs[11] }
        };
        Panel panel[numPanels] { { 2, &knobs[4], &knobs[10] }, { 2, &knobs[0], &knobs[8] }, { 2, &knobs[6], &knobs[11] }, { 2, &knobs[2], &knobs[9] } };
        
        SadLabel xAxisLabel { pad.xString, false, false, 0.f };
        SadLabel yAxisLabel { pad.yString, true, false, -0.5f };
        SadTextButton button { pad.buttonString };
        SadBox phaseBox { "PHASE TABLES" };
        std::unique_ptr<FileChooser> fc;
        std::function<void()> loadFile, saveFile;
        std::function<void (const FileChooser&)> loadCallback, saveCallback;
    };
    
    
//    struct Dials : DeviantScreen {
//        Dials(APVTS& s, UndoManager* uM) : DeviantScreen(s, uM) {
////            drive.setIMG(Data::DRIVE_png, Data::DRIVE_pngSize);
////            saturate.setIMG(Data::SATURATE_png, Data::SATURATE_pngSize);
//
//            driveKnob = std::make_unique<sadistic::EmpiricalSlider>(true);
//            saturationKnob = std::make_unique<sadistic::EmpiricalSlider>();
//            gateKnob = std::make_unique<sadistic::OuterSlider>(false);
//            hiSlider = std::make_unique<sadistic::OuterSlider>(true, true);
//            loSlider = std::make_unique<sadistic::OuterSlider>(false, true);
//            blendKnob = std::make_unique<Slider>();
//
//            sadistic::setWidgets(*blendKnob, *driveKnob, llaf, *saturationKnob, *hiSlider, olaf, *gateKnob, *loSlider, valueLabel, suffixLabel, shadowsIntoLight);
//
//            hiSlider->onDragStart = [&,this]() {showIntegerValue(*hiSlider, valueLabel, suffixLabel);};
//            hiSlider->onValueChange = [&,this]() {
//                if (loSlider->getValue() > hiSlider->getValue())
//                    loSlider->setValue(hiSlider->getValue(), sendNotificationSync);
//                showIntegerValue(*hiSlider, valueLabel, suffixLabel);};
//            hiSlider->onDragEnd = [&,this]() { hideValue(valueLabel, suffixLabel);};
//            loSlider->onDragStart = [&,this]() { showIntegerValue(*loSlider, valueLabel, suffixLabel);};
//            loSlider->onValueChange = [&,this]() {
//                if (hiSlider->getValue() < loSlider->getValue())
//                    hiSlider->setValue(loSlider->getValue(), sendNotificationSync);
//                showIntegerValue(*loSlider, valueLabel, suffixLabel);};
//            loSlider->onDragEnd = [&,this]() { hideValue(valueLabel, suffixLabel);};
//
//            driveKnob->onDragStart = [&,this]() {
//                showEmpiricalValue(*driveKnob, valueLabel, drive);
//                drive.setTransform(AffineTransform::rotation(std::powf((float)driveKnob->getValue() / 200,2),saturationKnob->getX() + saturationKnob->getWidth()/2,saturationKnob->getY() + saturationKnob->getWidth()/2));};
//            driveKnob->onValueChange = [&,this]() {
//                showEmpiricalValue(*driveKnob, valueLabel, drive);
//                drive.setTransform(AffineTransform::rotation(std::powf((float)driveKnob->getValue() / 200,2),saturationKnob->getX() + saturationKnob->getWidth()/2,saturationKnob->getY() + saturationKnob->getWidth()/2));};
//            driveKnob->onDragEnd = [&,this]() { hideValue(valueLabel, drive);};
//
//            saturationKnob->onDragStart = [&,this]() {
//                showEmpiricalValue(*saturationKnob, valueLabel, saturate);
//                saturate.setTransform(AffineTransform::rotation((float)saturationKnob->getValue() / 10.f - 0.2f,saturationKnob->getX() + saturationKnob->getWidth()/2,saturationKnob->getY() + saturationKnob->getWidth()/2));};
//            saturationKnob->onValueChange = [&,this]() {
//                showEmpiricalValue(*saturationKnob, valueLabel, saturate);
//                saturate.setTransform(AffineTransform::rotation((float)saturationKnob->getValue() / 10.f - 0.2f,saturationKnob->getX() + saturationKnob->getWidth()/2,saturationKnob->getY() + saturationKnob->getWidth()/2));};
//            saturationKnob->onDragEnd = [&,this]() { hideValue(valueLabel, saturate);};
//
//            gateKnob->onDragStart = [&,this]() { showIntegerValue(*gateKnob, valueLabel, suffixLabel);};
//            gateKnob->onValueChange = [&,this]() { showIntegerValue(*gateKnob, valueLabel, suffixLabel);};
//            gateKnob->onDragEnd = [&,this]() { hideValue(valueLabel, suffixLabel);};
//
//            blendKnob->onDragStart = [&,this]() { showIntegerValue(*blendKnob, valueLabel, suffixLabel);};
//            blendKnob->onValueChange = [&,this]() { showIntegerValue(*blendKnob, valueLabel, suffixLabel); };
//            blendKnob->onDragEnd = [&,this]() { hideValue(valueLabel, suffixLabel);};
//
//            addAllAndMakeVisible(*this, *blendKnob, *saturationKnob, *driveKnob, saturate, drive, suffixLabel, valueLabel);
//
//            gateAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, "staticDeviationGate", *gateKnob);
//            driveAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, "staticDeviationDrive", *driveKnob);
//            saturationAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, "staticDeviationSaturation", *saturationKnob);
//            blendAttachment = std::make_unique<APVTS::SliderAttachment>(apvts, "blend", *blendKnob);
//
//            loSlider->setNormalisableRange({ 20.0, 20000.0, 0.0, 0.2 });
//            hiSlider->setNormalisableRange({ 20.0, 20000.0, 0.0, 0.2 });
//
//            hideValue(valueLabel, suffixLabel);
//            drive.setVisible(false);
//            saturate.setVisible(false);
//
//            saturationKnob->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
//            driveKnob->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
//        }
//
//        void paint(Graphics&) override {}
//
//        void resized() override {
//            const auto bounds { getBounds() };
//            driveKnob->setMouseDragSensitivity(static_cast<int>((double)bounds.getHeight() * 0.98));
//            saturationKnob->setMouseDragSensitivity(static_cast<int>((double)bounds.getHeight() * 0.98));
//
//            auto h { getHeight() }, w { getWidth() }, diameter { jmax(5 * w / 8, 5 * h / 4) };
//
//            driveKnob->setSize(diameter, diameter);
//            saturationKnob->setSize(diameter, diameter);
//            driveKnob->setTopRightPosition(getWidth()/2 - 50, h/2 - diameter/2);
//            saturationKnob->setTopLeftPosition(w/2 + 50, h/2 - diameter/2);
//            hiSlider->setBounds(driveKnob->getBounds().expanded(30, 30));
//            loSlider->setBounds(driveKnob->getBounds().expanded(30, 30));
//            gateKnob->setBounds(saturationKnob->getBounds().expanded(30, 30));
//
//            gateKnob->setRotaryParameters(degreesToRadians(283.f), degreesToRadians(315.f), true);
//
//            loSlider->setRotaryParameters(degreesToRadians(130.f), degreesToRadians(55.f), true);
//            hiSlider->setRotaryParameters(degreesToRadians(130.f), degreesToRadians(55.f), true);
//            Rectangle<int> r { bounds.getX(), bounds.getY(), (driveKnob->getRight() - 40) - bounds.getX(), h };
//            r.reduce(0, jmax(r.getHeight()/6, r.getWidth()/6));
//            valueLabel.setBounds(r);
//            suffixLabel.setBounds(r.withX(saturationKnob->getX() + 40));
//            drive.setBounds(saturationKnob->getBounds().reduced(driveKnob->getWidth()/6));
//            saturate.setBounds(saturationKnob->getBounds().reduced(saturationKnob->getWidth()/8));
//            auto blendBounds { Rectangle<int>(static_cast<int>((float)(saturationKnob->getX() - driveKnob->getRight() + getWidth()/8) * (float)getHeight()/(float)driveKnob->getHeight()), 20) };
//            blendKnob->setSize(blendBounds.getWidth(), blendBounds.getHeight());
//            blendKnob->setCentrePosition(bounds.getCentre().x, blendBounds.getHeight()/2);
//
//            valueLabel.setFont(getSadisticFont(jmin(valueLabel.getHeight() / 2, valueLabel.getWidth() / 2)));
//            suffixLabel.setFont(getSadisticFont(jmin(suffixLabel.getHeight() / 2, suffixLabel.getWidth() / 2)));
//
//            saturationKnob->setMouseDragSensitivity(bounds.getHeight() / 2);
//            driveKnob->setMouseDragSensitivity(bounds.getHeight() / 2);
//            gateKnob->setMouseDragSensitivity(bounds.getHeight() / 4);
//        }
//        void updateWaveTable() override {}
//        void init() override {}
//
//        sadistic::EmpiricalLAF       llaf;
//        sadistic::OuterLookAndFeel     olaf;
//        SadChild<Image> drive, saturate;
//        sadistic::TransLabel valueLabel, suffixLabel;
//        std::unique_ptr<Slider> driveKnob, saturationKnob, gateKnob, hiSlider, loSlider, blendKnob;
//        std::unique_ptr<APVTS::SliderAttachment> hiAttachment, driveAttachment, saturationAttachment, gateAttachment, blendAttachment, loAttachment;
//        Font shadowsIntoLight     { Font(Typeface::createSystemTypefaceFor(Data::shadowsIntoLight_ttf, (size_t) Data::shadowsIntoLight_ttfSize)) };
//    };
//
//    template<typename WavePadType> struct MoreDials : DeviantScreen {
//        static constexpr int numSliders { 12 }, numPanels { 4 };
//        struct Panel : Component {
//            Panel(int n, EmpiricalSlider* s, EmpiricalSlider* bS) : num(n) {
//                for (int i { 0 }; i < num; ++i) {
//                    sliders[i] = &s[i];
//                    addAndMakeVisible(sliders[i]);
//                }
//                sliders[num] = bS;
//                addAndMakeVisible(sliders[num]);
//            }
//            void resized() override {
//                const auto localBounds { getLocalBounds() };
//                Rectangle<int> bounds[4];
//                bounds[0] = getLocalBounds();
//                int removal = bounds[0].getHeight()/(num+1);
//                for (int i = 1; i <= num; ++i) bounds[i] = bounds[0].removeFromBottom(removal);
//                for (int i { 0 }; i <= num; ++i) {
//                    if(sliders[i]->isLeft) sliders[i]->setBounds(bounds[i]);
//                    else sliders[i]->setBounds(bounds[i]);
//                    sliders[i]->setTextBoxStyle(sliders[i]->isLeft ? Slider::TextBoxLeft : Slider::TextBoxRight, false, localBounds.getHeight()/num, localBounds.getHeight()/num);
//                }
//            }
//            int num;
//            EmpiricalSlider* sliders[4] { nullptr };
//        };
//        MoreDials(APVTS& s, UndoManager* uM) : DeviantScreen(s, uM), pad(*this) {
//            for(int j { 0 }; j < numPanels; ++j) {
//                for(int i { 0 }; i <= panel[j].num; ++i) {
//                    if(j < 2) panel[j].sliders[i]->setLookAndFeel(&lelaf);
//                    else panel[j].sliders[i]->setLookAndFeel(&relaf);
//                    panel[j].sliders[i]->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
//                    if(j > 1) {
//                        if(!panel[j].sliders[i]->isDefaultHigh) panel[j].sliders[i]->setRotaryParameters(0.0f, 5.81333f, true);
//                        else panel[j].sliders[i]->setRotaryParameters(5.81333f, 0.0f, true);
//                    }
//                    else {
//                        if(panel[j].sliders[i]->isDefaultHigh) panel[j].sliders[i]->setRotaryParameters(degreesToRadians(513.f), degreesToRadians(180.f), true);
//                        else panel[j].sliders[i]->setRotaryParameters(degreesToRadians(180.f), degreesToRadians(513.f), true);
//                    }
//                    panel[j].sliders[i]->setDoubleClickReturnValue(true, 1.0f, ModifierKeys::altModifier);
//                    panel[j].sliders[i]->setMouseDragSensitivity (50);
//                    panel[j].sliders[i]->setNumDecimalPlacesToDisplay(2);
//                    addAndMakeVisible(panel[j]);
//                }
//                addAndMakeVisible(panel[j]);
//            }
//            addAndMakeVisible(frame);
//            addAndMakeVisible(pad);
//            knobs[1].setNormalisableRange({ knobs[1].getMinimum(), knobs[1].getMaximum(), 0.0, 0.2 });
//            knobs[4].setNormalisableRange({ knobs[4].getMinimum(), knobs[4].getMaximum(), 0.0, 0.2 });
//            knobs[5].setNormalisableRange({ knobs[5].getMinimum(), knobs[5].getMaximum(), 0.0, 0.6 });
//        }
//
//        void paint(Graphics&) override {
////            if (newDataHere) { init(); newDataHere = false; pad.repaint(); }
//
//        }
//        void resized() override {
//            Rectangle<int> middle { getLocalBounds() };
//            Rectangle<int> bounds[4];
//            bounds[0] = middle.removeFromLeft(getWidth()/8);
//            bounds[1] = bounds[0].removeFromTop(getHeight()/2);
//            bounds[2] = middle.removeFromRight(getWidth()/8);
//            bounds[3] = bounds[2].removeFromTop(getHeight()/2);
//            for(int j { 0 }; j < numPanels; ++j) panel[j].setBounds(bounds[j]);
//            frame.setBounds(getLocalBounds().reduced(getLocalBounds().getWidth()/8, 0));
//            pad.setBounds(middle);
//        }
//
//        void updateWaveTable() override { pad.setTheTable(); }
//        void init() override { pad.getTheTable(); }
//
//        Frame frame;
//        WavePadType pad;
//        LeftEmpiricalLAF lelaf;
//        RightEmpiricalLAF relaf;
//        EmpiricalLAF elaf;
//        Image   volume, filter, in, out, hz, dB;
//        sadistic::TransLabel valueLabel, suffixLabel;
//        EmpiricalSlider knobs[numSliders] { { true, true }, { true, true }, { false, true }, { false, true }, { true, true }, { true, true, true }, { false, true }, { false, true, true }, { true, true, true }, { false, true, true }, { true, true, true }, { false, true, true } };
//        APVTS::SliderAttachment attachments[numSliders] {
//            { apvts, { pad.wavePadType + "AtanDrive" },            knobs[0] },
//            { apvts, { pad.wavePadType + "DeviationGate" },        knobs[1] },
//            { apvts, { pad.wavePadType + "WaveShaperBlend" },      knobs[2] },
//            { apvts, { pad.wavePadType + "BitCrusherFloor" },      knobs[3] },
//            { apvts, { pad.wavePadType + "DeviationDrive" },       knobs[4] },
//            { apvts, { pad.wavePadType + "DeviationSaturation" },  knobs[5] },
//            { apvts,                   { "filterBLow" },           knobs[6] },
//            { apvts,                   { "filterBHigh" },          knobs[7] },
//            { apvts, { pad.wavePadType + "AtanBlend" },            knobs[8] },
//            { apvts, { pad.wavePadType + "BitCrusherBlend" },      knobs[9] },
//            { apvts, { pad.wavePadType + "DeviationBlend" },       knobs[10] },
//            { apvts,                   { "filterBBlend" },         knobs[11] }
//        };
//        Panel panel[numPanels] { { 2, &knobs[4], &knobs[10] }, { 2, &knobs[0], &knobs[8] }, { 2, &knobs[6], &knobs[11] }, { 2, &knobs[2], &knobs[9] } };
//    };
    
}
