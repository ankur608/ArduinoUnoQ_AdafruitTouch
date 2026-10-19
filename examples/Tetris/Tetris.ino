/*
  Tetris.ino - Authentic 8-Bit Falling Block Arcade Game
  Part of the ArduinoUnoQ_AdafruitTouch library.
  
  Controls:
  - Touch [ < ] and [ > ] on the left to move tetrominoes left and right.
  - Touch [ ROTATE ] on the right to rotate piece 90 degrees.
  - Touch [ DROP ] on the right to soft-drop piece.
  
  Board: Arduino UNO Q (STM32U585)
  Shield: Adafruit 2.8" Capacitive TFT (PID 1947)
*/

#include <ArduinoUnoQ_AdafruitTouch.h>

UnoQ_AdafruitTouch screen;

#define SCREEN_W 320
#define SCREEN_H 240

// Matrix Configuration (Standard 10 x 20)
#define COLS 10
#define ROWS 20
#define BLOCK_SIZE 10

#define FIELD_X 110
#define FIELD_Y  20
#define FIELD_W (COLS * BLOCK_SIZE)
#define FIELD_H (ROWS * BLOCK_SIZE)

uint16_t grid[ROWS][COLS];

// Touch Button Coordinates
#define BTN_LEFT_X   12
#define BTN_LEFT_Y  145
#define BTN_LEFT_W   42
#define BTN_LEFT_H   75

#define BTN_RIGHT_X  58
#define BTN_RIGHT_Y 145
#define BTN_RIGHT_W  42
#define BTN_RIGHT_H  75

#define BTN_ROT_X   220
#define BTN_ROT_Y   145
#define BTN_ROT_W   88
#define BTN_ROT_H   35

#define BTN_DROP_X  220
#define BTN_DROP_Y  185
#define BTN_DROP_W  88
#define BTN_DROP_H   35

// 7 Official Tetrominoes (4x4 bitmasks)
const uint16_t TETROMINOES[7][4] = {
  // I (Cyan)
  { 0x0F00, 0x2222, 0x00F0, 0x4444 },
  // O (Yellow)
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
  ILI9341_CYAN,
  ILI9341_YELLOW,
  0xB1DF, // Purple
  ILI9341_GREEN,
  ILI9341_RED,
  ILI9341_BLUE,
  ILI9341_ORANGE
};

int curPiece, curRotation, curX, curY;
int nextPiece;

int score = 0;
int lines = 0;
int level = 1;
bool gameOver = false;

unsigned long lastDropTime = 0;
unsigned long dropInterval = 650; // Milliseconds per step

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

void drawBlock(int gx, int gy, uint16_t color) {
  int x = FIELD_X + gx * BLOCK_SIZE;
  int y = FIELD_Y + gy * BLOCK_SIZE;
  if (color == 0) {
    screen.tft.fillRect(x, y, BLOCK_SIZE, BLOCK_SIZE, ILI9341_BLACK);
  } else {
    screen.tft.fillRect(x, y, BLOCK_SIZE, BLOCK_SIZE, color);
    screen.tft.drawRect(x, y, BLOCK_SIZE, BLOCK_SIZE, 0x2104);
  }
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
          drawBlock(gx, gy, col);
        }
      }
    }
  }
}

void drawSidebar() {
  // Score & Level
  screen.tft.fillRect(215, 20, 95, 115, ILI9341_BLACK);
  screen.tft.drawRoundRect(215, 20, 95, 115, 6, 0x4208);

  screen.tft.setTextColor(ILI9341_YELLOW);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(224, 28);
  screen.tft.print("NEXT");

  // Next piece preview
  uint16_t mask = TETROMINOES[nextPiece][0];
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      int nx = 230 + c * 8;
      int ny = 44 + r * 8;
      if (mask & (0x8000 >> (r * 4 + c))) {
        screen.tft.fillRect(nx, ny, 7, 7, PIECE_COLORS[nextPiece]);
      }
    }
  }

  screen.tft.setTextColor(ILI9341_WHITE);
  screen.tft.setCursor(224, 82);
  screen.tft.print("SCORE");
  screen.tft.setCursor(224, 94);
  screen.tft.print(score);

  screen.tft.setCursor(224, 110);
  screen.tft.print("LINES: ");
  screen.tft.print(lines);
}

