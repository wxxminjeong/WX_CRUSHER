/*
  ==============================================================================
    WxLookAndFeel.cpp
  ==============================================================================
*/

#include "WxLookAndFeel.h"
#include "BinaryData.h"

//==============================================================================
wx::Fonts::Fonts()
    : displayTypeface(juce::Typeface::createSystemTypefaceFor(BinaryData::AntonRegular_ttf,
                                                              (size_t)BinaryData::AntonRegular_ttfSize)),
      monoTypeface(juce::Typeface::createSystemTypefaceFor(BinaryData::SpaceMonoBold_ttf,
                                                           (size_t)BinaryData::SpaceMonoBold_ttfSize))
{
}

juce::Font wx::Fonts::display(float height) const
{
    return juce::Font(juce::FontOptions(displayTypeface).withHeight(height));
}

juce::Font wx::Fonts::mono(float height) const
{
    return juce::Font(juce::FontOptions(monoTypeface).withHeight(height));
}

//==============================================================================
WxLookAndFeel::WxLookAndFeel()
{
    using namespace wx;

    setColour(juce::ResizableWindow::backgroundColourId, Palette::background);

    setColour(juce::Slider::textBoxTextColourId, Palette::text);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxHighlightColourId, Palette::red.withAlpha(0.5f));

    setColour(juce::Label::textColourId, Palette::text);
    setColour(juce::Label::textWhenEditingColourId, Palette::text);
    setColour(juce::Label::outlineWhenEditingColourId, Palette::red);
    setColour(juce::TextEditor::backgroundColourId, Palette::background);
    setColour(juce::TextEditor::textColourId, Palette::text);
    setColour(juce::TextEditor::highlightColourId, Palette::red.withAlpha(0.5f));
    setColour(juce::TextEditor::focusedOutlineColourId, Palette::red);
    setColour(juce::CaretComponent::caretColourId, Palette::red);
}

//==============================================================================
void WxLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                     float sliderPos, float startAngle, float endAngle, juce::Slider& slider)
{
    using namespace wx;

    const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float toAngle = startAngle + sliderPos * (endAngle - startAngle);

    const float lineWidth = juce::jmax(2.0f, radius * 0.09f);
    const float arcRadius = radius - lineWidth * 1.5f;

    // 80% 를 넘으면 BLOOD RED (MIX / OUTPUT 처럼 '파괴'가 아닌 노브와 꺼진 스테이지는 제외)
    const auto& properties = slider.getProperties();
    const bool isHot = sliderPos > 0.8f
                       && ! properties.contains(neverHotProperty)
                       && ! (bool)properties.getWithDefault(stageOffProperty, false);
    const auto valueColour = isHot ? Palette::red : Palette::text;

    // 값 아크는 보통 맨 왼쪽에서 시작하지만, OUTPUT 은 0dB 에서 양쪽으로 뻗습니다.
    const float origin = (float)(double)properties.getWithDefault(arcOriginProperty, 0.0);
    const float arcLow = juce::jmin(origin, sliderPos), arcHigh = juce::jmax(origin, sliderPos);
    const bool hasArc = arcHigh - arcLow > 0.001f;
    const float originAngle = startAngle + origin * (endAngle - startAngle);

    // 1. 눈금 (바깥쪽 작은 점 11개)
    for (int i = 0; i <= 10; ++i)
    {
        const float position = (float)i / 10.0f;
        const float tickAngle = startAngle + position * (endAngle - startAngle);
        const auto tick = centre.getPointOnCircumference(radius - lineWidth * 0.3f, tickAngle);
        const bool isPassed = hasArc && position >= arcLow - 0.001f && position <= arcHigh + 0.001f;
        g.setColour(isPassed ? valueColour.withAlpha(0.8f) : Palette::track);
        g.fillEllipse(juce::Rectangle<float>(lineWidth * 0.45f, lineWidth * 0.45f).withCentre(tick));
    }

    // 2. 트랙
    juce::Path track;
    track.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    g.setColour(Palette::track);
    g.strokePath(track, juce::PathStrokeType(lineWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 3. 값 아크 (+ 빨갛게 번지는 글로우)
    if (origin > 0.0f)
    {
        // 기준점 (OUTPUT 0dB) 표시
        g.setColour(Palette::dim);
        g.fillEllipse(juce::Rectangle<float>(lineWidth * 0.7f, lineWidth * 0.7f)
                          .withCentre(centre.getPointOnCircumference(arcRadius + lineWidth * 1.1f, originAngle)));
    }

    if (hasArc)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                               juce::jmin(originAngle, toAngle), juce::jmax(originAngle, toAngle), true);

        if (isHot)
        {
            for (int i = 3; i >= 1; --i)
            {
                g.setColour(Palette::red.withAlpha(0.10f));
                g.strokePath(valueArc, juce::PathStrokeType(lineWidth * (1.0f + (float)i * 1.2f),
                                                            juce::PathStrokeType::curved,
                                                            juce::PathStrokeType::rounded));
            }
        }

        g.setColour(valueColour);
        g.strokePath(valueArc, juce::PathStrokeType(lineWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 4. 노브 몸통
    const float bodyRadius = arcRadius - lineWidth * 1.7f;
    const auto body = juce::Rectangle<float>(bodyRadius * 2.0f, bodyRadius * 2.0f).withCentre(centre);

    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff161616), centre.x, body.getY(),
                                           juce::Colour(0xff030303), centre.x, body.getBottom(), false));
    g.fillEllipse(body);
    g.setColour(isHot ? Palette::deepRed : juce::Colour(0xff2c2c2c));
    g.drawEllipse(body, 1.5f);

    // 5. 바늘
    const auto needleStart = centre.getPointOnCircumference(bodyRadius * 0.25f, toAngle);
    const auto needleEnd = centre.getPointOnCircumference(bodyRadius * 0.82f, toAngle);
    g.setColour(valueColour);
    g.drawLine({ needleStart, needleEnd }, lineWidth * 0.75f);

    if (! slider.isEnabled())
    {
        g.setColour(Palette::background.withAlpha(0.6f));
        g.fillEllipse(bounds.withSizeKeepingCentre(radius * 2.0f, radius * 2.0f));
    }
}

