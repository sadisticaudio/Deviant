#include "deviant.h"
#include "RoutingMatrix.h"
//#include "Dials.h"
namespace sadistic {

}

template<> void sadistic::SadChild<Path>::setIMG (const char* img, const int imgSize) {
//    image.loadPathFromData(img, (size_t) imgSize);
    
}
template<> void sadistic::SadChild<Image>::setIMG (const char* img, const int imgSize) {
//    image = ImageCache::getFromMemory (img, imgSize);
    
}

template<> void sadistic::SadChild<Path>::paint (Graphics& g) {
    g.setColour(Colours::grey);
    auto b { getLocalBounds().toFloat().reduced(0.8f) };
//    image.scaleToFit(b.getY(), b.getY(), b.getWidth(), b.getHeight(), Justification::centred);
//    g.fillPath(image);
    
}
template<> void sadistic::SadChild<Image>::paint (Graphics&) {}

void sadistic::RoutingMatrix::InsertionPoint::itemDropped (const juce::DragAndDropTarget::SourceDetails& details) {
    std::cout << "ItemDropped!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
    if(details.sourceComponent != this) {
        for(int i = 0; i < numFX; ++i) {
            if(details.sourceComponent == &matrix.effects[i]){
                auto destinationRoute { effect ? (int)effect->routeSlider.getValue() : route };
                auto destinationIndex { effect ? (int)effect->indexSlider.getValue() : matrix.fx[route].size()-1 };
                matrix.moveEffect(&matrix.effects[i], destinationRoute, destinationIndex);
            }
        }
    }
    matrix.repaint();
}
bool sadistic::RoutingMatrix::EffectRoutingState::operator<(const EffectRoutingState& other) const {
    if (route < other.route) return true;
    else if (route == other.route && index < other.index) return true;
    return false;
}

void sadistic::RoutingMatrix::Effect::mouseDrag (const MouseEvent& e) {
    std::cout << "mouseDrag():" << getX() << " " << getY() << " " << e.x - getWidth()/2 << " " << e.y - getHeight()/2 << std::endl;
    matrix.startDragging(effectID, this);
}

void sadistic::RoutingMatrix::Effect::mouseDown (const MouseEvent& e) {
    std::cout << "mouseDrag():" << getX() << " " << getY() << " " << e.x - getWidth()/2 << " " << e.y - getHeight()/2 << std::endl;
    t1 = hi_res::now();
}

void sadistic::RoutingMatrix::Effect::mouseUp (const MouseEvent& e) {
    std::cout << "mouseUp():" << getX() << " " << getY() << " " << e.x - getWidth()/2 << " " << e.y - getHeight()/2 << std::endl;
    t2 = hi_res::now();
    duration<double, std::milli> ms_double = t2 - t1;
    if(ms_double.count() < 100.0) { bool enabled = (bool)enabledSlider.getValue(); enabledSlider.setValue(enabled ? false : true); if (!enabled) setAlpha(1.f); else setAlpha(0.5f); }
    for (auto& i : matrix.effects) i.insertionPoint.setTransparency(0.f);
    for (auto& i : matrix.endPoint) i.setTransparency(0.f);
    matrix.repaint();
}

void sadistic::RoutingMatrix::InsertionPoint::itemDragEnter (const SourceDetails& /*dragSourceDetails*/) {
    std::cout << "DragAndDropCarrier():: itemDragEnter()" << std::endl;
    if (effect) {
        effect->setBounds(effect->getBounds().translated(20, 0));
        int idx { (int)effect->indexSlider.getValue() };
        if (idx < matrix.fx[route].size()-1 && idx > 0) matrix.fx[route][idx-1]->setBounds(matrix.fx[route][idx-1]->getBounds().translated(-15, 0));
    }
    //what a fucking mess
    else if (matrix.fx[route].size()-1 > 0) matrix.fx[route][matrix.fx[route].size() - 2]->setBounds(matrix.fx[route][matrix.fx[route].size()-2]->getBounds().translated(-15, 0));
    setTransparency(0.3f);
    repaint(); }

