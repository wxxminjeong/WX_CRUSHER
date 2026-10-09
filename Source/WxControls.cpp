/*
  ==============================================================================
    WxControls.cpp
  ==============================================================================
*/

#include "WxControls.h"
#include "WxDsp.h"

//==============================================================================
WxKnobModule::WxKnobModule(juce::AudioProcessorValueTreeState& apvts,
                           const juce::String& numeralToShow, const juce::String& titleToShow,
                           const juce::String& captionToShow, const juce::String& valueID,
                           const juce::String& enableID)
    : numeral(numeralToShow), title(titleToShow), caption(captionToShow),
      hasPowerButton(enableID.isNotEmpty())
{
    // 1. 노브
    knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 110, 24);

    if (auto* parameter = apvts.getParameter(valueID))
        knob.setTextValueSuffix(" " + parameter->getLabel());

    if (! hasPowerButton)
        knob.getProperties().set(wx::neverHotProperty, true);

    // OUTPUT 은 0dB 를 기준으로 양쪽으로 아크를 그립니다.
    if (valueID == wx::ParamID::output)
        if (auto* parameter = apvts.getParameter(valueID))
            knob.getProperties().set(wx::arcOriginProperty, (double)parameter->convertTo0to1(0.0f));

    addAndMakeVisible(knob);
    knobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, valueID, knob);

    // 더블클릭하면 기본값으로 (어태치먼트가 범위를 정한 뒤에 설정해야 합니다)
    if (auto* parameter = apvts.getParameter(valueID))
        knob.setDoubleClickReturnValue(true, (double)parameter->convertFrom0to1(parameter->getDefaultValue()));

    // 숫자 칸에 숫자가 아닌 걸 입력하면 (빈 칸, "off" ...) 지금 값을 그대로 둡니다.
    knob.valueFromTextFunction = [this, parseText = knob.valueFromTextFunction](const juce::String& text)
    {
        if (parseText == nullptr || ! text.containsAnyOf("0123456789"))
            return knob.getValue();

        return parseText(text);
    };

    knob.onValueChange = [this] { repaint(); };

    // 2. ON/OFF 버튼
    if (hasPowerButton)
    {
        powerButton.setClickingTogglesState(true);
        addAndMakeVisible(powerButton);
        powerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, enableID, powerButton);
        powerButton.onClick = [this] { refreshStageLook(); };
    }

    refreshStageLook();
}

//==============================================================================
void WxKnobModule::setActivity(float newActivity)
{
    newActivity = juce::jlimit(0.0f, 1.0f, newActivity);

    // 아주 작은 값은 꺼짐으로 (BITCRUSH / CLIPPER 와 같은 기준)
    // 단, 켜져 있던 LED 는 0.2초 동안 계속 작아야 꺼집니다. (경계에서 깜빡이지 않도록)
    if (newActivity < 0.01f)
    {
        if (activity > 0.0f && ++framesBelowThreshold < 12)
            return;

        newActivity = 0.0f;
    }
    else
    {
        framesBelowThreshold = 0;
    }

    // 켜짐 ↔ 꺼짐이 바뀔 때는 아무리 작은 변화라도 꼭 반영해야 LED 가 켜진 채로 남지 않습니다.
    const bool litChanged = (newActivity > 0.0f) != (activity > 0.0f);

    if (litChanged || std::abs(newActivity - activity) > 0.01f)
    {
        activity = newActivity;
        repaint(getLocalBounds().removeFromBottom(footerHeight)); // LED 줄만 다시 그리기
    }
}

bool WxKnobModule::isStageOn() const
{
    return ! hasPowerButton || powerButton.getToggleState();
}

float WxKnobModule::getKnobPosition()
{
    return (float)knob.valueToProportionOfLength(knob.getValue());
}

void WxKnobModule::refreshStageLook()
{
    // 꺼진 스테이지는 어둡게 (노브도 빨갛게 변하지 않도록)
    knob.getProperties().set(wx::stageOffProperty, ! isStageOn());
    knob.setAlpha(isStageOn() ? 1.0f : 0.3f);
    knob.repaint();
    repaint();
}

//==============================================================================
void WxKnobModule::paint(juce::Graphics& g)
{
    using namespace wx;

    const auto bounds = getLocalBounds().toFloat().reduced(0.5f);
    const bool isOn = isStageOn();
    const bool isHot = hasPowerButton && isOn && getKnobPosition() > 0.8f;

    // 패널
    g.setColour(Palette::panel.withAlpha(0.92f));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(isHot ? Palette::deepRed : Palette::panelBorder);
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    // 번호 + 이름
    auto header = getLocalBounds().removeFromTop(40).reduced(14, 0);

    if (numeral.isNotEmpty())
    {
        g.setColour(isOn ? Palette::red : Palette::dim);
        g.setFont(fonts->mono(13.0f));
        g.drawText(numeral, header.removeFromLeft(26), juce::Justification::centredLeft, false);
    }

    g.setColour(isOn ? Palette::text : Palette::dim);
    g.setFont(fonts->display(hasPowerButton ? 28.0f : 22.0f));
    g.drawText(title, header, hasPowerButton ? juce::Justification::centredLeft : juce::Justification::centred, false);

    // LED + 설명
    auto footer = getLocalBounds().removeFromBottom(footerHeight).reduced(14, 0);
    g.setFont(fonts->mono(13.0f));

    if (hasPowerButton)
    {
        const float level = isOn ? activity : 0.0f;
        const auto led = juce::Rectangle<float>(9.0f, 9.0f).withCentre({ (float)footer.getX() + 5.0f, (float)footer.getCentreY() });

        if (level > 0.0f)
        {
            g.setColour(Palette::red.withAlpha(0.25f * level));
            g.fillEllipse(led.expanded(5.0f * level));
        }

        g.setColour(level > 0.0f ? Palette::red.withAlpha(0.35f + 0.65f * level) : Palette::track);
        g.fillEllipse(led);

        footer.removeFromLeft(16);
        g.setColour(isOn ? Palette::dim : Palette::dim.withAlpha(0.6f));
        g.drawText(caption, footer, juce::Justification::centredLeft, false);
    }
    else
    {
        g.setColour(Palette::dim);
        g.drawText(caption, footer, juce::Justification::centred, false);
    }
}

void WxKnobModule::resized()
{
    auto area = getLocalBounds();
    auto header = area.removeFromTop(40);
    area.removeFromBottom(footerHeight);

    if (hasPowerButton)
        powerButton.setBounds(header.removeFromRight(62).withSizeKeepingCentre(44, 20));

    // 남은 공간 가운데에 노브 (+ 아래 숫자 칸)
    const int knobSize = juce::jmin(area.getWidth() - 24, area.getHeight() - 28);
    knob.setBounds(area.withSizeKeepingCentre(knobSize + 20, knobSize + 28));
}
