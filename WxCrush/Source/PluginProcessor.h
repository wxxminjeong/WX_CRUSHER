/*
  ==============================================================================
    WX CRUSH - 디지털 풍화 (Bit Reduction)
    소리의 해상도를 16bit → 1bit까지 떨어뜨려 계단 현상을 만듭니다.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../Shared/WxProcessorBase.h"

//==============================================================================
class WxCrushAudioProcessor : public WxProcessorBase
{
public:
    WxCrushAudioProcessor();

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;

private:
    // 🎛️ [핵심] 파라미터 목록을 만드는 도우미 함수
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxCrushAudioProcessor)
};