void sadistic::RoutingMatrix::InsertionPoint::itemDragExit (const SourceDetails& /*dragSourceDetails*/) {
    std::cout << "DragAndDropCarrier():: itemDragExit()" << std::endl;
    if (effect) {
        effect->setBounds(effect->getBounds().translated(-20, 0));
        int idx { (int)effect->indexSlider.getValue() };
        if (idx < matrix.fx[route].size()-1 && idx > 0) matrix.fx[route][idx-1]->setBounds(matrix.fx[route][idx-1]->getBounds().translated(15, 0));
    }
    else if (matrix.fx[route].size()-1 > 0) matrix.fx[route][matrix.fx[route].size() - 2]->setBounds(matrix.fx[route][matrix.fx[route].size()-2]->getBounds().translated(15, 0));
    setTransparency(0.f);
    repaint(); }

void sadistic::EmpiricalLAF::drawLinearSlider (Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& slider)  {
    if (slider.isBar())  {
        g.setColour (slider.findColour (Slider::trackColourId));
        g.fillRect (slider.isHorizontal() ? Rectangle<float> (static_cast<float> (x), y + 0.5f, sliderPos - x, height - 1.0f)
                    : Rectangle<float> (x + 0.5f, sliderPos, width - 1.0f, y + (height - sliderPos)));
    }
    else {
        auto isTwoVal   = (style == Slider::SliderStyle::TwoValueVertical   || style == Slider::SliderStyle::TwoValueHorizontal);
        auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);
        auto trackWidth = jmin (4.0f, slider.isHorizontal() ? height * 0.25f : width * 0.25f);
        Point<float> startPoint (slider.isHorizontal() ? x : x + width * 0.5f, slider.isHorizontal() ? y + height * 0.5f : height + y);
        Point<float> endPoint (slider.isHorizontal() ? width + x : startPoint.x, slider.isHorizontal() ? startPoint.y : y);
        Path backgroundTrack;
        backgroundTrack.startNewSubPath (startPoint);
        backgroundTrack.lineTo (endPoint);
        g.setColour (slider.findColour (Slider::backgroundColourId));
        g.strokePath (backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });
        
        Path valueTrack;
        Point<float> minPoint, maxPoint;
        
        if (isTwoVal || isThreeVal) {
            minPoint = { slider.isHorizontal() ? minSliderPos : width * 0.5f, slider.isHorizontal() ? height * 0.5f : minSliderPos };
            maxPoint = { slider.isHorizontal() ? maxSliderPos : width * 0.5f, slider.isHorizontal() ? height * 0.5f : maxSliderPos };
        }
        else {
            auto kx = slider.isHorizontal() ? sliderPos : (x + width * 0.5f);
            auto ky = slider.isHorizontal() ? (y + height * 0.5f) : sliderPos;
            minPoint = startPoint;
            maxPoint = { kx, ky };
        }
        
        valueTrack.startNewSubPath (minPoint);
        valueTrack.lineTo (isThreeVal ? maxPoint : maxPoint);
        g.setColour (Colour(0xaa000000));
        g.strokePath (valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });
        
        if (! isTwoVal) {
            g.setColour (slider.findColour (Slider::thumbColourId));
            Path skull;
            skull.loadPathFromData(sadistic::Numbers::eight_svg, (size_t) sadistic::Numbers::eight_svgSize);
            skull.scaleToFit(roundToInt(maxPoint.x)-4, roundToInt(maxPoint.y)-5, 10, 10, true);
        }
    }
}

