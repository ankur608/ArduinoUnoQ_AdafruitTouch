/*
  MangaReader.ino - Japanese Manga E-Ink Display Emulator (Saitama Edition)
  Part of the ArduinoUnoQ_AdafruitTouch library.
  
  Features:
  - Authentic Japanese Manga page layout with classic panel gutters
  - Halftone screentone dithering & dynamic action speed lines
  - Features Saitama (One Punch Man) in English with manga sound effects (*SFX*)
  - 3 Interactive Views (Tap screen to flip pages with simulated E-Ink refresh):
      View 1: Full 3-Panel Manga Page (Setup -> Menace -> Serious Punch!)
      View 2: High-Detail Full-Screen "Serious Mode" Saitama Close-up
      View 3: Comedic "Special Bargain Day at the Supermarket!" Gag
  
  Board: Arduino UNO Q (STM32U585)
  Shield: Adafruit 2.8" Capacitive TFT (PID 1947)
  Author: ankur608
*/

#include <ArduinoUnoQ_AdafruitTouch.h>

UnoQ_AdafruitTouch screen;

#define SCREEN_W 240
#define SCREEN_H 320

// High-Contrast Manga Palette
#define INK_WHITE  0xFFFF
#define INK_BLACK  0x0000
#define INK_GRAY   0x8410  // 50% Halftone gray
#define INK_LIGHT  0xCE79  // 25% Screentone tint

int currentView = 0;
const int TOTAL_VIEWS = 3;

// ==============================================================================
// SIMULATED E-INK REFRESH FLASH
// ==============================================================================
void triggerEInkFlash() {
  screen.tft.fillScreen(INK_BLACK);
  delay(30);
  screen.tft.fillScreen(INK_WHITE);
  delay(40);
}

// ==============================================================================
// MANGA GRAPHICS UTILITIES (Screentone, Speedlines, Speech Bubbles)
// ==============================================================================

// Screentone / Halftone Dither: draws classic manga dot matrix
void drawScreentone(int x, int y, int w, int h, int step) {
  for (int j = y; j < y + h; j += step) {
    int offset = ((j / step) % 2) * (step / 2);
    for (int i = x + offset; i < x + w; i += step) {
      screen.tft.drawPixel(i, j, INK_BLACK);
    }
  }
}

// Radial Speed Lines bursting from focus point
void drawSpeedLines(int cx, int cy, int rMin, int rMax, int count) {
  for (int i = 0; i < count; i++) {
    float angle = (i * (360.0 / count)) * 0.01745;
    int x1 = cx + cos(angle) * rMin;
    int y1 = cy + sin(angle) * rMin;
    int x2 = cx + cos(angle) * rMax;
    int y2 = cy + sin(angle) * rMax;
    screen.tft.drawLine(x1, y1, x2, y2, INK_BLACK);
  }
}

// Oval Speech Bubble with tail
void drawSpeechBubble(int x, int y, int w, int h, int tailDir) {
  // Fill bubble white
  screen.tft.fillRoundRect(x, y, w, h, 8, INK_WHITE);
  screen.tft.drawRoundRect(x, y, w, h, 8, INK_BLACK);
  screen.tft.drawRoundRect(x - 1, y - 1, w + 2, h + 2, 8, INK_BLACK);

  // Tail
  if (tailDir == 1) { // Tail pointing bottom-left
    screen.tft.fillTriangle(x + 10, y + h - 1, x + 22, y + h - 1, x + 4, y + h + 8, INK_WHITE);
    screen.tft.drawLine(x + 10, y + h - 1, x + 4, y + h + 8, INK_BLACK);
    screen.tft.drawLine(x + 22, y + h - 1, x + 4, y + h + 8, INK_BLACK);
  } else if (tailDir == 2) { // Tail pointing bottom-right
    screen.tft.fillTriangle(x + w - 24, y + h - 1, x + w - 10, y + h - 1, x + w - 6, y + h + 8, INK_WHITE);
    screen.tft.drawLine(x + w - 24, y + h - 1, x + w - 6, y + h + 8, INK_BLACK);
    screen.tft.drawLine(x + w - 10, y + h - 1, x + w - 6, y + h + 8, INK_BLACK);
  }
}

