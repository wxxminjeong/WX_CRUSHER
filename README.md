# WX CRUSHER

![WX CRUSHER](./screenshot.png)

[English](#english) · [한국어](#한국어)

## English

A digital distortion plugin (VST3) with three stages you can control separately: **Drive**, **Bitcrush** and **Clipper**.
Built with JUCE.

### Signal chain

```
Input → Drive → Bitcrush → Clipper → Mix → Output
```

### Controls

| Section | Knob | Range | What it does |
| :--- | :--- | :--- | :--- |
| Drive | Gain | 0 to +26 dB | Boosts the input level. The more gain, the harder the signal hits the clipper. |
| Bitcrush | Bit depth | 16 to 1 bit | Lowers the bit resolution and adds stepped, gritty noise. 16 bit leaves the sound unchanged. |
| Clipper | Ceiling | 0 to −24 dB | Cuts off everything above the ceiling (hard clipping), turning peaks into flat tops. A lower ceiling clips more and makes the output quieter. |
| Mix | Dry / wet | 0 to 100 % | Blends the original signal with the processed one. |
| Output | Level | −24 to +12 dB | Final output level. |

- Drive, Bitcrush and Clipper each have an **ON / OFF** button.
- On the three stage knobs, turning right always means more effect. For Bitcrush and Clipper the number goes *down* as you turn right.
- Knob changes and ON / OFF are smoothed, so automation and switching don't click.

### Display

- **WAVE** – Oscilloscope. Grey is the input, white is the output. The output turns red while the clipper is clipping.
- **SPECTRUM** – Frequency spectrum of the input (grey) and output (white). Shows the harmonics the distortion adds.
- **TRANSFER** – How the current settings turn input level into output level. The dashed diagonal is the unprocessed signal and the red area is the difference. Red dots show the current input level. With high Drive the input axis zooms in; the zoom is shown in the top-right corner.
- **IN / OUT** – Peak meters with peak hold.
- **LEDs** under each stage light up while that stage is actually changing the signal.

### Usage

1. Insert WX CRUSHER on a track.
2. Turn up **Drive**. With the Clipper on at 0 dB, the loud parts start to get clipped flat.
3. Turn **Bitcrush** to the right for grit.
4. Lower the **Clipper** ceiling for more clipping, or turn it off to hear Drive and Bitcrush alone.
5. Use **Mix** to bring back some of the clean signal and **Output** to match the level.

Tips:

- Double-click a knob to reset it. Click the value under a knob to type a number.
- Drag the bottom-right corner to resize the window (75 % to 160 %).
- Knob values, the selected display and the window size are saved with your DAW project.
- With the Clipper off and Drive up, the output can be up to 26 dB louder than the input. Turn down Output.
- The sound of v0.1 (the old one-knob version at 100 %): Drive +26 dB, Bitcrush 3 bit, Clipper 0 dB, Output −1.9 dB.

### Installation

1. Download `wxCrusher.vst3` from [Releases](../../releases).
2. Copy it to `C:\Program Files\Common Files\VST3`.
3. Rescan plugins in your DAW.

### Building from source

Requirements: JUCE 8 (Projucer), Visual Studio 2026 or Xcode, C++17.

1. Open `wxCrusher.jucer` in Projucer.
2. If Projucer can't find the JUCE modules, set the module path in the exporter settings to your JUCE `modules` folder.
3. Click **Save and Open in IDE** and build the Release configuration.
   With Visual Studio the plugin ends up in `Builds/VisualStudio2026/x64/Release/VST3/`.

| File | Contents |
| :--- | :--- |
| `Source/PluginProcessor` | Parameters, smoothing, audio processing |
| `Source/WxDsp.h` | Drive / Bitcrush / Clipper formulas (also used to draw TRANSFER) |
| `Source/WxVisualTap.h` | Passes audio data to the display without locking the audio thread |
| `Source/PluginEditor` | Window layout and the 60 fps display update |
| `Source/WxControls` | Stage panel (title, ON / OFF, knob, LED) |
| `Source/WxDisplays` | WAVE, SPECTRUM, TRANSFER and the meters |
| `Source/WxLookAndFeel` | Colours, fonts, knob and button drawing |

### Credits

Made by wxxmin.

Fonts: [Anton](https://github.com/googlefonts/AntonFont) and [Space Mono](https://github.com/googlefonts/spacemono), included under the SIL Open Font License 1.1 (`Source/Fonts`).

---

## 한국어

**Drive**, **Bitcrush**, **Clipper** 세 단계를 각각 따로 조절할 수 있는 디지털 디스토션 플러그인(VST3)입니다.
JUCE로 만들었습니다.

### 신호 흐름

```
Input → Drive → Bitcrush → Clipper → Mix → Output
```

### 노브

| 섹션 | 노브 | 범위 | 하는 일 |
| :--- | :--- | :--- | :--- |
| Drive | Gain | 0 ~ +26 dB | 입력 소리를 키웁니다. 많이 키울수록 Clipper에서 더 세게 잘립니다. |
| Bitcrush | Bit depth | 16 ~ 1 bit | 비트 해상도를 낮춰 계단 모양의 거친 노이즈를 만듭니다. 16 bit면 소리가 그대로입니다. |
| Clipper | Ceiling | 0 ~ −24 dB | 천장(Ceiling)을 넘는 부분을 잘라내(하드 클리핑) 파형의 꼭대기를 평평하게 만듭니다. 천장이 낮을수록 더 많이 잘리고 출력은 작아집니다. |
| Mix | Dry / wet | 0 ~ 100 % | 원래 소리와 처리된 소리를 섞습니다. |
| Output | Level | −24 ~ +12 dB | 최종 출력 크기입니다. |

- Drive, Bitcrush, Clipper에는 각각 **ON / OFF** 버튼이 있습니다.
- 세 스테이지 노브는 모두 오른쪽으로 돌릴수록 효과가 세집니다. Bitcrush와 Clipper는 오른쪽으로 돌릴수록 숫자가 *작아집니다*.
- 노브 변화와 ON / OFF 전환은 부드럽게 처리되기 때문에, 오토메이션을 쓰거나 스테이지를 켜고 끌 때 '틱' 소리가 나지 않습니다.

### 화면

- **WAVE** – 오실로스코프(파형). 회색이 입력, 흰색이 출력입니다. Clipper가 소리를 자르는 동안 출력 파형이 빨갛게 변합니다.
- **SPECTRUM** – 입력(회색)과 출력(흰색)의 주파수 스펙트럼입니다. 디스토션이 더한 배음이 보입니다.
- **TRANSFER** – 지금 설정이 입력 크기를 출력 크기로 어떻게 바꾸는지 보여주는 그래프입니다. 점선 대각선은 처리하지 않은 소리, 빨간 영역은 그 차이입니다. 빨간 점은 지금 들어오는 소리의 크기입니다. Drive가 높으면 입력 축이 확대되고, 확대 배율은 오른쪽 위에 표시됩니다.
- **IN / OUT** – 피크 홀드가 있는 피크 미터입니다.
- 각 스테이지 아래의 **LED**는 그 스테이지가 실제로 소리를 바꾸고 있을 때 켜집니다.

### 사용법

1. 트랙에 WX CRUSHER를 겁니다.
2. **Drive**를 올립니다. Clipper가 켜져 있고 0 dB이면 큰 소리부터 평평하게 잘리기 시작합니다.
3. 거친 질감이 필요하면 **Bitcrush**를 오른쪽으로 돌립니다.
4. 더 세게 자르려면 **Clipper** 천장을 낮추고, Drive와 Bitcrush만 들어보려면 Clipper를 끕니다.
5. **Mix**로 원래 소리를 섞고 **Output**으로 크기를 맞춥니다.

팁:

- 노브를 더블클릭하면 기본값으로 돌아갑니다. 노브 아래 숫자를 클릭하면 값을 직접 입력할 수 있습니다.
- 오른쪽 아래 모서리를 드래그하면 창 크기를 바꿀 수 있습니다 (75 % ~ 160 %).
- 노브 값, 선택한 화면, 창 크기는 DAW 프로젝트에 같이 저장됩니다.
- Clipper를 끄고 Drive를 올리면 출력이 입력보다 최대 26 dB 커질 수 있습니다. Output을 내려 주세요.
- v0.1(예전 원노브 버전을 100 %로 돌린 상태)과 같은 소리: Drive +26 dB, Bitcrush 3 bit, Clipper 0 dB, Output −1.9 dB.

### 설치

1. [Releases](../../releases)에서 `wxCrusher.vst3`를 받습니다.
2. `C:\Program Files\Common Files\VST3`에 복사합니다.
3. DAW에서 플러그인을 다시 스캔합니다.

### 직접 빌드하기

필요한 것: JUCE 8 (Projucer), Visual Studio 2026 또는 Xcode, C++17.

1. Projucer에서 `wxCrusher.jucer`를 엽니다.
2. Projucer가 JUCE 모듈을 찾지 못하면 exporter 설정에서 모듈 경로를 내 JUCE의 `modules` 폴더로 지정합니다.
3. **Save and Open in IDE**를 누르고 Release 설정으로 빌드합니다.
   Visual Studio에서는 `Builds/VisualStudio2026/x64/Release/VST3/`에 플러그인이 만들어집니다.

| 파일 | 내용 |
| :--- | :--- |
| `Source/PluginProcessor` | 파라미터, 스무딩, 오디오 처리 |
| `Source/WxDsp.h` | Drive / Bitcrush / Clipper 수식 (TRANSFER 그래프도 같은 수식 사용) |
| `Source/WxVisualTap.h` | 오디오 스레드를 멈추지 않고 화면으로 오디오 데이터를 넘김 |
| `Source/PluginEditor` | 창 배치, 초당 60번 화면 갱신 |
| `Source/WxControls` | 스테이지 패널 (이름, ON / OFF, 노브, LED) |
| `Source/WxDisplays` | WAVE, SPECTRUM, TRANSFER, 미터 |
| `Source/WxLookAndFeel` | 색, 폰트, 노브와 버튼 그리기 |

### 크레딧

wxxmin이 만들었습니다.

폰트: [Anton](https://github.com/googlefonts/AntonFont), [Space Mono](https://github.com/googlefonts/spacemono) — SIL Open Font License 1.1 (`Source/Fonts`).
