/*
  ==============================================================================
    WxLookAndFeel.h

    🖤 WX CRUSHER 디자인 규칙: PITCH BLACK / STARK WHITE / BLOOD RED
    색, 폰트, 노브와 버튼 모양을 전부 여기서 정합니다.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace wx
{
    // 🎨 팔레트
    namespace Palette
    {
        inline const juce::Colour background  { 0xff000000 };
        inline const juce::Colour panel       { 0xff070707 };
        inline const juce::Colour panelBorder { 0xff1d1d1d };
        inline const juce::Colour grid        { 0xff161616 };
        inline const juce::Colour track       { 0xff262626 };
        inline const juce::Colour dim         { 0xff6a6a6a };
        inline const juce::Colour text        { 0xfff2f2f2 };
        inline const juce::Colour red         { 0xffff1a1a };
        inline const juce::Colour deepRed     { 0xff7a0000 };
    }

    //==============================================================================
    // 이 속성이 붙은 노브는 끝까지 돌려도 빨갛게 변하지 않습니다. (MIX, OUTPUT)
    inline const juce::Identifier neverHotProperty { "wxNeverHot" };

    //==============================================================================
    // 🔤 플러그인 안에 넣어둔 폰트 (어느 컴퓨터에서나 똑같이 보이도록)
    //    Anton (제목, 큰 글씨) / Space Mono Bold (숫자, 라벨)  - 둘 다 SIL OFL 라이선스
    struct Fonts
    {
        Fonts();

        juce::Font display(float height) const;
        juce::Font mono(float height) const;

        juce::Typeface::Ptr displayTypeface, monoTypeface;
    };
}

//==============================================================================
class WxLookAndFeel : public juce::LookAndFeel_V4
{
public:
    WxLookAndFeel();

    // 🎛️ 노브: 회색 트랙 + 흰색 값 아크, 80% 를 넘으면 피처럼 빨갛게 번집니다.
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;

    // ⏻ 스테이지 ON/OFF 버튼
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    // 📺 WAVE / SPECTRUM 탭
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&,
                        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    // 노브 아래 숫자 칸
    juce::Font getLabelFont(juce::Label&) override;
    juce::Label* createSliderTextBox(juce::Slider&) override;

private:
    juce::SharedResourcePointer<wx::Fonts> fonts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxLookAndFeel)
};