// Bold Japanese/English Manga Sound Effect (SFX)
void drawSFX(int x, int y, const char* sfx, int size) {
  screen.tft.setTextSize(size);
  // White outline
  for (int dx = -2; dx <= 2; dx++) {
    for (int dy = -2; dy <= 2; dy++) {
      screen.tft.setTextColor(INK_WHITE);
      screen.tft.setCursor(x + dx, y + dy);
      screen.tft.print(sfx);
    }
  }
  // Black text
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setCursor(x, y);
  screen.tft.print(sfx);
}

// ==============================================================================
// VIEW 1: FULL 1-PAGE 3-PANEL MANGA SPREAD
// ==============================================================================
void drawView1_FullPage() {
  triggerEInkFlash();

  // Top Page Header
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(8, 6);
  screen.tft.print("ONE PUNCH MAN #01");
  screen.tft.setCursor(170, 6);
  screen.tft.print("ENGLISH ED.");
  screen.tft.drawFastHLine(6, 17, SCREEN_W - 12, INK_BLACK);

  // ----------------------------------------------------------------------------
  // PANEL 1: THE HERO ARRIVES (Y: 22 to 105)
  // ----------------------------------------------------------------------------
  screen.tft.drawRect(8, 22, SCREEN_W - 16, 85, INK_BLACK);
  screen.tft.drawRect(9, 23, SCREEN_W - 18, 83, INK_BLACK);

  // Background screentone on right half
  drawScreentone(120, 24, SCREEN_W - 138, 81, 4);

  // Saitama Iconic Oval Head (Left side)
  int hx = 52, hy = 62;
  screen.tft.fillCircle(hx, hy, 26, INK_WHITE);
  screen.tft.drawCircle(hx, hy, 26, INK_BLACK);
  screen.tft.drawCircle(hx, hy, 25, INK_BLACK);

  // Bald shine reflection highlight
  screen.tft.drawArc(hx - 8, hy - 14, 10, 8, 220, 290, INK_BLACK);

  // Nonchalant egg face: simple oval eyes & straight mouth
  screen.tft.drawFastHLine(hx - 14, hy - 4, 8, INK_BLACK);
  screen.tft.drawFastHLine(hx + 6, hy - 4, 8, INK_BLACK);
  screen.tft.fillCircle(hx - 10, hy, 2, INK_BLACK); // Left pupil
  screen.tft.fillCircle(hx + 10, hy, 2, INK_BLACK); // Right pupil
  screen.tft.drawPixel(hx, hy + 6, INK_BLACK);      // Simple nose dot
  screen.tft.drawFastHLine(hx - 7, hy + 13, 14, INK_BLACK); // Flat mouth

  // Suit Collar & Zipper
  screen.tft.drawLine(hx - 14, hy + 25, hx - 22, hy + 44, INK_BLACK);
  screen.tft.drawLine(hx + 14, hy + 25, hx + 22, hy + 44, INK_BLACK);
  screen.tft.drawFastVLine(hx, hy + 24, 20, INK_BLACK);

  // Speech Bubble: Saitama
  drawSpeechBubble(105, 30, 118, 48, 1);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(112, 38);
  screen.tft.print("I'm just a guy");
  screen.tft.setCursor(112, 50);
  screen.tft.print("who's a HERO");
  screen.tft.setCursor(112, 62);
  screen.tft.print("for FUN.");

  // Small SFX
  drawSFX(88, 76, "*BLANK*", 1);

  // ----------------------------------------------------------------------------
  // PANEL 2: THE MONSTER'S THREAT (Y: 114 to 195)
  // ----------------------------------------------------------------------------
  screen.tft.drawRect(8, 114, SCREEN_W - 16, 82, INK_BLACK);
  screen.tft.drawRect(9, 115, SCREEN_W - 18, 80, INK_BLACK);

  // Heavy cross-hatching menace background
  for (int i = 10; i < 90; i += 6) {
    screen.tft.drawLine(10, 116 + i, 90, 116, INK_BLACK);
  }

  // Giant Monster Silhouette (Left side)
  screen.tft.fillTriangle(25, 185, 55, 128, 85, 185, INK_BLACK);
  screen.tft.fillCircle(55, 142, 16, INK_BLACK);
  // Menacing Glowing Eyes
  screen.tft.fillTriangle(44, 138, 52, 142, 44, 144, INK_WHITE);
  screen.tft.fillTriangle(66, 138, 58, 142, 66, 144, INK_WHITE);
  // Sharp jagged fangs
  for (int fx = 40; fx < 70; fx += 6) {
    screen.tft.fillTriangle(fx, 158, fx + 3, 166, fx + 6, 158, INK_WHITE);
  }

  // Monster Speech Bubble (Spiky dialog)
  drawSpeechBubble(95, 120, 130, 52, 1);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(102, 128);
  screen.tft.print("PREPARE TO PERISH!");
  screen.tft.setCursor(102, 140);
  screen.tft.print("I AM DISASTER LEVEL");
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setCursor(102, 152);
  screen.tft.print("DRAGON -- DIE!!!");

  // Heavy Rumble Sound Effect
  drawSFX(14, 172, "ゴゴゴ *RUMBLE*", 1);

  // ----------------------------------------------------------------------------
  // PANEL 3: THE SERIOUS PUNCH (Y: 204 to 298)
  // ----------------------------------------------------------------------------
  screen.tft.drawRect(8, 204, SCREEN_W - 16, 95, INK_BLACK);
  screen.tft.drawRect(9, 205, SCREEN_W - 18, 93, INK_BLACK);

  // Dramatic explosive radial speed lines bursting from center
  int px = 145, py = 250;
  drawSpeedLines(px, py, 32, 115, 36);

  // Saitama's Giant Fist rushing toward camera (Foreground Perspective)
  screen.tft.fillCircle(px, py, 26, INK_WHITE);
  screen.tft.drawCircle(px, py, 26, INK_BLACK);
  screen.tft.drawCircle(px, py, 25, INK_BLACK);

  // Fist Knuckles & Glove Shading
  for (int k = -18; k <= 18; k += 12) {
    screen.tft.drawCircle(px + k, py - 10, 6, INK_BLACK);
    screen.tft.drawCircle(px + k, py - 10, 5, INK_BLACK);
  }
  screen.tft.drawFastHLine(px - 20, py + 2, 40, INK_BLACK);
  // Red glove screentone
  drawScreentone(px - 18, py + 5, 36, 16, 3);

  // Iconic Technique Callout Badge
  screen.tft.fillRect(14, 212, 105, 26, INK_BLACK);
  screen.tft.setTextColor(INK_WHITE);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(18, 216);
  screen.tft.print("SERIOUS SERIES:");
  screen.tft.setCursor(18, 226);
  screen.tft.print("SERIOUS PUNCH!");

  // Giant Impact Sound Effect
  drawSFX(15, 255, "*KABOOM!*", 2);

  // Bottom Navigation Bar
  screen.tft.drawFastHLine(6, 305, SCREEN_W - 12, INK_BLACK);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(8, 310);
  screen.tft.print("< TAP SCREEN TO FLIP PAGE >");
  screen.tft.setCursor(185, 310);
  screen.tft.print("PAGE 1/3");
}

