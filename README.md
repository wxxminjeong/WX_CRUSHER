# ★ WX CRUSHER ★

![C++](https://img.shields.io/badge/C++-17-000000.svg?style=flat&logo=c%2B%2B&logoColor=white)
![JUCE](https://img.shields.io/badge/JUCE-FRAMEWORK-000000.svg?style=flat&logo=juce&logoColor=white)
![VST3](https://img.shields.io/badge/VST3-COMPATIBLE-000000.svg?style=flat)

> **"TOTAL SONIC ANNIHILATION."**

**WX CRUSHER** is a high-gain, digital distortion engine engineered for **Rage, Digicore, and Dark Trap** production.
Abandoning analog warmth for digital coldness, it forces any signal into a ruthlessly hard-clipped square wave.

Three stages. Full control. Zero mercy.

![Plugin UI](./screenshot.png)

https://github.com/user-attachments/assets/1ff514e6-a1e2-47ea-90b7-d61829fa6f78

<sub>▲ v0.1 (one-knob version) demo</sub>


## 🕸️ SIGNAL CHAIN

```
INPUT → I. DRIVE → II. CRUSH → III. DIE → MIX → OUTPUT
```

| STAGE | KNOB | RANGE | EFFECT |
| :--- | :--- | :--- | :--- |
| **I. DRIVE** | GAIN | **0 → +26 dB** (×1 → ×20) | Extreme input gain. |
| **II. CRUSH** | BIT DEPTH | **16 → 1 bit** | Bit-depth reduction. Heavy quantization noise. 16 bit = untouched. |
| **III. DIE** | CEILING | **0 → −24 dB** | Hard clip at the ceiling. Forces sine waves into square waves. |
| MIX | DRY / WET | 0 → 100 % | Blend the destroyed signal with the clean one. |
| OUTPUT | LEVEL | −24 → +12 dB | Final level. |

* Every stage has its own **ON / OFF** switch.
* Every stage knob works the same way: **turn right = more destruction.**
* Knob moves and ON / OFF switches are smoothed — no clicks, safe to automate.

## 📺 VISUAL FEEDBACK

* **WAVE** — Oscilloscope. Input (grey ghost) vs output (white). Bleeds **red** while DIE is clipping.
* **SPECTRUM** — Input vs output spectrum. Watch the harmonics pile up.
* **TRANSFER** — The input → output curve of your current settings. The red area is how far it is from clean; the red dots show where the incoming signal hits the curve right now.
* **IN / OUT** — Peak meters with peak hold.
* **LEDs** — DRIVE / CRUSH glow with their knob amount. DIE only lights up when it is *actually* clipping.
* **BLOOD RED** glow behind everything, following how hard the signal is being destroyed.

### **[ HANDLING ]**
* Double-click a knob to reset it. Click the number to type a value.
* Drag the corner to resize (75 % – 160 %). Size and view are saved with your project.

### **[ v0.1 SOUND ]**
The old one-knob version at 100 % = **DRIVE +26 dB → CRUSH 3 bit → DIE 0 dB → OUTPUT −1.9 dB**.

## 🦇 INSTALLATION

### USERS
1.  Grab the latest **`.vst3`** from [**Releases**](../../releases).
2.  Drop it into your VST3 directory:
    * `C:\Program Files\Common Files\VST3`
3.  Rescan DAW.

### DEVELOPERS
* **IDE:** Visual Studio 2026 / Xcode
* **Framework:** JUCE 8
* **Standard:** C++17

Open `wxCrusher.jucer` in Projucer → **Save and Open in IDE** → build.

| FILE | ROLE |
| :--- | :--- |
| `PluginProcessor` | Parameters, smoothing, audio processing |
| `WxDsp.h` | The three stage formulas (shared by the audio and the TRANSFER display) |
| `WxVisualTap.h` | Lock-free audio → screen data path |
| `PluginEditor` | Main layout, 60 fps update loop |
| `WxControls` | Stage module (title, ON / OFF, knob, LED) |
| `WxDisplays` | WAVE / SPECTRUM / TRANSFER / meters |
| `WxLookAndFeel` | Colours, fonts, knob and button drawing |

Fonts: [Anton](https://github.com/googlefonts/AntonFont) and [Space Mono](https://github.com/googlefonts/spacemono), embedded under the SIL Open Font License 1.1 (`Source/Fonts`).

---

### **★ ENGINEERED BY WXXMIN ★**
*No copyright intended. Just pure noise.*
