/*
  ==============================================================================
    PluginEditor.cpp
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WxMainView::WxMainView(WxCrusherAudioProcessor& p)
    : audioProcessor(p),
      display(p.displayMode),
      driveModule(p.apvts, "I", "DRIVE", "GAIN", wx::ParamID::drive, wx::ParamID::driveOn),
      crushModule(p.apvts, "II", "CRUSH", "BIT DEPTH", wx::ParamID::crush, wx::ParamID::crushOn),
      dieModule(p.apvts, "III", "DIE", "CEILING", wx::ParamID::die, wx::ParamID::dieOn),
      mixModule(p.apvts, "", "MIX", "DRY / WET", wx::ParamID::mix),
      outputModule(p.apvts, "", "OUTPUT", "LEVEL", wx::ParamID::output),
      pulledInput((size_t)WxVisualTap::capacity), pulledOutput((size_t)WxVisualTap::capacity)
{
    for (auto* child : std::initializer_list<juce::Component*> { &display, &transferCurve, &inputMeter, &outputMeter,
                                                                 &driveModule, &crushModule, &dieModule,
                                                                 &mixModule, &outputModule })
        addAndMakeVisible(child);

    // 자식들을 다 붙인 뒤에 디자인을 입혀야 노브 숫자 칸까지 전부 적용됩니다.
    setLookAndFeel(&lookAndFeel);

    // 창을 열기 전에 쌓여 있던 오래된 샘플과 레벨은 버립니다.
    auto& tap = audioProcessor.visualTap;
    while (tap.pullSamples(pulledInput.data(), pulledOutput.data(), (int)pulledInput.size()) > 0) {}
    tap.takeInputPeak();
    tap.takeOutputPeak();
    tap.takeClipAmount();

    setSize(baseWidth, baseHeight);
    startTimerHz(60);
}

WxMainView::~WxMainView()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

//==============================================================================
void WxMainView::timerCallback()
{
    auto& tap = audioProcessor.visualTap;

    // 1. 오디오 스레드가 보낸 샘플 → 파형 / 스펙트럼
    display.setSampleRate(tap.sampleRate.load());

    for (;;)
    {
        const int numPulled = tap.pullSamples(pulledInput.data(), pulledOutput.data(), (int)pulledInput.size());

        if (numPulled <= 0)
            break;

        display.pushSamples(pulledInput.data(), pulledOutput.data(), numPulled);
    }

    // 2. 레벨 미터
    const float inputPeak = tap.takeInputPeak();
    inputMeter.setLevel(inputPeak);
    outputMeter.setLevel(tap.takeOutputPeak());

    // 3. DIE 에서 실제로 잘리는 중이면 확 켜지고, 멈추면 천천히 꺼집니다.
    const float clipRatio = tap.takeClipAmount();
    const float clipTarget = clipRatio > 0.0f ? juce::jmin(1.0f, 0.45f + clipRatio * 2.0f) : 0.0f;
    clipActivity = juce::jmax(clipTarget, clipActivity * 0.9f);
    if (clipActivity < 0.01f)
        clipActivity = 0.0f;

    display.setClipActivity(clipActivity);
    display.refresh();

    // 4. 전달 곡선 = 지금 노브 설정 + 지금 들어오는 소리 크기
    const auto settings = audioProcessor.getCurrentSettings();
    inputLevel = juce::jmax(inputPeak, inputLevel * 0.92f);
    transferCurve.setSettings(settings);
    transferCurve.setInputLevel(inputLevel);

    // 5. 스테이지 LED : DRIVE / CRUSH 는 노브 양, DIE 는 실제로 잘리는 양
    driveModule.setActivity(settings.driveOn > 0.5f ? driveModule.getKnobPosition() : 0.0f);
    crushModule.setActivity(settings.crushOn > 0.5f ? crushModule.getKnobPosition() : 0.0f);
    dieModule.setActivity(settings.dieOn > 0.5f ? clipActivity : 0.0f);

    // 6. 🩸 배경 글로우 : 노브를 많이 돌릴수록 + 실제로 잘릴수록 붉게
    float destruction = 0.0f;
    for (auto* module : { &driveModule, &crushModule, &dieModule })
        if (module->isStageOn())
            destruction += module->getKnobPosition() / 3.0f;

    const float newGlow = juce::jlimit(0.0f, 1.0f, 0.55f * destruction + 0.45f * clipActivity);

    if (std::abs(newGlow - glow) > 0.02f || (newGlow <= 0.0f && glow > 0.0f))
    {
        glow = newGlow;
        repaint();
    }
}

//==============================================================================
void WxMainView::paint(juce::Graphics& g)
{
    using namespace wx;

    // 1. 배경: 완전한 검은색
    g.fillAll(Palette::background);

    // 2. 🩸 Rage Mode 글로우 (아래쪽에서 붉게 번짐)
    if (glow > 0.0f)
    {
        juce::ColourGradient gradient(Palette::red.withAlpha(0.28f * glow), (float)baseWidth * 0.5f, (float)baseHeight,
                                      juce::Colours::transparentBlack, (float)baseWidth * 0.5f, 0.0f, true);
        g.setGradientFill(gradient);
        g.fillAll();
    }

    // 3. 메인 타이틀
    g.setColour(Palette::text);
    g.setFont(fonts->display(44.0f));
    g.drawText("WX CRUSHER", 22, 8, 300, 50, juce::Justification::centredLeft, false);

    g.setFont(fonts->mono(11.0f));
    g.setColour(Palette::red);
    g.drawText("DRIVE / CRUSH / DIE", 232, 26, 200, 20, juce::Justification::centredLeft, false);

    // ✍️ Signature (wxxmin)
    g.setColour(Palette::dim);
    g.drawText("wxxmin", baseWidth - 200, 26, 178, 20, juce::Justification::centredRight, false);

    // 4. 신호 흐름 화살표 (DRIVE › CRUSH › DIE)
    g.setColour(Palette::dim);
    for (auto* module : { &driveModule, &crushModule })
    {
        const auto bounds = module->getBounds().toFloat();
        const float x = bounds.getRight() + 8.0f;
        const float y = bounds.getCentreY();

        juce::Path arrow;
        arrow.startNewSubPath(x - 3.0f, y - 6.0f);
        arrow.lineTo(x + 3.0f, y);
        arrow.lineTo(x - 3.0f, y + 6.0f);
        g.strokePath(arrow, juce::PathStrokeType(1.6f));
    }

    // 5. I/O 미터 패널
    const auto meterPanel = inputMeter.getBounds().getUnion(outputMeter.getBounds()).toFloat().expanded(4.0f, 6.0f);
    g.setColour(Palette::panel.withAlpha(0.92f));
    g.fillRoundedRectangle(meterPanel, 6.0f);
    g.setColour(Palette::panelBorder);
    g.drawRoundedRectangle(meterPanel, 6.0f, 1.0f);
}

void WxMainView::resized()
{
    // 위: 시각화 줄
    display.setBounds(20, 64, 600, 262);
    transferCurve.setBounds(632, 64, 238, 262);
    inputMeter.setBounds(882, 70, 27, 250);
    outputMeter.setBounds(909, 70, 27, 250);

    // 아래: 스테이지 줄  (DRIVE › CRUSH › DIE    MIX  OUTPUT)
    driveModule.setBounds(20, 338, 190, 242);
    crushModule.setBounds(226, 338, 190, 242);
    dieModule.setBounds(432, 338, 190, 242);
    mixModule.setBounds(638, 338, 145, 242);
    outputModule.setBounds(795, 338, 145, 242);
}

//==============================================================================
WxCrusherAudioProcessorEditor::WxCrusherAudioProcessorEditor(WxCrusherAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), mainView(p)
{
    addAndMakeVisible(mainView);

    // 지난번 창 크기는 제일 먼저 읽어둡니다.
    // (아래 setResizeLimits 가 바로 resized() 를 불러 editorWidth 를 덮어쓰기 때문)
    int width = audioProcessor.editorWidth.load();

    // 창 크기 조절 (비율 고정, 75% ~ 160%)
    constexpr double aspectRatio = (double)WxMainView::baseWidth / (double)WxMainView::baseHeight;
    const int minWidth = WxMainView::baseWidth * 3 / 4;
    const int maxWidth = WxMainView::baseWidth * 8 / 5;

    setResizable(true, true);
    setResizeLimits(minWidth, juce::roundToInt(minWidth / aspectRatio), maxWidth, juce::roundToInt(maxWidth / aspectRatio));
    getConstrainer()->setFixedAspectRatio(aspectRatio);

    // 지난번에 쓰던 창 크기로 열기 (처음이면 100%)
    if (width < minWidth || width > maxWidth)
        width = WxMainView::baseWidth;

    setSize(width, juce::roundToInt(width / aspectRatio));
}

WxCrusherAudioProcessorEditor::~WxCrusherAudioProcessorEditor()
{
}

//==============================================================================
void WxCrusherAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(wx::Palette::background);
}

void WxCrusherAudioProcessorEditor::resized()
{
    // 960 x 600 으로 만든 화면을 창 크기에 맞게 통째로 확대/축소
    const float scale = (float)getWidth() / (float)WxMainView::baseWidth;
    mainView.setTransform(juce::AffineTransform::scale(scale));

    audioProcessor.editorWidth = getWidth();
}