void sadistic::EmpiricalLAF::drawComboBox  (Graphics& g, int width, int height, bool, int, int, int, int, ComboBox& box) {
    auto cornerSize = box.findParentComponentOfClass<ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
    Rectangle<int> boxBounds (0, 0, width, height);
    
    g.setColour (box.findColour (ComboBox::backgroundColourId));
    g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);
    
    g.setColour (box.findColour (ComboBox::outlineColourId));
    g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);
    
    Rectangle<int> arrowZone (width - 30, 0, 20, height);
    Path path;
    path.startNewSubPath ((float) arrowZone.getX() + 3.0f, (float) arrowZone.getCentreY() - 2.0f);
    path.lineTo ((float) arrowZone.getCentreX(), (float) arrowZone.getCentreY() + 3.0f);
    path.lineTo ((float) arrowZone.getRight() - 3.0f, (float) arrowZone.getCentreY() - 2.0f);
    
    g.setColour (box.findColour (ComboBox::arrowColourId).withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
    g.strokePath (path, PathStrokeType (2.0f));
}

void sadistic::EmpiricalLAF::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                                        float rotaryStartAngle, float rotaryEndAngle, Slider& slider) {
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto& eSlider { static_cast<sadistic::EmpiricalSlider&>(slider) };
    bool left { eSlider.isLeft };
    Rectangle<float> r (x, y, width, height);
    
    

    Path need;
    
    Path needles[numNeedles], num[numNumbers];
    Path tensPlaceDigits[numNumbers], hundredsPlaceDigits[2];
    if (!eSlider.isSmall) {
        tensPlaceDigits[0].loadPathFromData(sadistic::Numbers::zero_svg, (size_t) sadistic::Numbers::zero_svgSize);
        tensPlaceDigits[1].loadPathFromData(sadistic::Numbers::one_svg, (size_t) sadistic::Numbers::one_svgSize);
        tensPlaceDigits[2].loadPathFromData(sadistic::Numbers::two_svg, (size_t) sadistic::Numbers::two_svgSize);
        tensPlaceDigits[3].loadPathFromData(sadistic::Numbers::three_svg, (size_t) sadistic::Numbers::three_svgSize);
        tensPlaceDigits[4].loadPathFromData(sadistic::Numbers::four_svg, (size_t) sadistic::Numbers::four_svgSize);
        tensPlaceDigits[5].loadPathFromData(sadistic::Numbers::five_svg, (size_t) sadistic::Numbers::five_svgSize);
        tensPlaceDigits[6].loadPathFromData(sadistic::Numbers::six_svg, (size_t) sadistic::Numbers::six_svgSize);
        tensPlaceDigits[7].loadPathFromData(sadistic::Numbers::seven_svg, (size_t) sadistic::Numbers::seven_svgSize);
        tensPlaceDigits[8].loadPathFromData(sadistic::Numbers::eight_svg, (size_t) sadistic::Numbers::eight_svgSize);
        tensPlaceDigits[9].loadPathFromData(sadistic::Numbers::nine_svg, (size_t) sadistic::Numbers::nine_svgSize);
        tensPlaceDigits[10].loadPathFromData(sadistic::Numbers::zero_svg, (size_t) sadistic::Numbers::zero_svgSize);
        tensPlaceDigits[11].loadPathFromData(sadistic::Numbers::one_svg, (size_t) sadistic::Numbers::one_svgSize);
        hundredsPlaceDigits[0].loadPathFromData(sadistic::Numbers::one_svg, (size_t) sadistic::Numbers::one_svgSize);
        hundredsPlaceDigits[1].loadPathFromData(sadistic::Numbers::one_svg, (size_t) sadistic::Numbers::one_svgSize);
        g.setColour(Colour(Colours::black.withAlpha(0.5f)));
        g.fillEllipse(r);
    }
    
    const auto centreX { static_cast<float>(x + width/2) }, top { static_cast<float>(y) };
    Rectangle<float> smallTick { centreX - 0.5f, top, 1.f, 6.f }, mediumTick { centreX - 0.5f, top, 1.f, 8.f }, bigTick { centreX - 1.f, top, 2.f, 10.f };

    for (int i = 0; i < (eSlider.isSmall ? 12 : numNeedles); i++) {
        if (i % 5 == 0){
            if ((i) % 2 == 0)
                needles[i].addRectangle (bigTick);
            needles[i].addRectangle (mediumTick);
        }
        else needles[i].addRectangle (smallTick);
    }
    
    if(eSlider.isSmall) {
        Path ring;
        ring.startNewSubPath(slider.getHeight()/2.f, 0.f);
        ring.addCentredArc(r.getCentreX(), r.getCentreY(), slider.getHeight()/2.f - 6.f, slider.getHeight()/2.f - 6.f, 0.f, left ? MathConstants<float>::pi : 0.f, left ? MathConstants<float>::twoPi : MathConstants<float>::pi, true);
        g.setColour(Colours::darkgrey.darker().withAlpha(0.7f));
        g.strokePath (ring, PathStrokeType (13.f), AffineTransform::translation(left ? slider.getHeight()/2 : -slider.getHeight()/2, 0));
        const float totalDegrees { degreesToRadians(270.f) };
        g.setColour (Colour(Colours::white));
        for (int i = 0; i < 12; i++)
            g.fillPath (needles[i], AffineTransform::rotation (angle - i * totalDegrees/12.f + degreesToRadians(90.f), r.getCentreX(), r.getCentreY()).translated(left ? slider.getHeight()/2 : -slider.getHeight()/2, 0));
    }
    else {
        g.setColour (Colour(Colours::white));
        for (int i = 0; i < numNeedles; i++)
            g.fillPath (needles[i], AffineTransform::rotation (angle - degreesToRadians(i * 3.f) + degreesToRadians(90.f), r.getCentreX(), r.getCentreY()));

        Rectangle<float> digitBox { 6.f, 16.f };
        float midY { static_cast<float>(height/2) }, spacing { left ? -digitBox.getWidth() : digitBox.getWidth() }, edge { static_cast<float>(left ? slider.getWidth() : 0.f) };
        Point<float> nearPos { edge + spacing * 3.f, midY }, midPos { edge + spacing * 5.f, midY }, farPos { edge + spacing * 7.f, midY };
        Rectangle<float> nearBox { digitBox.withCentre(nearPos) }, midBox { digitBox.withCentre(midPos) }, farBox { digitBox.withCentre(farPos) };

        const auto verticalFlip = AffineTransform::verticalFlip(digitBox.getHeight());
        auto hundredsBox = farBox, tensBox = midBox, onesBox = nearBox;

        for (int i = 0; i < numNumbers; ++i) {
            const AffineTransform rot = AffineTransform::rotation (angle - degreesToRadians(i * 30.f) + degreesToRadians(left ? 0.f : 180.f), r.getCentreX(), r.getCentreY());
            if(i == 10 && left == false) {
                onesBox = farBox;
                tensBox = midBox;
                hundredsBox = nearBox;
            }
            if(i == 1 && left == false) {
                onesBox = midBox;
                tensBox = nearBox;
            }
            if(i > 0) {
                tensPlaceDigits[i].applyTransform(verticalFlip);
                tensPlaceDigits[i].scaleToFit(tensBox.getX(), tensBox.getY(), tensBox.getWidth(), tensBox.getHeight(), true);
                g.fillPath(tensPlaceDigits[i], rot);//AffineTransform(x).followedBy(AffineTransform::verticalFlip(16.f)));
            }
            if(i > 9) {
                Path p;
                p.loadPathFromData(sadistic::Numbers::one_svg, (size_t) sadistic::Numbers::one_svgSize);
                p.applyTransform(verticalFlip);
                p.scaleToFit(hundredsBox.getX(), hundredsBox.getY(), hundredsBox.getWidth(), hundredsBox.getHeight(), true);
                g.fillPath(p, rot);
            }
            Path p;
            p.loadPathFromData(sadistic::Numbers::zero_svg, (size_t) sadistic::Numbers::zero_svgSize);
            p.scaleToFit(onesBox.getX(), onesBox.getY(), onesBox.getWidth(), onesBox.getHeight(), true);
            g.fillPath(p, rot);
        }
    }
}

