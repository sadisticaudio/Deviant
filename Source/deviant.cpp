#include "deviant.h"

void sadistic::FilterLAF::drawLinearSlider (Graphics& g, int x, int y, int width, int height, float sliderPos, float, float, const Slider::SliderStyle, Slider& slider)  {
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
    auto kx = slider.isHorizontal() ? sliderPos : (x + width * 0.5f);
    auto ky = slider.isHorizontal() ? (y + height * 0.5f) : sliderPos;
    minPoint = startPoint;
    maxPoint = { kx, ky };
    valueTrack.startNewSubPath (minPoint);
    valueTrack.lineTo (maxPoint);
    g.setColour (Colour(0xaa000000));
    g.strokePath (valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });
    
//    auto colour { slider.findColour (Slider::thumbColourId) };
    
    auto thumbWidth = getSliderThumbRadius (slider);
    
    Colour thumbColour { Colours::grey };
    
    if (slider.isMouseOver()) {
        float dryRGB[] { drySignalColour.getFloatRed(), drySignalColour.getFloatGreen(), drySignalColour.getFloatBlue() };
        float wetRGB[] { wetSignalColour.getFloatRed(), wetSignalColour.getFloatGreen(), wetSignalColour.getFloatBlue() };
        float val { static_cast<float>(slider.getValue()) };
        auto [rWet,gWet,bWet] = wetRGB;
        auto [rDry,gDry,bDry] = dryRGB;
        float newColour[] { val * rWet + (1.f - val) * rDry, val * gWet + (1.f - val) * gDry, val * bWet + (1.f - val) * bDry };
        thumbColour = Colour::fromFloatRGBA(newColour[0], newColour[1], newColour[2], 1.f);
    }
    
    g.setColour (thumbColour);
    g.fillEllipse (Rectangle<float> (static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre (maxPoint));
}

void sadistic::EmpiricalLAF::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                                        float rotaryStartAngle, float rotaryEndAngle, Slider& slider) {
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto& eSlider { static_cast<sadistic::EmpiricalSlider&>(slider) };
    bool left { eSlider.isLeft };
    Rectangle<float> r (x, y, width, height);
    Path needles[numNeedles];
    if (!eSlider.isSmall) {
        g.setColour(Colour(Colours::black.withAlpha(0.5f)));
        g.fillEllipse(r);
    }

    const auto centreX { static_cast<float>(x + width/2) }, top { static_cast<float>(y) };
    Rectangle<float> smallTick { centreX - 0.5f, top, 1.f, 6.f }, mediumTick { centreX - 0.5f, top, 1.f, 8.f }, bigTick { centreX - 1.f, top, 2.f, 10.f };

    auto needlesToDisplay { eSlider.isLeft ? numNeedles : numNeedles - 11 };
    for (int i = 0; i < (eSlider.isSmall ? 12 : needlesToDisplay); i++) {
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
        auto degreesBetweenTicks { 330.f / (eSlider.isLeft ? 110.f : 100.f) };
        g.setColour (Colour(Colours::white));
        for (int i = 0; i < numNeedles; i++)
            g.fillPath (needles[i], AffineTransform::rotation (angle - degreesToRadians(i * degreesBetweenTicks) + degreesToRadians(90.f), r.getCentreX(), r.getCentreY()));

        Rectangle<float> digitBox { 6.f, 16.f };
        float midY { static_cast<float>(height/2) }, spacing { left ? -digitBox.getWidth() : digitBox.getWidth() }, edge { static_cast<float>(left ? slider.getWidth() : 0.f) };
        Point<float> nearPos { edge + spacing * 3.f, midY }, midPos { edge + spacing * 5.f, midY }, farPos { edge + spacing * 7.f, midY };
        Rectangle<float> nearBox { digitBox.withCentre(nearPos) }, midBox { digitBox.withCentre(midPos) }, farBox { digitBox.withCentre(farPos) };

        auto hundredsBox = farBox, tensBox = midBox, onesBox = nearBox;

        auto numbersToDisplay { eSlider.isLeft ? numNumbers : numNumbers - 1 };
        for (int i = 0; i < numbersToDisplay; ++i) {
            const AffineTransform rot = AffineTransform::rotation (angle - degreesToRadians(i * 10.f * degreesBetweenTicks) + degreesToRadians(left ? 0.f : 180.f), r.getCentreX(), r.getCentreY());
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
                auto newTensBox = tensBox.reduced(tensBox.getWidth()/4.f, -tensBox.getHeight()/4.f);
                auto b = tensPlaceNums[i]->getBounds().toFloat();
                auto placementTransform { AffineTransform::fromTargetPoints(b.getTopLeft(), newTensBox.getTopLeft(), b.getTopRight(), newTensBox.getTopRight(), b.getBottomLeft(), newTensBox.getBottomLeft()) };
                tensPlaceNums[i]->draw(g, 1.f, placementTransform.followedBy(rot));
            }
            if(i > 9) {
                auto newHundredsBox = hundredsBox.reduced(hundredsBox.getWidth()/4.f, -hundredsBox.getHeight()/4.f);
                auto b = hundredsPlaceNums[i - 10]->getDrawableBounds().toFloat();
                auto placementTransform { AffineTransform::fromTargetPoints(b.getTopLeft(), newHundredsBox.getTopLeft(), b.getTopRight(), newHundredsBox.getTopRight(), b.getBottomLeft(), newHundredsBox.getBottomLeft()) };
                hundredsPlaceNums[i - 10]->draw(g, 1.f, placementTransform.followedBy(rot));
            }
            auto newOnesBox = onesBox.reduced(onesBox.getWidth()/4.f, -onesBox.getHeight()/4.f);
            auto b = onesPlaceNums[i]->getBounds().toFloat();
            auto placementTransform { AffineTransform::fromTargetPoints(b.getTopLeft(), newOnesBox.getTopLeft(), b.getTopRight(), newOnesBox.getTopRight(), b.getBottomLeft(), newOnesBox.getBottomLeft()) };
            onesPlaceNums[i]->draw(g, 1.f, placementTransform.followedBy(rot));
        }
    }
}

