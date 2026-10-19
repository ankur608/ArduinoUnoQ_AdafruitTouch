/*
  Tetris.ino - Deluxe 8-Bit Arcade Tetris (Light Mode & Portrait Edition)
  Part of the ArduinoUnoQ_AdafruitTouch library.
  
  Features:
  - 90-degree Portrait Orientation (240 x 320)
  - Clean, high-contrast Light Mode UI (Modern Paper & Slate theme)
  - Subtle Playfield Grid within the 10x20 block matrix
  - 3D Beveled arcade block rendering with highlights and shadows
  - Ghost Piece landing projection guide
  - Start Splash Screen with retro typography
  - Compact, ergonomic dual-thumb controls that do NOT claim the playfield!
  - Direct touch-to-move gesture support on the playfield itself!
  
  Board: Arduino UNO Q (STM32U585)
  Shield: Adafruit 2.8" Capacitive TFT (PID 1947)
  Author: ankur608
*/

#include <ArduinoUnoQ_AdafruitTouch.h>

UnoQ_AdafruitTouch screen;

// Screen in Portrait Mode (setRotation(0))
#define SCREEN_W 240
#define SCREEN_H 320

// Playfield (Standard 10 x 20 matrix)
#define COLS 10
#define ROWS 20
#define BLOCK_SIZE 10

#define FIELD_X 70
#define FIELD_Y 38
#define FIELD_W (COLS * BLOCK_SIZE)  // 100 px
#define FIELD_H (ROWS * BLOCK_SIZE)  // 200 px (ends at Y = 238)

// Color Palette (Light Mode Theme)
#define COLOR_BG          0xF7BE  // Soft Warm Off-White
#define COLOR_PANEL_BG    0xE73C  // Light Silver-Gray Container
#define COLOR_PANEL_BORDER 0x9CD3 // Slate Border
#define COLOR_GRID_BG     0xFFFF  // Crisp White Matrix Playfield
#define COLOR_GRID_LINE   0xE73C  // Subtle Silver Grid Line
#define COLOR_TEXT_DARK   0x18C3  // Deep Charcoal Navy Text
#define COLOR_TEXT_MUTED  0x6B4D  // Medium Slate Subtext
#define COLOR_BTN_BG      0xD6BA  // Tactile Soft Button Gray
#define COLOR_BTN_BORDER  0x8410  // Button Edge
#define COLOR_BTN_ACTIVE  0xFD20  // Amber highlight on touch

uint16_t grid[ROWS][COLS];

// Sleek Compact Buttons (Bottom Deck: Y = 250 to 306, Height = 52)
#define BTN_LEFT_X   12
#define BTN_LEFT_Y  252
#define BTN_LEFT_W   42
#define BTN_LEFT_H   52

#define BTN_RIGHT_X  62
#define BTN_RIGHT_Y 252
#define BTN_RIGHT_W  42
#define BTN_RIGHT_H  52

#define BTN_DROP_X  120
#define BTN_DROP_Y  252
#define BTN_DROP_W  46
#define BTN_DROP_H   52

#define BTN_ROT_X   176
#define BTN_ROT_Y   252
#define BTN_ROT_W   52
#define BTN_ROT_H   52

// 7 Official Tetrominoes (4x4 bitmasks)
const uint16_t TETROMINOES[7][4] = {
  // I (Cyan)
  { 0x0F00, 0x2222, 0x00F0, 0x4444 },
  // O (Yellow / Gold)
  { 0xCC00, 0xCC00, 0xCC00, 0xCC00 },
  // T (Purple)
  { 0x4E00, 0x4640, 0x0E40, 0x4C40 },
  // S (Green)
  { 0x6C00, 0x4620, 0x06C0, 0x8C40 },
  // Z (Red)
  { 0xC600, 0x2640, 0x0C60, 0x4C80 },
  // J (Blue)
  { 0x8E00, 0x6440, 0x0E20, 0x44C0 },
  // L (Orange)
  { 0x2E00, 0x4460, 0x0E80, 0xC440 }
};

