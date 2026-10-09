/*
  ==============================================================================
    WxDsp.h

    WX CRUSHER 의 3단계 DSP 수식 모음.
    오디오 처리(PluginProcessor)와 화면의 전달 곡선(Transfer Curve)이
    "완전히 같은 수식"을 쓰도록 여기 한 곳에만 적어둡니다.

    신호 흐름:  INPUT → I. DRIVE → II. CRUSH → III. DIE → MIX → OUTPUT
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace wx
{
    //==============================================================================
    // 🎛️ 파라미터 ID (DAW 오토메이션/저장에 쓰이는 이름이라 바꾸면 안 됩니다)
    namespace ParamID
    {
        inline constexpr const char* driveOn = "DRIVE_ON";
        inline constexpr const char* drive   = "DRIVE";
        inline constexpr const char* crushOn = "CRUSH_ON";
        inline constexpr const char* crush   = "CRUSH";
        inline constexpr const char* dieOn   = "DIE_ON";
        inline constexpr const char* die     = "DIE";
        inline constexpr const char* mix     = "MIX";
        inline constexpr const char* output  = "OUTPUT";
    }

    //==============================================================================
    // 📏 각 노브의 범위
    inline constexpr float maxDriveDb   = 26.0206f; // +26dB = 20배 (v0.1 과 같은 최대치)
    inline constexpr float cleanBits    = 16.0f;    // 16bit = 원음 그대로
    inline constexpr float minBits      = 1.0f;
    inline constexpr float minCeilingDb = -24.0f;
    inline constexpr float minOutputDb  = -24.0f;
    inline constexpr float maxOutputDb  = 12.0f;

    //==============================================================================
    // I. DRIVE : 소리를 무식하게 키웁니다.
    inline float drive(float x, float gain) noexcept
    {
        return x * gain;
    }

    // II. CRUSH : 소리의 해상도를 낮춰 계단을 만듭니다. (16bit → 1bit)
    inline float crush(float x, float bits) noexcept
    {
        if (bits >= cleanBits)
            return x;

        const float stepSize = std::exp2(-bits); // = 1 / 2^bits
        return std::round(x / stepSize) * stepSize;
    }

    // III. DIE : 천장(ceiling)을 넘는 소리를 가차 없이 잘라 사각파로 만듭니다.
    inline float die(float x, float ceiling) noexcept
    {
        return juce::jlimit(-ceiling, ceiling, x);
    }

    //==============================================================================
    // 한 샘플을 처리하는 데 필요한 값들 (오디오 스레드에서는 매 샘플 부드럽게 변합니다)
    struct Settings
    {
        // 스테이지 ON/OFF (0 = 꺼짐, 1 = 켜짐, 그 사이 = 클릭 없이 전환되는 중)
        float driveOn = 1.0f;
        float crushOn = 1.0f;
        float dieOn   = 1.0f;

        float driveGain  = 1.0f;      // 배수
        float bits       = cleanBits; // 비트
        float ceiling    = 1.0f;      // 클리핑 천장 (선형 크기)
        float mix        = 1.0f;      // 0 = 원음, 1 = 100% 이펙트
        float outputGain = 1.0f;      // 배수
    };

    // DRIVE → CRUSH → DIE → MIX → OUTPUT
    // isClipping 이 주어지면 DIE 에서 실제로 잘렸는지 알려줍니다. (화면의 DIE LED 용)
    inline float processSample(float input, const Settings& s, bool* isClipping = nullptr) noexcept
    {
        float wet = input;

        if (s.driveOn > 0.0f)
            wet += s.driveOn * (drive(wet, s.driveGain) - wet);

        if (s.crushOn > 0.0f)
            wet += s.crushOn * (crush(wet, s.bits) - wet);

        if (s.dieOn > 0.0f)
        {
            if (isClipping != nullptr && std::abs(wet) > s.ceiling)
                *isClipping = true;

            wet += s.dieOn * (die(wet, s.ceiling) - wet);
        }

        return s.outputGain * (input + s.mix * (wet - input));
    }

    //==============================================================================
    // 🔄 노브를 오른쪽으로 돌릴수록 값이 '작아지는' 범위 (CRUSH: 16 → 1bit, DIE: 0 → -24dB)
    //    세 노브 모두 "오른쪽 = 더 부서짐" 으로 방향을 맞추기 위해 씁니다.
    inline juce::NormalisableRange<float> makeReversedRange(float leftValue, float rightValue)
    {
        jassert(leftValue > rightValue);

        return { rightValue, leftValue,
                 [](float start, float end, float position) { return end - position * (end - start); },
                 [](float start, float end, float value) { return (end - value) / (end - start); },
                 [](float start, float end, float value) { return juce::jlimit(start, end, value); } };
    }
}
