# ArduinoUnoQ_AdafruitTouch

[![Arduino Library Manager](https://img.shields.io/badge/Arduino%20Library%20Manager-ArduinoUnoQ__AdafruitTouch-00979D.svg)](https://www.arduino.cc/reference/en/libraries/)
[![Release](https://img.shields.io/badge/Release-v1.0.0-blue.svg)](https://github.com/ankur608/ArduinoUnoQ_AdafruitTouch/releases)
[![Board: Arduino UNO Q](https://img.shields.io/badge/Board-Arduino%20UNO%20Q%20(STM32U585)-teal.svg)](https://www.arduino.cc/)
[![Shield: Adafruit 2.8" Capacitive TFT](https://img.shields.io/badge/Shield-Adafruit%20PID%201947-orange.svg)](https://www.adafruit.com/product/1947)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A streamlined, open-source Arduino library for interfacing the **Adafruit 2.8" TFT Touch Shield v2 with Capacitive Touch (PID 1947)** on the **Arduino UNO Q** (`arduino_uno_q_stm32u585xx`).

Eliminates all the low-level headache of custom ICSP SPI pin remapping (`MOSI=23`, `MISO=22`, `SCK=24`), FT6206 I2C setup, and manual orientation coordinate math into a clean, intuitive 1-line API!

---

## Features

* 🚀 **Zero-Config Hardware Setup:** Automatically maps dedicated STM32U585 SPI ICSP pins and initializes both the ILI9341 display and FT6206 capacitive controller with a single `screen.begin()`.
* 🔄 **Automated Coordinate Calibration:** Computes the coordinate transformation matrix for all 4 display rotations (`0`, `1`, `2`, `3`). `screen.getPoint()` always returns exact 1:1 screen pixel coordinates.
* 🕹️ **Complete Retro Arcade Suite:** Includes full playable implementations of **Pong** (60 FPS vs AI), **Breakout** (50-brick arcade), and **Tetris** (revised portrait light-mode with 3D beveled blocks and retro start splash).
* 📖 **Authentic Yusuke Murata Manga Reader:** Includes **ComicReader**, rendering 7 high-definition $240 \times 320$ Floyd-Steinberg dithered panels of *One-Punch Man* with simulated E-Paper bi-stable refresh.
* 📊 **E-Ink Specs Dashboard:** Multi-page e-paper emulator showcasing live Uno Q dual-core telemetry (Qualcomm QRB2210 MPU + STM32U585 MCU, 802.11ac Wi-Fi, BLE 5.0).
* 🎛️ **Modern Touch UI Controls:** Smooth capacitive sliders, toggle switches, finger-drawing canvas with color palettes, and interactive calibration targets.

---

## Installation

### Method 1: Arduino Library Manager (Recommended)
1. Open **Arduino IDE**.
2. Go to **Tools** $\rightarrow$ **Manage Libraries...** (or press `Ctrl + Shift + I`).
3. Search for **`ArduinoUnoQ_AdafruitTouch`** and click **Install**.

---

### Method 2: Arduino IDE (ZIP Installation)
1. On this GitHub repository page, click the green **Code** button and select **Download ZIP**.
2. Open **Arduino IDE**.
3. Go to **Sketch** $\rightarrow$ **Include Library** $\rightarrow$ **Add .ZIP Library...**
4. Select the downloaded `ArduinoUnoQ_AdafruitTouch-main.zip`.
5. Open any example from **File** $\rightarrow$ **Examples** $\rightarrow$ **ArduinoUnoQ_AdafruitTouch**.

---

### Method 3: Git Clone (Local Libraries)
Clone directly into your Arduino libraries directory:

```bash
# Windows
cd %USERPROFILE%\Documents\Arduino\libraries
git clone https://github.com/ankur608/ArduinoUnoQ_AdafruitTouch.git

# Linux / macOS
cd ~/Arduino/libraries
git clone https://github.com/ankur608/ArduinoUnoQ_AdafruitTouch.git
```

---

### Method 4: `arduino-cli`
Install directly using the Git repository URL:

```bash
arduino-cli lib install --git-url https://github.com/ankur608/ArduinoUnoQ_AdafruitTouch.git
```

---

## Required Dependencies

Install the core Adafruit driver dependencies:

```bash
arduino-cli lib install "Adafruit GFX Library"
arduino-cli lib install "Adafruit ILI9341"
arduino-cli lib install "Adafruit FT6206 Library"
```
*(Or search for them in the Arduino IDE Library Manager).*

---

## Quick Example (How Simple It Is)

```cpp
#include <ArduinoUnoQ_AdafruitTouch.h>

UnoQ_AdafruitTouch screen;

void setup() {
  Serial.begin(115200);

  // 1-line setup: initializes ILI9341 display and FT6206 touch in Landscape mode
  screen.begin();

  screen.tft.fillScreen(ILI9341_BLACK);
  screen.tft.setTextColor(ILI9341_WHITE);
  screen.tft.setTextSize(2);
  screen.tft.setCursor(20, 20);
  screen.tft.println("Uno Q Touch Ready!");
}

void loop() {
  if (screen.touched()) {
    // getPoint() automatically maps coordinates to match active screen orientation!
    TS_Point p = screen.getPoint();

    Serial.print("Touch -> X: ");
    Serial.print(p.x);
    Serial.print(" | Y: ");
    Serial.println(p.y);

    screen.tft.fillCircle(p.x, p.y, 4, ILI9341_CYAN);
  }
}
```

---

## Interactive Examples Included

Once installed, navigate in Arduino IDE to **File** $\rightarrow$ **Examples** $\rightarrow$ **ArduinoUnoQ_AdafruitTouch**:

| Example | Description | Orientation | Highlights |
| :--- | :--- | :---: | :--- |
| **`ComicReader`** | Authentic Yusuke Murata One-Punch Man Manga Issue | Portrait ($240 \times 320$) | 7 Full-screen dithered manga panels, fast SPI streaming, E-Paper inversion flash |
| **`Tetris`** | 8-Bit Falling Block Arcade | Portrait ($240 \times 320$) | Light theme, grid matrix, 3D beveled blocks, start splash screen, compact thumb controls |
| **`PongGame`** | Real-Time Table Tennis vs AI | Landscape ($320 \times 240$) | Fluid 60 FPS, 1:1 finger paddle tracking, dynamic AI opponent, scorekeeper |
| **`Breakout`** | Arcade Brick Breaker | Landscape ($320 \times 240$) | 50 colorful bricks, ball physics, paddle deflection angles, victory celebration |
| **`EInkDashboard`** | E-Paper Spec Telemetry | Portrait ($240 \times 320$) | Dual-core Uno Q specs (QRB2210 MPU + STM32U585 MCU), Wi-Fi/BLE telemetry |
| **`UIControlsDemo`** | Embedded Touch Widgets | Landscape ($320 \times 240$) | Animated sliding toggle switch, 0-100% slider with dynamic color preview box |
| **`TouchPaint`** | Finger Drawing Canvas | Landscape ($320 \times 240$) | 8-color interactive palette, smooth line interpolation, one-touch clear canvas |
| **`TouchDemo`** | Touch Calibration & Buttons | Landscape ($320 \times 240$) | Tactile on-screen buttons, 4 corner calibration targets, coordinate readout |

---

## Detailed Example Breakdown

### 1. `ComicReader` (Authentic Yusuke Murata One-Punch Man Manga Reader)
100% Authentic original Yusuke Murata manga artwork directly converted into high-definition 1-bit Floyd-Steinberg dithered E-Ink panels ($240 \times 320$):
* **Page 1 (Cover):** Tankōbon Volume 1 Cover — Saitama putting on his red hero glove with cape and belt.
* **Page 2 (The Moon Jump):** Chapter 35 — Kicked to outer space, standing on the moon's surface looking back at Earth.
* **Page 3 (Serious Series):** Chapter 36 — Intense close-up: *"Killer Move: Serious Series"*.
* **Page 4 (Serious Punch!):** Chapter 36 — Climax: Planetary atmospheric shockwave splitting the globe.
* **Page 5 (Death Punch):** Chapter 17 — Sparring match: The colossal forced-perspective *"DEATH"* punch stopped 1 inch from Genos's face.
* **Page 6 (Supermarket Bargain Gag):** Chapter 10 — Comic relief: Saitama shrieking *"Today is bargain day at the supermarket!!"*
* **Page 7 (The Legend: "OK."):** Chapter 36 — The world-famous nonchalant *"OK."* deadpan expression.
* **Navigation:** Tap right/center to advance; tap left edge to go back. Features authentic bi-stable E-Paper black/white inversion flash.

### 2. `Tetris` (Portrait Light-Mode Falling Block Arcade)
* Standard 10x20 playfield rendered on a clean paper-white background with subtle coordinate grid lines.
* Full 7-tetromino randomizer (I, O, T, S, Z, J, L) with ghost piece drop projection.
* 3D beveled block shaders for authentic arcade aesthetics.
* Retro 8-bit start splash screen with high-score tracking.
* Compact, ergonomic bottom thumb controls (`LEFT`, `RIGHT`, `ROTATE`, `DROP`) leaving the active playfield 100% unobstructed.

### 3. `EInkDashboard` (E-Paper Uno Q Specs & Wireless Telemetry)
* Simulated electronic ink display with dual-core architectural specifications:
  * **Core 1:** STM32U585 ARM Cortex-M33 MCU @ 160MHz (2MB Flash, 786KB SRAM).
  * **Core 2:** Qualcomm QRB2210 Quad-Core Kryo MPU @ 2.0GHz (4GB LPDDR4, 16GB eMMC).
  * **Connectivity:** Dual-band 802.11ac Wi-Fi, Bluetooth 5.0 BLE, high-speed UART router bridge.
  * **Pinout & Telemetry:** Dedicated ICSP SPI mapping table and live peripheral status.

---

## Hardware Pin Mapping (Arduino UNO Q to Adafruit Shield)

The Adafruit 2.8" Shield connects SPI solely through the 6-pin **ICSP header** (not pins 11–13). On the Arduino UNO Q's STM32U585 core, these are mapped to dedicated microcontroller pins:

| Shield Signal | Function | UNO Q Digital Pin | STM32U585 GPIO | Hardware Route |
| :--- | :--- | :---: | :---: | :--- |
| **TFT CS** | Display Chip Select | `10` | Standard Digital Pin | Dedicated Chip Select |
| **TFT DC** | Data / Command | `9` | Standard Digital Pin | Display Command Line |
| **MOSI** | SPI Master Out | **`23`** | ICSP Header (`PC3`) | SPI Primary Bus |
| **MISO** | SPI Master In | **`22`** | ICSP Header (`PC2`) | SPI Primary Bus |
| **SCK** | SPI Clock | **`24`** | ICSP Header (`PD1`) | SPI Primary Bus |
| **TFT RST** | Reset | `-1` | Tied to hardware reset | Hardware Reset Line |
| **SDA** | FT6206 I2C Data | `SDA` | Hardware `Wire` (Address `0x38`) | Capacitive Touch Bus |
| **SCL** | FT6206 I2C Clock | `SCL` | Hardware `Wire` | Capacitive Touch Bus |

---

## Automatic Coordinate Calibration

The FT6206 capacitive digitizer natively operates in Portrait orientation ($240 \times 320$). This library's `screen.getPoint()` function automatically computes the transformation matrix for whatever rotation you select:

```cpp
screen.setRotation(0); // Portrait (240x320)
screen.setRotation(1); // Landscape (320x240) - Default for Uno Q
screen.setRotation(2); // Inverted Portrait (240x320)
screen.setRotation(3); // Inverted Landscape (320x240)
```

---

## Compatible Hardware

| Shield Version | Product ID | Touch Chip | Status |
| :--- | :--- | :--- | :--- |
| **Capacitive Touch** | **Adafruit PID 1947** | **FT6206 (I2C)** | **Fully Supported** |
| Resistive Touch | Adafruit PID 1651 | STMPE610 (SPI) | Not supported (Uses SPI touch controller) |

---

## Author & Maintainer

* **Author:** `ankur608`
* **Maintainer:** `ankur608 <ankur608@users.noreply.github.com>`
* **Repository:** [https://github.com/ankur608/ArduinoUnoQ_AdafruitTouch](https://github.com/ankur608/ArduinoUnoQ_AdafruitTouch)

---

## License

This project is licensed under the **MIT License**. Free to use in personal, academic, commercial, and open-source embedded applications.
