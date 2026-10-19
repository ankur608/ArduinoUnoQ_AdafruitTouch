/*
  UIControlsDemo.ino - Modern Embedded Dashboard with Interactive Touch Widgets
  Part of the ArduinoUnoQ_AdafruitTouch library.
  
  Demonstrates:
  - Interactive Toggle Switch (ON / OFF) with animated sliding pill
  - Smooth Touch Slider (0 - 100%) with live value readout
  - Dynamic Color Box preview that changes color as the slider moves
  - Instant tactile button response with color feedback
  
  Board: Arduino UNO Q (STM32U585)
  Shield: Adafruit 2.8" Capacitive TFT (PID 1947)
*/

#include <ArduinoUnoQ_AdafruitTouch.h>

UnoQ_AdafruitTouch screen;

// Widget Dimensions & Positions
// 1. Toggle Switch
#define TOGGLE_X     30
#define TOGGLE_Y     75
#define TOGGLE_W     70
#define TOGGLE_H     34
bool toggleState = true;

// 2. Touch Slider
#define SLIDER_X     30
#define SLIDER_Y    155
#define SLIDER_W    180
#define SLIDER_H     24
int sliderVal = 65; // 0 to 100

// 3. Color Preview Box
#define PREVIEW_X   230
#define PREVIEW_Y    75
#define PREVIEW_W    65
#define PREVIEW_H   104

// 4. Momentary Action Button
#define ACTION_BTN_X 30
#define ACTION_BTN_Y 195
#define ACTION_BTN_W 265
#define ACTION_BTN_H 34

void drawToggle(bool state) {
  uint16_t bg = state ? ILI9341_GREEN : 0x4208; // Dark Slate if OFF
  screen.tft.fillRoundRect(TOGGLE_X, TOGGLE_Y, TOGGLE_W, TOGGLE_H, TOGGLE_H / 2, bg);
  screen.tft.drawRoundRect(TOGGLE_X, TOGGLE_Y, TOGGLE_W, TOGGLE_H, TOGGLE_H / 2, ILI9341_WHITE);

  int knobR = (TOGGLE_H / 2) - 3;
  int knobX = state ? (TOGGLE_X + TOGGLE_W - knobR - 4) : (TOGGLE_X + knobR + 4);
  int knobY = TOGGLE_Y + (TOGGLE_H / 2);

  // Knob
  screen.tft.fillCircle(knobX, knobY, knobR, ILI9341_WHITE);

  // Label text
  screen.tft.fillRect(TOGGLE_X + TOGGLE_W + 12, TOGGLE_Y + 8, 70, 20, ILI9341_BLACK);
  screen.tft.setTextColor(state ? ILI9341_GREEN : ILI9341_LIGHTGREY);
  screen.tft.setTextSize(2);
  screen.tft.setCursor(TOGGLE_X + TOGGLE_W + 12, TOGGLE_Y + 8);
  screen.tft.print(state ? "ON" : "OFF");
}

void drawSlider(int val) {
  // Background groove
  screen.tft.fillRoundRect(SLIDER_X, SLIDER_Y, SLIDER_W, SLIDER_H, 6, 0x2104);
  screen.tft.drawRoundRect(SLIDER_X, SLIDER_Y, SLIDER_W, SLIDER_H, 6, ILI9341_DARKGREY);

  // Filled active bar
  int fillW = (val * (SLIDER_W - 4)) / 100;
  if (fillW > 0) {
    screen.tft.fillRoundRect(SLIDER_X + 2, SLIDER_Y + 2, fillW, SLIDER_H - 4, 4, ILI9341_CYAN);
  }

  // Value text
  screen.tft.fillRect(SLIDER_X, SLIDER_Y - 20, 140, 16, ILI9341_BLACK);
  screen.tft.setTextColor(ILI9341_WHITE);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(SLIDER_X, SLIDER_Y - 18);
  screen.tft.print("BRIGHTNESS: ");
  screen.tft.print(val);
  screen.tft.print("%");
}

