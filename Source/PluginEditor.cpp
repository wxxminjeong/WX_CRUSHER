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
    tap.clearLevels();

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

    const auto settings = audioProcessor.getCurrentSettings();

    // 2. 레벨 : 새 오디오 블록이 왔을 때만 갱신합니다.
    //    (버퍼가 큰 DAW 에서는 화면 한 프레임 동안 블록이 안 올 수도 있어서, 그때 0 으로 보면 미터가 깜빡입니다.)
    //    200ms 넘게 아무것도 안 오면 재생이 멈춘 것으로 보고 천천히 내립니다.
    auto takeIfNew = [](float taken, float& held)
    {
        if (taken >= 0.0f) // WxVisualTap::noNewBlock(-1) 이면 이전 값 유지
            held = taken;

        return taken >= 0.0f;
    };

    const bool hasNewAudio = takeIfNew(tap.takeInputPeak(), heldInputPeak);
    takeIfNew(tap.takeOutputPeak(), heldOutputPeak);
    takeIfNew(tap.takeClipAmount(), heldClipRatio);
    takeIfNew(tap.takeCrushAmount(), heldCrushRatio);

    framesWithoutAudio = hasNewAudio ? 0 : framesWithoutAudio + 1;

    if (framesWithoutAudio > 12)
        heldInputPeak = heldOutputPeak = heldClipRatio = heldCrushRatio = 0.0f;

    // 떨어지는 속도는 블록이 아니라 화면 프레임 기준 (버퍼 크기와 상관없이 같은 속도)
    inputMeter.setLevel(heldInputPeak);
    outputMeter.setLevel(heldOutputPeak);

    // 확 켜지고, 천천히 꺼지는 엔벨로프
    auto follow = [](float current, float target)
    {
        const float next = juce::jmax(target, current * 0.9f);
        return next < 0.01f ? 0.0f : next;
    };

    inputLevel = juce::jmax(heldInputPeak, inputLevel * 0.92f);
    if (inputLevel < 1.0e-3f)
        inputLevel = 0.0f;

    // DIE : 잘린 샘플이 있으면 확 켜짐
    clipActivity = follow(clipActivity, heldClipRatio > 0.0f ? juce::jmin(1.0f, 0.45f + heldClipRatio * 2.0f) : 0.0f);

    // CRUSH : 바뀐 양 0.1% → 꺼짐, 1% → 1/3, 10% → 2/3, 100% → 최대
    crushActivity = follow(crushActivity, heldCrushRatio > 1.0e-3f ? juce::jlimit(0.0f, 1.0f, std::log10(heldCrushRatio * 1000.0f) / 3.0f) : 0.0f);

    // 소리가 들어오고 있는 정도 (-60dB 이하 = 0, -30dB 이상 = 1)
    const float presence = juce::jlimit(0.0f, 1.0f, (juce::Decibels::gainToDecibels(inputLevel, -100.0f) + 60.0f) / 30.0f);

    // 3. 파형 : 잘린 소리가 실제로 출력에 섞여 나갈 때만 빨갛게 (MIX 0% 면 안 보임)
    display.setClipActivity(clipActivity * settings.mix);
    display.refresh();

    // 4. 전달 곡선 = 지금 노브 설정 + 지금 들어오는 소리 크기
    transferCurve.setSettings(settings);
    transferCurve.setInputLevel(inputLevel);

    // 5. 💡 스테이지 LED = "이 스테이지가 지금 실제로 소리를 바꾸고 있는가"
    //    DRIVE : 노브 양 x 소리가 있는지 / CRUSH : 실제로 바뀐 양 / DIE : 실제로 잘린 양
    driveModule.setActivity(settings.driveOn > 0.5f ? driveModule.getKnobPosition() * presence : 0.0f);
    crushModule.setActivity(settings.crushOn > 0.5f ? crushActivity : 0.0f);
    dieModule.setActivity(settings.dieOn > 0.5f ? clipActivity : 0.0f);

    // 6. 🩸 배경 글로우 : 소리가 나는 동안, 노브를 많이 돌릴수록 + 실제로 잘릴수록 붉게
    float destruction = 0.0f;
    for (auto* module : { &driveModule, &crushModule, &dieModule })
        if (module->isStageOn())
            destruction += module->getKnobPosition() / 3.0f;

    const float glowTarget = settings.mix * juce::jlimit(0.0f, 1.0f, 0.55f * destruction * presence + 0.45f * clipActivity);
    glowLevel += (glowTarget - glowLevel) * (glowTarget > glowLevel ? 0.35f : 0.08f);
    if (glowLevel < 0.005f)
        glowLevel = 0.0f;

    // 화면 전체를 다시 그리는 건 비싸니까, 1/32 단계로 바뀔 때만 다시 그립니다.
    const float newGlow = std::round(glowLevel * 32.0f) / 32.0f;

    if (std::abs(newGlow - glow) > 0.001f)
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

    g.setFont(fonts->mono(13.0f));
    g.setColour(Palette::red);
    g.drawText("DRIVE / CRUSH / DIE", 232, 26, 240, 20, juce::Justification::centredLeft, false);

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