//==============================================================================
void WxLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                     bool isHighlighted, bool)
{
    using namespace wx;

    const auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    const bool isOn = button.getToggleState();
    const float corner = bounds.getHeight() * 0.5f;

    if (isOn)
    {
        g.setColour(Palette::red);
        g.fillRoundedRectangle(bounds, corner);
    }
    else
    {
        g.setColour(isHighlighted ? Palette::text : Palette::dim);
        g.drawRoundedRectangle(bounds, corner, 1.2f);
    }

    g.setColour(isOn ? Palette::background : (isHighlighted ? Palette::text : Palette::dim));
    g.setFont(fonts->mono(bounds.getHeight() * 0.72f));
    g.drawText(isOn ? "ON" : "OFF", bounds, juce::Justification::centred, false);
}

//==============================================================================
void WxLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                         bool, bool)
{
    // 탭 아래 빨간 밑줄만 그립니다.
    if (button.getToggleState())
    {
        auto bounds = button.getLocalBounds().toFloat();
        g.setColour(wx::Palette::red);
        g.fillRect(bounds.removeFromBottom(2.0f).reduced(bounds.getWidth() * 0.15f, 0.0f));
    }
}

void WxLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button, bool isHighlighted, bool)
{
    using namespace wx;

    g.setColour(button.getToggleState() ? Palette::text : (isHighlighted ? Palette::text.withAlpha(0.7f) : Palette::dim));
    g.setFont(fonts->mono((float)button.getHeight() * 0.6f));
    g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, false);
}

//==============================================================================
juce::Font WxLookAndFeel::getLabelFont(juce::Label& label)
{
    return fonts->mono(juce::jlimit(10.0f, 22.0f, (float)label.getHeight() * 0.62f));
}

juce::Label* WxLookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    auto* label = LookAndFeel_V4::createSliderTextBox(slider);
    label->setJustificationType(juce::Justification::centred);
    label->setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
    return label;
}
