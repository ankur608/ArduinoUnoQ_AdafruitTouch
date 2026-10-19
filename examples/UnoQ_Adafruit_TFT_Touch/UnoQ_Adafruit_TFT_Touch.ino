/*
  UnoQ_Adafruit_TFT_Touch.ino
  
  Demonstration & Boilerplate for interfacing the Adafruit 2.8" TFT Shield 
  with Capacitive Touch (PID 1947) on the Arduino UNO Q (STM32U585).

  Features:
  - ILI9341 SPI Display initialization on Uno Q ICSP header pins
  - FT6206 Capacitive Touch over hardware I2C (address 0x38)
  - Accurate landscape rotation mapping (setRotation(1))
  - Interactive touch test UI with responsive on-screen buttons
  
  Board: Arduino UNO Q (arduino:zephyr:unoq)
  Shield: Adafruit 2.8" TFT Touch Shield v2 - Capacitive (PID 1947)
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>

// ==============================================================================
// ARDUINO UNO Q PIN MAPPING (ICSP Header to Adafruit Shield)
// ==============================================================================
#define TFT_DC    9
#define TFT_CS   10
#define TFT_MOSI 23  // Uno Q ICSP Header MOSI (STM32 PC3)
#define TFT_MISO 22  // Uno Q ICSP Header MISO (STM32 PC2)
#define TFT_CLK  24  // Uno Q ICSP Header SCK  (STM32 PD1)
#define TFT_RST  -1  // Tied to board reset

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
Adafruit_FT6206 ctp = Adafruit_FT6206();

#define SCREEN_W 320
#define SCREEN_H 240

bool touchOnline = false;

// Button Coordinates
#define BTN_Y 120
#define BTN_W 120
#define BTN_H 70
#define BTN_A_X 25
#define BTN_B_X 175

void drawButton(int x, int y, int w, int h, const char* label, bool pressed, uint16_t activeColor) {
  uint16_t fill = pressed ? activeColor : 0x18E3;
  tft.fillRoundRect(x, y, w, h, 8, fill);
  tft.drawRoundRect(x, y, w, h, 8, ILI9341_WHITE);

  tft.setTextColor(pressed ? ILI9341_BLACK : ILI9341_WHITE);
  tft.setTextSize(2);
  
  // Center label roughly
  int textX = x + (w / 2) - (strlen(label) * 6);
  int textY = y + (h / 2) - 8;
  tft.setCursor(textX, textY);
  tft.print(label);
}

void drawUI(bool btnAPressed, bool btnBPressed) {
  // Corner Test Targets for Coordinate Alignment
  tft.drawRect(0, 0, 30, 30, ILI9341_YELLOW);
  tft.drawRect(SCREEN_W - 30, 0, 30, 30, ILI9341_YELLOW);
  tft.drawRect(0, SCREEN_H - 30, 30, 30, ILI9341_YELLOW);
  tft.drawRect(SCREEN_W - 30, SCREEN_H - 30, 30, 30, ILI9341_YELLOW);

  tft.setTextSize(1);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(4, 10);              tft.print("TL");
  tft.setCursor(SCREEN_W - 24, 10);  tft.print("TR");
  tft.setCursor(4, SCREEN_H - 20);   tft.print("BL");
  tft.setCursor(SCREEN_W - 24, SCREEN_H - 20); tft.print("BR");

  // Interactive buttons
  drawButton(BTN_A_X, BTN_Y, BTN_W, BTN_H, btnAPressed ? "PRESSED!" : "BUTTON A", btnAPressed, ILI9341_CYAN);
  drawButton(BTN_B_X, BTN_Y, BTN_W, BTN_H, btnBPressed ? "PRESSED!" : "BUTTON B", btnBPressed, ILI9341_GREEN);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize ILI9341 Display
  tft.begin();
  tft.setRotation(1); // Landscape 320x240
  tft.fillScreen(ILI9341_BLACK);

  // Title Bar
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 10);
  tft.println("Uno Q + Adafruit TFT");
  tft.drawFastHLine(0, 32, SCREEN_W, ILI9341_DARKGREY);

  // Initialize FT6206 Capacitive Touch Controller over I2C
  Wire.begin();
  touchOnline = ctp.begin(40); // Sensitivity threshold 40

  tft.setTextSize(1);
  tft.setCursor(20, 42);
  if (touchOnline) {
    tft.setTextColor(ILI9341_GREEN);
    tft.println("FT6206 Capacitive Touch: ONLINE");
  } else {
    tft.setTextColor(ILI9341_RED);
    tft.println("FT6206 Capacitive Touch: NOT DETECTED");
  }

  tft.setTextColor(ILI9341_LIGHTGREY);
  tft.setCursor(20, 58);
  tft.println("Touch targets or buttons to test calibration:");

  drawUI(false, false);
}

void loop() {
  bool btnA = false;
  bool btnB = false;

  if (touchOnline && ctp.touched()) {
    TS_Point p = ctp.getPoint();

    // =========================================================================
    // ROTATION 1 COORDINATE TRANSFORMATION
    // =========================================================================
    // Raw FT6206 orientation: p.x in [0, 240], p.y in [0, 320]
    // In Landscape setRotation(1):
    //   Screen X (0 .. 320) = 320 - p.y
    //   Screen Y (0 .. 240) = p.x
    int touchX = 320 - p.y;
    int touchY = p.x;

    if (touchX >= 0 && touchX < SCREEN_W && touchY >= 0 && touchY < SCREEN_H) {
      // Check Button A hit
      if (touchX >= BTN_A_X && touchX <= BTN_A_X + BTN_W &&
          touchY >= BTN_Y   && touchY <= BTN_Y + BTN_H) {
        btnA = true;
      }
      
      // Check Button B hit
      if (touchX >= BTN_B_X && touchX <= BTN_B_X + BTN_W &&
          touchY >= BTN_Y   && touchY <= BTN_Y + BTN_H) {
        btnB = true;
      }

      // Live touch coordinate readout
      tft.fillRect(20, 78, 280, 24, ILI9341_BLACK);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(1);
      tft.setCursor(20, 84);
      tft.print("Touch: X=");
      tft.print(touchX);
      tft.print("  Y=");
      tft.print(touchY);
      tft.print("  Raw: (");
      tft.print(p.x);
      tft.print(",");
      tft.print(p.y);
      tft.print(")");

      // Draw touch dot marker
      tft.fillCircle(touchX, touchY, 3, ILI9341_MAGENTA);

      Serial.print("Touch Event -> X: ");
      Serial.print(touchX);
      Serial.print(" | Y: ");
      Serial.println(touchY);
    }
  }

  // Update button visual state
  static bool lastA = false;
  static bool lastB = false;

  if (btnA != lastA || btnB != lastB) {
    drawButton(BTN_A_X, BTN_Y, BTN_W, BTN_H, btnA ? "PRESSED!" : "BUTTON A", btnA, ILI9341_CYAN);
    drawButton(BTN_B_X, BTN_Y, BTN_W, BTN_H, btnB ? "PRESSED!" : "BUTTON B", btnB, ILI9341_GREEN);
    lastA = btnA;
    lastB = btnB;
  }

  delay(20);
}
