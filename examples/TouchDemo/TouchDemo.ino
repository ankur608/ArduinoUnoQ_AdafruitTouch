/*
  TouchDemo.ino - Example sketch for ArduinoUnoQ_AdafruitTouch library.
  
  Demonstrates:
  - 1-line setup of display and touch
  - Automatic rotation alignment
  - Drawing interactive buttons and reading calibrated touches
*/

#include <ArduinoUnoQ_AdafruitTouch.h>

UnoQ_AdafruitTouch screen;

#define BTN_A_X 25
#define BTN_B_X 175
#define BTN_Y   120
#define BTN_W   120
#define BTN_H    70

void drawButton(int x, int y, int w, int h, const char* label, bool pressed, uint16_t color) {
  uint16_t fill = pressed ? color : 0x18E3;
  screen.tft.fillRoundRect(x, y, w, h, 8, fill);
  screen.tft.drawRoundRect(x, y, w, h, 8, ILI9341_WHITE);

  screen.tft.setTextColor(pressed ? ILI9341_BLACK : ILI9341_WHITE);
  screen.tft.setTextSize(2);
  int textX = x + (w / 2) - (strlen(label) * 6);
  int textY = y + (h / 2) - 8;
  screen.tft.setCursor(textX, textY);
  screen.tft.print(label);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize display and touch (default: rotation 1 landscape 320x240)
  bool ok = screen.begin();

  screen.tft.fillScreen(ILI9341_BLACK);
  screen.tft.setTextColor(ILI9341_WHITE);
  screen.tft.setTextSize(2);
  screen.tft.setCursor(20, 10);
  screen.tft.println("Uno Q + Adafruit TFT");
  screen.tft.drawFastHLine(0, 32, screen.width(), ILI9341_DARKGREY);

  screen.tft.setTextSize(1);
  screen.tft.setCursor(20, 42);
  if (ok) {
    screen.tft.setTextColor(ILI9341_GREEN);
    screen.tft.println("Capacitive Touch: ONLINE & READY");
  } else {
    screen.tft.setTextColor(ILI9341_RED);
    screen.tft.println("Capacitive Touch: NOT DETECTED (Check Shield)");
  }

  // Draw corner test alignment boxes
  screen.tft.drawRect(0, 0, 25, 25, ILI9341_YELLOW);
  screen.tft.drawRect(screen.width() - 25, 0, 25, 25, ILI9341_YELLOW);
  screen.tft.drawRect(0, screen.height() - 25, 25, 25, ILI9341_YELLOW);
  screen.tft.drawRect(screen.width() - 25, screen.height() - 25, 25, 25, ILI9341_YELLOW);

  drawButton(BTN_A_X, BTN_Y, BTN_W, BTN_H, "BUTTON A", false, ILI9341_CYAN);
  drawButton(BTN_B_X, BTN_Y, BTN_W, BTN_H, "BUTTON B", false, ILI9341_GREEN);
}

void loop() {
  bool btnA = false;
  bool btnB = false;

  if (screen.touched()) {
    // getPoint() automatically returns coordinates calibrated to the current rotation!
    TS_Point p = screen.getPoint();

    if (p.x >= BTN_A_X && p.x <= BTN_A_X + BTN_W && p.y >= BTN_Y && p.y <= BTN_Y + BTN_H) {
      btnA = true;
    }
    if (p.x >= BTN_B_X && p.x <= BTN_B_X + BTN_W && p.y >= BTN_Y && p.y <= BTN_Y + BTN_H) {
      btnB = true;
    }

    // Live coordinate readout
    screen.tft.fillRect(20, 65, 280, 20, ILI9341_BLACK);
    screen.tft.setTextColor(ILI9341_WHITE);
    screen.tft.setTextSize(1);
    screen.tft.setCursor(20, 70);
    screen.tft.print("Touch: X=");
    screen.tft.print(p.x);
    screen.tft.print("  Y=");
    screen.tft.print(p.y);

    screen.tft.fillCircle(p.x, p.y, 3, ILI9341_MAGENTA);

    Serial.print("Touch -> X: ");
    Serial.print(p.x);
    Serial.print(" | Y: ");
    Serial.println(p.y);
  }

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
