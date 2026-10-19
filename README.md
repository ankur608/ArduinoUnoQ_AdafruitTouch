# ArduinoUnoQ_AdafruitTouch

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: Arduino Uno Q](https://img.shields.io/badge/Board-Arduino%20UNO%20Q%20(STM32U585)-00979D.svg)](https://www.arduino.cc/)

A streamlined, open-source Arduino library for interfacing the **Adafruit 2.8" TFT Touch Shield v2 with Capacitive Touch (PID 1947)** on the **Arduino UNO Q** (`arduino_uno_q_stm32u585xx`).

Eliminates all the low-level headache of custom ICSP SPI pin remapping (`MOSI=23`, `MISO=22`, `SCK=24`), FT6206 I2C setup, and manual landscape rotation math into a clean, 1-line API!

---

## Installation

### Method 1: Arduino IDE (ZIP Installation)
1. On this GitHub repository page, click the green **Code** button and select **Download ZIP**.
2. Open **Arduino IDE**.
3. Go to **Sketch** $\rightarrow$ **Include Library** $\rightarrow$ **Add .ZIP Library...**
4. Select the downloaded `ArduinoUnoQ_AdafruitTouch-main.zip`.
5. Open the example from **File** $\rightarrow$ **Examples** $\rightarrow$ **ArduinoUnoQ_AdafruitTouch** $\rightarrow$ **TouchDemo**.

---

### Method 2: Git Clone (Arduino IDE / Local Libraries)
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

### Method 3: Arduino App Lab
1. In **Arduino App Lab**, navigate to your active Workspace or Project.
2. In the project folder, ensure the library is present in your sketch folder or imported via the App Lab Library Manager.
3. Include the header directly:
   ```cpp
   #include <ArduinoUnoQ_AdafruitTouch.h>
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

  // 1-line setup: initializes ILI9341 and FT6206 touch in Landscape mode
  screen.begin();

  screen.tft.fillScreen(ILI9341_BLACK);
  screen.tft.setTextColor(ILI9341_WHITE);
  screen.tft.setTextSize(2);
  screen.tft.setCursor(20, 20);
  screen.tft.println("Uno Q Touch Ready!");
}

void loop() {
  if (screen.touched()) {
    // getPoint() automatically maps coordinates to match the screen orientation!
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

1. **`TouchDemo`**:
   * Interactive on-screen buttons (`BUTTON A`, `BUTTON B`) that light up with color changes on touch.
   * Four corner targets (`TL`, `TR`, `BL`, `BR`) verifying 1:1 hardware coordinate calibration.
   * Live touch coordinate readout and touch cursor points.

2. **`TouchPaint`**:
   * Full finger-drawing canvas with continuous smooth line interpolation.
   * 8-color interactive palette bar (Red, Green, Blue, Yellow, Cyan, Magenta, White, Eraser).
   * One-touch "CLEAR" button.

3. **`UIControlsDemo`**:
   * Modern embedded dashboard with animated sliding toggle switch (ON / OFF).
   * Smooth touch slider ($0 - 100\%$) with live value readout.
   * Real-time dynamic color preview box that changes intensity as the slider is dragged.
   * Momentary tactile action button with pulse feedback.

4. **`PongGame` (Ping-Pong vs AI)**:
   * Retro arcade table tennis at fluid 60 FPS.
   * Player paddle directly tracks your finger vertically with 1:1 capacitive precision.
   * Dynamic AI opponent, multi-angle bounce physics, and increasing ball speeds.

5. **`Breakout` (Classic Brick Breaker)**:
   * 50 colorful bricks across 5 rows (Red, Orange, Yellow, Green, Cyan).
   * Smooth horizontal touch-paddle tracking, life counter, score multiplier, and victory state.

6. **`Tetris` (8-Bit Falling Block Arcade)**:
   * Authentic 10x20 matrix with all 7 official tetrominoes (I, O, T, S, Z, J, L).
   * 90-degree Portrait mode ($240 \times 320$) with Light theme, playfield grid lines, and 3D beveled blocks.
   * Start splash screen and ergonomic bottom touch controls.

7. **`EInkDashboard` (E-Paper Display Emulator & Uno Q System Specs)**:
   * Realistic monochrome E-Paper aesthetic with simulated black/white flash transitions.
   * Header with Wi-Fi, BLE, and battery status icons.
   * 3 interactive multi-page specification cards:
     - **Page 1:** Dual-Core Architecture (STM32U585 MCU @ 160MHz + Qualcomm QRB2210 MPU @ 2.0GHz, 4GB RAM, 16GB eMMC).
     - **Page 2:** Wireless & Networking (802.11ac Wi-Fi, BLE 5.0, high-speed UART router bridge).
     - **Page 3:** Adafruit 2.8" Shield Pinout, ICSP SPI remapping, and live peripheral telemetry.
   * Touch navigation bar with `PREV`, `REFRESH`, and `NEXT` buttons.

8. **`ComicReader` (Saitama One Punch Man E-Ink Comic in English)**:
   * Authentic Japanese manga layout with high graphics, panel gutters, speech bubbles, and sound effects.
   * Procedural screentone/halftone dithering and dynamic 36-ray explosive speed lines.
   * 3 Interactive Manga Pages (Tap screen to turn pages with simulated E-Ink flash refresh):
     - **Page 1 (Full 3-Panel Story):** Saitama's nonchalant face ("Hero for fun") $\rightarrow$ Dragon-level monster menace $\rightarrow$ forced-perspective Serious Punch!
     - **Page 2 (Serious Mode):** Dramatic full-screen close-up of Serious Saitama with intense cross-hatching, forehead veins, and Japanese `ゴ ゴ ゴ` SFX.
     - **Page 3 (Comedic Gag):** "Bargain Day at the Supermarket!" with sweat drops, shocked face, and City Z discount grocery flyer receipt.

---

## Hardware Pin Mapping (Arduino UNO Q to Adafruit Shield)

The Adafruit 2.8" Shield connects SPI solely through the 6-pin **ICSP header** (not pins 11–13). On the Arduino UNO Q's STM32U585 core, these are mapped to dedicated microcontroller pins:

| Shield Signal | Function | UNO Q Digital Pin | STM32U585 GPIO |
| :--- | :--- | :--- | :--- |
| **TFT CS** | Display Chip Select | `10` | Standard Digital Pin |
| **TFT DC** | Data / Command | `9` | Standard Digital Pin |
| **MOSI** | SPI Master Out | **`23`** | ICSP Header (`PC3`) |
| **MISO** | SPI Master In | **`22`** | ICSP Header (`PC2`) |
| **SCK** | SPI Clock | **`24`** | ICSP Header (`PD1`) |
| **TFT RST** | Reset | `-1` | Tied to hardware reset |
| **SDA** | FT6206 I2C Data | `SDA` | Hardware `Wire` (Address `0x38`) |
| **SCL** | FT6206 I2C Clock | `SCL` | Hardware `Wire` |

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

## Compatible Shields

| Shield Version | Product ID | Touch Chip | Status |
| :--- | :--- | :--- | :--- |
| **Capacitive Touch** | **Adafruit PID 1947** | **FT6206 (I2C)** | **Fully Supported** |
| Resistive Touch | Adafruit PID 1651 | STMPE610 (SPI) | Not supported (Uses SPI touch controller) |

---

## License

MIT License. Free to use in any personal, commercial, or open-source projects.
