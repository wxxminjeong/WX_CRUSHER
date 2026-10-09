/*
  ==============================================================================
    WxProcessorBase.h

    WX 플러그인 3종(DRIVE / CRUSH / DIE)이 함께 쓰는 공통 뼈대입니다.
    채널 설정, 프로그램, 상태 저장/불러오기처럼 매번 똑같은 코드를 여기 모았습니다.
    각 플러그인은 파라미터 목록, processBlock, createEditor만 만들면 됩니다.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class WxProcessorBase : public juce::AudioProcessor
{
public:
    explicit WxProcessorBase(juce::AudioProcessorValueTreeState::ParameterLayout layout);
    ~WxProcessorBase() override = default;

    //==============================================================================
    void prepareToPlay(double, int) override {}
    void releaseResources() override {}

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    //==============================================================================
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; } // 호스트에 따라 0이면 문제가 생겨서 최소 1
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // 🎛️ [핵심] 노브 값을 관리하는 비서 (APVTS)
    juce::AudioProcessorValueTreeState apvts;

    // 🔄 노브를 오른쪽으로 돌릴수록 값이 '작아지는' 범위를 만듭니다.
    //    예) CRUSH: 16bit(왼쪽) → 1bit(오른쪽), DIE: 0dB(왼쪽) → -24dB(오른쪽)
    //    덕분에 세 플러그인 모두 "오른쪽으로 돌릴수록 더 부서진다"로 방향이 같아집니다.
    static juce::NormalisableRange<float> makeReversedRange(float leftValue, float rightValue);

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxProcessorBase)
};