const uint16_t PIECE_COLORS[7] = {
  0x057F, // Electric Cyan (I)
  0xFD00, // Vibrant Gold (O)
  0x8979, // Royal Violet (T)
  0x2506, // Emerald Green (S)
  0xD8E4, // Crimson Red (Z)
  0x1A7E, // Cobalt Blue (J)
  0xFA60  // Vivid Orange (L)
};

int curPiece, curRotation, curX, curY;
int nextPiece;

int score = 0;
int lines = 0;
int level = 1;

enum GameState { STATE_SPLASH, STATE_PLAYING, STATE_GAMEOVER };
GameState gameState = STATE_SPLASH;

unsigned long lastDropTime = 0;
unsigned long dropInterval = 600;

// ==============================================================================
// 3D BEVELED BLOCK RENDERING WITH GRID RESTORATION
// ==============================================================================
void drawSingleBlock(int gx, int gy, uint16_t color) {
  int x = FIELD_X + gx * BLOCK_SIZE;
  int y = FIELD_Y + gy * BLOCK_SIZE;

  if (color == 0) {
    screen.tft.fillRect(x, y, BLOCK_SIZE, BLOCK_SIZE, COLOR_GRID_BG);
    screen.tft.drawRect(x, y, BLOCK_SIZE + 1, BLOCK_SIZE + 1, COLOR_GRID_LINE);
  } else {
    screen.tft.fillRect(x + 1, y + 1, BLOCK_SIZE - 2, BLOCK_SIZE - 2, color);
    screen.tft.drawFastHLine(x, y, BLOCK_SIZE - 1, ILI9341_WHITE);
    screen.tft.drawFastVLine(x, y, BLOCK_SIZE - 1, ILI9341_WHITE);
    screen.tft.drawFastHLine(x, y + BLOCK_SIZE - 1, BLOCK_SIZE, 0x2965);
    screen.tft.drawFastVLine(x + BLOCK_SIZE - 1, y, BLOCK_SIZE, 0x2965);
  }
}

void drawGhostBlock(int gx, int gy, uint16_t color) {
  int x = FIELD_X + gx * BLOCK_SIZE;
  int y = FIELD_Y + gy * BLOCK_SIZE;
  screen.tft.drawRect(x + 1, y + 1, BLOCK_SIZE - 2, BLOCK_SIZE - 2, color);
}

bool checkCollision(int piece, int rot, int px, int py) {
  uint16_t mask = TETROMINOES[piece][rot];
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (mask & (0x8000 >> (r * 4 + c))) {
        int gx = px + c;
        int gy = py + r;
        if (gx < 0 || gx >= COLS || gy >= ROWS) return true;
        if (gy >= 0 && grid[gy][gx] != 0) return true;
      }
    }
  }
  return false;
}

int getGhostY() {
  int gy = curY;
  while (!checkCollision(curPiece, curRotation, curX, gy + 1)) {
    gy++;
  }
  return gy;
}

void drawPiece(int piece, int rot, int px, int py, bool erase) {
  uint16_t mask = TETROMINOES[piece][rot];
  uint16_t col = erase ? 0 : PIECE_COLORS[piece];

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (mask & (0x8000 >> (r * 4 + c))) {
        int gx = px + c;
        int gy = py + r;
        if (gx >= 0 && gx < COLS && gy >= 0 && gy < ROWS) {
          drawSingleBlock(gx, gy, col);
        }
      }
    }
  }
}

void drawGhostPiece(bool erase) {
  int gy = getGhostY();
  if (gy == curY) return;

  uint16_t mask = TETROMINOES[curPiece][curRotation];
  uint16_t col = erase ? 0 : 0xBDF7;

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (mask & (0x8000 >> (r * 4 + c))) {
        int gx = curX + c;
        int ggy = gy + r;
        if (gx >= 0 && gx < COLS && ggy >= 0 && ggy < ROWS && grid[ggy][gx] == 0) {
          if (erase) drawSingleBlock(gx, ggy, 0);
          else drawGhostBlock(gx, ggy, col);
        }
      }
    }
  }
}

