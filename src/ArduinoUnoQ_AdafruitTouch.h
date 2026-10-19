#ifndef ARDUINO_UNO_Q_ADAFRUIT_TOUCH_H
#define ARDUINO_UNO_Q_ADAFRUIT_TOUCH_H

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>

// ==============================================================================
// ARDUINO UNO Q HARDWARE PIN MAPPING
// ==============================================================================
// The Adafruit 2.8" Shield taps SPI solely through the 6-pin ICSP header.
// On the Uno Q (STM32U585), these map to dedicated hardware pins:
#define UNOQ_TFT_DC    9
#define UNOQ_TFT_CS   10
#define UNOQ_TFT_MOSI 23  // Uno Q ICSP Header MOSI (STM32 PC3)
#define UNOQ_TFT_MISO 22  // Uno Q ICSP Header MISO (STM32 PC2)
#define UNOQ_TFT_CLK  24  // Uno Q ICSP Header SCK  (STM32 PD1)
#define UNOQ_TFT_RST  -1  // Tied to system reset

class UnoQ_AdafruitTouch {
public:
  Adafruit_ILI9341 tft;
  Adafruit_FT6206 touch;
  bool touchAvailable;
  uint8_t rotation;

  UnoQ_AdafruitTouch() 
    : tft(UNOQ_TFT_CS, UNOQ_TFT_DC, UNOQ_TFT_MOSI, UNOQ_TFT_CLK, UNOQ_TFT_RST, UNOQ_TFT_MISO),
      touch(),
      touchAvailable(false),
      rotation(1) {}

  /**
   * Initializes both the ILI9341 display and FT6206 capacitive touch digitizer.
   * @param touchSensitivity I2C sensitivity threshold (default: 40)
   * @return true if capacitive touch controller was detected on I2C (0x38), false otherwise.
   */
  bool begin(uint8_t touchSensitivity = 40) {
    tft.begin();
    setRotation(1); // Default landscape 320x240

    Wire.begin();
    touchAvailable = touch.begin(touchSensitivity);
    return touchAvailable;
  }

  /**
   * Sets display and touch coordinate rotation.
   * @param r 0 = Portrait, 1 = Landscape (default), 2 = Portrait Inverted, 3 = Landscape Inverted
   */
  void setRotation(uint8_t r) {
    rotation = r % 4;
    tft.setRotation(rotation);
  }

  /**
   * Returns true if one or more fingers are touching the screen.
   */
  bool touched() {
    if (!touchAvailable) return false;
    return touch.touched();
  }

  /**
   * Reads capacitive touch coordinates and automatically transforms them to
   * match the active display rotation (0, 1, 2, or 3).
   * @return TS_Point with x, y matched 1:1 to pixel coordinates.
   */
  TS_Point getPoint() {
    if (!touchAvailable) return TS_Point(0, 0, 0);
    TS_Point raw = touch.getPoint();
    TS_Point mapped = raw;

    switch (rotation) {
      case 0: // Portrait (240x320)
        mapped.x = raw.x;
        mapped.y = raw.y;
        break;
      case 1: // Landscape (320x240) - Standard for Uno Q
        mapped.x = 320 - raw.y;
        mapped.y = raw.x;
        break;
      case 2: // Inverted Portrait (240x320)
        mapped.x = 240 - raw.x;
        mapped.y = 320 - raw.y;
        break;
      case 3: // Inverted Landscape (320x240)
        mapped.x = raw.y;
        mapped.y = 240 - raw.x;
        break;
    }
    return mapped;
  }

  int16_t width()  { return tft.width(); }
  int16_t height() { return tft.height(); }
};

#endif // ARDUINO_UNO_Q_ADAFRUIT_TOUCH_H