// ==============================================================================
// VIEW 2: SERIOUS MODE SAITAMA CLOSE-UP (Dynamic Hatching & Speedlines)
// ==============================================================================
void drawView2_SeriousMode() {
  triggerEInkFlash();

  // Full Screen Border
  screen.tft.drawRect(4, 4, SCREEN_W - 8, SCREEN_H - 8, INK_BLACK);
  screen.tft.drawRect(5, 5, SCREEN_W - 10, SCREEN_H - 10, INK_BLACK);

  // Dramatic Speed Lines originating from head center
  int cx = 120, cy = 135;
  drawSpeedLines(cx, cy, 58, 175, 48);

  // Intense Chiseled Skull & Head
  screen.tft.fillCircle(cx, cy - 25, 54, INK_WHITE);
  screen.tft.drawCircle(cx, cy - 25, 54, INK_BLACK);
  screen.tft.drawCircle(cx, cy - 25, 53, INK_BLACK);

  // Chiseled Jaw & Chin
  screen.tft.fillTriangle(cx - 46, cy - 8, cx + 46, cy - 8, cx, cy + 50, INK_WHITE);
  screen.tft.drawLine(cx - 46, cy - 8, cx, cy + 50, INK_BLACK);
  screen.tft.drawLine(cx + 46, cy - 8, cx, cy + 50, INK_BLACK);
  screen.tft.drawLine(cx - 45, cy - 8, cx, cy + 49, INK_BLACK);
  screen.tft.drawLine(cx + 45, cy - 8, cx, cy + 49, INK_BLACK);

  // Deep Shaded Brow & Eye Sockets (Manga Cross-Hatching)
  screen.tft.fillRect(cx - 38, cy - 35, 76, 16, INK_BLACK);

  // Intense Piercing Pupils (Sharp triangles cut into black brow)
  screen.tft.fillTriangle(cx - 28, cy - 30, cx - 12, cy - 25, cx - 14, cy - 31, INK_WHITE);
  screen.tft.fillCircle(cx - 20, cy - 27, 2, INK_BLACK);

  screen.tft.fillTriangle(cx + 28, cy - 30, cx + 12, cy - 25, cx + 14, cy - 31, INK_WHITE);
  screen.tft.fillCircle(cx + 20, cy - 27, 2, INK_BLACK);

  // Intense Forehead Vein & Furrow Lines
  screen.tft.drawFastVLine(cx, cy - 50, 12, INK_BLACK);
  screen.tft.drawLine(cx - 6, cy - 48, cx - 14, cy - 54, INK_BLACK);
  screen.tft.drawLine(cx + 6, cy - 48, cx + 14, cy - 54, INK_BLACK);

  // Sharp Nose & Tight Gritted Mouth
  screen.tft.drawLine(cx, cy - 18, cx - 4, cy + 5, INK_BLACK);
  screen.tft.drawFastHLine(cx - 5, cy + 5, 8, INK_BLACK);

  screen.tft.drawFastHLine(cx - 18, cy + 22, 36, INK_BLACK);
  screen.tft.drawFastHLine(cx - 14, cy + 24, 28, INK_BLACK);
  // Chin shadow hatching
  for (int y = cy + 28; y < cy + 45; y += 3) {
    screen.tft.drawFastHLine(cx - (cy + 45 - y) / 2, y, (cy + 45 - y), INK_BLACK);
  }

  // Trapezius / Muscular Neck Hatching
  screen.tft.drawLine(cx - 35, cy + 32, cx - 55, cy + 90, INK_BLACK);
  screen.tft.drawLine(cx + 35, cy + 32, cx + 55, cy + 90, INK_BLACK);
  drawScreentone(cx - 30, cy + 48, 60, 40, 3);

  // Huge Japanese Kanbun Action SFX
  drawSFX(16, 215, "ゴ ゴ ゴ", 3);
  drawSFX(150, 215, "ゴ ゴ ゴ", 3);

  // Caption Banner at Bottom
  screen.tft.fillRect(18, 260, 204, 34, INK_BLACK);
  screen.tft.setTextColor(INK_WHITE);
  screen.tft.setTextSize(2);
  screen.tft.setCursor(28, 270);
  screen.tft.print("SERIOUS MODE.");

  // Footer Navigation
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(8, 306);
  screen.tft.print("< TAP SCREEN TO FLIP PAGE >");
  screen.tft.setCursor(185, 306);
  screen.tft.print("PAGE 2/3");
}

