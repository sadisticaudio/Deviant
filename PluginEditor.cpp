#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Images.h"

DeviantEditor::DeviantEditor (DeviantProcessor& p)
: AudioProcessorEditor (&p), processor (p)
{
    shadowsIntoLight = Font(Typeface::createSystemTypefaceFor(Images::shadowsIntoLight_ttf, Images::shadowsIntoLight_ttfSize));
    saturate.loadPathFromData(Images::saturate_svg, Images::saturate_svgSize);
    drive.loadPathFromData(Images::drive_svg, Images::drive_svgSize);
    driveChild.setSVG(Images::drive_svg, Images::drive_svgSize);
    saturateChild.setSVG(Images::saturate_svg, Images::saturate_svgSize);
    
    volume = ImageCache::getFromMemory (Images::volume_png, Images::volume_pngSize);
    filter = ImageCache::getFromMemory (Images::filter_png, Images::filter_pngSize);
    in = ImageCache::getFromMemory (Images::in_png, Images::in_pngSize);
    out = ImageCache::getFromMemory (Images::out_png, Images::out_pngSize);
    hz = ImageCache::getFromMemory (Images::hz_png, Images::hz_pngSize);
    dB = ImageCache::getFromMemory (Images::dB_png, Images::dB_pngSize);
    
    driveKnob = std::make_unique<sadistic::EmpiricalSlider>();
    saturationKnob = std::make_unique<sadistic::EmpiricalSlider>();
    gateKnob = std::make_unique<sadistic::UpperSlider>();
    filterKnob = std::make_unique<sadistic::UpperSlider>();
    gainKnob = std::make_unique<sadistic::LowerSlider>();
    volumeKnob = std::make_unique<sadistic::LowerSlider>();
    blendKnob = std::make_unique<Slider>();
    
    driveKnob->addListener(this);
    saturationKnob->addListener(this);
    
    sadistic::setWidgets(blendKnob, blaf, driveKnob, llaf, saturationKnob, rlaf, filterKnob, urlaf, gateKnob, ullaf, gainKnob, mlaf, volumeKnob, mlaf, valueLabel, suffixLabel, slaf, shadowsIntoLight);
    
    filterKnob->onDragStart = [&,this]() {showIntegerValue(filterKnob, valueLabel, suffixLabel);};
    filterKnob->onValueChange = [&,this]() { showIntegerValue(filterKnob, valueLabel, suffixLabel);};
    filterKnob->onDragEnd = [&,this]() { hideValue(valueLabel, suffixLabel);};
    
    driveKnob->onDragStart = [&,this]() {
        showEmpiricalValue(driveKnob, valueLabel, driveChild);
        driveChild.setTransform(AffineTransform::rotation(std::powf(driveKnob->getValue() / 200,2),400.f,100.f));};
    driveKnob->onValueChange = [&,this]() {
        showEmpiricalValue(driveKnob, valueLabel, driveChild);
        driveChild.setTransform(AffineTransform::rotation(std::powf(driveKnob->getValue() / 200,2),400,100));};
    driveKnob->onDragEnd = [&,this]() { hideValue(valueLabel, driveChild);};
    
    saturationKnob->onDragStart = [&,this]() {
        showEmpiricalValue(saturationKnob, valueLabel, saturateChild);
        saturateChild.setTransform(AffineTransform::rotation(saturationKnob->getValue() / 10 - 1.8,400,100));};
    saturationKnob->onValueChange = [&,this]() {
        showEmpiricalValue(saturationKnob, valueLabel, saturateChild);
        saturateChild.setTransform(AffineTransform::rotation(saturationKnob->getValue() * 2,400,100));};
    saturationKnob->onDragEnd = [&,this]() { hideValue(valueLabel, saturateChild);};
    
    gateKnob->onDragStart = [&,this]() { showIntegerValue(gateKnob, valueLabel, suffixLabel);};
    gateKnob->onValueChange = [&,this]() { showIntegerValue(gateKnob, valueLabel, suffixLabel);};
    gateKnob->onDragEnd = [&,this]() { hideValue(valueLabel, suffixLabel);};
    gainKnob->onDragStart = [&,this]() { showLevelValue(gainKnob, valueLabel, suffixLabel);};
    gainKnob->onValueChange = [&,this]() { showLevelValue(gainKnob, valueLabel, suffixLabel);};
    gainKnob->onDragEnd = [&,this]() { hideValue(valueLabel, suffixLabel);};
    blendKnob->onDragStart = [&,this]() { showIntegerValue(blendKnob, valueLabel, suffixLabel);};
    blendKnob->onValueChange = [&,this]() { showIntegerValue(blendKnob, valueLabel, suffixLabel);};
    blendKnob->onDragEnd = [&,this]() { hideValue(valueLabel, suffixLabel);};
    volumeKnob->onDragStart = [&,this]() { showLevelValue(volumeKnob, valueLabel, suffixLabel);};
    volumeKnob->onValueChange = [&,this]() { showLevelValue(volumeKnob, valueLabel, suffixLabel);};
    volumeKnob->onDragEnd = [&,this]() { hideValue(valueLabel, suffixLabel);};

    addAndMakeVisible(*volumeKnob);
    addAndMakeVisible(*gateKnob);
    addAndMakeVisible(*gainKnob);
    addAndMakeVisible(*filterKnob);
    addAndMakeVisible(*driveKnob);
    addAndMakeVisible(*saturationKnob);
    addAndMakeVisible(*blendKnob);
    addAndMakeVisible(valueLabel);
    addAndMakeVisible(suffixLabel);
    addChildComponent(driveChild);
    addChildComponent(saturateChild);
    
    volumeAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "volume", *volumeKnob);
    gateAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "gate", *gateKnob);
    gainAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "gain", *gainKnob);
    filterAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "filter", *filterKnob);
    driveAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "drive", *driveKnob);
    saturationAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "saturation", *saturationKnob);
    blendAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.getValueTreeState(), "blend", *blendKnob);

    driveKnob->setSkewFactorFromMidPoint(500);
    gateKnob->setSkewFactorFromMidPoint(25);
    gainKnob->setSkewFactorFromMidPoint(0.75);
    volumeKnob->setSkewFactorFromMidPoint(0.75);
    
    hideValue(valueLabel, suffixLabel);
    driveChild.setVisible(false);
    saturateChild.setVisible(false);

    setSize (500, 200);
}

