/*
  ==============================================================================
    PluginEditor.h
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class WxCrusherAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    WxCrusherAudioProcessorEditor(WxCrusherAudioProcessor&);
    ~WxCrusherAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // 우리의 플러그인 본체(뇌)를 가리키는 포인터
    WxCrusherAudioProcessor& audioProcessor;

    // 🎛️ 1. 눈에 보이는 '노브' (Slider)
    juce::Slider mainKnob;

    // 🔗 2. 노브와 DSP를 연결해주는 '접착제' (Attachment)
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mainKnobAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WxCrusherAudioProcessorEditor)
};