String freqString(int i) {
    if (i > 999) {
        int first =(int)(i / 1000);
        auto remainder = i - first * 1000;
        if (i > 10000 || (int)(i / 1000) == (float) i / 1000.f) return String(String(i).dropLastCharacters(3)+ "K");
        else return String(String(i).dropLastCharacters(3)+ "." + String(remainder).dropLastCharacters(2) + "K");
    }
    else return String(i);
}

void sadistic::LeftEmpiricalLAF::drawLabel (Graphics& g, Label& label) {
    auto text { label.getText().getDoubleValue() };
    auto* parent = label.getParentComponent();
    auto parentBounds = parent->getLocalBounds();
    auto localArea = label.getLocalBounds().withCentre({label.getLocalBounds().getWidth()/2, parentBounds.getHeight()/2});
    auto textArea = localArea.reduced(5);
    g.setFont (getSadisticFont (textArea.getHeight()));
    if (! label.isBeingEdited()) {
        g.setColour (Colours::grey.withAlpha(label.isEnabled() ? 1.0f : 0.5f));
        g.drawFittedText (text > 1000 ? freqString((int)text) : String(text, text < 10.0 ? 2 : (text < 100.0 ? 1 : 0)), textArea, Justification::centred, 1, 0.05f);
        g.setColour (Colours::white.darker());
    }
    else if (label.isEnabled()) {
        g.setColour (Colours::white.darker());
        g.drawFittedText (text > 1000 ? freqString((int)text) : String(text, text < 10.0 ? 2 : (text < 100.0 ? 1 : 0)), textArea, Justification::centred, 1, 0.05f);
    }
    Path rectangle;
    g.setColour(Colours::white.darker());
    rectangle.addRoundedRectangle(localArea, 5);
    g.strokePath(rectangle, PathStrokeType(3.f));
    
}