void DeviantEditor::paint (Graphics& g)
{
    //background ui stuff
    g.setColour(Colours::black);
    g.fillRect(getLocalBounds());
    g.setColour(Colour(0xeb228092));    g.fillEllipse(230,-70,340,340);
    g.setColour(Colour(0xbf037f2a));    g.fillEllipse(-70,-70,340,340);
    g.setColour(Colour(0xbb1b6372));    g.fillEllipse(250,-50,300,300);
    g.setColour(Colour(0xdf035f2a));    g.fillEllipse(-50,-50,300,300);
    g.setColour(Colour(0xbf183f52));    g.fillEllipse(270,-30,260,260);
    g.setColour(Colour(0xbf033f2a));    g.fillEllipse(-30,-30,260,260);

    // beggining of the arrows, set color to green for left arrow
    Path triangles[6];
    g.setColour(Colour(0xbb03cf2a));
    g.fillRect(225,90,15,10);
    triangles[0].addTriangle(225,90,240,90,240,80);     g.fillPath(triangles[0]);
    g.fillRect(240,80,50,20);
    
    // set the color to blue for the right arrow
    g.setColour(Colour(0x9952BAD5));
    g.fillRect(275,100,5,10);   g.fillRect(260,100,15,10);  g.fillRect(210,100,50,20);
    triangles[1].addTriangle(275,110,260,110,260,120);  g.fillPath(triangles[1]);
    g.setColour(Colour(0xd752BAD5));    g.fillRect(275,90,5,10);
    triangles[2].addTriangle(275,90,260,100,275,100);   g.fillPath(triangles[2]);
    triangles[3].addTriangle(300,100,280,120,280,80);   g.fillPath(triangles[3]);
    
    // set the color back to green for the overlapping green arrowhead
    g.setColour(Colour(0xbb03cf2a));    g.fillRect(220,90,5,20);
    triangles[4].addTriangle(225,110,240,100,225,100);  g.fillPath(triangles[4]);
    g.setColour(Colour(0xdd03cf2a));
    triangles[5].addTriangle(200,100,220,80,220,120);   g.fillPath(triangles[5]);

    g.setColour(Colours::black);
    g.drawImageWithin(volume,236,160,25,25,64);
    g.drawImageWithin(filter, 230, 25, 45, 15, 64);
    g.drawImageWithin(in, 196, 150, 15, 15, 64);
    g.drawImageWithin(out, 291, 151, 20, 20, 64);
    g.drawImageWithin(dB, 195, 45, 20, 20, 64);
    g.drawImageWithin(hz, 283, 44, 20, 20, 64);
    
    drive.scaleToFit(225.f,85.f,50.f,13.f,false);
    saturate.scaleToFit(227.f,99.f,50.f,18.f,false);
    g.strokePath(drive, PathStrokeType(0.4f));
    g.strokePath(saturate, PathStrokeType(0.4f));
    g.setColour(Colour(0x44000000));    g.fillPath(drive);  g.fillPath(saturate);
}

void DeviantEditor::resized()
{
    filterKnob->setBounds(250,-50,300,300);
    gateKnob->setBounds(-50, -50, 300, 300);
    gainKnob->setBounds (-50, -50, 300, 300);
    volumeKnob->setBounds(250, -50, 300, 300);
    driveKnob->setBounds(0, 0, 200, 200);
    blendKnob->setBounds(160, -2, 180, 20);
    saturationKnob->setBounds(300, 0, 200, 200);
    valueLabel.setBounds(0,0,200,200);
    suffixLabel.setBounds(300,0,200,200);
    driveChild.setBounds(360,60,80,80);
    saturateChild.setBounds(360,58,120,100);
}
