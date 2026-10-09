/*
  ==============================================================================
    WxControls.cpp
  ==============================================================================
*/

#include "WxControls.h"

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

    addAndMakeVisible(knob);
    knobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, valueID, knob);

    // 더블클릭하면 기본값으로 (어태치먼트가 범위를 정한 뒤에 설정해야 합니다)
    if (auto* parameter = apvts.getParameter(valueID))
        knob.setDoubleClickReturnValue(true, (double)parameter->convertFrom0to1(parameter->getDefaultValue()));

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

    if (std::abs(newActivity - activity) > 0.01f)
    {
        activity = newActivity;
        repaint();
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
    // 꺼진 스테이지는 어둡게
    knob.setAlpha(isStageOn() ? 1.0f : 0.3f);
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
        g.setFont(fonts->mono(12.0f));
        g.drawText(numeral, header.removeFromLeft(26), juce::Justification::centredLeft, false);
    }

    g.setColour(isOn ? Palette::text : Palette::dim);
    g.setFont(fonts->display(hasPowerButton ? 28.0f : 22.0f));
    g.drawText(title, header, hasPowerButton ? juce::Justification::centredLeft : juce::Justification::centred, false);

    // LED + 설명
    auto footer = getLocalBounds().removeFromBottom(34).reduced(14, 0);
    g.setFont(fonts->mono(11.0f));

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
        g.setColour(isOn ? Palette::dim : Palette::track);
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
    area.removeFromBottom(34);

    if (hasPowerButton)
        powerButton.setBounds(header.removeFromRight(62).withSizeKeepingCentre(44, 20));

    // 남은 공간 가운데에 노브 (+ 아래 숫자 칸)
    const int knobSize = juce::jmin(area.getWidth() - 24, area.getHeight() - 28);
    knob.setBounds(area.withSizeKeepingCentre(knobSize + 20, knobSize + 28));
}
