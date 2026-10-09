/*
  ==============================================================================
    WX CRUSH - PluginProcessor.cpp
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "../../Shared/WxKnobEditor.h"

//==============================================================================
WxCrushAudioProcessor::WxCrushAudioProcessor()
    : WxProcessorBase(createParameterLayout())
{
}

//==============================================================================
void WxCrushAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // 노브 값 (16bit ~ 1bit)
    float bitDepth = *apvts.getRawParameterValue("BITS");

    // 16bit(노브 맨 왼쪽)면 아무 처리도 하지 않고 원음을 내보냅니다. (CPU 절약)
    if (bitDepth >= 16.0f) return;

    // 계단 한 칸의 크기. 비트가 낮을수록 계단이 커집니다.
    float stepSize = 1.0f / std::pow(2.0f, bitDepth);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // 가장 가까운 계단 높이로 소리를 강제로 맞춥니다.
            channelData[sample] = std::round(channelData[sample] / stepSize) * stepSize;
        }
    }
}

//==============================================================================
juce::AudioProcessorEditor* WxCrushAudioProcessor::createEditor()
{
    return new WxKnobEditor(*this, "BITS", "CRUSH");
}

//==============================================================================
// 🎛️ [핵심] 파라미터 목록 정의 함수
juce::AudioProcessorValueTreeState::ParameterLayout WxCrushAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "BITS", 1 },    // ID
        "Bits",                            // 이름
        makeReversedRange(16.0f, 1.0f),    // 노브 왼쪽 16bit → 오른쪽 1bit
        16.0f,                             // 기본값 (원음)
        juce::AudioParameterFloatAttributes()
            .withLabel("bit")
            .withStringFromValueFunction([](float value, int) { return juce::String(value, 1); })
    ));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WxCrushAudioProcessor();
}
