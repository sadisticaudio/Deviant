#include "sadistic.h"
#include "Images.h"
#include <numeric>
using namespace sadistic;

void SlidingLookAndFeel::drawLabel (Graphics& g, Label& label) {
    g.fillAll (label.findColour (Label::backgroundColourId));
    if (! label.isBeingEdited()) {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const Font font (getLabelFont (label));
        g.setColour (label.findColour (Label::textColourId).withMultipliedAlpha (alpha));
        g.setFont (font);
        auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());
        g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                          jmax (1, (int) (textArea.getHeight() / font.getHeight())),
                          label.getMinimumHorizontalScale());
        g.setColour (label.findColour (Label::outlineColourId).withMultipliedAlpha (alpha));
    }
    else if (label.isEnabled()) {
        g.setColour (label.findColour (Label::outlineColourId));
    }
    g.drawRect (label.getLocalBounds());
}

void BlendLookAndFeel::drawLinearSlider (Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& slider)  {
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
            Image skull = ImageCache::getFromMemory (Images::skull_png, Images::skull_pngSize);
            g.drawImageWithin(skull, roundToInt(maxPoint.x)-4, roundToInt(maxPoint.y)-5, 10, 10, 64);
        }
        
        if (isTwoVal || isThreeVal) {
            auto sr = jmin (trackWidth, (slider.isHorizontal() ? height : width) * 0.4f);
            auto pointerColour = Colour(Colour(0xff50ffee));
            
            if (slider.isHorizontal()) {
                drawPointer (g, minSliderPos - sr, jmax (0.0f, y + height * 0.5f - trackWidth * 2.0f), trackWidth * 2.0f, pointerColour, 2);
                drawPointer (g, maxSliderPos - trackWidth, jmin (y + height - trackWidth * 2.0f, y + height * 0.5f), trackWidth * 2.0f, pointerColour, 4);
            }
            else {
                drawPointer (g, jmax (0.0f, x + width * 0.5f - trackWidth * 2.0f), minSliderPos - trackWidth, trackWidth * 2.0f, pointerColour, 1);
                drawPointer (g, jmin (x + width - trackWidth * 2.0f, x + width * 0.5f), maxSliderPos - sr, trackWidth * 2.0f, pointerColour, 3);
            }
        }
    }
}

void LeftLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                                        float rotaryStartAngle, float rotaryEndAngle, Slider& slider) {
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    Rectangle<float> r (x, y, width, height);
    Rectangle<float> r0 (width * 0.13f, height * 0.13f, width * 0.74f, height * 0.74f);
    Rectangle<float> r05 (width * 0.1f, height * 0.1f, width * 0.8f, height * 0.8f);
    Rectangle<float> r1 (width * 0.095f, height * 0.095f, width * 0.81f, height * 0.81f);
    Rectangle<float> r2 (width * 0.06f, height * 0.06f, width * 0.88f, height * 0.88f);
    Rectangle<float> r3 {1.f,6.f};
    Rectangle<float> r4 {2.f,10.f};
    Rectangle<float> r5 {1.f,8.f};
    int needleSize = 111, pathSize = 12, pathSizeX = 12;
    float pathWidth = 6.f, pathHeight = 16.f, newPathWidth = 16.f, newPathHeight = 6.f;
    Path needles[111], needleZero, path[12], pathZero, circle[3];
    g.setColour(Colour(0x8803cf2a));
    circle[0].addEllipse(0, 0, 200, 200);   g.fillPath(circle[0]);
    circle[1].addEllipse(36, 35, 128, 128); g.fillPath(circle[1]);
    circle[2].addEllipse(43,47,113,113);    g.fillPath(circle[2]);
    for (int i = 0; i < needleSize; i++)    {
        if ((i + 1) % 5 == 0){
            if ((i + 1) % 2 == 0){
                needles[i].addRectangle (r4.withPosition ({ r.getCentreX() - (r4.getWidth() / 2.0f), r.getY() }));}
            needles[i].addRectangle (r5.withPosition ({ r.getCentreX() - (r5.getWidth() / 2.0f), r.getY() }));}
        else {needles[i].addRectangle (r3.withPosition ({ r.getCentreX() - (r3.getWidth() / 2.0f), r.getY() }));}}
    g.setColour (Colour(0xff000000));
    for (int i = 0; i < needleSize; i++) {
        g.fillPath (needles[i], AffineTransform::rotation (angle - degreesToRadians(i * 3.f) + degreesToRadians(87.f), r.getCentreX(), r.getCentreY()));}
    needleZero.addRectangle (r4.withPosition ({ r.getCentreX() - (r4.getWidth() / 2.0f), r.getY() }));
    g.fillPath (needleZero, AffineTransform::rotation (angle + degreesToRadians(90.f), r.getCentreX(), r.getCentreY()));
    path[0].loadPathFromData(Images::zero_svg, Images::zero_svgSize);
    path[1].loadPathFromData(Images::one_svg, Images::one_svgSize);
    path[2].loadPathFromData(Images::two_svg, Images::two_svgSize);
    path[3].loadPathFromData(Images::three_svg, Images::three_svgSize);
    path[4].loadPathFromData(Images::four_svg, Images::four_svgSize);
    path[5].loadPathFromData(Images::five_svg, Images::five_svgSize);
    path[6].loadPathFromData(Images::six_svg, Images::six_svgSize);
    path[7].loadPathFromData(Images::seven_svg, Images::seven_svgSize);
    path[8].loadPathFromData(Images::eight_svg, Images::eight_svgSize);
    path[9].loadPathFromData(Images::nine_svg, Images::nine_svgSize);
    path[10].loadPathFromData(Images::zero_svg, Images::zero_svgSize);
    path[11].loadPathFromData(Images::one_svg, Images::one_svgSize);
    pathZero.loadPathFromData(Images::zero_svg, Images::zero_svgSize);
    const juce::AffineTransform rot90 = AffineTransform::rotation(degreesToRadians(90.f),pathWidth/2.f, pathHeight/2.f);
    pathZero.applyTransform(rot90);
    
    for (int i = 1; i < pathSize; i++)  {
        const juce::AffineTransform flip = AffineTransform::verticalFlip(newPathHeight).followedBy(AffineTransform::rotation(degreesToRadians(270.f),newPathWidth/2, newPathHeight/2));
        path[i].applyTransform(flip);
        path[i].scaleToFit(r1.getCentreX() - (newPathWidth / 2.0f), r1.getY(), newPathWidth, newPathHeight, false);}
    
    for (int i = 0; i < pathSizeX; i++) {
        const juce::AffineTransform rot = pathZero.getTransformToScaleToFit(r2.getCentreX() - (newPathWidth / 2.0f), r2.getY(), newPathWidth, newPathHeight, false);
        pathZero.applyTransform(rot);}
    
    for (int i = 0; i < pathSizeX; i++) {
        g.fillPath (pathZero, AffineTransform::rotation (angle - degreesToRadians(i * 30.f) + degreesToRadians(90.f), r2.getCentreX(), r2.getCentreY()));}
    
    for (int i = 2; i < 11; i++) {
        g.fillPath (path[i], AffineTransform::rotation (angle - degreesToRadians(i * 30.f) + degreesToRadians(90.f), r1.getCentreX(), r1.getCentreY()));}
    
    float newestPathWidth = 16.f, newestPathHeight = 4.f;
    path[1].scaleToFit(r0.getCentreX() - (newestPathWidth / 2.0f), r0.getY(), newestPathWidth, newestPathHeight, false);
    
    g.fillPath (path[1], AffineTransform::rotation (angle - degreesToRadians(300.f) + degreesToRadians(90.f), r0.getCentreX(), r0.getCentreY()));
    
    g.fillPath (path[1], AffineTransform::rotation (angle - degreesToRadians(330.f) + degreesToRadians(90.f), r0.getCentreX(), r0.getCentreY()));
    
    path[1].scaleToFit(r1.getCentreX() - (newestPathWidth / 2.0f), r1.getY(), newestPathWidth, newestPathHeight, false);
    
    g.fillPath (path[1], AffineTransform::rotation (angle - degreesToRadians(30.f) + degreesToRadians(90.f), r1.getCentreX(), r1.getCentreY()));
    
    path[1].scaleToFit(r05.getCentreX() - (newestPathWidth / 2.0f), r05.getY(), newestPathWidth, newestPathHeight, false);
    
    g.fillPath (path[1], AffineTransform::rotation (angle - degreesToRadians(330.f) + degreesToRadians(90.f), r05.getCentreX(), r05.getCentreY()));
}

void RightLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                                         float rotaryStartAngle, float rotaryEndAngle, Slider& slider) {
    float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    Rectangle<float> r (x, y, width, height), r0 (width * 0.11f, height * 0.11f, width * 0.78f, height * 0.78f), r00 (width * 0.12f, height * 0.12f, width * 0.76f, height * 0.76f), r05 (width * 0.085f, height * 0.085f, width * 0.83f, height * 0.83f), r1 (width * 0.095f, height * 0.095f, width * 0.81f, height * 0.81f), r2 (width * 0.06f, height * 0.06f, width * 0.88f, height * 0.88f);
    Path circle[3];
    g.setColour(Colour(0x8852BAD5));
    g.fillEllipse(0.f, 0.f, 200.f, 200.f);
    g.fillEllipse(36.f, 35.f, 128.f, 128.f);
    g.fillEllipse(43.f,47.f,113.f,113.f);
    g.setColour(Colour(0x44000000));
    g.strokePath(circle[0], PathStrokeType(1.f));
    g.strokePath(circle[1], PathStrokeType(2.f));
    g.strokePath(circle[2], PathStrokeType(2.f));
    
    Rectangle<float> r3 {1.f,6.f}, r4 {2.f,10.f}, r5 {1.f,8.f};
    int needleSize {111}, pathSize {12};
    float pathWidth {6.f}, pathHeight {16.f};
    Path needles[111], needleZero, path[12], pathZero, pathNewZero;
    
    for (int i = 0; i < needleSize; i++) {
        if ((i + 1) % 5 == 0){
            if ((i + 1) % 2 == 0){
                needles[i].addRectangle (r4.withPosition ({ r.getCentreX() - (r4.getWidth() / 2.0f), r.getY() }));}
            needles[i].addRectangle (r5.withPosition ({ r.getCentreX() - (r5.getWidth() / 2.0f), r.getY() }));}
        else {needles[i].addRectangle (r3.withPosition ({ r.getCentreX() - (r3.getWidth() / 2.0f), r.getY() }));}
    }
    
    g.setColour (Colour(0xff000000));
    for (int i = 0; i<needleSize; i++) {
        g.fillPath (needles[i], AffineTransform::rotation (angle - degreesToRadians(i * 3.f) + degreesToRadians(87.f), r.getCentreX(), r.getCentreY()));
    }
    
    needleZero.addRectangle (r4.withPosition ({ r.getCentreX() - (r4.getWidth() / 2.0f), r.getY() }));
    g.fillPath (needleZero, AffineTransform::rotation (angle + degreesToRadians(90.f), r.getCentreX(), r.getCentreY()));
    
    path[0].loadPathFromData(Images::zero_svg, Images::zero_svgSize);
    path[1].loadPathFromData(Images::one_svg, Images::one_svgSize);
    path[2].loadPathFromData(Images::two_svg, Images::two_svgSize);
    path[3].loadPathFromData(Images::three_svg, Images::three_svgSize);
    path[4].loadPathFromData(Images::four_svg, Images::four_svgSize);
    path[5].loadPathFromData(Images::five_svg, Images::five_svgSize);
    path[6].loadPathFromData(Images::six_svg, Images::six_svgSize);
    path[7].loadPathFromData(Images::seven_svg, Images::seven_svgSize);
    path[8].loadPathFromData(Images::eight_svg, Images::eight_svgSize);
    path[9].loadPathFromData(Images::nine_svg, Images::nine_svgSize);
    path[10].loadPathFromData(Images::zero_svg, Images::zero_svgSize);
    path[11].loadPathFromData(Images::one_svg, Images::one_svgSize);
    pathZero.loadPathFromData(Images::zero_svg, Images::zero_svgSize);
    pathNewZero.loadPathFromData(Images::zero_svg, Images::zero_svgSize);
    const juce::AffineTransform rot90 {AffineTransform::rotation(degreesToRadians(90.f),pathWidth/2.f, pathHeight/2.f)}, flip {AffineTransform::verticalFlip(pathWidth).followedBy(AffineTransform::rotation(degreesToRadians(90.f),pathWidth/2.f, pathHeight/2.f))};
    pathZero.applyTransform(rot90);
    pathNewZero.applyTransform(rot90);
    
    for (int i = 0; i < pathSize; i++) {
        float newPathWidth = 16.f, newPathHeight = 6.f;
        path[i].applyTransform(flip);
        path[i].scaleToFit(r2.getCentreX() - (newPathWidth / 2.0f), r2.getY(), newPathWidth, newPathHeight, false);
    }
    for (int i = 0; i < 10; i++) {
        float newerPathWidth = 16.f, newerPathHeight = 6.f;
        const juce::AffineTransform rot = pathZero.getTransformToScaleToFit(r1.getCentreX() - (newerPathWidth / 2.0f), r1.getY(), newerPathWidth, newerPathHeight, false);
        pathZero.applyTransform(rot);
    }
    for (int i = 2; i < 10; i++) {
        g.fillPath (pathZero, AffineTransform::rotation (angle - degreesToRadians(i * 30.f) + degreesToRadians(90.f), r1.getCentreX(), r1.getCentreY()));
    }
    for (int i = 1; i<10; i++) {
        if (i != 1) {
            g.fillPath (path[i], AffineTransform::rotation (angle - degreesToRadians(i * 30.f) + degreesToRadians(90.f), r2.getCentreX(), r2.getCentreY()));
        }
    }
    
    float newestPathWidth = 16.f, newestPathHeight = 4.f;
    path[1].scaleToFit(r2.getCentreX() - (newestPathWidth / 2.0f), r2.getY(), newestPathWidth, newestPathHeight, false);
    
    g.fillPath (path[1], AffineTransform::rotation (angle - degreesToRadians(1.f * 30.f) + degreesToRadians(90.f), r2.getCentreX(), r2.getCentreY()));
    g.fillPath (path[1], AffineTransform::rotation (angle - degreesToRadians(10.f * 30.f) + degreesToRadians(90.f), r2.getCentreX(), r2.getCentreY()));
    g.fillPath (path[1], AffineTransform::rotation (angle - degreesToRadians(11.f * 30.f) + degreesToRadians(90.f), r2.getCentreX(), r2.getCentreY()));
    path[1].scaleToFit(r05.getCentreX() - (newestPathWidth / 2.0f), r05.getY(), newestPathWidth, newestPathHeight, false);
    g.fillPath (path[1], AffineTransform::rotation (angle - degreesToRadians(11.f * 30.f) + degreesToRadians(90.f), r05.getCentreX(), r05.getCentreY()));
    pathNewZero.scaleToFit(r05.getCentreX() - (newestPathWidth / 2.0f), r05.getY(), newestPathWidth, newestPathHeight + 2.f, false);
    g.fillPath (pathNewZero, AffineTransform::rotation (angle - degreesToRadians(10.f * 30.f) + degreesToRadians(90.f), r05.getCentreX(), r05.getCentreY()));
    g.fillPath (pathNewZero, AffineTransform::rotation (angle - degreesToRadians(1.f * 30.f) + degreesToRadians(90.f), r05.getCentreX(), r05.getCentreY()));
    path[0].scaleToFit(r00.getCentreX() - (newestPathWidth / 2.0f), r00.getY(), newestPathWidth, newestPathHeight + 2.f, false);
    g.fillPath (path[0], AffineTransform::rotation (angle - degreesToRadians(10.f * 30.f) + degreesToRadians(90.f), r00.getCentreX(), r00.getCentreY()));
    path[0].scaleToFit(r0.getCentreX() - (newestPathWidth / 2.0f), r0.getY(), newestPathWidth, newestPathHeight + 2.f, false);
    g.fillPath (path[0], AffineTransform::rotation (angle - degreesToRadians(11.f * 30.f) + degreesToRadians(90.f), r0.getCentreX(), r0.getCentreY()));
    pathZero.scaleToFit(r2.getCentreX() - (8.0f), r2.getY(), 16.f, 6.f, false);
    g.fillPath (pathZero, AffineTransform::rotation (angle + degreesToRadians(90.f), r2.getCentreX(), r2.getCentreY()));
}

void MiddleLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) {
    auto outline {Colour(0x00000000)}, fill {Colour(0xaa000000)};
    auto bounds {Rectangle<int> (x, y, width, height).toFloat().reduced (10)};
    auto radius {130.0f}, lineW {4.0f}, arcRadius {radius - lineW * 0.5f}, toAngle {rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle)};
    Path backgroundArc;
    backgroundArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (outline);
    g.strokePath (backgroundArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));
    
    if (slider.isEnabled()) {
        Path valueArc;
        valueArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (fill);
        g.strokePath (valueArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));
    }
    
    auto thumbWidth = lineW * 1.5f;
    Point<float> thumbPoint (bounds.getCentreX() + arcRadius * std::cos (toAngle - MathConstants<float>::halfPi), bounds.getCentreY() + arcRadius * std::sin (toAngle - MathConstants<float>::halfPi));
    g.setColour (slider.findColour (Slider::thumbColourId));
    g.fillEllipse (Rectangle<float> (thumbWidth, thumbWidth).withCentre (thumbPoint));
}

void UpperLeftLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) {
    auto outline {Colour(0xaa000000)}, fill {Colour(0xff377077)};
    auto bounds {Rectangle<int> (x, y, width, height).toFloat().reduced (10)};
    auto radius {130.0f}, lineW {4.0f}, arcRadius {radius - lineW * 0.5f}, toAngle {rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle)};
    Path backgroundArc;
    backgroundArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (outline);
    g.strokePath (backgroundArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));
    if (slider.isEnabled()) {
        Path valueArc;
        valueArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);
        g.setColour (fill);
        g.strokePath (valueArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));
    }
    
    auto thumbWidth = lineW * 1.5f;
    Point<float> thumbPoint (bounds.getCentreX() + arcRadius * std::cos (toAngle - MathConstants<float>::halfPi), bounds.getCentreY() + arcRadius * std::sin (toAngle - MathConstants<float>::halfPi));
    g.setColour (slider.findColour (Slider::thumbColourId));
    g.fillEllipse (Rectangle<float> (thumbWidth, thumbWidth).withCentre (thumbPoint));
}

void UpperRightLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) {
    auto outline {Colour(0xaa000000)}, fill    {Colour(0xff3a7778)};
    auto bounds {Rectangle<int> (x, y, width, height).toFloat().reduced (10)};
    auto radius {130.0f}, toAngle {rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle)}, lineW {4.0f}, arcRadius {radius - lineW * 0.5f};
    Path backgroundArc;
    backgroundArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (outline);
    g.strokePath (backgroundArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));
    
    if (slider.isEnabled()) {
        Path valueArc;
        valueArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);
        g.setColour (fill);
        g.strokePath (valueArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));
    }
    
    auto thumbWidth {lineW * 1.5f};
    Point<float> thumbPoint (bounds.getCentreX() + arcRadius * std::cos (toAngle - MathConstants<float>::halfPi), bounds.getCentreY() + arcRadius * std::sin (toAngle - MathConstants<float>::halfPi));
    g.setColour (slider.findColour (Slider::thumbColourId));
    g.fillEllipse (Rectangle<float> (thumbWidth, thumbWidth).withCentre (thumbPoint));
}

bool UpperSlider::hitTest (int x, int y)    {
    auto centre = Point<int> (150,150);    return centre.getDistanceFrom(Point<int> (x,y)) < 135 && centre.getDistanceFrom(Point<int> (x,y)) > 115 && y < 130;}