void sadistic::RightEmpiricalLAF::drawLabel (Graphics& g, Label& label) {
    auto text { label.getText().getDoubleValue() };
    auto* parent = label.getParentComponent();
    auto parentBounds = parent->getLocalBounds();
    auto localArea = label.getLocalBounds().withCentre({ label.getLocalBounds().getRight() - label.getLocalBounds().getWidth()/2, parentBounds.getHeight()/2});
    auto textArea = localArea.reduced(5);
    g.setFont (getSadisticFont (textArea.getHeight()));
    if (! label.isBeingEdited()) {
        g.setColour (Colours::grey.withAlpha(label.isEnabled() ? 1.0f : 0.5f));
        g.drawFittedText (text > 1000 ? freqString((int)text) : String(text, text < 10.0 ? 2 : (text < 100.0 ? 1 : 0)), textArea, Justification::centred, 1, 0.15f);
        g.setColour (Colours::white.darker());
    }
    else if (label.isEnabled()) {
        g.setColour (Colours::white.darker());
        g.drawFittedText (text > 1000 ? freqString((int)text) : String(text, text < 10.0 ? 2 : (text < 100.0 ? 1 : 0)), textArea, Justification::centred, 1, 0.05f);
    }
    Path rectangle;
    g.setColour(Colours::white.darker());
    rectangle.addRoundedRectangle(localArea, 5);
    g.strokePath(rectangle, PathStrokeType(3.f));
    
}

