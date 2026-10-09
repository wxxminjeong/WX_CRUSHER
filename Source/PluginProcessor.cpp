/*
  ==============================================================================

    WX CRUSHER - PluginProcessor.cpp

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WxCrusherAudioProcessor::WxCrusherAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    // 🎛️ [핵심] APVTS 초기화 (비서 채용)
    apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    driveOnParam = apvts.getRawParameterValue(wx::ParamID::driveOn);
    driveParam   = apvts.getRawParameterValue(wx::ParamID::drive);
    crushOnParam = apvts.getRawParameterValue(wx::ParamID::crushOn);
    crushParam   = apvts.getRawParameterValue(wx::ParamID::crush);
    dieOnParam   = apvts.getRawParameterValue(wx::ParamID::dieOn);
    dieParam     = apvts.getRawParameterValue(wx::ParamID::die);
    mixParam     = apvts.getRawParameterValue(wx::ParamID::mix);
    outputParam  = apvts.getRawParameterValue(wx::ParamID::output);
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
    return false;
}

bool WxCrusherAudioProcessor::producesMidi() const
{
    return false;
}

bool WxCrusherAudioProcessor::isMidiEffect() const
{
    return false;
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

void WxCrusherAudioProcessor::setCurrentProgram(int)
{
}

const juce::String WxCrusherAudioProcessor::getProgramName(int)
{
    return {};
}

void WxCrusherAudioProcessor::changeProgramName(int, const juce::String&)
{
}

//==============================================================================
void WxCrusherAudioProcessor::prepareToPlay(double sampleRate, int)
{
    // 노브를 돌리면 30ms 동안 부드럽게 따라갑니다.
    constexpr double rampSeconds = 0.03;

    for (auto* smoother : { &driveOnSmoothed, &driveGainSmoothed, &crushOnSmoothed, &bitsSmoothed,
                            &dieOnSmoothed, &ceilingSmoothed, &mixSmoothed, &outputGainSmoothed })
        smoother->reset(sampleRate, rampSeconds);

    // 재생을 시작할 때는 램프 없이 바로 현재 노브 값에서 출발합니다.
    const auto settings = getCurrentSettings();
    driveOnSmoothed.setCurrentAndTargetValue(settings.driveOn);
    driveGainSmoothed.setCurrentAndTargetValue(settings.driveGain);
    crushOnSmoothed.setCurrentAndTargetValue(settings.crushOn);
    bitsSmoothed.setCurrentAndTargetValue(settings.bits);
    dieOnSmoothed.setCurrentAndTargetValue(settings.dieOn);
    ceilingSmoothed.setCurrentAndTargetValue(settings.ceiling);
    mixSmoothed.setCurrentAndTargetValue(settings.mix);
    outputGainSmoothed.setCurrentAndTargetValue(settings.outputGain);

    visualTap.sampleRate = sampleRate;
}

void WxCrusherAudioProcessor::releaseResources()
{
}

bool WxCrusherAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // 모노 / 스테레오만 지원하고, 입력과 출력 채널 수는 같아야 합니다.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}

//==============================================================================
wx::Settings WxCrusherAudioProcessor::getCurrentSettings() const noexcept
{
    wx::Settings s;
    s.driveOn    = driveOnParam->load() >= 0.5f ? 1.0f : 0.0f;
    s.driveGain  = juce::Decibels::decibelsToGain(driveParam->load());
    s.crushOn    = crushOnParam->load() >= 0.5f ? 1.0f : 0.0f;
    s.bits       = crushParam->load();
    s.dieOn      = dieOnParam->load() >= 0.5f ? 1.0f : 0.0f;
    s.ceiling    = juce::Decibels::decibelsToGain(dieParam->load());
    s.mix        = mixParam->load() / 100.0f;
    s.outputGain = juce::Decibels::decibelsToGain(outputParam->load());
    return s;
}

void WxCrusherAudioProcessor::updateSmootherTargets() noexcept
{
    const auto settings = getCurrentSettings();
    driveOnSmoothed.setTargetValue(settings.driveOn);
    driveGainSmoothed.setTargetValue(settings.driveGain);
    crushOnSmoothed.setTargetValue(settings.crushOn);
    bitsSmoothed.setTargetValue(settings.bits);
    dieOnSmoothed.setTargetValue(settings.dieOn);
    ceilingSmoothed.setTargetValue(settings.ceiling);
    mixSmoothed.setTargetValue(settings.mix);
    outputGainSmoothed.setTargetValue(settings.outputGain);
}

void WxCrusherAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    // 1. 쓰지 않는 채널은 깨끗하게 비웁니다.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    // 2. 노브 값 → 스무더 목표값
    updateSmootherTargets();

    const int numChannels = juce::jmin(totalNumInputChannels, buffer.getNumChannels());
    if (numChannels <= 0 || numSamples <= 0)
        return;

    // ================================================================
    // 😈 INPUT → DRIVE → CRUSH → DIE → MIX → OUTPUT
    // ================================================================
    auto* const* channelData = buffer.getArrayOfWritePointers();
    const float channelScale = 1.0f / (float)numChannels;

    // 📺 화면으로 보낼 정보 (오디오 스레드에서 메모리 할당 없이 스택에 모았다가 보냅니다)
    constexpr int tapChunkSize = 256;
    float tapInput[tapChunkSize];
    float tapOutput[tapChunkSize];
    int tapCount = 0;

    float inputPeak = 0.0f, outputPeak = 0.0f;
    wx::StageActivity activity;

    wx::Settings settings;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // 스무더는 채널과 상관없이 샘플마다 한 칸씩 움직입니다.
        settings.driveOn    = driveOnSmoothed.getNextValue();
        settings.driveGain  = driveGainSmoothed.getNextValue();
        settings.crushOn    = crushOnSmoothed.getNextValue();
        settings.bits       = bitsSmoothed.getNextValue();
        settings.dieOn      = dieOnSmoothed.getNextValue();
        settings.ceiling    = ceilingSmoothed.getNextValue();
        settings.mix        = mixSmoothed.getNextValue();
        settings.outputGain = outputGainSmoothed.getNextValue();

        float inputSum = 0.0f, outputSum = 0.0f;

        for (int channel = 0; channel < numChannels; ++channel)
        {
            const float inputSignal = channelData[channel][sample];
            const float outputSignal = wx::processSample(inputSignal, settings, &activity);
            channelData[channel][sample] = outputSignal;

            inputSum += inputSignal;
            outputSum += outputSignal;
            inputPeak = juce::jmax(inputPeak, std::abs(inputSignal));
            outputPeak = juce::jmax(outputPeak, std::abs(outputSignal));
        }

        tapInput[tapCount] = inputSum * channelScale;
        tapOutput[tapCount] = outputSum * channelScale;

        if (++tapCount == tapChunkSize)
        {
            visualTap.pushSamples(tapInput, tapOutput, tapCount);
            tapCount = 0;
        }
    }

    if (tapCount > 0)
        visualTap.pushSamples(tapInput, tapOutput, tapCount);

    // CRUSH 활동량은 소리가 거의 없을 때(평균 -80dB 미만)는 0 으로 봅니다.
    const float crushRatio = activity.crushInput > 1.0e-4f * (float)(numSamples * numChannels)
                                 ? activity.crushChange / activity.crushInput : 0.0f;

    visualTap.pushLevels(inputPeak, outputPeak,
                         (float)activity.clippedSamples / (float)(numSamples * numChannels),
                         crushRatio);
}

//==============================================================================
bool WxCrusherAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* WxCrusherAudioProcessor::createEditor()
{
    return new WxCrusherAudioProcessorEditor(*this);
}

//==============================================================================
void WxCrusherAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // 💾 DAW 프로젝트를 저장할 때 노브 값과 화면 설정을 같이 저장합니다.
    auto state = apvts.copyState();
    state.setProperty("displayMode", displayMode.load(), nullptr);
    state.setProperty("editorWidth", editorWidth.load(), nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void WxCrusherAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // 📂 프로젝트를 다시 열면 저장해 둔 값을 되돌려 놓습니다.
    auto xml = getXmlFromBinary(data, sizeInBytes);

    if (xml == nullptr || ! xml->hasTagName(apvts.state.getType()))
        return;

    auto state = juce::ValueTree::fromXml(*xml);
    displayMode = (int)state.getProperty("displayMode", 0);
    editorWidth = (int)state.getProperty("editorWidth", 0);

    state.removeProperty("displayMode", nullptr);
    state.removeProperty("editorWidth", nullptr);
    apvts.replaceState(state);
}

//==============================================================================
// 🎛️ [핵심] 파라미터 목록 정의 함수
juce::AudioProcessorValueTreeState::ParameterLayout WxCrusherAudioProcessor::createParameterLayout()
{
    using namespace wx;

    // 화면/DAW 에 보이는 숫자 모양 (+12.0, -6.0, 8.5 ...)
    auto withSign = [](float value, int)
    {
        if (std::abs(value) < 0.05f)
            return juce::String("0.0");

        return (value > 0.0f ? "+" : "") + juce::String(value, 1);
    };

    auto oneDecimal = [](float value, int) { return juce::String(value, 1); };
    auto noDecimal = [](float value, int) { return juce::String(juce::roundToInt(value)); };

    // 숫자를 직접 입력할 때: 숫자가 없거나("off", "nan", 빈 칸) 이상한 값이면 기본값으로
    auto parseOr = [](float fallback)
    {
        return [fallback](const juce::String& text)
        {
            const auto trimmed = text.trim();

            if (! trimmed.containsAnyOf("0123456789"))
                return fallback;

            const float value = trimmed.getFloatValue();
            return std::isfinite(value) ? value : fallback;
        };
    };

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // --- I. DRIVE ---
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParamID::driveOn, 1 }, "Drive On", true));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamID::drive, 1 }, "Drive",
        juce::NormalisableRange<float>(0.0f, maxDriveDb),  // 0 ~ +26dB (1배 ~ 20배)
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB").withStringFromValueFunction(withSign)
                                             .withValueFromStringFunction(parseOr(0.0f))));

    // --- II. CRUSH ---
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParamID::crushOn, 1 }, "Crush On", true));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamID::crush, 1 }, "Crush Bits",
        makeReversedRange(cleanBits, minBits),               // 노브 왼쪽 16bit → 오른쪽 1bit
        cleanBits,
        juce::AudioParameterFloatAttributes().withLabel("bit").withStringFromValueFunction(oneDecimal)
                                             .withValueFromStringFunction(parseOr(cleanBits))));

    // --- III. DIE ---
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParamID::dieOn, 1 }, "Die On", true));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamID::die, 1 }, "Die Ceiling",
        makeReversedRange(0.0f, minCeilingDb),               // 노브 왼쪽 0dB → 오른쪽 -24dB
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB").withStringFromValueFunction(withSign)
                                             .withValueFromStringFunction(parseOr(0.0f))));

    // --- MIX / OUTPUT ---
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamID::mix, 1 }, "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction(noDecimal)
                                             .withValueFromStringFunction(parseOr(100.0f))));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamID::output, 1 }, "Output",
        juce::NormalisableRange<float>(minOutputDb, maxOutputDb),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB").withStringFromValueFunction(withSign)
                                             .withValueFromStringFunction(parseOr(0.0f))));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WxCrusherAudioProcessor();
}