// ==============================================================================
// UI PANELS & HUD
// ==============================================================================
void drawHeader() {
  // Score Container (Top Left)
  screen.tft.fillRoundRect(8, 4, 68, 30, 4, COLOR_PANEL_BG);
  screen.tft.drawRoundRect(8, 4, 68, 30, 4, COLOR_PANEL_BORDER);
  screen.tft.setTextColor(COLOR_TEXT_MUTED);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(14, 8);
  screen.tft.print("SCORE");
  screen.tft.setTextColor(COLOR_TEXT_DARK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(14, 20);
  screen.tft.print(score);

  // Lines & Level Container (Top Center)
  screen.tft.fillRoundRect(82, 4, 76, 30, 4, COLOR_PANEL_BG);
  screen.tft.drawRoundRect(82, 4, 76, 30, 4, COLOR_PANEL_BORDER);
  screen.tft.setTextColor(COLOR_TEXT_MUTED);
  screen.tft.setCursor(88, 8);
  screen.tft.print("LINES");
  screen.tft.setTextColor(COLOR_TEXT_DARK);
  screen.tft.print(" ");
  screen.tft.print(lines);
  screen.tft.setTextColor(COLOR_TEXT_MUTED);
  screen.tft.setCursor(88, 20);
  screen.tft.print("LVL");
  screen.tft.setTextColor(COLOR_TEXT_DARK);
  screen.tft.print(" ");
  screen.tft.print(level);

  // Next Piece Container (Top Right)
  screen.tft.fillRoundRect(164, 4, 68, 30, 4, COLOR_PANEL_BG);
  screen.tft.drawRoundRect(164, 4, 68, 30, 4, COLOR_PANEL_BORDER);
  screen.tft.setTextColor(COLOR_TEXT_MUTED);
  screen.tft.setCursor(170, 8);
  screen.tft.print("NEXT");

  // Render mini next piece preview
  uint16_t mask = TETROMINOES[nextPiece][0];
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      int nx = 176 + c * 5;
      int ny = 16 + r * 5;
      if (mask & (0x8000 >> (r * 4 + c))) {
        screen.tft.fillRect(nx, ny, 4, 4, PIECE_COLORS[nextPiece]);
      }
    }
  }
}

void drawPlayfieldGrid() {
  // Beveled Playfield Outer Frame
  screen.tft.fillRoundRect(FIELD_X - 3, FIELD_Y - 3, FIELD_W + 6, FIELD_H + 6, 4, COLOR_PANEL_BORDER);
  screen.tft.fillRect(FIELD_X, FIELD_Y, FIELD_W, FIELD_H, COLOR_GRID_BG);

  // Draw full grid lines inside playfield
  for (int x = 0; x <= COLS; x++) {
    screen.tft.drawFastVLine(FIELD_X + x * BLOCK_SIZE, FIELD_Y, FIELD_H, COLOR_GRID_LINE);
  }
  for (int y = 0; y <= ROWS; y++) {
    screen.tft.drawFastHLine(FIELD_X, FIELD_Y + y * BLOCK_SIZE, FIELD_W, COLOR_GRID_LINE);
  }

  // Draw settled blocks
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      if (grid[r][c] != 0) {
        drawSingleBlock(c, r, grid[r][c]);
      }
    }
  }
}