bool LowerSlider::hitTest (int x, int y)    {
    return Point<int> (150,150).getDistanceFrom(Point<int> (x,y)) > 115 && Point<int> (150, 150).getDistanceFrom(Point<int> (x,y)) < 135 && y > 170;}

bool EmpiricalSlider::hitTest (int x, int y)    {
    auto centre = Point<int>(getWidth()/2,getHeight()/2);
    return Point<int>(getWidth()/2,getHeight()/2).getDistanceFrom(Point<int> (x,y)) < 101;};

bool TransLabel::hitTest (int x, int y)    {
    return x<1 && y<1;};

void sadistic::showEmpiricalValue(std::unique_ptr<Slider>& slider, Label& valueLabel, Component& child) {
    valueLabel.setText (String(roundToInt(std::pow((slider->getValue() - slider->getMinimum()) / (slider->getMaximum() - slider->getMinimum()),1.01 * slider->getSkewFactor()) * (111.00))), dontSendNotification);
    child.setVisible(true);};

void sadistic::hideValue(Label& valueLabel, Label& suffixLabel) {
    valueLabel.setText ("", dontSendNotification);
    suffixLabel.setText ("", dontSendNotification);};

void sadistic::hideValue(Label& valueLabel, Component& child) {
    valueLabel.setText ("", dontSendNotification);
    child.setVisible(false);};

void sadistic::showIntegerValue(std::unique_ptr<Slider>& slider, Label& valueLabel, Label& suffixLabel)   {
    valueLabel.setText (String(roundToInt(slider->getValue())), dontSendNotification);
    suffixLabel.setText (slider->getTextValueSuffix(), dontSendNotification);};

void sadistic::showLevelValue(std::unique_ptr<Slider>& slider, Label& valueLabel, Label& suffixLabel) {
    double normal = std::pow(((slider->getValue() - slider->getMinimum()) / (slider->getMaximum() - slider->getMinimum())),slider->getSkewFactor());
    String currentValueString;
    if(normal == 0.00)  {currentValueString = "-inf";}
    else if(normal == 1.00)  {currentValueString = "Unity";}
    else{currentValueString = String((log10(normal) * 20),1,false);}
    valueLabel.setText (currentValueString, dontSendNotification);
    suffixLabel.setText (slider->getTextValueSuffix(), dontSendNotification);};

