/*
  ==============================================================================
    WxDisplays.cpp
  ==============================================================================
*/

#include "WxDisplays.h"

namespace
{
    void drawPanel(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        g.setColour(wx::Palette::panel.withAlpha(0.92f));
        g.fillRoundedRectangle(bounds, 6.0f);
        g.setColour(wx::Palette::panelBorder);
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
    }
}

//==============================================================================
// 🌊 WxScopeView
//==============================================================================
WxScopeView::WxScopeView()
    : inputHistory((size_t)historySize, 0.0f), outputHistory((size_t)historySize, 0.0f)
{
    setInterceptsMouseClicks(false, false);
}

void WxScopeView::pushSamples(const float* input, const float* output, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        inputHistory[(size_t)writeIndex] = input[i];
        outputHistory[(size_t)writeIndex] = output[i];
        writeIndex = (writeIndex + 1) & (historySize - 1);
    }
}

// indexFromWrite = 몇 샘플 전인지 (1 = 가장 최근 샘플)
float WxScopeView::getHistory(const std::vector<float>& history, int indexFromWrite) const
{
    return history[(size_t)((writeIndex - indexFromWrite) & (historySize - 1))];
}

juce::Path WxScopeView::makeWavePath(const std::vector<float>& history, int startOffset, int length,
                                     juce::Rectangle<float> area) const
{
    juce::Path path;

    const float centreY = area.getCentreY();
    const float halfHeight = area.getHeight() * 0.42f;
    auto toY = [&](float value) { return centreY - juce::jlimit(-1.2f, 1.2f, value) * halfHeight; };

    const int numColumns = juce::jmax(2, (int)area.getWidth() * 2);

    if (length <= numColumns)
    {
        for (int i = 0; i < length; ++i)
        {
            const float x = area.getX() + area.getWidth() * (float)i / (float)(length - 1);
            const float y = toY(getHistory(history, startOffset - i));

            if (i == 0) path.startNewSubPath(x, y);
            else        path.lineTo(x, y);
        }

        return path;
    }

    // 샘플이 화면 픽셀보다 많으면 칸마다 최소/최대만 그립니다.
    for (int column = 0; column < numColumns; ++column)
    {
        const int first = column * length / numColumns;
        const int last = juce::jmax(first + 1, (column + 1) * length / numColumns);

        float low = 10.0f, high = -10.0f;
        for (int i = first; i < last; ++i)
        {
            const float value = getHistory(history, startOffset - i);
            low = juce::jmin(low, value);
            high = juce::jmax(high, value);
        }

        const float x = area.getX() + area.getWidth() * (float)column / (float)(numColumns - 1);

        if (column == 0) path.startNewSubPath(x, toY(low));
        else             path.lineTo(x, toY(low));

        path.lineTo(x, toY(high));
    }

    return path;
}

