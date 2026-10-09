# WX CRUSHER

A digital distortion plugin (VST3) with three stages you can control separately: **Drive**, **Bitcrush** and **Clipper**.
Built with JUCE.

![WX CRUSHER](./screenshot.png)

## Signal chain

```
Input → Drive → Bitcrush → Clipper → Mix → Output
```

## Controls

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

## Display

- **WAVE** – Oscilloscope. Grey is the input, white is the output. The output turns red while the clipper is clipping.
- **SPECTRUM** – Frequency spectrum of the input (grey) and output (white). Shows the harmonics the distortion adds.
- **TRANSFER** – How the current settings turn input level into output level. The dashed diagonal is the unprocessed signal and the red area is the difference. Red dots show the current input level. With high Drive the input axis zooms in; the zoom is shown in the top-right corner.
- **IN / OUT** – Peak meters with peak hold.
- **LEDs** under each stage light up while that stage is actually changing the signal.

## Usage

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
- The sound of v0.1 (the old one-knob version at 100 %): Drive +26 dB, Bitcrush 3 bit, Clipper 0 dB, Output −1.9 dB. [v0.1 demo video](https://github.com/user-attachments/assets/1ff514e6-a1e2-47ea-90b7-d61829fa6f78)

## Installation

1. Download `wxCrusher.vst3` from [Releases](../../releases).
2. Copy it to `C:\Program Files\Common Files\VST3`.
3. Rescan plugins in your DAW.

## Building from source

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

## Credits

Made by wxxmin.

Fonts: [Anton](https://github.com/googlefonts/AntonFont) and [Space Mono](https://github.com/googlefonts/spacemono), included under the SIL Open Font License 1.1 (`Source/Fonts`).
