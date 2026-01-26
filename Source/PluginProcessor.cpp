/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WxCrusherAudioProcessor::WxCrusherAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    // 🎛️ [핵심] APVTS 초기화 (비서 채용)
    apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
}

WxCrusherAudioProcessor::~WxCrusherAudioProcessor()
{
}

//==============================================================================
const juce::String WxCrusherAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WxCrusherAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool WxCrusherAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool WxCrusherAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double WxCrusherAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WxCrusherAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int WxCrusherAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WxCrusherAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String WxCrusherAudioProcessor::getProgramName(int index)
{
    return {};
}

void WxCrusherAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void WxCrusherAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // 초기화 작업이 필요하면 여기에 작성합니다.
}

void WxCrusherAudioProcessor::releaseResources()
{
    // 메모리 해제가 필요하면 여기에 작성합니다.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WxCrusherAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void WxCrusherAudioProcessor::processBlock(juce::AudioBuffer<float>&buffer, juce::MidiBuffer & midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // 1. 쓰지 않는 채널은 깨끗하게 비웁니다.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // ================================================================
    // 😈 WX RAGE MODE LOGIC
    // ================================================================

    // 0. 노브 값 가져오기 (0.0 ~ 100.0)
    // 이제 범위가 100까지이므로, 계산을 위해 100으로 나눠 0.0~1.0으로 만듭니다.
    float rawValue = *apvts.getRawParameterValue("GRITTY");
    float knobValue = rawValue / 100.0f;

    // 노브가 0(1% 미만)이면 아무 처리도 하지 않고 원음을 내보냅니다. (CPU 절약)
    if (knobValue < 0.01f) return;

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float inputSignal = channelData[sample];

            // --- 1단계: 무식한 입력 드라이브 (Extreme Drive) ---
            // 노브를 돌리면 입력 소리가 최대 20배까지 커집니다.
            float driveAmount = 1.0f + (knobValue * 19.0f);
            inputSignal *= driveAmount;


            // --- 2단계: 디지털 풍화 (Bit Reduction) ---
            // 소리의 해상도를 강제로 낮춰 계단 현상을 만듭니다.
            // 16비트 -> 3비트까지 급격하게 떨어뜨립니다.
            float bitDepth = 16.0f - (knobValue * 13.0f);
            float stepSize = 1.0f / std::pow(2.0f, bitDepth);

            if (stepSize > 0.0f) {
                inputSignal = std::round(inputSignal / stepSize) * stepSize;
            }


            // --- 3단계: 하드 클리핑 (Hard Clipping) ---
            // 1.0을 넘는 소리는 가차 없이 잘라버립니다. (Square Wave화)
            // 이것이 Rage 장르 특유의 "찢어지는 소리"를 만듭니다.
            if (inputSignal > 1.0f)
                inputSignal = 1.0f;
            else if (inputSignal < -1.0f)
                inputSignal = -1.0f;


            // --- 4단계: 출력 레벨 정리 ---
            // 소리가 너무 커서 귀가 아플 수 있으니 출력단에서 살짝 줄여줍니다.
            inputSignal *= 0.8f;

            // 처리된 소리를 다시 버퍼에 넣습니다.
            channelData[sample] = inputSignal;
        }
    }
}

//==============================================================================
bool WxCrusherAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* WxCrusherAudioProcessor::createEditor()
{
    // "WxCrusherAudioProcessorEditor"가 바로 우리가 만든 디자인 파일입니다.
    return new WxCrusherAudioProcessorEditor(*this);
}

//==============================================================================
void WxCrusherAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // 저장 기능 (나중에 구현)
}

void WxCrusherAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // 불러오기 기능 (나중에 구현)
}

//==============================================================================
// 🎛️ [핵심] 파라미터 목록 정의 함수
juce::AudioProcessorValueTreeState::ParameterLayout WxCrusherAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "GRITTY",       // ID
        "Gritty Knob",  // 이름
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), // 👈 범위 변경! (0 ~ 100)
        0.0f            // 기본값
    ));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WxCrusherAudioProcessor();
}