void WxScopeView::paint(juce::Graphics& g)
{
    using namespace wx;

    auto area = getLocalBounds().toFloat();
    const float centreY = area.getCentreY();
    const float halfHeight = area.getHeight() * 0.42f;

    // 1. 격자
    g.setColour(Palette::grid);
    for (int i = 1; i < 10; ++i)
        g.drawVerticalLine(juce::roundToInt(area.getX() + area.getWidth() * (float)i / 10.0f), area.getY(), area.getBottom());

    g.setColour(Palette::track);
    g.drawHorizontalLine(juce::roundToInt(centreY), area.getX(), area.getRight());

    // 0 dBFS 선 (이 밖으로 나가면 DAW 에서 클리핑)
    const float dashes[] = { 4.0f, 4.0f };
    g.setColour(Palette::deepRed.withAlpha(0.8f));
    for (float level : { 1.0f, -1.0f })
    {
        const float y = centreY - level * halfHeight;
        g.drawDashedLine({ area.getX(), y, area.getRight(), y }, dashes, 2, 1.0f);
    }

    g.setFont(fonts->mono(11.0f));
    g.setColour(Palette::deepRed);
    g.drawText("0 dBFS", area.withHeight(14.0f).withY(centreY - halfHeight - 15.0f).reduced(4.0f, 0.0f),
               juce::Justification::centredRight, false);

    // 2. 트리거 : 입력이 0 을 아래→위로 지나는 지점에서 시작해 파형이 흔들리지 않게
    const int displayLength = juce::jlimit(256, 8192, juce::roundToInt(sampleRate * 0.03));
    const int searchRange = juce::jmin(historySize - displayLength - 2, juce::roundToInt(sampleRate / 20.0));

    int startOffset = displayLength;
    for (int offset = displayLength; offset < displayLength + searchRange; ++offset)
    {
        if (getHistory(inputHistory, offset + 1) < 0.0f && getHistory(inputHistory, offset) >= 0.0f)
        {
            startOffset = offset;
            break;
        }
    }

    // 3. 입력 (회색 잔상)
    g.setColour(Palette::dim.withAlpha(0.55f));
    g.strokePath(makeWavePath(inputHistory, startOffset, displayLength, area), juce::PathStrokeType(1.2f));

    // 4. 출력 (흰색, 클리핑 중이면 빨갛게 번짐)
    const auto outputPath = makeWavePath(outputHistory, startOffset, displayLength, area);

    if (clipActivity > 0.0f)
    {
        g.setColour(Palette::red.withAlpha(0.35f * clipActivity));
        g.strokePath(outputPath, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    g.setColour(Palette::text);
    g.strokePath(outputPath, juce::PathStrokeType(1.6f));

    // 5. 시간 표시
    g.setFont(fonts->mono(11.0f));
    g.setColour(Palette::dim);
    g.drawText(juce::String(juce::roundToInt(1000.0 * displayLength / sampleRate)) + " ms",
               area.removeFromBottom(14.0f).reduced(4.0f, 0.0f), juce::Justification::centredRight, false);
}

//==============================================================================
// 📊 WxSpectrumView
//==============================================================================
WxSpectrumView::WxSpectrumView()
    : inputRing((size_t)fftSize, 0.0f), outputRing((size_t)fftSize, 0.0f),
      fftData((size_t)fftSize * 2, 0.0f),
      inputDb((size_t)numBins + 1, minDb), outputDb((size_t)numBins + 1, minDb)
{
    setInterceptsMouseClicks(false, false);
}

void WxSpectrumView::pushSamples(const float* input, const float* output, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        inputRing[(size_t)ringIndex] = input[i];
        outputRing[(size_t)ringIndex] = output[i];
        ringIndex = (ringIndex + 1) & (fftSize - 1);
    }
}

void WxSpectrumView::analyse(const std::vector<float>& ring, std::vector<float>& smoothedDb)
{
    // 오래된 샘플 → 최신 샘플 순서로 꺼내기
    for (int i = 0; i < fftSize; ++i)
        fftData[(size_t)i] = ring[(size_t)((ringIndex + i) & (fftSize - 1))];

    std::fill(fftData.begin() + fftSize, fftData.end(), 0.0f);

    window.multiplyWithWindowingTable(fftData.data(), (size_t)fftSize);
    fft.performFrequencyOnlyForwardTransform(fftData.data(), true);

    // 해닝 창을 쓴 사인파 하나가 0dB 로 보이도록 크기를 맞춥니다.
    const float scale = 4.0f / (float)fftSize;

    for (int bin = 0; bin <= numBins; ++bin)
    {
        const float db = juce::Decibels::gainToDecibels(fftData[(size_t)bin] * scale, minDb);
        auto& shown = smoothedDb[(size_t)bin];
        shown = juce::jmax(db, shown - 1.5f); // 올라갈 땐 바로, 내려갈 땐 천천히
    }
}

void WxSpectrumView::updateSpectrum()
{
    analyse(inputRing, inputDb);
    analyse(outputRing, outputDb);
    repaint();
}

float WxSpectrumView::frequencyToX(float frequency, float width) const
{
    const float maxFrequency = juce::jmin(20000.0f, (float)sampleRate * 0.5f);
    return width * std::log(frequency / 20.0f) / std::log(maxFrequency / 20.0f);
}

float WxSpectrumView::dbToY(float db, juce::Rectangle<float> area) const
{
    return juce::jmap(juce::jlimit(minDb, maxDb, db), minDb, maxDb, area.getBottom(), area.getY());
}

juce::Path WxSpectrumView::makeSpectrumPath(const std::vector<float>& db, juce::Rectangle<float> area, bool closed) const
{
    juce::Path path;

    const float maxFrequency = juce::jmin(20000.0f, (float)sampleRate * 0.5f);
    const float binsPerHz = (float)fftSize / (float)sampleRate;
    const int width = juce::jmax(2, (int)area.getWidth());

    auto frequencyAt = [&](int column) { return 20.0f * std::pow(maxFrequency / 20.0f, (float)column / (float)width); };

    for (int column = 0; column <= width; ++column)
    {
        // 이 픽셀 칸에 들어가는 주파수 빈들 중 가장 큰 값 (저음 쪽은 빈 사이를 보간)
        const float binLow = frequencyAt(column) * binsPerHz;
        const float binHigh = frequencyAt(column + 1) * binsPerHz;

        float value;
        if (binHigh - binLow < 1.0f)
        {
            const int index = juce::jlimit(0, numBins - 1, (int)binLow);
            const float fraction = juce::jlimit(0.0f, 1.0f, binLow - (float)index);
            value = db[(size_t)index] + fraction * (db[(size_t)index + 1] - db[(size_t)index]);
        }
        else
        {
            value = minDb;
            for (int bin = (int)binLow; bin <= juce::jmin(numBins, (int)binHigh); ++bin)
                value = juce::jmax(value, db[(size_t)bin]);
        }

        const float x = area.getX() + (float)column;
        const float y = dbToY(value, area);

        if (column == 0)
        {
            if (closed)
            {
                path.startNewSubPath(x, area.getBottom());
                path.lineTo(x, y);
            }
            else
            {
                path.startNewSubPath(x, y);
            }
        }
        else
        {
            path.lineTo(x, y);
        }
    }

    if (closed)
    {
        path.lineTo(area.getRight(), area.getBottom());
        path.closeSubPath();
    }

    return path;
}

void WxSpectrumView::paint(juce::Graphics& g)
{
    using namespace wx;

    auto area = getLocalBounds().toFloat();
    auto labels = area.removeFromBottom(14.0f);

    // 1. 격자
    g.setFont(fonts->mono(11.0f));

    for (float db : { 0.0f, -24.0f, -48.0f, -72.0f })
    {
        const float y = dbToY(db, area);
        g.setColour(db > -1.0f ? Palette::deepRed.withAlpha(0.8f) : Palette::grid);
        g.drawHorizontalLine(juce::roundToInt(y), area.getX(), area.getRight());
        g.setColour(Palette::dim.withAlpha(0.7f));
        g.drawText(juce::String(juce::roundToInt(db)), juce::Rectangle<float>(area.getX() + 4.0f, y + 1.0f, 40.0f, 12.0f),
                   juce::Justification::centredLeft, false);
    }

    const std::pair<float, const char*> marks[] = { { 50.0f, "50" }, { 100.0f, "100" }, { 200.0f, "200" }, { 500.0f, "500" },
                                                    { 1000.0f, "1k" }, { 2000.0f, "2k" }, { 5000.0f, "5k" }, { 10000.0f, "10k" } };
    for (const auto& [frequency, text] : marks)
    {
        const float x = area.getX() + frequencyToX(frequency, area.getWidth());
        g.setColour(Palette::grid);
        g.drawVerticalLine(juce::roundToInt(x), area.getY(), area.getBottom());
        g.setColour(Palette::dim.withAlpha(0.7f));
        g.drawText(text, juce::Rectangle<float>(x - 20.0f, labels.getY(), 40.0f, labels.getHeight()),
                   juce::Justification::centred, false);
    }

    // 2. 입력 (회색 면) / 출력 (빨간 면 + 흰 선)
    g.setColour(Palette::dim.withAlpha(0.22f));
    g.fillPath(makeSpectrumPath(inputDb, area, true));

    g.setColour(Palette::red.withAlpha(0.16f));
    g.fillPath(makeSpectrumPath(outputDb, area, true));

    g.setColour(Palette::text);
    g.strokePath(makeSpectrumPath(outputDb, area, false), juce::PathStrokeType(1.4f));
}

//==============================================================================
// 🖥️ WxDisplay
//==============================================================================
WxDisplay::WxDisplay(std::atomic<int>& displayModeToUse)
    : displayMode(displayModeToUse)
{
    addAndMakeVisible(scope);
    addChildComponent(spectrum);

    for (auto* tab : { &waveTab, &spectrumTab })
    {
        tab->setClickingTogglesState(true);
        tab->setRadioGroupId(4321);
        tab->setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(*tab);
    }

    waveTab.onClick = [this] { if (waveTab.getToggleState()) { displayMode = 0; showMode(0); } };
    spectrumTab.onClick = [this] { if (spectrumTab.getToggleState()) { displayMode = 1; showMode(1); } };

    showMode(displayMode.load());
}

void WxDisplay::pushSamples(const float* input, const float* output, int numSamples)
{
    scope.pushSamples(input, output, numSamples);
    spectrum.pushSamples(input, output, numSamples);
}

void WxDisplay::setSampleRate(double newSampleRate)
{
    if (newSampleRate > 0.0)
    {
        scope.setSampleRate(newSampleRate);
        spectrum.setSampleRate(newSampleRate);
    }
}

void WxDisplay::setClipActivity(float newActivity)
{
    scope.setClipActivity(newActivity);
}

void WxDisplay::refresh()
{
    if (displayMode.load() != shownMode)
        showMode(displayMode.load());

    if (shownMode == 1)
        spectrum.updateSpectrum();
    else
        scope.repaint();
}

void WxDisplay::showMode(int mode)
{
    shownMode = mode == 1 ? 1 : 0;

    scope.setVisible(shownMode == 0);
    spectrum.setVisible(shownMode == 1);
    waveTab.setToggleState(shownMode == 0, juce::dontSendNotification);
    spectrumTab.setToggleState(shownMode == 1, juce::dontSendNotification);
}

void WxDisplay::paint(juce::Graphics& g)
{
    using namespace wx;

    drawPanel(g, getLocalBounds().toFloat().reduced(0.5f));

    // 범례 (오른쪽 위)
    auto legend = getLocalBounds().removeFromTop(32).reduced(14, 0).removeFromRight(120).toFloat();
    g.setFont(fonts->mono(11.0f));

    auto drawKey = [&](juce::Rectangle<float> slot, juce::Colour colour, const juce::String& text)
    {
        g.setColour(colour);
        g.fillRect(slot.removeFromLeft(12.0f).withSizeKeepingCentre(12.0f, 2.0f));
        g.setColour(Palette::dim);
        g.drawText(text, slot.withTrimmedLeft(5.0f), juce::Justification::centredLeft, false);
    };

    drawKey(legend.removeFromLeft(56.0f), Palette::dim, "IN");
    drawKey(legend, Palette::text, "OUT");
}

void WxDisplay::resized()
{
    auto area = getLocalBounds();
    auto header = area.removeFromTop(32).reduced(10, 4);

    waveTab.setBounds(header.removeFromLeft(64));
    spectrumTab.setBounds(header.removeFromLeft(96));

    const auto content = area.reduced(10, 0).withTrimmedBottom(10);
    scope.setBounds(content);
    spectrum.setBounds(content);
}

//==============================================================================
// 📈 WxTransferCurve
//==============================================================================
void WxTransferCurve::setSettings(const wx::Settings& newSettings)
{
    auto differs = [](float a, float b) { return std::abs(a - b) > 1.0e-4f; };

    if (differs(settings.driveOn, newSettings.driveOn) || differs(settings.driveGain, newSettings.driveGain)
        || differs(settings.crushOn, newSettings.crushOn) || differs(settings.bits, newSettings.bits)
        || differs(settings.dieOn, newSettings.dieOn) || differs(settings.ceiling, newSettings.ceiling)
        || differs(settings.mix, newSettings.mix) || differs(settings.outputGain, newSettings.outputGain))
    {
        settings = newSettings;
        repaint();
    }
}

void WxTransferCurve::setInputLevel(float newLevel)
{
    newLevel = juce::jlimit(0.0f, 1.0f, newLevel);

    if (std::abs(newLevel - inputLevel) > 0.002f)
    {
        inputLevel = newLevel;
        repaint();
    }
}

juce::Rectangle<float> WxTransferCurve::getPlotArea() const
{
    const auto area = getLocalBounds().toFloat().reduced(16.0f).withTrimmedTop(22.0f);
    const float side = juce::jmin(area.getWidth(), area.getHeight());
    return area.withSizeKeepingCentre(side, side);
}

void WxTransferCurve::paint(juce::Graphics& g)
{
    using namespace wx;

    drawPanel(g, getLocalBounds().toFloat().reduced(0.5f));

    // 제목
    g.setFont(fonts->mono(11.0f));
    g.setColour(Palette::dim);
    const auto titleArea = getLocalBounds().removeFromTop(32).reduced(14, 0);
    g.drawText("TRANSFER", titleArea, juce::Justification::centredLeft, false);
    g.drawText("IN > OUT", titleArea, juce::Justification::centredRight, false);

    const auto plot = getPlotArea();
    constexpr float range = 1.1f;
    auto toX = [&](float v) { return plot.getX() + (v + range) / (2.0f * range) * plot.getWidth(); };
    auto toY = [&](float v) { return plot.getBottom() - (v + range) / (2.0f * range) * plot.getHeight(); };

    // 1. 격자
    g.setColour(Palette::grid);
    for (float v : { -1.0f, -0.5f, 0.5f, 1.0f })
    {
        g.drawVerticalLine(juce::roundToInt(toX(v)), plot.getY(), plot.getBottom());
        g.drawHorizontalLine(juce::roundToInt(toY(v)), plot.getX(), plot.getRight());
    }

    g.setColour(Palette::track);
    g.drawVerticalLine(juce::roundToInt(toX(0.0f)), plot.getY(), plot.getBottom());
    g.drawHorizontalLine(juce::roundToInt(toY(0.0f)), plot.getX(), plot.getRight());

    // 2. 원음 기준선 (y = x)
    const float dashes[] = { 3.0f, 3.0f };
    g.setColour(Palette::dim.withAlpha(0.6f));
    g.drawDashedLine({ toX(-1.0f), toY(-1.0f), toX(1.0f), toY(1.0f) }, dashes, 2, 1.0f);

    // 3. 지금 설정의 입력 → 출력 곡선
    juce::Graphics::ScopedSaveState clip(g);
    g.reduceClipRegion(plot.toNearestInt());

    const int numPoints = juce::jmax(64, (int)plot.getWidth() * 6);
    juce::Path curve, difference;

    for (int i = 0; i < numPoints; ++i)
    {
        const float x = -1.0f + 2.0f * (float)i / (float)(numPoints - 1);
        const float y = wx::processSample(x, settings);

        if (i == 0)
        {
            curve.startNewSubPath(toX(x), toY(y));
            difference.startNewSubPath(toX(x), toY(x));
            difference.lineTo(toX(x), toY(y));
        }
        else
        {
            curve.lineTo(toX(x), toY(y));
            difference.lineTo(toX(x), toY(y));
        }
    }

    // 원음과 달라진 만큼 빨갛게 칠합니다. (= 찌그러진 양)
    difference.lineTo(toX(1.0f), toY(1.0f));
    difference.closeSubPath();
    g.setColour(Palette::red.withAlpha(0.14f));
    g.fillPath(difference);

    g.setColour(Palette::text);
    g.strokePath(curve, juce::PathStrokeType(2.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::square));

    // 4. 지금 들어오는 소리의 크기 (곡선의 어느 부분을 쓰고 있는지)
    if (inputLevel > 0.001f)
    {
        g.setColour(Palette::red.withAlpha(0.35f));
        for (float level : { inputLevel, -inputLevel })
            g.drawVerticalLine(juce::roundToInt(toX(level)), plot.getY(), plot.getBottom());

        g.setColour(Palette::red);
        for (float level : { inputLevel, -inputLevel })
        {
            const float y = juce::jlimit(-range, range, wx::processSample(level, settings));
            g.fillEllipse(juce::Rectangle<float>(7.0f, 7.0f).withCentre({ toX(level), toY(y) }));
        }
    }
}

//==============================================================================
// 📶 WxMeter
//==============================================================================
WxMeter::WxMeter(const juce::String& labelToShow)
    : label(labelToShow)
{
    setInterceptsMouseClicks(false, false);
}

void WxMeter::setLevel(float linearPeak)
{
    const float db = juce::Decibels::gainToDecibels(linearPeak, minDb);
    const float previousLevel = levelDb, previousPeak = peakHoldDb;

    // 올라갈 땐 바로, 내려갈 땐 초당 약 60dB
    levelDb = db >= levelDb ? db : juce::jmax(db, levelDb - 1.0f);

    if (db >= peakHoldDb)
    {
        peakHoldDb = db;
        peakHoldFrames = 45;
    }
    else if (peakHoldFrames > 0)
    {
        --peakHoldFrames;
    }
    else
    {
        peakHoldDb = juce::jmax(minDb, peakHoldDb - 0.6f);
    }

    if (std::abs(previousLevel - levelDb) > 0.05f || std::abs(previousPeak - peakHoldDb) > 0.05f)
        repaint();
}

float WxMeter::dbToY(float db, juce::Rectangle<float> bar) const
{
    return juce::jmap(juce::jlimit(minDb, maxDb, db), minDb, maxDb, bar.getBottom(), bar.getY());
}

void WxMeter::paint(juce::Graphics& g)
{
    using namespace wx;

    auto area = getLocalBounds().toFloat();
    const auto labelArea = area.removeFromBottom(16.0f);
    const auto readoutArea = area.removeFromTop(16.0f);
    const auto bar = area.reduced(0.0f, 4.0f).withSizeKeepingCentre(10.0f, area.getHeight() - 8.0f);

    // 숫자 (피크 홀드)
    const bool isOver = peakHoldDb > 0.0f;
    g.setFont(fonts->mono(9.0f));
    g.setColour(isOver ? Palette::red : Palette::dim);
    const auto readout = peakHoldDb <= minDb + 0.5f ? juce::String("-inf")
                                                   : (isOver ? "+" : "") + juce::String(peakHoldDb, 1);
    g.drawText(readout, readoutArea, juce::Justification::centred, false);

    // 막대 배경
    g.setColour(Palette::grid);
    g.fillRect(bar);

    // LED 칸 (아래에서 위로)
    constexpr float segmentHeight = 3.0f, gap = 1.0f;
    for (float y = bar.getBottom() - segmentHeight; y >= bar.getY(); y -= segmentHeight + gap)
    {
        const float segmentDb = juce::jmap(y + segmentHeight * 0.5f, bar.getBottom(), bar.getY(), minDb, maxDb);

        if (segmentDb > levelDb)
            break;

        g.setColour(segmentDb > 0.0f ? Palette::red : (segmentDb > -12.0f ? Palette::text : Palette::text.withAlpha(0.55f)));
        g.fillRect(bar.getX(), y, bar.getWidth(), segmentHeight);
    }

    // 0dB 표시 + 피크 홀드 선
    g.setColour(Palette::red.withAlpha(0.7f));
    g.fillRect(bar.getX() - 3.0f, dbToY(0.0f, bar), bar.getWidth() + 6.0f, 1.0f);

    if (peakHoldDb > minDb)
    {
        g.setColour(isOver ? Palette::red : Palette::text);
        g.fillRect(bar.getX(), dbToY(peakHoldDb, bar) - 1.0f, bar.getWidth(), 2.0f);
    }

    // 이름
    g.setFont(fonts->mono(11.0f));
    g.setColour(Palette::dim);
    g.drawText(label, labelArea, juce::Justification::centred, false);
}
