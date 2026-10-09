/*
  ==============================================================================
    WxVisualTap.h

    🔌 오디오 스레드 → 화면 스레드로 시각화 데이터를 넘기는 통로.
    오디오 스레드는 절대 기다리면 안 되므로 잠금(lock) 없는 FIFO 와 atomic 만 씁니다.
      - 파형/스펙트럼용 샘플 (입력 모노, 출력 모노)
      - 입력/출력 피크 레벨
      - DIE 스테이지에서 실제로 잘려나간 샘플 비율
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class WxVisualTap
{
public:
    static constexpr int capacity = 1 << 15;

    WxVisualTap()
        : inputSamples((size_t)capacity), outputSamples((size_t)capacity)
    {
    }

    //==============================================================================
    // [오디오 스레드] 샘플을 넣습니다. 화면이 닫혀 있어 꽉 차면 넘치는 만큼 그냥 버립니다.
    void pushSamples(const float* input, const float* output, int numSamples) noexcept
    {
        const auto scope = fifo.write(numSamples);

        copyIn(input, output, scope.startIndex1, scope.blockSize1, 0);
        copyIn(input, output, scope.startIndex2, scope.blockSize2, scope.blockSize1);
    }

    // [오디오 스레드] 블록 하나를 처리할 때마다 레벨 정보를 남깁니다.
    //   clipRatio  = DIE 에서 잘린 샘플 비율 (0 ~ 1)
    //   crushRatio = CRUSH 가 바꾼 양 / 들어간 소리 크기
    void pushLevels(float inputPeak, float outputPeak, float clipRatio, float crushRatio) noexcept
    {
        storeMax(inputPeakLevel, inputPeak);
        storeMax(outputPeakLevel, outputPeak);
        storeMax(clipAmount, clipRatio);
        storeMax(crushAmount, crushRatio);
        blockCount.fetch_add(1);
    }

    //==============================================================================
    // [화면 스레드] 쌓인 샘플을 꺼냅니다. 꺼낸 개수를 돌려줍니다.
    int pullSamples(float* input, float* output, int maxSamples) noexcept
    {
        const auto scope = fifo.read(maxSamples);

        copyOut(input, output, scope.startIndex1, scope.blockSize1, 0);
        copyOut(input, output, scope.startIndex2, scope.blockSize2, scope.blockSize1);

        return scope.blockSize1 + scope.blockSize2;
    }

    // [화면 스레드] 마지막으로 읽은 뒤의 최대값을 가져오고 0으로 비웁니다.
    float takeInputPeak() noexcept  { return inputPeakLevel.exchange(0.0f); }
    float takeOutputPeak() noexcept { return outputPeakLevel.exchange(0.0f); }
    float takeClipAmount() noexcept { return clipAmount.exchange(0.0f); }
    float takeCrushAmount() noexcept { return crushAmount.exchange(0.0f); }

    // [화면 스레드] 쌓여 있던 레벨 정보를 전부 버립니다. (창을 새로 열 때)
    void clearLevels() noexcept
    {
        takeInputPeak();
        takeOutputPeak();
        takeClipAmount();
        takeCrushAmount();
    }

    // [화면 스레드] 지금까지 처리된 블록 수 - 화면 프레임 사이에 새 오디오가 왔는지 확인용
    uint32_t getBlockCount() const noexcept { return blockCount.load(); }

    // 스펙트럼의 주파수 눈금을 그리기 위한 샘플레이트
    std::atomic<double> sampleRate { 44100.0 };

private:
    void copyIn(const float* input, const float* output, int start, int size, int offset) noexcept
    {
        if (size <= 0)
            return;

        std::copy(input + offset, input + offset + size, inputSamples.begin() + start);
        std::copy(output + offset, output + offset + size, outputSamples.begin() + start);
    }

    void copyOut(float* input, float* output, int start, int size, int offset) const noexcept
    {
        if (size <= 0)
            return;

        std::copy(inputSamples.begin() + start, inputSamples.begin() + start + size, input + offset);
        std::copy(outputSamples.begin() + start, outputSamples.begin() + start + size, output + offset);
    }

    static void storeMax(std::atomic<float>& target, float value) noexcept
    {
        auto current = target.load();
        while (value > current && ! target.compare_exchange_weak(current, value)) {}
    }

    juce::AbstractFifo fifo { capacity };
    std::vector<float> inputSamples, outputSamples;

    std::atomic<float> inputPeakLevel { 0.0f }, outputPeakLevel { 0.0f }, clipAmount { 0.0f }, crushAmount { 0.0f };
    std::atomic<uint32_t> blockCount { 0 };

    JUCE_DECLARE_NON_COPYABLE(WxVisualTap)
};
