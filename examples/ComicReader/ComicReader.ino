/*
  ComicReader.ino - Full Issue Japanese Comic E-Ink Display Emulator (Saitama Edition)
  Part of the ArduinoUnoQ_AdafruitTouch library.
  
  Complete 7-Page Manga Issue:
    - Cover Page: Tankōbon Volume 1 Cover with Hero Association Seal
    - Page 1: City Z in Peril! Monsters attack & sirens wail
    - Page 2: The Colossal Titan looms over the city skyline
    - Page 3: Saitama appears holding grocery bags ("Hero for fun")
    - Page 4: The Villain's endless monologue vs. Saitama picking his nose
    - Page 5: The Serious Shift! Chiseled jaw, speedlines & "SERIOUS SERIES..."
    - Page 6: Climax: SERIOUS PUNCH! Full-page explosive forced-perspective impact
    - Page 7: The Aftermath: The real tragedy... the bargain cabbage got bruised!
  
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
#define INK_GRAY   0x8410
#define INK_LIGHT  0xCE79

int currentPage = 0;
const int TOTAL_PAGES = 7;

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
// MANGA GRAPHICS UTILITIES
// ==============================================================================
void drawScreentone(int x, int y, int w, int h, int step) {
  for (int j = y; j < y + h; j += step) {
    int offset = ((j / step) % 2) * (step / 2);
    for (int i = x + offset; i < x + w; i += step) {
      screen.tft.drawPixel(i, j, INK_BLACK);
    }
  }
}

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

void drawSpeechBubble(int x, int y, int w, int h, int tailDir) {
  screen.tft.fillRoundRect(x, y, w, h, 6, INK_WHITE);
  screen.tft.drawRoundRect(x, y, w, h, 6, INK_BLACK);
  screen.tft.drawRoundRect(x - 1, y - 1, w + 2, h + 2, 6, INK_BLACK);

  if (tailDir == 1) { // Bottom-left tail
    screen.tft.fillTriangle(x + 10, y + h - 1, x + 22, y + h - 1, x + 4, y + h + 8, INK_WHITE);
    screen.tft.drawLine(x + 10, y + h - 1, x + 4, y + h + 8, INK_BLACK);
    screen.tft.drawLine(x + 22, y + h - 1, x + 4, y + h + 8, INK_BLACK);
  } else if (tailDir == 2) { // Bottom-right tail
    screen.tft.fillTriangle(x + w - 22, y + h - 1, x + w - 10, y + h - 1, x + w - 4, y + h + 8, INK_WHITE);
    screen.tft.drawLine(x + w - 22, y + h - 1, x + w - 4, y + h + 8, INK_BLACK);
    screen.tft.drawLine(x + w - 10, y + h - 1, x + w - 4, y + h + 8, INK_BLACK);
  }
}

void drawSFX(int x, int y, const char* sfx, int size) {
  screen.tft.setTextSize(size);
  for (int dx = -2; dx <= 2; dx++) {
    for (int dy = -2; dy <= 2; dy++) {
      screen.tft.setTextColor(INK_WHITE);
      screen.tft.setCursor(x + dx, y + dy);
      screen.tft.print(sfx);
    }
  }
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setCursor(x, y);
  screen.tft.print(sfx);
}

void drawPageFooter(int pageNum) {
  screen.tft.drawFastHLine(6, 305, SCREEN_W - 12, INK_BLACK);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(8, 310);
  screen.tft.print("< TAP LEFT / RIGHT TO FLIP >");
  screen.tft.setCursor(185, 310);
  screen.tft.print("P.");
  screen.tft.print(pageNum);
  screen.tft.print("/");
  screen.tft.print(TOTAL_PAGES);
}

// ==============================================================================
// COVER PAGE: TANKŌBON VOLUME 1
// ==============================================================================
void drawCoverPage() {
  triggerEInkFlash();

  screen.tft.drawRect(4, 4, SCREEN_W - 8, SCREEN_H - 8, INK_BLACK);
  screen.tft.drawRect(6, 6, SCREEN_W - 12, SCREEN_H - 12, INK_BLACK);

  // Manga Volume Badge
  screen.tft.fillRect(12, 12, 45, 14, INK_BLACK);
  screen.tft.setTextColor(INK_WHITE);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(16, 15);
  screen.tft.print("VOL. 01");

  // Hero Association Seal
  screen.tft.drawCircle(210, 20, 10, INK_BLACK);
  screen.tft.drawCircle(210, 20, 9, INK_BLACK);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setCursor(208, 17);
  screen.tft.print("H");

  // Bold Stylized Title Card
  screen.tft.fillRect(12, 32, SCREEN_W - 24, 44, INK_BLACK);
  screen.tft.setTextColor(INK_WHITE);
  screen.tft.setTextSize(3);
  screen.tft.setCursor(20, 42);
  screen.tft.print("ONE PUNCH");
  screen.tft.setTextSize(2);
  screen.tft.setCursor(170, 50);
  screen.tft.print("MAN");

  // Subtitle
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(28, 82);
  screen.tft.print("- THE STRONGEST HERO -");

  // Cover Portrait of Saitama (Center)
  int cx = 120, cy = 160;
  drawSpeedLines(cx, cy, 55, 120, 36);

  // Cape (Flowing behind shoulders)
  screen.tft.fillTriangle(cx - 50, cy + 20, cx - 75, cy + 85, cx - 20, cy + 70, INK_BLACK);
  screen.tft.fillTriangle(cx + 50, cy + 20, cx + 75, cy + 85, cx + 20, cy + 70, INK_BLACK);
  drawScreentone(cx - 70, cy + 30, 40, 50, 3);
  drawScreentone(cx + 30, cy + 30, 40, 50, 3);

  // Head
  screen.tft.fillCircle(cx, cy - 20, 38, INK_WHITE);
  screen.tft.drawCircle(cx, cy - 20, 38, INK_BLACK);
  screen.tft.drawCircle(cx, cy - 20, 37, INK_BLACK);
  // Head reflection highlight
  screen.tft.drawLine(cx - 20, cy - 35, cx - 10, cy - 44, INK_BLACK);
  screen.tft.drawLine(cx - 10, cy - 44, cx, cy - 44, INK_BLACK);

  // Classic Deadpan Face
  screen.tft.drawFastHLine(cx - 18, cy - 18, 12, INK_BLACK);
  screen.tft.drawFastHLine(cx + 6, cy - 18, 12, INK_BLACK);
  screen.tft.fillCircle(cx - 12, cy - 14, 2, INK_BLACK);
  screen.tft.fillCircle(cx + 12, cy - 14, 2, INK_BLACK);
  screen.tft.drawPixel(cx, cy - 4, INK_BLACK);
  screen.tft.drawFastHLine(cx - 8, cy + 6, 16, INK_BLACK);

  // Zipper & Suit
  screen.tft.drawLine(cx - 20, cy + 18, cx - 35, cy + 60, INK_BLACK);
  screen.tft.drawLine(cx + 20, cy + 18, cx + 35, cy + 60, INK_BLACK);
  screen.tft.drawFastVLine(cx, cy + 18, 42, INK_BLACK);

  // Japanese Sound Effect on Cover
  drawSFX(20, 195, "ド ン !", 3);

  // Author Credit Box
  screen.tft.drawRect(16, 235, SCREEN_W - 32, 28, INK_BLACK);
  screen.tft.setCursor(24, 241);
  screen.tft.print("STORY & ART: ANKUR608");
  screen.tft.setCursor(24, 251);
  screen.tft.print("ARDUINO UNO Q MANGA READER");

  // Tap to Read Button
  screen.tft.fillRoundRect(24, 274, SCREEN_W - 48, 30, 6, INK_BLACK);
  screen.tft.setTextColor(INK_WHITE);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(55, 285);
  screen.tft.print("TAP SCREEN TO OPEN ISSUE");
}

// ==============================================================================
// PAGE 1: CITY Z IN PERIL
// ==============================================================================
void drawPage1_CityInPeril() {
  triggerEInkFlash();

  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(8, 6);
  screen.tft.print("CH. 1: THE DISASTER");
  screen.tft.drawFastHLine(6, 17, SCREEN_W - 12, INK_BLACK);

  // Panel 1: Destroyed City Skyline (Y: 22 to 140)
  screen.tft.drawRect(8, 22, SCREEN_W - 16, 118, INK_BLACK);
  screen.tft.drawRect(9, 23, SCREEN_W - 18, 116, INK_BLACK);

  // Skyline Buildings with broken crumbling silhouettes
  screen.tft.fillRect(14, 60, 24, 78, INK_BLACK);
  screen.tft.fillRect(44, 45, 30, 93, INK_BLACK);
  // Broken tilted building
  screen.tft.fillTriangle(80, 50, 110, 70, 95, 138, INK_BLACK);
  screen.tft.fillRect(115, 75, 28, 63, INK_BLACK);
  screen.tft.fillRect(148, 40, 36, 98, INK_BLACK);
  screen.tft.fillRect(190, 65, 32, 73, INK_BLACK);

  // Smoke & Cloud Plumes (Screentone)
  drawScreentone(12, 25, SCREEN_W - 24, 30, 3);
  drawSFX(20, 28, "*WEE-OOO-WEE!*", 1);

  // Narration Box (Top Left)
  screen.tft.fillRect(14, 26, 120, 26, INK_WHITE);
  screen.tft.drawRect(14, 26, 120, 26, INK_BLACK);
  screen.tft.setCursor(18, 30);
  screen.tft.print("CITY Z - GHOST TOWN");
  screen.tft.setCursor(18, 40);
  screen.tft.print("EMERGENCY LEVEL: DRAGON");

  // Panel 2: Fleeing Civilians (Y: 148 to 225)
  screen.tft.drawRect(8, 148, SCREEN_W - 16, 77, INK_BLACK);
  screen.tft.drawRect(9, 149, SCREEN_W - 18, 75, INK_BLACK);

  // Speed lines of running crowd
  for (int x = 12; x < SCREEN_W - 15; x += 14) {
    screen.tft.drawLine(x, 210, x + 25, 155, INK_BLACK);
  }

  // Fleeing Silhouette Figures
  screen.tft.fillCircle(45, 172, 8, INK_BLACK);
  screen.tft.fillTriangle(35, 215, 45, 180, 55, 215, INK_BLACK);
  screen.tft.fillCircle(85, 168, 8, INK_BLACK);
  screen.tft.fillTriangle(75, 215, 85, 176, 95, 215, INK_BLACK);

  // Panic Speech Bubble
  drawSpeechBubble(110, 155, 115, 46, 1);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setCursor(116, 163);
  screen.tft.print("RUN FOR YOUR LIVES!");
  screen.tft.setCursor(116, 175);
  screen.tft.print("IT CRUSHED CITY A");
  screen.tft.setCursor(116, 187);
  screen.tft.print("WITH ONE FOOTSTEP!!");

  // Panel 3: Ground Shaking (Y: 232 to 298)
  screen.tft.drawRect(8, 232, SCREEN_W - 16, 66, INK_BLACK);
  screen.tft.drawRect(9, 233, SCREEN_W - 18, 64, INK_BLACK);

  // Earthquake Jagged Fault Lines
  int ey = 265;
  for (int x = 12; x < SCREEN_W - 20; x += 18) {
    screen.tft.drawLine(x, ey, x + 9, ey - 8, INK_BLACK);
    screen.tft.drawLine(x + 9, ey - 8, x + 18, ey + 8, INK_BLACK);
  }
  drawSFX(25, 240, "ズ ズ ズ", 2);
  drawSFX(120, 240, "*THUUUUUM!*", 2);

  drawPageFooter(1);
}

// ==============================================================================
// PAGE 2: THE COLOSSAL TITAN
// ==============================================================================
void drawPage2_TheTitan() {
  triggerEInkFlash();

  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(8, 6);
  screen.tft.print("CH. 1: THE APEX PREDATOR");
  screen.tft.drawFastHLine(6, 17, SCREEN_W - 12, INK_BLACK);

  // Massive Full Panel of Looming Titan (Y: 22 to 298)
  screen.tft.drawRect(8, 22, SCREEN_W - 16, 276, INK_BLACK);
  screen.tft.drawRect(9, 23, SCREEN_W - 18, 274, INK_BLACK);

  // Heavy Dark Menace Background
  drawScreentone(10, 24, SCREEN_W - 20, 160, 3);

  // Titan Head Silhouette (Massive Horns & Shoulders)
  int tx = 120, ty = 90;
  screen.tft.fillCircle(tx, ty, 42, INK_BLACK);
  // Giant Demon Horns
  screen.tft.fillTriangle(tx - 35, ty - 20, tx - 70, ty - 60, tx - 15, ty - 38, INK_BLACK);
  screen.tft.fillTriangle(tx + 35, ty - 20, tx + 70, ty - 60, tx + 15, ty - 38, INK_BLACK);

  // Glowing Slit White Eyes (Terrifying contrast)
  screen.tft.fillTriangle(tx - 28, ty - 8, tx - 10, ty - 4, tx - 24, ty + 2, INK_WHITE);
  screen.tft.fillTriangle(tx + 28, ty - 8, tx + 10, ty - 4, tx + 24, ty + 2, INK_WHITE);

  // Jagged Demon Mouth with huge fangs
  screen.tft.fillRect(tx - 26, ty + 12, 52, 14, INK_WHITE);
  for (int fx = tx - 24; fx < tx + 24; fx += 8) {
    screen.tft.fillTriangle(fx, ty + 12, fx + 4, ty + 20, fx + 8, ty + 12, INK_BLACK);
    screen.tft.fillTriangle(fx, ty + 26, fx + 4, ty + 18, fx + 8, ty + 26, INK_BLACK);
  }

  // Giant Muscular Chest & Veins
  screen.tft.fillRect(20, 132, SCREEN_W - 40, 164, INK_BLACK);
  // White highlight lines for pecs and abs
  screen.tft.drawFastHLine(50, 175, 140, INK_WHITE);
  screen.tft.drawFastVLine(tx, 140, 140, INK_WHITE);
  screen.tft.drawFastHLine(65, 215, 110, INK_WHITE);
  screen.tft.drawFastHLine(75, 255, 90, INK_WHITE);

  // Spiky Roaring Monster Dialogue Box
  drawSpeechBubble(20, 160, 200, 72, 1);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(28, 168);
  screen.tft.print("INSECTS OF HUMANITY!");
  screen.tft.setCursor(28, 180);
  screen.tft.print("I WAS BORN FROM POLLUTION!");
  screen.tft.setCursor(28, 192);
  screen.tft.print("MY BROTHER & I INVENTED");
  screen.tft.setCursor(28, 204);
  screen.tft.print("THE ULTIMATE MUSCLE STEROID!");
  screen.tft.setCursor(28, 216);
  screen.tft.print("I WILL CRUSH THIS WORLD!!");

  drawSFX(25, 245, "ゴ ゴ ゴ *ROAAAR!*", 2);

  drawPageFooter(2);
}

// ==============================================================================
// PAGE 3: ENTER THE HERO (GROCERY BAGS)
// ==============================================================================
void drawPage3_EnterHero() {
  triggerEInkFlash();

  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(8, 6);
  screen.tft.print("CH. 1: HERO FOR FUN");
  screen.tft.drawFastHLine(6, 17, SCREEN_W - 12, INK_BLACK);

  // Panel 1: Saitama Walking in Ruined Street (Y: 22 to 155)
  screen.tft.drawRect(8, 22, SCREEN_W - 16, 133, INK_BLACK);
  screen.tft.drawRect(9, 23, SCREEN_W - 18, 131, INK_BLACK);

  // Background Rubble
  drawScreentone(12, 25, SCREEN_W - 24, 45, 4);

  // Saitama Walking Figure holding Grocery Bag
  int sx = 80, sy = 90;
  // Head
  screen.tft.fillCircle(sx, sy - 28, 16, INK_WHITE);
  screen.tft.drawCircle(sx, sy - 28, 16, INK_BLACK);
  screen.tft.fillCircle(sx - 5, sy - 27, 2, INK_BLACK);
  screen.tft.fillCircle(sx + 5, sy - 27, 2, INK_BLACK);
  screen.tft.drawFastHLine(sx - 4, sy - 18, 8, INK_BLACK);

  // Body & Cape
  screen.tft.fillTriangle(sx - 15, sy - 12, sx - 28, sy + 35, sx - 6, sy + 25, INK_BLACK); // Cape
  screen.tft.drawRect(sx - 8, sy - 12, 16, 32, INK_BLACK); // Torso
  screen.tft.drawLine(sx - 6, sy + 20, sx - 12, sy + 55, INK_BLACK); // Left leg
  screen.tft.drawLine(sx + 6, sy + 20, sx + 12, sy + 55, INK_BLACK); // Right leg

  // Grocery Plastic Bag with Long Green Leek / Onion sticking out!
  screen.tft.drawRect(sx + 10, sy + 5, 14, 18, INK_BLACK);
  screen.tft.drawLine(sx + 14, sy + 5, sx + 24, sy - 16, INK_BLACK); // Leek stalk
  screen.tft.fillCircle(sx + 24, sy - 16, 3, INK_BLACK); // Leek tip

  // Saitama Dialogue
  drawSpeechBubble(115, 32, 110, 56, 1);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(120, 40);
  screen.tft.print("Man, what's with");
  screen.tft.setCursor(120, 52);
  screen.tft.print("all the noise?");
  screen.tft.setCursor(120, 64);
  screen.tft.print("I was just buying");
  screen.tft.setCursor(120, 76);
  screen.tft.print("dinner ingredients.");

  // Panel 2: Monster Looking Down Shocked (Y: 162 to 298)
  screen.tft.drawRect(8, 162, SCREEN_W - 16, 136, INK_BLACK);
  screen.tft.drawRect(9, 163, SCREEN_W - 18, 134, INK_BLACK);

  // Monster Giant Eye looking down
  screen.tft.drawCircle(45, 205, 30, INK_BLACK);
  screen.tft.fillCircle(45, 205, 14, INK_BLACK);
  screen.tft.fillCircle(48, 202, 4, INK_WHITE); // Reflection

  // Monster Dialogue
  drawSpeechBubble(85, 172, 140, 50, 1);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setCursor(92, 180);
  screen.tft.print("WHO ARE YOU?!");
  screen.tft.setCursor(92, 192);
  screen.tft.print("SOME KIND OF WEIRD");
  screen.tft.setCursor(92, 204);
  screen.tft.print("YELLOW SUITED BALDY?!");

  // Saitama Iconic Nonchalant Reply
  drawSpeechBubble(65, 235, 160, 44, 2);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(72, 243);
  screen.tft.print("I'm just a guy who's");
  screen.tft.setCursor(72, 255);
  screen.tft.print("a HERO FOR FUN.");

  drawPageFooter(3);
}

// ==============================================================================
// PAGE 4: THE MONOLOGUE VS. PICKING NOSE
// ==============================================================================
void drawPage4_Monologue() {
  triggerEInkFlash();

  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(8, 6);
  screen.tft.print("CH. 1: WRAP IT UP");
  screen.tft.drawFastHLine(6, 17, SCREEN_W - 12, INK_BLACK);

  // Panel 1: Villain Ranting (Y: 22 to 145)
  screen.tft.drawRect(8, 22, SCREEN_W - 16, 123, INK_BLACK);
  screen.tft.drawRect(9, 23, SCREEN_W - 18, 121, INK_BLACK);

  drawScreentone(12, 25, 80, 115, 3);
  drawSFX(14, 55, "*YAP!*", 2);
  drawSFX(14, 95, "*RANT!*", 2);

  // Enormous monologue wall of text bubble
  drawSpeechBubble(80, 28, 145, 105, 1);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(86, 36);
  screen.tft.print("YOU FOOL! LISTEN TO");
  screen.tft.setCursor(86, 48);
  screen.tft.print("MY 10-PAGE TRAGIC");
  screen.tft.setCursor(86, 60);
  screen.tft.print("BACKSTORY! MY CELL");
  screen.tft.setCursor(86, 72);
  screen.tft.print("STRUCTURE MUTATED AT");
  screen.tft.setCursor(86, 84);
  screen.tft.print("MOLECULAR VELOCITY--");
  screen.tft.setCursor(86, 96);
  screen.tft.print("I AM INVINCIBLE!");
  screen.tft.setCursor(86, 108);
  screen.tft.print("FEAR MY DARKNESS!");

  // Panel 2: Saitama Picking His Nose (Y: 152 to 298)
  screen.tft.drawRect(8, 152, SCREEN_W - 16, 146, INK_BLACK);
  screen.tft.drawRect(9, 153, SCREEN_W - 18, 144, INK_BLACK);

  // Saitama Iconic Bored Face
  int hx = 65, hy = 220;
  screen.tft.fillCircle(hx, hy, 32, INK_WHITE);
  screen.tft.drawCircle(hx, hy, 32, INK_BLACK);
  screen.tft.drawCircle(hx, hy, 31, INK_BLACK);

  // Bored half-lidded oval eyes
  screen.tft.drawFastHLine(hx - 14, hy - 8, 10, INK_BLACK);
  screen.tft.drawFastHLine(hx + 6, hy - 8, 10, INK_BLACK);
  screen.tft.fillCircle(hx - 9, hy - 5, 2, INK_BLACK);
  screen.tft.fillCircle(hx + 11, hy - 5, 2, INK_BLACK);

  // Pinky finger inside nostril!
  screen.tft.drawLine(hx + 24, hy + 28, hx + 4, hy + 4, INK_BLACK);
  screen.tft.drawLine(hx + 27, hy + 28, hx + 7, hy + 4, INK_BLACK);
  screen.tft.fillCircle(hx + 4, hy + 4, 3, INK_BLACK); // In nostril

  // Nonchalant Dialogue
  drawSpeechBubble(112, 168, 115, 60, 1);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(118, 176);
  screen.tft.print("Hey, buddy...");
  screen.tft.setCursor(118, 188);
  screen.tft.print("Can you wrap it up?");
  screen.tft.setCursor(118, 200);
  screen.tft.print("I've got miso soup");
  screen.tft.setCursor(118, 212);
  screen.tft.print("on the stove.");

  // Villain Vein Popping in Anger
  drawSFX(120, 245, "*VEIN POP!*", 1);
  drawSFX(120, 265, "WHAT?!", 2);

  drawPageFooter(4);
}

// ==============================================================================
// PAGE 5: THE SERIOUS SHIFT
// ==============================================================================
void drawPage5_SeriousShift() {
  triggerEInkFlash();

  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(8, 6);
  screen.tft.print("CH. 1: SERIOUS SHIFT");
  screen.tft.drawFastHLine(6, 17, SCREEN_W - 12, INK_BLACK);

  // Panel 1: Villain Rushing in with Giant Fist (Y: 22 to 135)
  screen.tft.drawRect(8, 22, SCREEN_W - 16, 113, INK_BLACK);
  screen.tft.drawRect(9, 23, SCREEN_W - 18, 111, INK_BLACK);

  drawSpeedLines(50, 80, 20, 95, 24);
  drawSpeechBubble(95, 32, 130, 48, 1);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setCursor(102, 40);
  screen.tft.print("DIE, INSOLENT BRAT!");
  screen.tft.setCursor(102, 52);
  screen.tft.print("ULTIMATE CRUSHING");
  screen.tft.setCursor(102, 64);
  screen.tft.print("DESTRUCTION FIST!!!");

  drawSFX(85, 92, "*WHOOOOOOSH!*", 2);

  // Panel 2: The Face Shift - Close-up Eyes (Y: 142 to 298)
  screen.tft.drawRect(8, 142, SCREEN_W - 16, 156, INK_BLACK);
  screen.tft.drawRect(9, 143, SCREEN_W - 18, 154, INK_BLACK);

  // Intense Horizontal Action Letterbox Eyes
  int ey = 205;
  drawSpeedLines(120, ey, 50, 160, 44);

  // Deep shadow across brow
  screen.tft.fillRect(20, ey - 32, SCREEN_W - 40, 22, INK_BLACK);

  // Sharp, Piercing Serious Eyes
  screen.tft.fillTriangle(60, ey - 22, 100, ey - 15, 68, ey - 12, INK_WHITE);
  screen.tft.fillCircle(82, ey - 18, 4, INK_BLACK);
  screen.tft.fillCircle(83, ey - 19, 1, INK_WHITE); // Sharp glare dot

  screen.tft.fillTriangle(180, ey - 22, 140, ey - 15, 172, ey - 12, INK_WHITE);
  screen.tft.fillCircle(158, ey - 18, 4, INK_BLACK);
  screen.tft.fillCircle(157, ey - 19, 1, INK_WHITE);

  // Forehead furrow tension lines
  screen.tft.drawFastVLine(120, ey - 44, 14, INK_BLACK);
  screen.tft.drawLine(114, ey - 42, 106, ey - 48, INK_BLACK);
  screen.tft.drawLine(126, ey - 42, 134, ey - 48, INK_BLACK);

  // Japanese Menace Onomatopoeia
  drawSFX(16, ey + 15, "ゴ ゴ ゴ", 2);
  drawSFX(160, ey + 15, "ゴ ゴ ゴ", 2);

  // The Whispered Technique Setup Banner
  screen.tft.fillRect(24, 252, SCREEN_W - 48, 28, INK_BLACK);
  screen.tft.setTextColor(INK_WHITE);
  screen.tft.setTextSize(2);
  screen.tft.setCursor(32, 258);
  screen.tft.print("SERIOUS SERIES...");

  drawPageFooter(5);
}

// ==============================================================================
// PAGE 6: THE CLIMAX — SERIOUS PUNCH!
// ==============================================================================
void drawPage6_SeriousPunch() {
  triggerEInkFlash();

  screen.tft.drawRect(4, 4, SCREEN_W - 8, SCREEN_H - 8, INK_BLACK);
  screen.tft.drawRect(5, 5, SCREEN_W - 10, SCREEN_H - 10, INK_BLACK);

  // Massive 48-Ray Radial Burst across the entire screen!
  int fx = 120, fy = 150;
  drawSpeedLines(fx, fy, 40, 180, 52);

  // Giant Forced-Perspective Red Glove Fist
  screen.tft.fillCircle(fx, fy, 46, INK_WHITE);
  screen.tft.drawCircle(fx, fy, 46, INK_BLACK);
  screen.tft.drawCircle(fx, fy, 45, INK_BLACK);
  screen.tft.drawCircle(fx, fy, 44, INK_BLACK);

  // 4 Massive Knuckles
  for (int k = -30; k <= 30; k += 20) {
    screen.tft.drawCircle(fx + k, fy - 18, 10, INK_BLACK);
    screen.tft.drawCircle(fx + k, fy - 18, 9, INK_BLACK);
  }
  // Glove creasing & screentone
  screen.tft.drawFastHLine(fx - 36, fy + 6, 72, INK_BLACK);
  drawScreentone(fx - 34, fy + 10, 68, 28, 3);

  // Shattered Air Shockwaves
  for (int r = 60; r <= 95; r += 16) {
    screen.tft.drawCircle(fx, fy, r, INK_BLACK);
  }

  // Giant Bold Impact SFX
  drawSFX(16, 22, "*KAAA-BOOOOM!*", 2);

  // Technique Banner
  screen.tft.fillRect(16, 225, SCREEN_W - 32, 42, INK_BLACK);
  screen.tft.setTextColor(INK_WHITE);
  screen.tft.setTextSize(2);
  screen.tft.setCursor(24, 232);
  screen.tft.print("SERIOUS PUNCH!");
  screen.tft.setTextSize(1);
  screen.tft.setCursor(24, 252);
  screen.tft.print("STRATOSPHERE CLOUDS PARTED!");

  drawPageFooter(6);
}

// ==============================================================================
// PAGE 7: THE REAL TRAGEDY — BRUISED CABBAGE!
// ==============================================================================
void drawPage7_Aftermath() {
  triggerEInkFlash();

  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(8, 6);
  screen.tft.print("CH. 1: THE REAL TRAGEDY");
  screen.tft.drawFastHLine(6, 17, SCREEN_W - 12, INK_BLACK);

  // Panel 1: Saitama Shocked over Bruised Cabbage (Y: 22 to 175)
  screen.tft.drawRect(8, 22, SCREEN_W - 16, 153, INK_BLACK);
  screen.tft.drawRect(9, 23, SCREEN_W - 18, 151, INK_BLACK);

  // Saitama Kneeling in Despair
  int kx = 65, ky = 100;
  screen.tft.fillCircle(kx, ky - 20, 24, INK_WHITE);
  screen.tft.drawCircle(kx, ky - 20, 24, INK_BLACK);
  // Crying cartoon stream eyes
  screen.tft.fillTriangle(kx - 10, ky - 24, kx - 14, ky - 6, kx - 6, ky - 6, INK_BLACK);
  screen.tft.fillTriangle(kx + 10, ky - 24, kx + 6, ky - 6, kx + 14, ky - 6, INK_BLACK);
  // Screaming open mouth
  screen.tft.fillRoundRect(kx - 8, ky - 4, 16, 12, 4, INK_BLACK);

  // Bruised Cabbage on Ground with Crack Lines
  int cbx = 65, cby = 138;
  screen.tft.fillCircle(cbx, cby, 14, INK_WHITE);
  screen.tft.drawCircle(cbx, cby, 14, INK_BLACK);
  screen.tft.drawLine(cbx - 8, cby - 6, cbx + 4, cby + 6, INK_BLACK); // Crack
  screen.tft.drawFastHLine(cbx - 10, cby, 12, INK_BLACK);

  // Agony Dialogue Bubble
  drawSpeechBubble(110, 32, 115, 68, 1);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(116, 40);
  screen.tft.print("NOOOOOOOOO!!");
  screen.tft.setCursor(116, 52);
  screen.tft.print("THE SPECIAL SALE");
  screen.tft.setCursor(116, 64);
  screen.tft.print("CABBAGE GOT");
  screen.tft.setCursor(116, 76);
  screen.tft.print("BRUISED ON IMPACT!");

  drawSFX(18, 145, "*DESPAIR!*", 1);

  // Panel 2: Genos Arriving in Jet Blast (Y: 182 to 298)
  screen.tft.drawRect(8, 182, SCREEN_W - 16, 116, INK_BLACK);
  screen.tft.drawRect(9, 183, SCREEN_W - 18, 114, INK_BLACK);

  // Genos Cyborg Silhouette
  int gx = 45, gy = 235;
  screen.tft.fillCircle(gx, gy - 16, 14, INK_BLACK);
  screen.tft.fillCircle(gx - 4, gy - 16, 3, INK_WHITE); // Glowing cyborg eye
  // Jet thruster smoke
  drawSpeedLines(gx, gy + 15, 8, 30, 16);

  // Genos Serious Analysis Bubble
  drawSpeechBubble(80, 190, 145, 62, 1);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(86, 198);
  screen.tft.print("MASTER SAITAMA!");
  screen.tft.setCursor(86, 210);
  screen.tft.print("I DETECTED EXTREME");
  screen.tft.setCursor(86, 222);
  screen.tft.print("ENERGY! WHAT FIEND");
  screen.tft.setCursor(86, 234);
  screen.tft.print("CAUSED SUCH DESPAIR?!");

  // Saitama Final Punchline
  drawSpeechBubble(70, 258, 155, 34, 1);
  screen.tft.setTextColor(INK_BLACK);
  screen.tft.setCursor(76, 265);
  screen.tft.print("Genos... tonight's");
  screen.tft.setCursor(76, 277);
  screen.tft.print("miso soup is ruined.");

  // Authentic Manga FIN Stamp
  screen.tft.fillRect(16, 262, 42, 26, INK_BLACK);
  screen.tft.setTextColor(INK_WHITE);
  screen.tft.setTextSize(2);
  screen.tft.setCursor(22, 268);
  screen.tft.print("FIN");

  drawPageFooter(7);
}

// ==============================================================================
// PAGE DISPATCHER
// ==============================================================================
void renderCurrentPage() {
  switch (currentPage) {
    case 0: drawCoverPage();           break;
    case 1: drawPage1_CityInPeril();   break;
    case 2: drawPage2_TheTitan();      break;
    case 3: drawPage3_EnterHero();     break;
    case 4: drawPage4_Monologue();     break;
    case 5: drawPage5_SeriousShift();  break;
    case 6: drawPage6_SeriousPunch();  break;
    case 7: drawPage7_Aftermath();     break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize in Portrait orientation (Rotation 0)
  screen.begin();
  screen.setRotation(0); // 240 x 320

  renderCurrentPage();
}

void loop() {
  if (screen.touched()) {
    TS_Point raw = screen.touch.getPoint();
    int touchX = 240 - raw.x;
    int touchY = 320 - raw.y;

    // Left edge tap goes back, anywhere else advances!
    if (touchX < 60 && currentPage > 0) {
      currentPage--;
    } else {
      currentPage = (currentPage + 1) % (TOTAL_PAGES + 1);
    }

    renderCurrentPage();
    delay(280); // Debounce page turn
  }

  delay(20);
}
