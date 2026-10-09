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

    uint32_t lastBlockCount = 0;   // 오디오 스레드가 처리한 블록 수 (새 오디오가 왔는지 확인용)
    int framesWithoutAudio = 0;

    float inputLevel = 0.0f;       // 전달 곡선 / LED 에 쓰는 입력 크기 (천천히 꺼짐)
    float crushActivity = 0.0f;    // CRUSH 가 실제로 소리를 바꾸는 정도 (천천히 꺼짐)
    float clipActivity = 0.0f;     // DIE 에서 실제로 잘리고 있는 정도 (천천히 꺼짐)
    float glowLevel = 0.0f;        // 배경 글로우 (부드럽게 따라감)
    float glow = 0.0f;             // 지금 그려진 배경 글로우 (1/32 단계)

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
