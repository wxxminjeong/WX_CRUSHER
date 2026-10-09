/*
  ==============================================================================
    WX DRIVE - PluginProcessor.cpp
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "../../Shared/WxKnobEditor.h"

//==============================================================================
WxDriveAudioProcessor::WxDriveAudioProcessor()
    : WxProcessorBase(createParameterLayout())
{
}

//==============================================================================
void WxDriveAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // 노브 값 (0 ~ +26dB)을 곱할 배수 (1배 ~ 20배)로 바꿉니다.
    float driveDb = *apvts.getRawParameterValue("DRIVE");
    float driveAmount = juce::Decibels::decibelsToGain(driveDb);

    // 모든 채널의 소리를 그만큼 키웁니다. (클리핑은 WX DIE의 몫)
    buffer.applyGain(driveAmount);
}

//==============================================================================
juce::AudioProcessorEditor* WxDriveAudioProcessor::createEditor()
{
    return new WxKnobEditor(*this, "DRIVE", "DRIVE");
}

//==============================================================================
// 🎛️ [핵심] 파라미터 목록 정의 함수
juce::AudioProcessorValueTreeState::ParameterLayout WxDriveAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "DRIVE", 1 },   // ID
        "Drive",                           // 이름
        juce::NormalisableRange<float>(0.0f, juce::Decibels::gainToDecibels(20.0f)), // 0 ~ +26dB (1배 ~ 20배)
        0.0f,                              // 기본값 (원음)
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
    return new WxDriveAudioProcessor();
}