void drawButtons() {
  // Left Button
  screen.tft.fillRoundRect(BTN_LEFT_X, BTN_LEFT_Y, BTN_LEFT_W, BTN_LEFT_H, 6, 0x18C3);
  screen.tft.drawRoundRect(BTN_LEFT_X, BTN_LEFT_Y, BTN_LEFT_W, BTN_LEFT_H, 6, ILI9341_WHITE);
  screen.tft.setTextColor(ILI9341_WHITE);
  screen.tft.setTextSize(3);
  screen.tft.setCursor(BTN_LEFT_X + 12, BTN_LEFT_Y + 25);
  screen.tft.print("<");

  // Right Button
  screen.tft.fillRoundRect(BTN_RIGHT_X, BTN_RIGHT_Y, BTN_RIGHT_W, BTN_RIGHT_H, 6, 0x18C3);
  screen.tft.drawRoundRect(BTN_RIGHT_X, BTN_RIGHT_Y, BTN_RIGHT_W, BTN_RIGHT_H, 6, ILI9341_WHITE);
  screen.tft.setCursor(BTN_RIGHT_X + 14, BTN_RIGHT_Y + 25);
  screen.tft.print(">");

  // Rotate Button
  screen.tft.fillRoundRect(BTN_ROT_X, BTN_ROT_Y, BTN_ROT_W, BTN_ROT_H, 6, 0x2A69);
  screen.tft.drawRoundRect(BTN_ROT_X, BTN_ROT_Y, BTN_ROT_W, BTN_ROT_H, 6, ILI9341_WHITE);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(BTN_ROT_X + 22, BTN_ROT_Y + 13);
  screen.tft.print("ROTATE");

  // Drop Button
  screen.tft.fillRoundRect(BTN_DROP_X, BTN_DROP_Y, BTN_DROP_W, BTN_DROP_H, 6, 0x8000);
  screen.tft.drawRoundRect(BTN_DROP_X, BTN_DROP_Y, BTN_DROP_W, BTN_DROP_H, 6, ILI9341_WHITE);
  screen.tft.setCursor(BTN_DROP_X + 28, BTN_DROP_Y + 13);
  screen.tft.print("DROP");
}

void spawnPiece() {
  curPiece = nextPiece;
  nextPiece = random(0, 7);
  curRotation = 0;
  curX = (COLS / 2) - 2;
  curY = 0;

  drawSidebar();

  if (checkCollision(curPiece, curRotation, curX, curY)) {
    gameOver = true;
  } else {
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
      // Shift rows down
      for (int y = r; y > 0; y--) {
        for (int c = 0; c < COLS; c++) {
          grid[y][c] = grid[y - 1][c];
        }
      }
      for (int c = 0; c < COLS; c++) grid[0][c] = 0;
      r++; // Recheck this row
    }
  }

  if (linesCleared > 0) {
    lines += linesCleared;
    score += (linesCleared == 1) ? 100 : (linesCleared == 2) ? 300 : (linesCleared == 3) ? 500 : 800;
    if (dropInterval > 150) dropInterval -= linesCleared * 15;

    // Full board redraw
    for (int r = 0; r < ROWS; r++) {
      for (int c = 0; c < COLS; c++) {
        drawBlock(c, r, grid[r][c]);
      }
    }
    drawSidebar();
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

void setup() {
  Serial.begin(115200);
  delay(100);

  screen.begin();
  randomSeed(analogRead(0) + millis());

  screen.tft.fillScreen(ILI9341_BLACK);

  // Field Outline
  screen.tft.drawRect(FIELD_X - 2, FIELD_Y - 2, FIELD_W + 4, FIELD_H + 4, ILI9341_WHITE);

  // Clear Matrix
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) grid[r][c] = 0;
  }

  nextPiece = random(0, 7);
  drawButtons();
  spawnPiece();
}

