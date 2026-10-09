/*
  ==============================================================================
    WxKnobEditor.h

    WX 플러그인 3종이 함께 쓰는 화면입니다. (노브 1개 + LED 1개)
    검은 배경 / 흰 노브 / 끝까지 돌리면 붉은 글로우 - WX CRUSHER 디자인 그대로.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "WxProcessorBase.h"

//==============================================================================
class WxKnobEditor : public juce::AudioProcessorEditor
{
public:
    // parameterID : 노브와 연결할 파라미터 ID (예: "BITS")
    // name        : 화면에 보일 이름 (예: "CRUSH" → 타이틀 "WX CRUSH", LED 라벨 "CRUSH")
    WxKnobEditor(WxProcessorBase& p, const juce::String& parameterID, const juce::String& name);
    ~WxKnobEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::String displayName;

    // 🎛️ 1. 눈에 보이는 '노브' (Slider)
    juce::Slider mainKnob;

    // 🔗 2. 노브와 DSP를 연결해주는 '접착제' (Attachment)
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mainKnobAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxKnobEditor)
};