// ==============================================================================
// VIEW 3: COMEDIC GAG - "SPECIAL BARGAIN DAY!"
// ==============================================================================
void drawView3_ComedicGag() {
  triggerEInkFlash();

  screen.tft.drawRect(6, 6, SCREEN_W - 12, SCREEN_H - 12, INK_BLACK);
  screen.tft.drawRect(7, 7, SCREEN_W - 14, SCREEN_H - 14, INK_BLACK);

  // Top Title Bar
  screen.tft.fillRect(8, 8, SCREEN_W - 16, 22, INK_BLACK);
  screen.tft.setTextColor(INK_WHITE);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(20, 14);
  screen.tft.print("SPECIAL BONUS EPISODE: SUPERMARKET");

  // Saitama Shocked Face (Eyes popping wide!)
  int sx = 70, sy = 85;
  screen.tft.fillCircle(sx, sy, 34, INK_WHITE);
  screen.tft.drawCircle(sx, sy, 34, INK_BLACK);
  screen.tft.drawCircle(sx, sy, 33, INK_BLACK);

  // Huge round shocked eyes with tiny dot pupils
  screen.tft.drawCircle(sx - 12, sy - 6, 9, INK_BLACK);
  screen.tft.fillCircle(sx - 12, sy - 6, 2, INK_BLACK);
  screen.tft.drawCircle(sx + 12, sy - 6, 9, INK_BLACK);
  screen.tft.fillCircle(sx + 12, sy - 6, 2, INK_BLACK);

  // Jaw dropped wide open (Shocked O mouth!)
  screen.tft.fillRoundRect(sx - 10, sy + 10, 20, 18, 6, INK_BLACK);

  // Sweat Drops flying off head!
  screen.tft.fillTriangle(sx + 36, sy - 20, sx + 42, sy - 24, sx + 39, sy - 14, INK_BLACK);
  screen.tft.fillTriangle(sx - 36, sy - 20, sx - 42, sy - 24, sx - 39, sy - 14, INK_BLACK);

  // Huge Scream Speech Bubble
  drawSpeechBubble(118, 45, 110, 68, 1);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(124, 52);
  screen.tft.print("TODAY IS...");
  screen.tft.setTextSize(1);
  screen.tft.setCursor(124, 65);
  screen.tft.print("BARGAIN DAY AT");
  screen.tft.setCursor(124, 77);
  screen.tft.print("THE SUPERMARKET!");
  screen.tft.setCursor(124, 92);
  screen.tft.print("I MUST RUN!!!");

  // Large Shock Sound Effect
  drawSFX(20, 128, "*SHOCK!*", 2);

  // Supermarket Flyer Receipt Card in Bottom Half
  int rx = 18, ry = 155;
  screen.tft.drawRect(rx, ry, SCREEN_W - 36, 125, INK_BLACK);
  drawScreentone(rx + 2, ry + 2, SCREEN_W - 40, 18, 2);
  screen.tft.fillRect(rx + 15, ry + 4, 175, 14, INK_WHITE);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(rx + 20, ry + 7);
  screen.tft.print("CITY Z SUPERMARKET FLYER");

  screen.tft.setCursor(rx + 10, ry + 28);
  screen.tft.print("* PRIME BEEF:   70% OFF!");
  screen.tft.setCursor(rx + 10, ry + 42);
  screen.tft.print("* CABBAGE HEAD:  50 YEN");
  screen.tft.setCursor(rx + 10, ry + 56);
  screen.tft.print("* TOKYO TOFU:    30 YEN");
  screen.tft.setCursor(rx + 10, ry + 70);
  screen.tft.print("* INSTANT UDON:  FREE WITH COUPON");

  screen.tft.drawFastHLine(rx + 8, ry + 84, SCREEN_W - 52, INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(rx + 10, ry + 92);
  screen.tft.print("CLOSING IN: 10 MINUTES!");
  screen.tft.setCursor(rx + 10, ry + 106);
  screen.tft.print("GENOS: 'MASTER, I WILL ASSIST!'");

  // Footer Navigation
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(8, 306);
  screen.tft.print("< TAP SCREEN TO RETURN TO P1 >");
  screen.tft.setCursor(185, 306);
  screen.tft.print("PAGE 3/3");
}

void renderView() {
  switch (currentView) {
    case 0: drawView1_FullPage();     break;
    case 1: drawView2_SeriousMode();  break;
    case 2: drawView3_ComedicGag();   break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize in Portrait orientation (Rotation 0)
  screen.begin();
  screen.setRotation(0); // 240 x 320

  renderView();
}

void loop() {
  if (screen.touched()) {
    currentView = (currentView + 1) % TOTAL_VIEWS;
    renderView();
    delay(280); // Debounce page turn
  }

  delay(20);
}
