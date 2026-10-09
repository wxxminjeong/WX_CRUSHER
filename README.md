# ★ WX CRUSHER ★

![C++](https://img.shields.io/badge/C++-17-000000.svg?style=flat&logo=c%2B%2B&logoColor=white)
![JUCE](https://img.shields.io/badge/JUCE-FRAMEWORK-000000.svg?style=flat&logo=juce&logoColor=white)
![VST3](https://img.shields.io/badge/VST3-COMPATIBLE-000000.svg?style=flat)

> **"TOTAL SONIC ANNIHILATION."**

**WX CRUSHER** is a suite of digital distortion plugins engineered for **Rage, Digicore, and Dark Trap** production.
Abandoning analog warmth for digital coldness, it forces any signal into a ruthlessly hard-clipped square wave.

Every stage of the old one-knob engine is now **its own VST3**.
Chain them, reorder them, automate them separately.

Three plugins. Zero mercy.

https://github.com/user-attachments/assets/1ff514e6-a1e2-47ea-90b7-d61829fa6f78

<sub>▲ v0.1 (one-knob version) demo</sub>


## 🕸️ THE SUITE

| PLUGIN | KNOB | RANGE | EFFECT |
| :--- | :--- | :--- | :--- |
| **WX DRIVE** | DRIVE | **0 → +26 dB** (×1 → ×20) | Extreme gain. No clipping inside — that's DIE's job. |
| **WX CRUSH** | BITS | **16 → 1 bit** | Bit-depth reduction. Heavy quantization noise. 16 bit = untouched. |
| **WX DIE** | CEILING | **0 → -24 dB** | Hard clip at the ceiling. Forces sine waves into square waves. |

Every knob works the same way: **turn right = more destruction.**

### **[ VISUAL FEEDBACK ]**
* **⚪ LED:** Lights up the moment the knob leaves its clean position
* **🔴 GLOW:** Blood red background past **80%**

### **[ AESTHETIC ]**
* **PITCH BLACK** Background
* **STARK WHITE** Controls
* **BLOOD RED** Visuals on max capacity

## 🏴‍☠️ THE ORIGINAL CHAIN

The v0.1 one-knob engine, rebuilt from the separate plugins:

```
WX DRIVE  →  WX CRUSH  →  WX DIE
 +26 dB       3 bit        0 dB
```

| STAGE | PLUGIN | PROCESS |
| :--- | :--- | :--- |
| **I** | **WX DRIVE** | Input signal amplified by up to **20x (+26dB)**. |
| **II** | **WX CRUSH** | Bit-depth reduced down to **1 bit**. Introduces heavy quantization noise. |
| **III** | **WX DIE** | Signal is aggressively clamped at the **ceiling**. |

> v0.1 also trimmed the output by -1.9 dB (×0.8) after clipping. Pull the fader down by 1.9 dB to match it exactly.

## 🦇 INSTALLATION

### USERS
1.  Grab **`wxDrive.vst3`**, **`wxCrush.vst3`**, **`wxDie.vst3`** from [**Releases**](../../releases).
2.  Drop them into your VST3 directory:
    * `C:\Program Files\Common Files\VST3`
3.  Rescan DAW. They show up under **wxxmin**.

### DEVELOPERS
* **IDE:** Visual Studio 2026 / Xcode
* **Framework:** JUCE 8
* **Standard:** C++17

```
WxDrive/wxDrive.jucer   → WX DRIVE
WxCrush/wxCrush.jucer   → WX CRUSH
WxDie/wxDie.jucer       → WX DIE
Shared/                 → processor base + UI shared by all three
```

Open each `.jucer` in Projucer → **Save and Open in IDE** → build.
Each project has its own `Builds/` folder, so all three can be built side by side.

---

### **★ ENGINEERED BY WXXMIN ★**
*No copyright intended. Just pure noise.*
