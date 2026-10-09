/*
  ==============================================================================
    WX DRIVE - 무식한 입력 드라이브 (Extreme Drive)
    입력 소리를 최대 +26dB(약 20배)까지 키웁니다.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../Shared/WxProcessorBase.h"

//==============================================================================
class WxDriveAudioProcessor : public WxProcessorBase
{
public:
    WxDriveAudioProcessor();

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;

private:
    // 🎛️ [핵심] 파라미터 목록을 만드는 도우미 함수
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxDriveAudioProcessor)
};
