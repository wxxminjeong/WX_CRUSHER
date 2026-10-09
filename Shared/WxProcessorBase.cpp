/*
  ==============================================================================
    WxProcessorBase.cpp
  ==============================================================================
*/

#include "WxProcessorBase.h"

//==============================================================================
WxProcessorBase::WxProcessorBase(juce::AudioProcessorValueTreeState::ParameterLayout layout)
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    // 🎛️ [핵심] APVTS 초기화 (비서 채용) - 파라미터 목록은 각 플러그인이 넘겨줍니다.
    apvts(*this, nullptr, "Parameters", std::move(layout))
{
}

//==============================================================================
bool WxProcessorBase::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // 모노 / 스테레오만 지원하고, 입력과 출력 채널 수는 같아야 합니다.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}

//==============================================================================
void WxProcessorBase::getStateInformation(juce::MemoryBlock& destData)
{
    // 💾 DAW 프로젝트를 저장할 때 노브 값도 같이 저장합니다.
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void WxProcessorBase::setStateInformation(const void* data, int sizeInBytes)
{
    // 📂 프로젝트를 다시 열면 저장해 둔 노브 값을 되돌려 놓습니다.
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

//==============================================================================
juce::NormalisableRange<float> WxProcessorBase::makeReversedRange(float leftValue, float rightValue)
{
    jassert(leftValue > rightValue);

    // NormalisableRange는 (작은 값 ~ 큰 값)이어야 해서 범위는 그대로 두고,
    // 노브 위치(0.0 ~ 1.0) ↔ 실제 값 변환만 거꾸로 뒤집습니다.
    return { rightValue, leftValue,
             [](float start, float end, float position) { return end - position * (end - start); },
             [](float start, float end, float value) { return (end - value) / (end - start); },
             [](float start, float end, float value) { return juce::jlimit(start, end, value); } };
}