void sadistic::OuterLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, Slider& slider) {
    auto outline {Colour(0x00000000)}, fill {Colour(0xaa000000)};
    auto bounds {Rectangle<int> (x, y, width, height).toFloat().reduced (10)};
    auto radius { static_cast<float>(slider.getWidth()/2) }, lineW {4.0f}, arcRadius {radius - lineW * 0.5f}, toAngle {rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle)};
    Path backgroundArc;
    backgroundArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (outline);
    if (slider.isEnabled()) {
        Path valueArc;
        valueArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (fill);
//        g.strokePath (valueArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));
    }
    auto& rSlider { dynamic_cast<OuterSlider&>(slider) };
    auto top { rSlider.isOnTop() }, left { rSlider.isOnLeft() };
    if(left) {
        auto digitBox { Rectangle<float>(20.f, 6.f).withCentre({ 10.f, height/2.f }) };

        Path p;
        auto filterWidth { 2.f };
        if(top) {
            p.addRectangle(digitBox.getCentreX() - 6.f, digitBox.getCentreY() - filterWidth/2.f, 16.f, filterWidth);
            p.addTriangle(digitBox.getX(), digitBox.getBottom(), digitBox.getCentreX() - 6.f, digitBox.getCentreY() - filterWidth, digitBox.getCentreX() - 6.f, digitBox.getCentreY() + filterWidth);
        }
        else {
            p.startNewSubPath(digitBox.getX(), digitBox.getCentreY());
            p.lineTo(digitBox.getCentreX() + 6.f, digitBox.getCentreY());
            p.lineTo(digitBox.getRight(), digitBox.getBottom());
        }

        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const auto rot = AffineTransform::rotation (angle + MathConstants<float>::halfPi, slider.getLocalBounds().getCentreX(), slider.getLocalBounds().getCentreY());
        g.setColour(Colours::white);
        g.strokePath(p, {PathStrokeType(5.f)}, rot);
        g.fillPath(p, rot);
}
    Point<float> thumbPoint (bounds.getCentreX() + arcRadius * std::cos (toAngle - MathConstants<float>::halfPi), bounds.getCentreY() + arcRadius * std::sin (toAngle - MathConstants<float>::halfPi));
    g.setColour (slider.findColour (Slider::thumbColourId));
}

bool sadistic::OuterSlider::hitTest (int x, int y)    {
    const auto rParams = getRotaryParameters();
    NormalisableRange<double> range { getMinimum(), getMaximum(), 0.0, getSkewFactor() };
    auto bounds {getLocalBounds().toFloat().reduced (10)};
    auto radius { static_cast<float>(getWidth()/2) }, lineW {4.0f}, arcRadius {radius - lineW * 0.5f}, toAngle { static_cast<float>(rParams.startAngleRadians + range.convertTo0to1(getValue()) * (rParams.endAngleRadians - rParams.startAngleRadians)) };
    
    Point<int> thumbPoint (static_cast<int>(bounds.getCentreX() + arcRadius * std::cos (toAngle - MathConstants<float>::halfPi)), static_cast<int>(bounds.getCentreY() + arcRadius * std::sin (toAngle - MathConstants<float>::halfPi)));
    
    return Point<int>(x,y).getDistanceFrom(thumbPoint) < 20;
};

bool sadistic::EmpiricalSlider::hitTest (int x, int y)    {
    auto centre = Point<int>(getLocalBounds().getCentre());
    return centre.getDistanceFrom({ x, y }) < getWidth()/2;};

bool sadistic::TransLabel::hitTest (int x, int y)    {
    return x<1 && y<1;};

void sadistic::showEmpiricalValue(Slider& slider, Label& valueLabel, Component& child) {
    valueLabel.setText (String(roundToInt(std::pow((slider.getValue() - slider.getMinimum()) / (slider.getMaximum() - slider.getMinimum()),1.01 * slider.getSkewFactor()) * (111.00))), dontSendNotification);
    child.setVisible(true);};

void sadistic::hideValue(Label& valueLabel, Label& suffixLabel) {
    valueLabel.setText ("", dontSendNotification);
    suffixLabel.setText ("", dontSendNotification);};

void sadistic::hideValue(Label& valueLabel, Component& child) {
    valueLabel.setText ("", dontSendNotification);
    child.setVisible(false);};

void sadistic::showIntegerValue(Slider& slider, Label& valueLabel, Label& suffixLabel)   {
    valueLabel.setText (String(roundToInt(slider.getValue())), dontSendNotification);
    suffixLabel.setText (slider.getTextValueSuffix(), dontSendNotification);};

