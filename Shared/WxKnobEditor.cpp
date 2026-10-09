/*
  ==============================================================================
    WxKnobEditor.cpp
  ==============================================================================
*/

#include "WxKnobEditor.h"

//==============================================================================
WxKnobEditor::WxKnobEditor(WxProcessorBase& p, const juce::String& parameterID, const juce::String& name)
    : AudioProcessorEditor(&p), displayName(name)
{
    setSize(400, 500);

    // 1. 노브 스타일
    mainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mainKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 30);

    // 단위(dB, bit)는 파라미터에 적어둔 라벨을 그대로 씁니다.
    if (auto* parameter = p.apvts.getParameter(parameterID))
        mainKnob.setTextValueSuffix(" " + parameter->getLabel());

    // 🎨 Opium/Rage 스타일 컬러 팔레트
    mainKnob.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    mainKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::white);
    mainKnob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
    mainKnob.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    mainKnob.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::black);
    mainKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::black);

    addAndMakeVisible(mainKnob);

    mainKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, parameterID, mainKnob);

    // 값이 변할 때마다 다시 그리기
    mainKnob.onValueChange = [this] { repaint(); };
}

WxKnobEditor::~WxKnobEditor()
{
}

//==============================================================================
void WxKnobEditor::paint(juce::Graphics& g)
{
    // 1. 배경: 완전한 검은색
    g.fillAll(juce::Colours::black);

    // 2. 메인 타이틀 (WX DRIVE / WX CRUSH / WX DIE)
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions("Impact", 45.0f, juce::Font::plain));
    g.drawFittedText("WX " + displayName, 0, 40, getWidth(), 50, juce::Justification::centred, 1);

    // ================================================================
    // 💡 LED & 라벨 그리기
    // ================================================================

    // 노브가 얼마나 돌아갔는지 (0.0 = 맨 왼쪽, 1.0 = 맨 오른쪽)
    // 실제 값(dB, bit)과 상관없이 세 플러그인 모두 같은 기준으로 LED를 켭니다.
    float amount = (float)mainKnob.valueToProportionOfLength(mainKnob.getValue());
    int centerX = getWidth() / 2;
    int ledY = 360;
    int ledSize = 15;

    juce::Colour offColor = juce::Colours::darkgrey.withAlpha(0.4f);
    juce::Colour onColor = juce::Colours::red;

    // 노브를 조금이라도 돌리면 LED ON
    bool isOn = amount > 0.0f;
    g.setColour(isOn ? onColor : offColor);
    g.fillEllipse(centerX - (ledSize / 2), ledY, ledSize, ledSize);

    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.setColour(isOn ? juce::Colours::white : juce::Colours::silver);
    g.drawText(displayName, centerX - 50, ledY + 20, 100, 20, juce::Justification::centred);

    // 🩸 Rage Mode 효과 (80% 넘게 돌리면 배경에 붉은색 글로우 추가)
    if (amount > 0.8f) {
        juce::ColourGradient gradient(juce::Colours::red.withAlpha(0.2f), centerX, ledY,
            juce::Colours::transparentBlack, centerX, 0, true);
        g.setGradientFill(gradient);
        g.fillAll();
    }

    // ================================================================
    // ✍️ Signature (wxxmin)
    // ================================================================
    g.setColour(juce::Colours::darkgrey);
    g.setFont(juce::FontOptions("Arial", 12.0f, juce::Font::italic));
    g.drawText("wxxmin", getWidth() - 70, getHeight() - 30, 60, 20, juce::Justification::bottomRight);
}

void WxKnobEditor::resized()
{
    // 노브는 타이틀과 LED 사이
    mainKnob.setBounds((getWidth() / 2) - 100, 130, 200, 200);
}