void sadistic::setWidgets(std::unique_ptr<Slider>& blendKnob, BlendLookAndFeel& blaf, std::unique_ptr<Slider>& driveKnob, LeftLookAndFeel& llaf, std::unique_ptr<Slider>& saturationKnob, RightLookAndFeel& rlaf, std::unique_ptr<Slider>& filterKnob, UpperRightLookAndFeel& urlaf, std::unique_ptr<Slider>& gateKnob, UpperLeftLookAndFeel& ullaf, std::unique_ptr<Slider>& gainKnob, MiddleLookAndFeel& gmlaf, std::unique_ptr<Slider>& volumeKnob, MiddleLookAndFeel& vmlaf, Label& valueLabel, Label& suffixLabel, SlidingLookAndFeel& slaf, Font& font) {
    
    blendKnob->setScrollWheelEnabled(true);
    blendKnob->setSliderStyle(Slider::SliderStyle::LinearHorizontal);
    blendKnob->setDoubleClickReturnValue(true,1.0f,ModifierKeys::altModifier);
    blendKnob->setMouseDragSensitivity (50);
    blendKnob->setLookAndFeel(&blaf);
    blendKnob->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    blendKnob->setColour(Slider::thumbColourId, Colours::cadetblue);
    blendKnob->setTextValueSuffix("%");
    
    driveKnob->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    driveKnob->setRotaryParameters(0.0f, 5.81333f, true);
    driveKnob->setDoubleClickReturnValue(true,1.0f,ModifierKeys::altModifier);
    driveKnob->setMouseDragSensitivity (100);
    driveKnob->setLookAndFeel(&llaf);
    driveKnob->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    driveKnob->setTextValueSuffix(" ;)");
    
    saturationKnob->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    saturationKnob->setRotaryParameters(degreesToRadians(180.f), degreesToRadians(513.f), true);
    saturationKnob->setDoubleClickReturnValue(true,1.0f,ModifierKeys::altModifier);
    saturationKnob->setMouseDragSensitivity (100);
    saturationKnob->setLookAndFeel(&rlaf);
    saturationKnob->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    saturationKnob->setTextValueSuffix(" :)");
    
    gateKnob->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    gateKnob->setRotaryParameters(degreesToRadians(76.f),degreesToRadians(50.f), true);
    gateKnob->setDoubleClickReturnValue(true,1.0f,ModifierKeys::altModifier);
    gateKnob->setMouseDragSensitivity (60);
    gateKnob->setLookAndFeel(&ullaf);
    gateKnob->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    gateKnob->setColour(Slider::thumbColourId, Colour(Colour(0xff50af43)));
    gateKnob->setTextValueSuffix(" dB");
    
    filterKnob->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    filterKnob->setRotaryParameters(degreesToRadians(284.f),degreesToRadians(310.f), true);
    filterKnob->setDoubleClickReturnValue(true,1.0f,ModifierKeys::altModifier);
    filterKnob->setMouseDragSensitivity (60);
    filterKnob->setLookAndFeel(&urlaf);
    filterKnob->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    filterKnob->setColour(Slider::thumbColourId, Colour(Colour(0xff50af43)));
    filterKnob->setTextValueSuffix(" Hz");
    
    gainKnob->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    gainKnob->setRotaryParameters(degreesToRadians(135.f),degreesToRadians(103.f), true);
    gainKnob->setDoubleClickReturnValue(true,1.0f,ModifierKeys::altModifier);
    gainKnob->setMouseDragSensitivity (60);
    gainKnob->setLookAndFeel(&gmlaf);
    gainKnob->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    gainKnob->setTextValueSuffix(" dB");
    
    volumeKnob->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    volumeKnob->setRotaryParameters(degreesToRadians(225.f),degreesToRadians(257.f), true);
    volumeKnob->setDoubleClickReturnValue(true,1.0f,ModifierKeys::altModifier);
    volumeKnob->setMouseDragSensitivity (60);
    volumeKnob->setLookAndFeel(&vmlaf);
    volumeKnob->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    volumeKnob->setTextValueSuffix(" dB");
    
    font.setHeight(80);
    
    valueLabel.setLookAndFeel(&slaf);
    suffixLabel.setLookAndFeel(&slaf);
    valueLabel.setJustificationType(Justification::centred);
    valueLabel.setFont(font);
    valueLabel.setColour(juce::Label::textColourId, juce::Colour(0, 0, 0));
    suffixLabel.setJustificationType(Justification::centred);
    suffixLabel.setFont(Font("Shadows Into Light","Regular",80.f));
    suffixLabel.setColour(juce::Label::textColourId, juce::Colour(0, 0, 0));
};

Child::Child () {
    setSize (80, 80);
}

void Child::setSVG (const char* svg, const int svgSize) {
    d.loadPathFromData(svg, svgSize);
}

void Child::paint (Graphics& g) {
    g.setColour(Colours::black);
    d.scaleToFit(0, 0, 80, 80, true);
    g.fillPath(d);
}

bool Child::hitTest (int x, int y) {
    return x>500 && y<1;
};

std::function<bool (float a, float b)> positive = [](float i, float h) { return i < h; };
std::function<bool (float a, float b)> negative = [](float i, float h) { return i > h; };