void drawPreview(int val, bool power) {
  screen.tft.drawRoundRect(PREVIEW_X, PREVIEW_Y, PREVIEW_W, PREVIEW_H, 8, ILI9341_WHITE);

  if (!power) {
    screen.tft.fillRoundRect(PREVIEW_X + 2, PREVIEW_Y + 2, PREVIEW_W - 4, PREVIEW_H - 4, 6, ILI9341_BLACK);
    screen.tft.setTextColor(0x528A);
    screen.tft.setTextSize(1);
    screen.tft.setCursor(PREVIEW_X + 16, PREVIEW_Y + 45);
    screen.tft.print("OFF");
  } else {
    // Generate color intensity from slider
    uint8_t r = (val * 31) / 100;
    uint8_t g = (val * 63) / 100;
    uint8_t b = (val * 31) / 100;
    uint16_t col = (r << 11) | (g << 5) | b;

    screen.tft.fillRoundRect(PREVIEW_X + 2, PREVIEW_Y + 2, PREVIEW_W - 4, PREVIEW_H - 4, 6, col);
  }
}

void drawActionButton(bool pressed) {
  uint16_t fill = pressed ? ILI9341_ORANGE : 0x2965;
  screen.tft.fillRoundRect(ACTION_BTN_X, ACTION_BTN_Y, ACTION_BTN_W, ACTION_BTN_H, 8, fill);
  screen.tft.drawRoundRect(ACTION_BTN_X, ACTION_BTN_Y, ACTION_BTN_W, ACTION_BTN_H, 8, ILI9341_WHITE);

  screen.tft.setTextColor(pressed ? ILI9341_BLACK : ILI9341_WHITE);
  screen.tft.setTextSize(2);
  screen.tft.setCursor(ACTION_BTN_X + 38, ACTION_BTN_Y + 8);
  screen.tft.print(pressed ? "ACTIVE PULSE!" : "HOLD TO TRIGGER");
}

void setup() {
  Serial.begin(115200);
  delay(100);

  screen.begin();
  screen.tft.fillScreen(ILI9341_BLACK);

  // Header Title
  screen.tft.fillRect(0, 0, screen.width(), 35, 0x18C3);
  screen.tft.drawFastHLine(0, 35, screen.width(), ILI9341_WHITE);
  screen.tft.setTextColor(ILI9341_WHITE);
  screen.tft.setTextSize(2);
  screen.tft.setCursor(15, 9);
  screen.tft.print("Uno Q Dashboard");

  // Subtitle
  screen.tft.setTextColor(ILI9341_CYAN);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(30, 48);
  screen.tft.print("POWER SWITCH");

  // Render initial widgets
  drawToggle(toggleState);
  drawSlider(sliderVal);
  drawPreview(sliderVal, toggleState);
  drawActionButton(false);
}

void loop() {
  static bool actionPressed = false;

  if (screen.touched()) {
    TS_Point p = screen.getPoint();

    // 1. Check Toggle Hit
    if (p.x >= TOGGLE_X && p.x <= TOGGLE_X + TOGGLE_W &&
        p.y >= TOGGLE_Y && p.y <= TOGGLE_Y + TOGGLE_H) {
      toggleState = !toggleState;
      drawToggle(toggleState);
      drawPreview(sliderVal, toggleState);
      delay(180); // Toggle debounce
      return;
    }

    // 2. Check Slider Drag
    if (p.x >= SLIDER_X && p.x <= SLIDER_X + SLIDER_W &&
        p.y >= SLIDER_Y - 10 && p.y <= SLIDER_Y + SLIDER_H + 10) {
      int newVal = ((p.x - SLIDER_X) * 100) / SLIDER_W;
      newVal = max(0, min(100, newVal));
      if (newVal != sliderVal) {
        sliderVal = newVal;
        drawSlider(sliderVal);
        drawPreview(sliderVal, toggleState);
      }
    }

    // 3. Check Action Button
    if (p.x >= ACTION_BTN_X && p.x <= ACTION_BTN_X + ACTION_BTN_W &&
        p.y >= ACTION_BTN_Y && p.y <= ACTION_BTN_Y + ACTION_BTN_H) {
      if (!actionPressed) {
        actionPressed = true;
        drawActionButton(true);
      }
    }
  } else {
    // Finger released
    if (actionPressed) {
      actionPressed = false;
      drawActionButton(false);
    }
  }

  delay(15);
}