void sadistic::showLevelValue(Slider& slider, Label& valueLabel, Label& suffixLabel) {
    double normal = std::pow(((slider.getValue() - slider.getMinimum()) / (slider.getMaximum() - slider.getMinimum())),slider.getSkewFactor());
    String currentValueString;
    if(normal == 0.00)  {currentValueString = "-inf";}
    else if(normal == 1.00)  {currentValueString = "Unity";}
    else{currentValueString = String((log10(normal) * 20),1,false);}
    valueLabel.setText (currentValueString, dontSendNotification);
    suffixLabel.setText (slider.getTextValueSuffix(), dontSendNotification);};

void sadistic::setWidgets(Slider& blendKnob, Slider& driveKnob, EmpiricalLAF& llaf, Slider& saturationKnob, Slider& hiKnob, OuterLookAndFeel& olaf, Slider& gateKnob, Slider& loKnob, Label& valueLabel, Label& suffixLabel, Font& font) {
    
    blendKnob.setScrollWheelEnabled(true);
    blendKnob.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
    blendKnob.setDoubleClickReturnValue(true,1.0f,ModifierKeys::altModifier);
    blendKnob.setMouseDragSensitivity (50);
    blendKnob.setLookAndFeel(&llaf);
    blendKnob.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    blendKnob.setColour(Slider::thumbColourId, Colours::cadetblue);
    blendKnob.setTextValueSuffix("%");
    
    driveKnob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    driveKnob.setRotaryParameters(0.0f, 5.81333f, true);
    driveKnob.setDoubleClickReturnValue(true,1.0f,ModifierKeys::altModifier);
    driveKnob.setMouseDragSensitivity (100);
    driveKnob.setLookAndFeel(&llaf);
    driveKnob.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    driveKnob.setTextValueSuffix(" ;)");
    
    saturationKnob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    saturationKnob.setRotaryParameters(degreesToRadians(180.f), degreesToRadians(513.f), true);
    saturationKnob.setDoubleClickReturnValue(true,1.0f,ModifierKeys::altModifier);
    saturationKnob.setMouseDragSensitivity (100);
    saturationKnob.setLookAndFeel(&llaf);
    saturationKnob.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    saturationKnob.setTextValueSuffix(" :)");
    
    gateKnob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    gateKnob.setRotaryParameters(degreesToRadians(76.f),degreesToRadians(50.f), true);
    gateKnob.setDoubleClickReturnValue(true,1.0f,ModifierKeys::altModifier);
    gateKnob.setMouseDragSensitivity (60);
    gateKnob.setLookAndFeel(&olaf);
    gateKnob.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    gateKnob.setColour(Slider::thumbColourId, Colour(Colour(0xff50af43)));
    gateKnob.setTextValueSuffix(" dB");
    
    hiKnob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    hiKnob.setDoubleClickReturnValue(true,1.0f,ModifierKeys::altModifier);
    hiKnob.setMouseDragSensitivity (60);
    hiKnob.setLookAndFeel(&olaf);
    hiKnob.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    hiKnob.setColour(Slider::thumbColourId, Colour(Colour(0xff50af43)));
    hiKnob.setTextValueSuffix(" Hz");

    loKnob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    loKnob.setDoubleClickReturnValue(true,1.0f,ModifierKeys::altModifier);
    loKnob.setMouseDragSensitivity (60);
    loKnob.setLookAndFeel(&olaf);
    loKnob.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    loKnob.setTextValueSuffix(" dB");
    
    font.setHeight(80);
    
    valueLabel.setLookAndFeel(&llaf);
    suffixLabel.setLookAndFeel(&llaf);
    valueLabel.setJustificationType(Justification::centred);
    
    valueLabel.setColour(juce::Label::textColourId, Colours::lightgrey);
    suffixLabel.setJustificationType(Justification::centred);
    suffixLabel.setColour(juce::Label::textColourId, Colours::lightgrey);
};
