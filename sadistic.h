/*
  sadistic.h


                    .=>>>>>>>=      )>>>
                  .=>>>>>>>>>>>      (>>>
                (>>>>>>>>>>>>>>>     )>>>>
              (>>>>>>>>>>>>>>>>>     >>>>>>
            (>>>>>>>>>>>>>>>>>>      >>>>>>>
          )>>>>>>>=   >>>>>>>>>      >>>>>>>
         )>>>>>>      )>>>>>>>      )>>>>>>>
       )>>>>>>         \>>>=        )>>>>>>>>
      (>>>>>=                       )>>>  >>>
     >>>>>>                         >>>>  )>>>
    >>>>>>                          >>>>   )>>
   >>>>>>                           >>>>   )>>>
  )>>>>>                            >>>>    >>>
 />>>>>>                           )>>>>    )>>>
 >>>>>>                            )>>>      >>>
(>>>>>>                            )>>>      )>>>
)>>>>>>                            )>>>       >>>
)>>>>>                             >>>>       >>>>
)>>>>>                             >>>>       )>>>
)>>>>>                             >>>>        >>>>
 >>>>>                            )>>>>        >>>>      .=>>>>=
 )>>>>>                           )>>>>        )>>>> .=>>>>>>>>>
  \>>>>                           )>>>         )>>>>>>>>>>>=<>>>
    >>>>.                         )>>>          >>>>>>><
     \>>>>=                       >>>>        .>>>>>>
        \>>>>>>>>===-...          >>>>      .>>>>>>>>
             \\\===>>>>>>>>>>=    >>>>  .=>>>>>>)>>>>
                           )>>>> )>>>>>>>>>>>=   )>>>
                           (>>>> )>>>>>>>=        >>>>
                        .(>>>=   )>>>>>           )>>>
                   .-=>>>><      )>>>>             >>>
            .-=>>>>>>=           )>>>    AUDIO     )>>
      (>>>>>>>>=                 )>>>
      \\\           SADISTIC

 
  Created by Frye Wilkins on 3/14/20.
  Copyright © 2020 Sadistic. All rights reserved.

*/

#pragma once
#include <JuceHeader.h>

namespace sadistic {
    
struct SlidingLookAndFeel   :   public LookAndFeel_V2   {
    void drawLabel (Graphics& g, Label& label) override;
};

struct BlendLookAndFeel :   public LookAndFeel_V4 {
    void drawLinearSlider (Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& slider) override;
};

struct LeftLookAndFeel    : public LookAndFeel_V4   {
    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                           float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override;
};

struct RightLookAndFeel    : public LookAndFeel_V4  {
    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                           float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override;
};

struct MiddleLookAndFeel    : public LookAndFeel_V4 {
    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override;
};

struct UpperLeftLookAndFeel    : public LookAndFeel_V4  {
    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override;
};

struct UpperRightLookAndFeel    : public LookAndFeel_V4 {
    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override;
};

class UpperSlider : public Slider   {public: bool hitTest (int x, int y) override;};
class LowerSlider : public Slider   {public: bool hitTest (int x, int y) override;};
class EmpiricalSlider : public Slider   {public: bool hitTest (int x, int y) override;};
class TransLabel  : public Label    {public: bool hitTest (int x, int y) override;};
void showEmpiricalValue(std::unique_ptr<Slider>& slider, Label& label1, Component& child);
void hideValue(Label& valueLabel, Label& suffixLabel);
void hideValue(Label& valueLabel, Component& child);
void showIntegerValue(std::unique_ptr<Slider>& slider, Label& label1, Label& label2);
void showLevelValue(std::unique_ptr<Slider>& slider, Label& label1, Label& label2);
void setWidgets(std::unique_ptr<Slider>& blendKnob, sadistic::BlendLookAndFeel& blaf, std::unique_ptr<Slider>& driveKnob, sadistic::LeftLookAndFeel& llaf, std::unique_ptr<Slider>& saturationKnob, sadistic::RightLookAndFeel& rlaf, std::unique_ptr<Slider>& filterKnob, sadistic::UpperRightLookAndFeel& urlaf, std::unique_ptr<Slider>& gateKnob, sadistic::UpperLeftLookAndFeel& ullaf, std::unique_ptr<Slider>& gainKnob, sadistic::MiddleLookAndFeel& gmlaf, std::unique_ptr<Slider>& volumeKnob, sadistic::MiddleLookAndFeel& vmlaf, Label& valueLabel, Label& suffixLabel, sadistic::SlidingLookAndFeel& slaf, Font& font);
    
struct Channel {
    std::deque<float> deque;
    std::vector<float> dspVECTOR;
    int turn;
    void reverseWave(int previous, int current);
    void FUCK(int previous, int current);
    void EXPO(int previous, int current, float saturation, float blend);
    float pop();
    void processChannel (int bufferLength, int anchor, float saturation, float blend);
    std::function<bool (float i, float h)> compare, compareOther;
};

class Queue {
    std::vector<Channel> mBuffer;
    int bufferLength, anchor;

public:
    int getAnchor ();
    void prepare (int numChannels, int samplesPerBlock);
    void enQueue (AudioBuffer<float>& buffer);
    void deQueue (AudioBuffer<float>& buffer);
    void process (AudioBuffer<float>& buffer, float saturation, float blend);
};
    
class Child  : public Component
{
public:
    Child ();
    void paint (Graphics& g) override;
    bool hitTest (int x, int y) override;
    void setSVG (const char* svg, const int svgSize);
    
private:
    Path d;
};
    
    

    
}
