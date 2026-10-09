/*
  ==============================================================================
    WX DIE - PluginProcessor.cpp
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "../../Shared/WxKnobEditor.h"

//==============================================================================
WxDieAudioProcessor::WxDieAudioProcessor()
    : WxProcessorBase(createParameterLayout())
{
}

//==============================================================================
void WxDieAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // 노브 값 (0dB ~ -24dB)을 실제 크기 (1.0 ~ 0.063)로 바꿉니다.
    float ceilingDb = *apvts.getRawParameterValue("CEILING");
    float ceiling = juce::Decibels::decibelsToGain(ceilingDb);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // 천장을 넘는 소리는 가차 없이 잘라버립니다. (Square Wave화)
            // 이것이 Rage 장르 특유의 "찢어지는 소리"를 만듭니다.
            channelData[sample] = juce::jlimit(-ceiling, ceiling, channelData[sample]);
        }
    }
}

//==============================================================================
juce::AudioProcessorEditor* WxDieAudioProcessor::createEditor()
{
    return new WxKnobEditor(*this, "CEILING", "DIE");
}

//==============================================================================
// 🎛️ [핵심] 파라미터 목록 정의 함수
juce::AudioProcessorValueTreeState::ParameterLayout WxDieAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "CEILING", 1 }, // ID
        "Ceiling",                         // 이름
        makeReversedRange(0.0f, -24.0f),   // 노브 왼쪽 0dB → 오른쪽 -24dB
        0.0f,                              // 기본값 (0dB에서 클리핑)
        juce::AudioParameterFloatAttributes()
            .withLabel("dB")
            .withStringFromValueFunction([](float value, int) { return juce::String(value, 1); })
    ));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WxDieAudioProcessor();
}
