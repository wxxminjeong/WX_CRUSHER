# WX CRUSHER 😈

![C++](https://img.shields.io/badge/C++-17-blue.svg?style=flat&logo=c%2B%2B)
![JUCE](https://img.shields.io/badge/Framework-JUCE%207-green.svg?style=flat&logo=juce)
![VST3](https://img.shields.io/badge/Format-VST3-orange.svg?style=flat)

> **"Make it Gritty. Make it Rage."**

**WX CRUSHER** is an aggressive **One-Knob Distortion VST plugin** designed specifically for **Digicore, Rage, and Hyperpop** genres. Inspired by the raw, crushed sounds of the *Opium* label aesthetic, it turns simple waveforms into destructive square waves with a single turn.

![Plugin UI](./screenshot.png)
*(Please add your screenshot image file here)*

## 🔥 Key Features

* **Extreme One-Knob Control:** Simultaneously controls Input Drive, Bit Reduction, and Hard Clipping.
* **3-Stage LED Feedback System:**
    * 🔴 **DRIVE:** Input gain boost initiates.
    * 🔴 **CRUSH:** Bit-depth reduction kicks in (down to 3-bit).
    * 🩸 **DIE:** Hard clipping + Full visual rage mode (Red Glow).
* **Opium Aesthetic UI:** Minimalist "Pitch Black" design with high-contrast visibility.
* **Auto-Gain Compensation:** Prevents ear damage by automatically balancing output volume.

## 🎛️ Under the Hood (DSP Logic)

The plugin processes audio through a rigorous 3-step chain based on the knob value (0% - 100%):

1.  **Extreme Drive (0% ~):**
    * Input signal is boosted up to **20x (approx +26dB)**.
2.  **Digital Decimation (40% ~):**
    * Applies aggressive **Bit-crushing**.
    * Resolution drops linearly from **16-bit down to 3-bit**, creating intense quantization noise.
3.  **Hard Clipping (80% ~):**
    * Signal is ruthlessly clamped between `-1.0` and `1.0`.
    * Transforms sine waves into near-perfect **Square waves** for that signature "torn speaker" sound.

## 🚀 Installation & Usage

### For Users
1.  Download the `.vst3` file from the [Releases](../../releases) page.
2.  Place the file in your VST3 folder:
    * **Windows:** `C:\Program Files\Common Files\VST3`
3.  Rescan plugins in your DAW (FL Studio, Ableton, etc.).

### For Developers (Build)
1.  Clone this repository.
2.  Open `wxCrusher.jucer` with **Projucer**.
3.  Select your exporter (Visual Studio 2022/2026 or Xcode).
4.  Save and open in IDE.
5.  Build in **Release** mode.

## 🛠 Tech Stack

* **Language:** C++
* **Framework:** JUCE
* **IDE:** Visual Studio 2026
* **Platform:** Windows (VST3)

---
**Developed by wxxmin**
