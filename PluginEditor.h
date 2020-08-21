#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class DeviantEditor  : public AudioProcessorEditor, Slider::Listener {
    
    sadistic::BlendLookAndFeel      blaf;
    sadistic::LeftLookAndFeel       llaf;
    sadistic::MiddleLookAndFeel     mlaf;
    sadistic::UpperLeftLookAndFeel  ullaf;
    sadistic::UpperRightLookAndFeel urlaf;
    sadistic::RightLookAndFeel      rlaf;
    sadistic::SlidingLookAndFeel    slaf;
    Image   volume, filter, in, out, hz, dB;
    Path    drive, saturate, s, d;
    sadistic::Child driveChild, saturateChild;
    sadistic::TransLabel valueLabel, suffixLabel;
    std::unique_ptr<Slider> driveKnob, saturationKnob, gateKnob, filterKnob, gainKnob, volumeKnob, blendKnob;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> filterAttachment, driveAttachment, saturationAttachment, gateAttachment, gainAttachment, blendAttachment, volumeAttachment;
    Font shadowsIntoLight;
    
    DeviantProcessor& processor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeviantEditor);
public:
    DeviantEditor (DeviantProcessor&);
    
    void sliderValueChanged     (Slider* slider) override {};
    void paint (Graphics&) override;
    void resized() override;
};
