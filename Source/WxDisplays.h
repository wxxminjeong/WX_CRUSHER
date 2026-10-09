/*
  ==============================================================================
    WxDisplays.h

    📺 시각화 모음
      - WxScopeView      : 파형 (입력 = 회색 잔상, 출력 = 흰색, 클리핑하면 빨갛게)
      - WxSpectrumView   : 스펙트럼 (입력 대비 출력에 새로 생긴 배음이 보입니다)
      - WxDisplay        : 위 두 개를 WAVE / SPECTRUM 탭으로 바꿔 보여주는 패널
      - WxTransferCurve  : 지금 노브 설정이 소리를 어떻게 바꾸는지 보여주는 입력→출력 곡선
      - WxMeter          : 입력 / 출력 레벨 미터
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "WxDsp.h"
#include "WxLookAndFeel.h"

//==============================================================================
class WxScopeView : public juce::Component
{
public:
    WxScopeView();

    void pushSamples(const float* input, const float* output, int numSamples);
    void setSampleRate(double newSampleRate) { sampleRate = newSampleRate; }
    void setClipActivity(float newActivity) { clipActivity = newActivity; }

    void paint(juce::Graphics&) override;

private:
    float getHistory(const std::vector<float>& history, int indexFromWrite) const;
    juce::Path makeWavePath(const std::vector<float>& history, int startOffset, int length,
                            juce::Rectangle<float> area) const;

    static constexpr int historySize = 1 << 15;
    std::vector<float> inputHistory, outputHistory;
    int writeIndex = 0;

    double sampleRate = 44100.0;
    float clipActivity = 0.0f;

    juce::SharedResourcePointer<wx::Fonts> fonts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxScopeView)
};

//==============================================================================
class WxSpectrumView : public juce::Component
{
public:
    WxSpectrumView();

    void pushSamples(const float* input, const float* output, int numSamples);
    void setSampleRate(double newSampleRate) { sampleRate = newSampleRate; }

    // 화면 타이머에서 한 프레임에 한 번 부릅니다.
    void updateSpectrum();

    void paint(juce::Graphics&) override;

private:
    static constexpr int fftOrder = 12;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int numBins = fftSize / 2;
    static constexpr float minDb = -96.0f, maxDb = 6.0f;

    void analyse(const std::vector<float>& ring, std::vector<float>& smoothedDb);
    float frequencyToX(float frequency, float width) const;
    float dbToY(float db, juce::Rectangle<float> area) const;
    juce::Path makeSpectrumPath(const std::vector<float>& db, juce::Rectangle<float> area, bool closed) const;

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { (size_t)fftSize, juce::dsp::WindowingFunction<float>::hann };

    std::vector<float> inputRing, outputRing;
    int ringIndex = 0;

    std::vector<float> fftData;
    std::vector<float> inputDb, outputDb;

    double sampleRate = 44100.0;

    juce::SharedResourcePointer<wx::Fonts> fonts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxSpectrumView)
};

//==============================================================================
class WxDisplay : public juce::Component
{
public:
    explicit WxDisplay(std::atomic<int>& displayModeToUse);

    void pushSamples(const float* input, const float* output, int numSamples);
    void setSampleRate(double newSampleRate);
    void setClipActivity(float newActivity);

    // 화면 타이머에서 한 프레임에 한 번 부릅니다.
    void refresh();

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void showMode(int mode);

    std::atomic<int>& displayMode;
    int shownMode = -1;

    WxScopeView scope;
    WxSpectrumView spectrum;
    juce::TextButton waveTab { "WAVE" }, spectrumTab { "SPECTRUM" };

    juce::SharedResourcePointer<wx::Fonts> fonts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxDisplay)
};

//==============================================================================
class WxTransferCurve : public juce::Component
{
public:
    WxTransferCurve() = default;

    void setSettings(const wx::Settings& newSettings);
    void setInputLevel(float newLevel);

    void paint(juce::Graphics&) override;

private:
    juce::Rectangle<float> getPlotArea() const;

    wx::Settings settings;
    float inputLevel = 0.0f;

    juce::SharedResourcePointer<wx::Fonts> fonts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxTransferCurve)
};

//==============================================================================
class WxMeter : public juce::Component
{
public:
    explicit WxMeter(const juce::String& labelToShow);

    // 화면 타이머에서 마지막 프레임 동안의 피크(선형)를 넘겨줍니다.
    void setLevel(float linearPeak);

    void paint(juce::Graphics&) override;

private:
    static constexpr float minDb = -60.0f, maxDb = 6.0f;
    float dbToY(float db, juce::Rectangle<float> bar) const;

    juce::String label;
    float levelDb = minDb, peakHoldDb = minDb;
    int peakHoldFrames = 0;

    juce::SharedResourcePointer<wx::Fonts> fonts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxMeter)
};
