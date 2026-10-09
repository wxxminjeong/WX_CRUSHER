/*
  ==============================================================================
    WxControls.h

    🎛️ 스테이지 모듈 : [번호 + 이름 + ON/OFF]  /  큰 노브  /  숫자  /  LED + 설명
    DRIVE, BITCRUSH, CLIPPER 는 ON/OFF 버튼과 LED 가 있고,
    MIX, OUTPUT 은 같은 모양에서 버튼과 LED 만 뺀 버전입니다.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "WxLookAndFeel.h"

//==============================================================================
class WxKnobModule : public juce::Component
{
public:
    // enableID 가 비어 있으면 ON/OFF 버튼과 LED 없이 노브만 만듭니다.
    WxKnobModule(juce::AudioProcessorValueTreeState& apvts,
                 const juce::String& numeral,     // "I", "II", "III" (없으면 "")
                 const juce::String& title,       // "DRIVE"
                 const juce::String& caption,     // "GAIN"
                 const juce::String& valueID,
                 const juce::String& enableID = {});

    // 💡 LED 밝기 (0 = 꺼짐, 1 = 최대)
    void setActivity(float newActivity);

    bool isStageOn() const;
    float getKnobPosition();         // 노브가 돌아간 정도 (0 ~ 1)

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void refreshStageLook();

    static constexpr int footerHeight = 34; // 아래쪽 LED + 설명 줄

    juce::SharedResourcePointer<wx::Fonts> fonts;
    juce::String numeral, title, caption;
    bool hasPowerButton;
    float activity = 0.0f;
    int framesBelowThreshold = 0;   // LED 가 꺼짐 기준 아래에 머문 프레임 수

    juce::Slider knob;
    juce::ToggleButton powerButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> knobAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxKnobModule)
};