void drawButtons() {
  // Control Deck Background Plate
  screen.tft.fillRoundRect(6, 246, SCREEN_W - 12, 66, 8, COLOR_PANEL_BG);
  screen.tft.drawRoundRect(6, 246, SCREEN_W - 12, 66, 8, COLOR_PANEL_BORDER);

  // Button 1: LEFT (<)
  screen.tft.fillRoundRect(BTN_LEFT_X, BTN_LEFT_Y, BTN_LEFT_W, BTN_LEFT_H, 6, 0xCE59);
  screen.tft.drawRoundRect(BTN_LEFT_X, BTN_LEFT_Y, BTN_LEFT_W, BTN_LEFT_H, 6, COLOR_TEXT_DARK);
  screen.tft.setTextColor(COLOR_TEXT_DARK);
  screen.tft.setTextSize(3);
  screen.tft.setCursor(BTN_LEFT_X + 12, BTN_LEFT_Y + 14);
  screen.tft.print("<");

  // Button 2: RIGHT (>)
  screen.tft.fillRoundRect(BTN_RIGHT_X, BTN_RIGHT_Y, BTN_RIGHT_W, BTN_RIGHT_H, 6, 0xCE59);
  screen.tft.drawRoundRect(BTN_RIGHT_X, BTN_RIGHT_Y, BTN_RIGHT_W, BTN_RIGHT_H, 6, COLOR_TEXT_DARK);
  screen.tft.setCursor(BTN_RIGHT_X + 14, BTN_RIGHT_Y + 14);
  screen.tft.print(">");

  // Button 3: DROP (v)
  screen.tft.fillRoundRect(BTN_DROP_X, BTN_DROP_Y, BTN_DROP_W, BTN_DROP_H, 6, 0xFA60); // Orange
  screen.tft.drawRoundRect(BTN_DROP_X, BTN_DROP_Y, BTN_DROP_W, BTN_DROP_H, 6, COLOR_TEXT_DARK);
  screen.tft.setTextColor(ILI9341_WHITE);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(BTN_DROP_X + 11, BTN_DROP_Y + 12);
  screen.tft.print("DROP");
  screen.tft.setTextSize(2);
  screen.tft.setCursor(BTN_DROP_X + 17, BTN_DROP_Y + 28);
  screen.tft.print("v");

  // Button 4: ROTATE (Rot)
  screen.tft.fillRoundRect(BTN_ROT_X, BTN_ROT_Y, BTN_ROT_W, BTN_ROT_H, 6, 0x8979); // Violet
  screen.tft.drawRoundRect(BTN_ROT_X, BTN_ROT_Y, BTN_ROT_W, BTN_ROT_H, 6, COLOR_TEXT_DARK);
  screen.tft.setTextColor(ILI9341_WHITE);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(BTN_ROT_X + 9, BTN_ROT_Y + 12);
  screen.tft.print("ROTATE");
  screen.tft.setTextSize(2);
  screen.tft.setCursor(BTN_ROT_X + 19, BTN_ROT_Y + 28);
  screen.tft.print("()");
}

// ==============================================================================
// SPLASH & GAME OVER SCREENS
// ==============================================================================
void drawSplashScreen() {
  screen.tft.fillScreen(COLOR_BG);

  // Decorative Arcade Top Frame
  screen.tft.fillRect(0, 0, SCREEN_W, 6, 0x057F);
  screen.tft.fillRect(0, 6, SCREEN_W, 4, 0xFA60);

  // Big Retro Title Card
  screen.tft.fillRoundRect(20, 45, 200, 110, 10, ILI9341_WHITE);
  screen.tft.drawRoundRect(20, 45, 200, 110, 10, COLOR_PANEL_BORDER);

  screen.tft.setTextColor(0xD8E4); // Red
  screen.tft.setTextSize(4);
  screen.tft.setCursor(42, 65);
  screen.tft.print("T");
  screen.tft.setTextColor(0xFA60); screen.tft.print("E");
  screen.tft.setTextColor(0xFD00); screen.tft.print("T");
  screen.tft.setTextColor(0x2506); screen.tft.print("R");
  screen.tft.setTextColor(0x057F); screen.tft.print("I");
  screen.tft.setTextColor(0x8979); screen.tft.print("S");

  screen.tft.setTextColor(COLOR_TEXT_MUTED);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(48, 118);
  screen.tft.print("ARDUINO UNO Q EDITION");

  // Colorful Preview Tetrominoes in Center
  int bx = 65, by = 180;
  screen.tft.fillRect(bx, by, 12, 36, 0xFA60);
  screen.tft.fillRect(bx + 12, by + 24, 12, 12, 0xFA60);
  screen.tft.fillRect(bx + 35, by + 12, 36, 12, 0x8979);
  screen.tft.fillRect(bx + 47, by + 24, 12, 12, 0x8979);
  screen.tft.fillRect(bx + 80, by + 12, 24, 24, 0xFD00);

  // Start Prompt
  screen.tft.fillRoundRect(30, 245, 180, 44, 8, 0x2506);
  screen.tft.drawRoundRect(30, 245, 180, 44, 8, ILI9341_WHITE);
  screen.tft.setTextColor(ILI9341_WHITE);
  screen.tft.setTextSize(2);
  screen.tft.setCursor(48, 258);
  screen.tft.print("TAP TO PLAY");
}