void sadistic::FilterLAF::drawRotarySlider (Graphics& g, int x, int y, int, int, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, Slider& slider) {
    auto& fKnob { static_cast<sadistic::FilterKnob&>(slider) };
    auto& icon { fKnob.icon };
    const auto colour { fKnob.colour };
    auto bounds = slider.getLocalBounds().toFloat();
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto transform = juce::AffineTransform:: rotation (toAngle, bounds.getCentreX(), bounds.getCentreY());
    icon->setTransformToFit (bounds.toFloat(), RectanglePlacement::centred);
    icon->setTransform(icon->getTransform().followedBy(transform));
    icon->drawAt(g, x, y, 1.f);//
    g.setColour(colour);
}

bool sadistic::EmpiricalSlider::hitTest (int x, int y)    {
    auto centre = Point<int>(getLocalBounds().getCentre());
    return centre.getDistanceFrom({ x, y }) < getWidth()/2;};

bool sadistic::TransLabel::hitTest (int x, int y) { return x<1 && y<1; };

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
    valueLabel.setText (String(roundToInt(slider.getValue() * 100.0)), dontSendNotification);
    suffixLabel.setText (slider.getTextValueSuffix(), dontSendNotification);};

void sadistic::showLevelValue(Slider& slider, Label& valueLabel, Label& suffixLabel) {
    double normal = std::pow(((slider.getValue() - slider.getMinimum()) / (slider.getMaximum() - slider.getMinimum())),slider.getSkewFactor());
    String currentValueString;
    if(normal == 0.00)  {currentValueString = "-inf";}
    else if(normal == 1.00)  {currentValueString = "Unity";}
    else{currentValueString = String((log10(normal) * 20),1,false);}
    valueLabel.setText (currentValueString, dontSendNotification);
    suffixLabel.setText (slider.getTextValueSuffix(), dontSendNotification);};
