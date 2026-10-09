/*
  ==============================================================================
    WX DIE - 하드 클리핑 (Hard Clipping)
    천장(Ceiling)을 넘는 소리를 가차 없이 잘라 사각파로 만듭니다.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../Shared/WxProcessorBase.h"

//==============================================================================
class WxDieAudioProcessor : public WxProcessorBase
{
public:
    WxDieAudioProcessor();

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;

private:
    // 🎛️ [핵심] 파라미터 목록을 만드는 도우미 함수
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxDieAudioProcessor)
};