float Channel::pop() {
    auto result = deque.front();
    deque.pop_front();
    return result;
}

void Channel::reverseWave(int previous, int current) {
    std::adjacent_difference(deque.begin() + previous - 1, deque.begin() + current, dspVECTOR.begin() + previous - 1);
    std::reverse(dspVECTOR.begin() + previous,dspVECTOR.begin() + current);
    std::transform(deque.begin() + previous - 1, deque.begin() + current - 1, dspVECTOR.begin() + previous, deque.begin() + previous, std::plus<float>());
}

void Channel::FUCK(int previous, int current) {
    float previousExtremity = deque.at(previous - 1);
    float currentExtremity = deque.at(current - 1);
    int numSamples = current - previous;
    float difference = (currentExtremity - previousExtremity) / numSamples;
    for (int i = 0; i < numSamples; ++i)
        deque[previous - 1 + i] = previousExtremity + (i * difference);
}

void Channel::EXPO(int previous, int current, float saturation, float blend) {
    float previousExtremity = deque.at(previous - 1);
    float magnitude = deque.at(current - 1) - previousExtremity;
    int numSamples = current - previous;
    
    for (int i = 0; i < current - previous; ++i)
        deque[previous + i] = deque[previous + i]  * (1 - (blend/100.f)) + (blend/100.f) * (previousExtremity + magnitude * pow(((i + 1) / numSamples),saturation));
//    int i = 0;
//    std::generate(deque.begin() + previous, deque.begin() + current, [&]() mutable {
//        auto newValue = previousExtremity + magnitude * pow((i / numSamples),saturation);
//        ++i;
//        return newValue;
//    });
//    for (int i = 0; i < numSamples; ++i)
//        deque[previous - 1 + i] = previousExtremity + magnitude * pow((i / numSamples),saturatioh);
}

//void Channel::EXPO(int previous, int current, const float& saturatioh) {
//    float previousExtremity = deque.at(previous - 1);
//    float currentExtremity = deque.at(current - 1);
//    int numSamples = current - previous;
//    float magnitude = currentExtremity - previousExtremity;
//    for (int i = 0; i < numSamples; ++i)
//        deque[previous - 1 + i] = previousExtremity + magnitude * pow((i / numSamples),saturatioh);
//}

void Channel::processChannel (int bufferLength, int anchor, float saturation, float blend) {
    for (auto i = anchor; i < anchor + bufferLength; ++i) {
            if (compare(deque[i], deque[i - 1])) {
                if (turn)
                    EXPO(turn, i, saturation, blend);
                turn = i;
                std::swap(compare, compareOther);
            }
    }
    if (turn)
        turn = jmax(turn -= bufferLength, 0);
}

int Queue::getAnchor () {
    return anchor;
}

void Queue::prepare (int numChannels, int samplesPerBlock) {
    bufferLength = samplesPerBlock;
    mBuffer.resize(numChannels);
    anchor = jmax(bufferLength, (4096 / bufferLength) * bufferLength);
    for (int channel = 0; channel < numChannels; ++channel) {
        mBuffer[channel].deque.resize(anchor);
        mBuffer[channel].dspVECTOR.resize(anchor + bufferLength);
        mBuffer[channel].turn = 0;
        mBuffer[channel].compare = positive;
        mBuffer[channel].compareOther = negative;
    }
}

void Queue::enQueue (AudioBuffer<float>& buffer) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        const float* channelRead = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample, ++channelRead)
            mBuffer[channel].deque.push_back(*channelRead);
    }
}

void Queue::deQueue (AudioBuffer<float>& buffer) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        float* channelWrite = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample, ++channelWrite)
            *channelWrite = mBuffer[channel].pop();
    }
}

void Queue::process (AudioBuffer<float>& buffer, float saturation, float blend) {
    enQueue(buffer);
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        mBuffer[channel].processChannel(buffer.getNumSamples(), anchor, saturation, blend);
    deQueue(buffer);
}