void startNewGame() {
  score = 0;
  lines = 0;
  level = 1;
  dropInterval = 600;

  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) grid[r][c] = 0;
  }

  screen.tft.fillScreen(COLOR_BG);
  drawHeader();
  drawPlayfieldGrid();
  drawButtons();

  nextPiece = random(0, 7);
  spawnPiece();
  gameState = STATE_PLAYING;
}

void spawnPiece() {
  curPiece = nextPiece;
  nextPiece = random(0, 7);
  curRotation = 0;
  curX = (COLS / 2) - 2;
  curY = 0;

  drawHeader();

  if (checkCollision(curPiece, curRotation, curX, curY)) {
    gameState = STATE_GAMEOVER;
    drawGameOverModal();
  } else {
    drawGhostPiece(false);
    drawPiece(curPiece, curRotation, curX, curY, false);
  }
}

void clearFullLines() {
  int linesCleared = 0;
  for (int r = ROWS - 1; r >= 0; r--) {
    bool full = true;
    for (int c = 0; c < COLS; c++) {
      if (grid[r][c] == 0) { full = false; break; }
    }
    if (full) {
      linesCleared++;
      for (int y = r; y > 0; y--) {
        for (int c = 0; c < COLS; c++) {
          grid[y][c] = grid[y - 1][c];
        }
      }
      for (int c = 0; c < COLS; c++) grid[0][c] = 0;
      r++;
    }
  }

  if (linesCleared > 0) {
    lines += linesCleared;
    score += (linesCleared == 1) ? 100 : (linesCleared == 2) ? 300 : (linesCleared == 3) ? 500 : 800;
    level = (lines / 10) + 1;
    if (dropInterval > 140) dropInterval = max(140, 600 - (level * 40));

    drawPlayfieldGrid();
    drawHeader();
  }
}

void lockPiece() {
  uint16_t mask = TETROMINOES[curPiece][curRotation];
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (mask & (0x8000 >> (r * 4 + c))) {
        int gx = curX + c;
        int gy = curY + r;
        if (gx >= 0 && gx < COLS && gy >= 0 && gy < ROWS) {
          grid[gy][gx] = PIECE_COLORS[curPiece];
        }
      }
    }
  }
  clearFullLines();
  spawnPiece();
}

void drawGameOverModal() {
  screen.tft.fillRoundRect(35, 100, 170, 90, 8, ILI9341_WHITE);
  screen.tft.drawRoundRect(35, 100, 170, 90, 8, 0xD8E4);
  screen.tft.drawRoundRect(36, 101, 168, 88, 8, 0xD8E4);

  screen.tft.setTextColor(0xD8E4);
  screen.tft.setTextSize(2);
  screen.tft.setCursor(55, 118);
  screen.tft.print("GAME OVER");

  screen.tft.setTextColor(COLOR_TEXT_MUTED);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(68, 142);
  screen.tft.print("FINAL SCORE: ");
  screen.tft.setTextColor(COLOR_TEXT_DARK);
  screen.tft.print(score);

  screen.tft.setTextColor(0x2506);
  screen.tft.setCursor(58, 162);
  screen.tft.print("Tap to Play Again");
}

