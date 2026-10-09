/*
  ==============================================================================

    WX CRUSHER - PluginProcessor.h
    INPUT → I. DRIVE → II. BITCRUSH → III. CLIPPER → MIX → OUTPUT

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "WxDsp.h"
#include "WxVisualTap.h"

//==============================================================================
/**
*/
class WxCrusherAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    WxCrusherAudioProcessor();
    ~WxCrusherAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // 🎛️ [핵심] 노브 값을 관리하는 비서 (APVTS)
    juce::AudioProcessorValueTreeState apvts;

    // 📺 화면(파형, 스펙트럼, 미터)으로 보낼 데이터 통로
    WxVisualTap visualTap;

    // 지금 노브 값으로 만든 DSP 설정 (스무딩 없이 바로) - 화면의 전달 곡선이 씁니다.
    wx::Settings getCurrentSettings() const noexcept;

    // 🖥️ 화면 설정 (파라미터는 아니지만 프로젝트에 같이 저장됩니다)
    std::atomic<int> displayMode { 0 };   // 0 = WAVE, 1 = SPECTRUM
    std::atomic<int> editorWidth { 0 };   // 마지막 창 크기 (0 = 기본 크기)

private:
    // 🎛️ [핵심] 파라미터 목록을 만드는 도우미 함수
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // 노브 값을 스무더의 목표값으로 넘깁니다.
    void updateSmootherTargets() noexcept;

    // 노브 값을 매 블록 빠르게 읽기 위한 포인터
    std::atomic<float>* driveOnParam = nullptr;
    std::atomic<float>* driveParam = nullptr;
    std::atomic<float>* crushOnParam = nullptr;
    std::atomic<float>* crushParam = nullptr;
    std::atomic<float>* clipOnParam = nullptr;
    std::atomic<float>* clipParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* outputParam = nullptr;

    // 🧈 노브를 돌리거나 ON/OFF 할 때 "틱" 소리가 나지 않도록 값을 부드럽게 바꿔주는 장치
    juce::SmoothedValue<float> driveOnSmoothed, driveGainSmoothed;
    juce::SmoothedValue<float> crushOnSmoothed, bitsSmoothed;
    juce::SmoothedValue<float> clipOnSmoothed, ceilingSmoothed;
    juce::SmoothedValue<float> mixSmoothed, outputGainSmoothed;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxCrusherAudioProcessor)
};
