/*
  TouchPaint.ino - Interactive Capacitive Touch Paint Canvas
  Part of the ArduinoUnoQ_AdafruitTouch library.
  
  Features:
  - 8-color interactive palette bar (Red, Green, Blue, Yellow, Cyan, Magenta, White, Eraser)
  - Clear Screen button
  - Smooth capacitive finger-drawing on 320x240 landscape display
  - Brush size indicator
  
  Board: Arduino UNO Q (STM32U585)
  Shield: Adafruit 2.8" Capacitive TFT (PID 1947)
*/

#include <ArduinoUnoQ_AdafruitTouch.h>

UnoQ_AdafruitTouch screen;

// Palette Configuration
#define PALETTE_Y      0
#define PALETTE_BOX_W 35
#define PALETTE_BOX_H 35
#define NUM_COLORS     8

const uint16_t paletteColors[NUM_COLORS] = {
  ILI9341_RED,
  ILI9341_GREEN,
  ILI9341_BLUE,
  ILI9341_YELLOW,
  ILI9341_CYAN,
  ILI9341_MAGENTA,
  ILI9341_WHITE,
  ILI9341_BLACK // Eraser
};

uint16_t currentColor = ILI9341_RED;
int brushRadius = 3;
int lastX = -1;
int lastY = -1;

void drawPalette() {
  for (int i = 0; i < NUM_COLORS; i++) {
    int x = i * PALETTE_BOX_W;
    screen.tft.fillRect(x, PALETTE_Y, PALETTE_BOX_W, PALETTE_BOX_H, paletteColors[i]);
    screen.tft.drawRect(x, PALETTE_Y, PALETTE_BOX_W, PALETTE_BOX_H, ILI9341_DARKGREY);

    // If active color, highlight with a white inner border
    if (paletteColors[i] == currentColor) {
      screen.tft.drawRect(x + 2, PALETTE_Y + 2, PALETTE_BOX_W - 4, PALETTE_BOX_H - 4, ILI9341_WHITE);
      screen.tft.drawRect(x + 3, PALETTE_Y + 3, PALETTE_BOX_W - 6, PALETTE_BOX_H - 6, ILI9341_WHITE);
    }
  }

  // Clear Button on far right
  int clearX = NUM_COLORS * PALETTE_BOX_W;
  int clearW = screen.width() - clearX;
  screen.tft.fillRect(clearX, PALETTE_Y, clearW, PALETTE_BOX_H, 0x3186); // Dark Slate
  screen.tft.drawRect(clearX, PALETTE_Y, clearW, PALETTE_BOX_H, ILI9341_WHITE);
  screen.tft.setTextColor(ILI9341_WHITE);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(clearX + 4, PALETTE_Y + 13);
  screen.tft.print("CLEAR");

  // Bottom border dividing palette and canvas
  screen.tft.drawFastHLine(0, PALETTE_BOX_H, screen.width(), ILI9341_LIGHTGREY);
}

void clearCanvas() {
  screen.tft.fillRect(0, PALETTE_BOX_H + 1, screen.width(), screen.height() - (PALETTE_BOX_H + 1), ILI9341_BLACK);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize display & touch
  screen.begin();

  screen.tft.fillScreen(ILI9341_BLACK);
  drawPalette();

  // Welcome hint (cleared on first touch)
  screen.tft.setTextColor(ILI9341_DARKGREY);
  screen.tft.setTextSize(2);
  screen.tft.setCursor(55, 120);
  screen.tft.print("Touch to draw!");
}

void loop() {
  if (screen.touched()) {
    TS_Point p = screen.getPoint();

    // Check if touch is within the top Palette bar
    if (p.y <= PALETTE_BOX_H) {
      int colIndex = p.x / PALETTE_BOX_W;
      if (colIndex < NUM_COLORS) {
        currentColor = paletteColors[colIndex];
        drawPalette();
        delay(120); // Touch debounce for palette selection
      } else {
        // Clear Button hit
        clearCanvas();
        delay(150);
      }
      lastX = -1;
      lastY = -1;
    } else {
      // Paint on canvas area
      if (currentColor == ILI9341_BLACK) {
        // Eraser mode (larger radius)
        screen.tft.fillCircle(p.x, p.y, 10, ILI9341_BLACK);
      } else {
        // Smooth line drawing between consecutive points
        if (lastX > 0 && lastY > PALETTE_BOX_H) {
          screen.tft.drawLine(lastX, lastY, p.x, p.y, currentColor);
          screen.tft.fillCircle(p.x, p.y, brushRadius, currentColor);
        } else {
          screen.tft.fillCircle(p.x, p.y, brushRadius, currentColor);
        }
      }
      lastX = p.x;
      lastY = p.y;
    }
  } else {
    // Finger lifted
    lastX = -1;
    lastY = -1;
  }
}
