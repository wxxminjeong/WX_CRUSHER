/*
  ==============================================================================
    PluginEditor.h

    ┌──────────────────────────────────────────────────────────────┐
    │ WX CRUSHER                                            wxxmin │
    │ ┌ WAVE | SPECTRUM ───────────────┐ ┌ TRANSFER ─┐ ┌ I/O ┐    │
    │ │                                │ │           │ │ ▮ ▮ │    │
    │ └────────────────────────────────┘ └───────────┘ └─────┘    │
    │ ┌ I DRIVE ┐ › ┌ II CRUSH ┐ › ┌ III DIE ┐  ┌ MIX ┐ ┌ OUTPUT ┐ │
    │ └─────────┘   └──────────┘   └─────────┘  └─────┘ └────────┘ │
    └──────────────────────────────────────────────────────────────┘
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "WxLookAndFeel.h"
#include "WxControls.h"
#include "WxDisplays.h"

//==============================================================================
// 화면 전체 (항상 960 x 600 기준으로 배치하고, 창 크기에 맞춰 통째로 확대/축소합니다)
class WxMainView : public juce::Component, private juce::Timer
{
public:
    static constexpr int baseWidth = 960;
    static constexpr int baseHeight = 600;

    explicit WxMainView(WxCrusherAudioProcessor&);
    ~WxMainView() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    WxCrusherAudioProcessor& audioProcessor;

    WxLookAndFeel lookAndFeel;
    juce::SharedResourcePointer<wx::Fonts> fonts;

    // 📺 시각화
    WxDisplay display;
    WxTransferCurve transferCurve;
    WxMeter inputMeter { "IN" }, outputMeter { "OUT" };

    // 🎛️ 3단계 + MIX / OUTPUT
    WxKnobModule driveModule, crushModule, dieModule, mixModule, outputModule;

    std::vector<float> pulledInput, pulledOutput;
    float clipActivity = 0.0f;   // DIE 에서 실제로 잘리고 있는 정도 (천천히 꺼짐)
    float inputLevel = 0.0f;     // 전달 곡선에 표시할 입력 크기
    float glow = 0.0f;           // 배경의 붉은 글로우

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxMainView)
};

//==============================================================================
class WxCrusherAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    WxCrusherAudioProcessorEditor(WxCrusherAudioProcessor&);
    ~WxCrusherAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // 우리의 플러그인 본체(뇌)를 가리키는 참조
    WxCrusherAudioProcessor& audioProcessor;

    WxMainView mainView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxCrusherAudioProcessorEditor)
};