void loop() {
  // 1. Touch Controls
  if (screen.touched()) {
    TS_Point p = screen.getPoint();

    if (gameOver) {
      gameOver = false;
      score = 0;
      lines = 0;
      dropInterval = 650;
      setup();
      return;
    }

    // Move Left
    if (p.x >= BTN_LEFT_X && p.x <= BTN_LEFT_X + BTN_LEFT_W &&
        p.y >= BTN_LEFT_Y && p.y <= BTN_LEFT_Y + BTN_LEFT_H) {
      if (!checkCollision(curPiece, curRotation, curX - 1, curY)) {
        drawPiece(curPiece, curRotation, curX, curY, true);
        curX--;
        drawPiece(curPiece, curRotation, curX, curY, false);
      }
      delay(90);
    }
    // Move Right
    else if (p.x >= BTN_RIGHT_X && p.x <= BTN_RIGHT_X + BTN_RIGHT_W &&
             p.y >= BTN_RIGHT_Y && p.y <= BTN_RIGHT_Y + BTN_RIGHT_H) {
      if (!checkCollision(curPiece, curRotation, curX + 1, curY)) {
        drawPiece(curPiece, curRotation, curX, curY, true);
        curX++;
        drawPiece(curPiece, curRotation, curX, curY, false);
      }
      delay(90);
    }
    // Rotate
    else if (p.x >= BTN_ROT_X && p.x <= BTN_ROT_X + BTN_ROT_W &&
             p.y >= BTN_ROT_Y && p.y <= BTN_ROT_Y + BTN_ROT_H) {
      int nextRot = (curRotation + 1) % 4;
      if (!checkCollision(curPiece, nextRot, curX, curY)) {
        drawPiece(curPiece, curRotation, curX, curY, true);
        curRotation = nextRot;
        drawPiece(curPiece, curRotation, curX, curY, false);
      }
      delay(120);
    }
    // Soft Drop
    else if (p.x >= BTN_DROP_X && p.x <= BTN_DROP_X + BTN_DROP_W &&
             p.y >= BTN_DROP_Y && p.y <= BTN_DROP_Y + BTN_DROP_H) {
      if (!checkCollision(curPiece, curRotation, curX, curY + 1)) {
        drawPiece(curPiece, curRotation, curX, curY, true);
        curY++;
        drawPiece(curPiece, curRotation, curX, curY, false);
      }
      delay(50);
    }
  }

  // 2. Gravity Step
  if (!gameOver && millis() - lastDropTime >= dropInterval) {
    if (!checkCollision(curPiece, curRotation, curX, curY + 1)) {
      drawPiece(curPiece, curRotation, curX, curY, true);
      curY++;
      drawPiece(curPiece, curRotation, curX, curY, false);
    } else {
      lockPiece();
    }
    lastDropTime = millis();
  }

  // 3. Game Over Screen
  if (gameOver) {
    screen.tft.fillRect(FIELD_X - 10, FIELD_Y + 70, FIELD_W + 20, 50, 0x10A2);
    screen.tft.drawRect(FIELD_X - 10, FIELD_Y + 70, FIELD_W + 20, 50, ILI9341_WHITE);
    screen.tft.setTextColor(ILI9341_RED);
    screen.tft.setTextSize(2);
    screen.tft.setCursor(FIELD_X + 4, FIELD_Y + 80);
    screen.tft.print("GAME OVER");

    screen.tft.setTextColor(ILI9341_WHITE);
    screen.tft.setTextSize(1);
    screen.tft.setCursor(FIELD_X + 5, FIELD_Y + 104);
    screen.tft.print("Tap to Restart");
  }

  delay(15);
}
