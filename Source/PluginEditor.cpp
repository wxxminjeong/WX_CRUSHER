/*
  ==============================================================================
    PluginEditor.cpp
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WxCrusherAudioProcessorEditor::WxCrusherAudioProcessorEditor(WxCrusherAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(400, 500); // 비율을 위해 세로를 450 -> 500으로 조금 더 늘려 여유를 줬습니다.

    // 1. 노브 스타일
    mainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mainKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 30);
    mainKnob.setTextValueSuffix(" %");

    // 🎨 Opium/Rage 스타일 컬러 팔레트
    mainKnob.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    mainKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::white);
    mainKnob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
    mainKnob.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    mainKnob.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::black);
    mainKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::black);

    addAndMakeVisible(mainKnob);

    mainKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "GRITTY", mainKnob);

    // 값이 변할 때마다 다시 그리기
    mainKnob.onValueChange = [this] { repaint(); };
}

WxCrusherAudioProcessorEditor::~WxCrusherAudioProcessorEditor()
{
}

//==============================================================================
void WxCrusherAudioProcessorEditor::paint(juce::Graphics& g)
{
    // 1. 배경: 완전한 검은색
    g.fillAll(juce::Colours::black);

    // 2. 메인 타이틀 (WX CRUSHER)
    g.setColour(juce::Colours::white);
    // 폰트를 조금 더 키우고 임팩트 있게
    g.setFont(juce::Font("Impact", 45.0f, juce::Font::plain));
    g.drawFittedText("WX CRUSHER", 0, 40, getWidth(), 50, juce::Justification::centred, 1);

    // ================================================================
    // 💡 3단계 LED & 라벨 그리기
    // ================================================================

    float val = mainKnob.getValue(); // 0 ~ 100
    int centerX = getWidth() / 2;
    int ledY = 360; // 노브와 겹치지 않게 아래로 내림
    int ledSize = 15; // LED 크기를 살짝 줄여 세련되게
    int gap = 60; // LED 사이 간격을 넓힘

    // 색상 정의
    juce::Colour offColor = juce::Colours::darkgrey.withAlpha(0.4f);
    juce::Colour onColor = juce::Colours::red;

    // 라벨 폰트 설정 (가독성 향상!)
    g.setFont(juce::Font(14.0f, juce::Font::bold)); // 굵은 글씨

    // --- 1단계: DRIVE ---
    bool isDriveOn = val > 0.0f;
    g.setColour(isDriveOn ? onColor : offColor);
    g.fillEllipse(centerX - gap - (ledSize / 2), ledY, ledSize, ledSize);

    // 글씨 (켜지면 흰색, 꺼지면 밝은 회색 - 잘 보이게 수정)
    g.setColour(isDriveOn ? juce::Colours::white : juce::Colours::silver);
    g.drawText("DRIVE", centerX - gap - 30, ledY + 20, 60, 20, juce::Justification::centred);

    // --- 2단계: CRUSH ---
    bool isCrushOn = val > 40.0f;
    g.setColour(isCrushOn ? onColor : offColor);
    g.fillEllipse(centerX - (ledSize / 2), ledY, ledSize, ledSize);

    g.setColour(isCrushOn ? juce::Colours::white : juce::Colours::silver);
    g.drawText("CRUSH", centerX - 30, ledY + 20, 60, 20, juce::Justification::centred);

    // --- 3단계: DIE ---
    bool isDieOn = val > 80.0f;
    g.setColour(isDieOn ? onColor : offColor);
    g.fillEllipse(centerX + gap - (ledSize / 2), ledY, ledSize, ledSize);

    g.setColour(isDieOn ? juce::Colours::white : juce::Colours::silver);
    g.drawText("DIE", centerX + gap - 30, ledY + 20, 60, 20, juce::Justification::centred);

    // 🩸 Rage Mode 효과 (80% 넘으면 배경에 붉은색 글로우 추가)
    if (isDieOn) {
        // 그라데이션 효과로 은은하게
        juce::ColourGradient gradient(juce::Colours::red.withAlpha(0.2f), centerX, ledY,
            juce::Colours::transparentBlack, centerX, 0, true);
        g.setGradientFill(gradient);
        g.fillAll();
    }

    // ================================================================
    // ✍️ Signature (wxxmin)
    // ================================================================
    g.setColour(juce::Colours::darkgrey); // 너무 튀지 않게 어두운 회색
    g.setFont(juce::Font("Arial", 12.0f, juce::Font::italic)); // 기울임꼴로 간지나게
    // 오른쪽 구석에 배치
    g.drawText("wxxmin", getWidth() - 70, getHeight() - 30, 60, 20, juce::Justification::bottomRight);
}

void WxCrusherAudioProcessorEditor::resized()
{
    // 노브 위치를 화면 정중앙보다 살짝 위로 올림 (타이틀과 LED 사이)
    // Y좌표를 130으로 설정
    mainKnob.setBounds((getWidth() / 2) - 100, 130, 200, 200);
}