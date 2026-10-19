/*
  ComicReader.ino - Authentic Yusuke Murata One Punch Man Manga Reader (E-Ink Edition)
  Part of the ArduinoUnoQ_AdafruitTouch library.
  
  Authentic High-Definition Manga Pages (One Punch Man):
    - Page 1: Tankōbon Volume 1 Cover - Saitama putting on hero glove ("Hero for Fun")
    - Page 2: Chapter 35 - Saitama kicked to the Moon, standing in low gravity looking at Earth
    - Page 3: Chapter 36 - Fierce determination: "Killer Move: Serious Series"
    - Page 4: Chapter 36 - Climax: Serious Punch shockwave splitting the atmosphere
    - Page 5: Chapter 17 - Genos sparring match: "DEATH" punch stopped 1 inch from face
    - Page 6: Chapter 10 - Supermarket Panic: "Today is bargain day at the supermarket!"
    - Page 7: Chapter 36 - The Legendary nonchalant "OK." response

  Features:
    - 100% Authentic Yusuke Murata original manga lineart (Floyd-Steinberg E-Ink dithered)
    - High-speed SPI stream rendering (instantaneous full 76,800 pixel redraw)
    - Simulated bi-stable E-Paper black/white inversion flash on page flip
    - Capacitive touch navigation: tap right/center for Next, left edge for Previous
    - Sleek page counter and chapter subtitle HUD
  
  Board: Arduino UNO Q (STM32U585)
  Shield: Adafruit 2.8" Capacitive TFT (PID 1947)
  Author: ankur608
*/

#include <ArduinoUnoQ_AdafruitTouch.h>
#include "comic_art.h"

UnoQ_AdafruitTouch screen;

#define SCREEN_W 240
#define SCREEN_H 320

// High-Contrast E-Ink Manga Palette
#define INK_WHITE  0xFFFF
#define INK_BLACK  0x0000

int currentPage = 0;

// ==============================================================================
// SIMULATED E-INK FLASH REFRESH
// ==============================================================================
void triggerEInkFlash() {
  // Bi-stable e-paper particles inversion pulse
  screen.tft.fillScreen(INK_BLACK);
  delay(35);
  screen.tft.fillScreen(INK_WHITE);
  delay(40);
}

// ==============================================================================
// FAST MANGA BITMAP STREAM RENDERER
// ==============================================================================
void drawMangaBitmap(const uint8_t *bitmap) {
  screen.tft.startWrite();
  screen.tft.setAddrWindow(0, 0, SCREEN_W, SCREEN_H);

  for (int i = 0; i < MANGA_PAGE_W * MANGA_PAGE_H / 8; i++) {
    uint8_t b = pgm_read_byte(&bitmap[i]);
    for (int bit = 7; bit >= 0; bit--) {
      uint16_t color = (b & (1 << bit)) ? INK_BLACK : INK_WHITE;
      screen.tft.pushColor(color);
    }
  }
  screen.tft.endWrite();
}

// ==============================================================================
// RENDER COMPLETE PAGE WITH E-INK HUD
// ==============================================================================
void renderPage(int pageIdx) {
  triggerEInkFlash();

  // 1. Draw the authentic original manga artwork
  drawMangaBitmap(MANGA_PAGES[pageIdx].bitmap);

  // 2. Minimalist Manga Navigation Overlay (Bottom Bar)
  screen.tft.fillRect(0, 306, SCREEN_W, 14, INK_WHITE);
  screen.tft.drawFastHLine(0, 305, SCREEN_W, INK_BLACK);

  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(6, 309);
  screen.tft.print(MANGA_PAGES[pageIdx].title);

  // Page counter badge (e.g. "1/7")
  char pageBadge[12];
  snprintf(pageBadge, sizeof(pageBadge), "[ %d/%d ]", pageIdx + 1, MANGA_TOTAL_PAGES);
  screen.tft.setCursor(192, 309);
  screen.tft.print(pageBadge);

  // If cover page, show subtle tap hint
  if (pageIdx == 0) {
    screen.tft.fillRect(35, 280, 170, 18, INK_BLACK);
    screen.tft.drawRect(33, 278, 174, 22, INK_WHITE);
    screen.tft.setTextColor(INK_WHITE);
    screen.tft.setTextSize(1);
    screen.tft.setCursor(44, 285);
    screen.tft.print("TAP SCREEN TO TURN PAGE");
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize display and touch in Portrait orientation
  screen.begin();
  screen.setRotation(0); // 240 x 320 Portrait

  renderPage(currentPage);
}

void loop() {
  if (screen.touched()) {
    TS_Point raw = screen.touch.getPoint();
    
    // In Rotation 0 (Portrait), coordinate inversion:
    int touchX = 240 - raw.x;
    int touchY = 320 - raw.y;

    // Navigation:
    // Left edge (X < 65) goes to PREVIOUS page
    // Anywhere else (Center / Right) goes to NEXT page
    if (touchX < 65) {
      if (currentPage > 0) {
        currentPage--;
      } else {
        currentPage = MANGA_TOTAL_PAGES - 1; // Wrap around to end
      }
    } else {
      currentPage = (currentPage + 1) % MANGA_TOTAL_PAGES;
    }

    renderPage(currentPage);
    delay(320); // Debounce touch
  }

  delay(20);
}