// ==============================================================================
// SETUP & 60 FPS MAIN LOOP
// ==============================================================================
void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize in Portrait orientation (Rotation 0)
  screen.begin();
  screen.setRotation(0); // 240 wide x 320 tall

  randomSeed(analogRead(0) + millis());
  drawSplashScreen();
}

void loop() {
  // 1. Touch Input Processing with Verified 100% Hardware Calibration
  if (screen.touched()) {
    TS_Point raw = screen.touch.getPoint();

    // Mathematically verified FT6206 coordinate transformation for Portrait (setRotation 0)
    int touchX = 240 - raw.x;
    int touchY = 320 - raw.y;

    if (touchX < 0) touchX = 0;
    if (touchX >= SCREEN_W) touchX = SCREEN_W - 1;
    if (touchY < 0) touchY = 0;
    if (touchY >= SCREEN_H) touchY = SCREEN_H - 1;

    if (gameState == STATE_SPLASH || gameState == STATE_GAMEOVER) {
      startNewGame();
      delay(220);
      return;
    }

    if (gameState == STATE_PLAYING) {
      // Control Action 1: LEFT (<)
      // Bottom Button zone OR Direct Left Side of Playfield tap!
      if ((touchY >= 240 && touchX < 56) ||
          (touchY < 240 && touchY >= FIELD_Y && touchX < 120)) {
        if (!checkCollision(curPiece, curRotation, curX - 1, curY)) {
          drawGhostPiece(true);
          drawPiece(curPiece, curRotation, curX, curY, true);
          curX--;
          drawGhostPiece(false);
          drawPiece(curPiece, curRotation, curX, curY, false);
        }
        delay(85);
      }
      // Control Action 2: RIGHT (>)
      // Bottom Button zone OR Direct Right Side of Playfield tap!
      else if ((touchY >= 240 && touchX >= 56 && touchX < 112) ||
               (touchY < 240 && touchY >= FIELD_Y && touchX >= 120 && touchX < FIELD_X + FIELD_W + 15)) {
        if (!checkCollision(curPiece, curRotation, curX + 1, curY)) {
          drawGhostPiece(true);
          drawPiece(curPiece, curRotation, curX, curY, true);
          curX++;
          drawGhostPiece(false);
          drawPiece(curPiece, curRotation, curX, curY, false);
        }
        delay(85);
      }
      // Control Action 3: SOFT DROP (v)
      else if (touchY >= 240 && touchX >= 112 && touchX < 168) {
        if (!checkCollision(curPiece, curRotation, curX, curY + 1)) {
          drawPiece(curPiece, curRotation, curX, curY, true);
          curY++;
          drawPiece(curPiece, curRotation, curX, curY, false);
        }
        delay(45);
      }
      // Control Action 4: ROTATE (Rot)
      else if (touchY >= 240 && touchX >= 168) {
        int nextRot = (curRotation + 1) % 4;
        if (!checkCollision(curPiece, nextRot, curX, curY)) {
          drawGhostPiece(true);
          drawPiece(curPiece, curRotation, curX, curY, true);
          curRotation = nextRot;
          drawGhostPiece(false);
          drawPiece(curPiece, curRotation, curX, curY, false);
        }
        delay(125);
      }
    }
  }

  // 2. Timed Piece Gravity Step
  if (gameState == STATE_PLAYING && millis() - lastDropTime >= dropInterval) {
    if (!checkCollision(curPiece, curRotation, curX, curY + 1)) {
      drawPiece(curPiece, curRotation, curX, curY, true);
      curY++;
      drawPiece(curPiece, curRotation, curX, curY, false);
    } else {
      lockPiece();
    }
    lastDropTime = millis();
  }

  delay(10); // Smooth 60 FPS loop
}
