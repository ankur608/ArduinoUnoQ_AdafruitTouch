/*
  Breakout.ino - Classic 8-Bit Arcade Brick Breaker
  Part of the ArduinoUnoQ_AdafruitTouch library.
  
  Controls:
  - Drag your finger horizontally anywhere on the screen to move the paddle.
  - Tap screen to launch the ball or restart after Game Over.
  
  Board: Arduino UNO Q (STM32U585)
  Shield: Adafruit 2.8" Capacitive TFT (PID 1947)
*/

#include <ArduinoUnoQ_AdafruitTouch.h>

UnoQ_AdafruitTouch screen;

#define SCREEN_W 320
#define SCREEN_H 240

// Brick Grid
#define BRICK_ROWS    5
#define BRICK_COLS   10
#define BRICK_W      28
#define BRICK_H      10
#define BRICK_GAP     3
#define BRICK_START_X 8
#define BRICK_START_Y 35

const uint16_t rowColors[BRICK_ROWS] = {
  ILI9341_RED,
  ILI9341_ORANGE,
  ILI9341_YELLOW,
  ILI9341_GREEN,
  ILI9341_CYAN
};

bool bricks[BRICK_ROWS][BRICK_COLS];
int bricksRemaining;

// Paddle
#define PADDLE_W  48
#define PADDLE_H   8
#define PADDLE_Y 218
float paddleX, prevPaddleX;

// Ball
#define BALL_SIZE 5
float ballX, ballY;
float prevBallX, prevBallY;
float ballVx, ballVy;
float ballSpeed;

// Game state
int score = 0;
int lives = 3;
bool ballAttached = true;
bool gameOver = false;
bool gameWon = false;

void initBricks() {
  bricksRemaining = BRICK_ROWS * BRICK_COLS;
  for (int r = 0; r < BRICK_ROWS; r++) {
    for (int c = 0; c < BRICK_COLS; c++) {
      bricks[r][c] = true;
    }
  }
}

void drawAllBricks() {
  for (int r = 0; r < BRICK_ROWS; r++) {
    for (int c = 0; c < BRICK_COLS; c++) {
      int bx = BRICK_START_X + c * (BRICK_W + BRICK_GAP);
      int by = BRICK_START_Y + r * (BRICK_H + BRICK_GAP);
      if (bricks[r][c]) {
        screen.tft.fillRect(bx, by, BRICK_W, BRICK_H, rowColors[r]);
        screen.tft.drawRect(bx, by, BRICK_W, BRICK_H, ILI9341_BLACK);
      } else {
        screen.tft.fillRect(bx, by, BRICK_W, BRICK_H, ILI9341_BLACK);
      }
    }
  }
}

void drawHUD() {
  screen.tft.fillRect(0, 0, SCREEN_W, 25, 0x10A2); // Deep navy bar
  screen.tft.drawFastHLine(0, 25, SCREEN_W, ILI9341_WHITE);

  screen.tft.setTextColor(ILI9341_WHITE);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(10, 8);
  screen.tft.print("SCORE: ");
  screen.tft.print(score);

  screen.tft.setCursor(SCREEN_W - 90, 8);
  screen.tft.print("LIVES: ");
  for (int i = 0; i < lives; i++) {
    screen.tft.fillRect(SCREEN_W - 40 + (i * 12), 8, 8, 8, ILI9341_RED);
  }
}

void resetPaddleAndBall() {
  paddleX = (SCREEN_W - PADDLE_W) / 2;
  prevPaddleX = paddleX;

  ballX = paddleX + (PADDLE_W / 2) - (BALL_SIZE / 2);
  ballY = PADDLE_Y - BALL_SIZE - 2;
  prevBallX = ballX;
  prevBallY = ballY;

  ballAttached = true;
  ballSpeed = 3.6;
}

void launchBall() {
  ballAttached = false;
  float angle = random(-40, 41) * 0.01745; // Launch upward at angle
  ballVx = sin(angle) * ballSpeed;
  ballVy = -cos(angle) * ballSpeed;
}

void setup() {
  Serial.begin(115200);
  delay(100);

  screen.begin();
  randomSeed(analogRead(0) + millis());

  screen.tft.fillScreen(ILI9341_BLACK);
  initBricks();
  drawAllBricks();
  drawHUD();
  resetPaddleAndBall();

  // Initial paddle render
  screen.tft.fillRoundRect(paddleX, PADDLE_Y, PADDLE_W, PADDLE_H, 3, ILI9341_WHITE);
  screen.tft.fillRect(ballX, ballY, BALL_SIZE, BALL_SIZE, ILI9341_YELLOW);

  screen.tft.setTextColor(ILI9341_YELLOW);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(SCREEN_W / 2 - 40, SCREEN_H / 2 + 35);
  screen.tft.print("TAP TO LAUNCH");
}

void loop() {
  // 1. Touch Input (Paddle tracking)
  if (screen.touched()) {
    TS_Point p = screen.getPoint();

    if (ballAttached && !gameOver && !gameWon) {
      launchBall();
      screen.tft.fillRect(SCREEN_W / 2 - 50, SCREEN_H / 2 + 30, 100, 20, ILI9341_BLACK);
    } else if (gameOver || gameWon) {
      score = 0;
      lives = 3;
      gameOver = false;
      gameWon = false;
      screen.tft.fillScreen(ILI9341_BLACK);
      initBricks();
      drawAllBricks();
      drawHUD();
      resetPaddleAndBall();
      return;
    }

    // Direct paddle positioning from touch X
    paddleX = p.x - (PADDLE_W / 2);
    if (paddleX < 0) paddleX = 0;
    if (paddleX > SCREEN_W - PADDLE_W) paddleX = SCREEN_W - PADDLE_W;
  }

  // 2. Ball Movement & Collision
  if (!ballAttached && !gameOver && !gameWon) {
    ballX += ballVx;
    ballY += ballVy;

    // Left / Right wall bounce
    if (ballX <= 0) {
      ballX = 0;
      ballVx = -ballVx;
    } else if (ballX >= SCREEN_W - BALL_SIZE) {
      ballX = SCREEN_W - BALL_SIZE;
      ballVx = -ballVx;
    }

    // Top wall bounce
    if (ballY <= 26) {
      ballY = 26;
      ballVy = -ballVy;
    }

    // Paddle collision
    if (ballY + BALL_SIZE >= PADDLE_Y && ballY <= PADDLE_Y + PADDLE_H) {
      if (ballX + BALL_SIZE >= paddleX && ballX <= paddleX + PADDLE_W) {
        ballY = PADDLE_Y - BALL_SIZE;
        float hitOffset = ((ballX + BALL_SIZE / 2.0) - (paddleX + PADDLE_W / 2.0)) / (PADDLE_W / 2.0);
        float bounceAngle = hitOffset * 1.05; // Up to ~60 degrees
        ballVx = sin(bounceAngle) * ballSpeed;
        ballVy = -cos(bounceAngle) * ballSpeed;
      }
    }

    // Brick collision detection
    for (int r = 0; r < BRICK_ROWS; r++) {
      for (int c = 0; c < BRICK_COLS; c++) {
        if (!bricks[r][c]) continue;

        int bx = BRICK_START_X + c * (BRICK_W + BRICK_GAP);
        int by = BRICK_START_Y + r * (BRICK_H + BRICK_GAP);

        if (ballX + BALL_SIZE >= bx && ballX <= bx + BRICK_W &&
            ballY + BALL_SIZE >= by && ballY <= by + BRICK_H) {
          // Brick destroyed!
          bricks[r][c] = false;
          bricksRemaining--;
          score += (BRICK_ROWS - r) * 10;
          drawHUD();

          // Erase brick on screen immediately
          screen.tft.fillRect(bx, by, BRICK_W, BRICK_H, ILI9341_BLACK);

          // Bounce ball
          ballVy = -ballVy;

          if (bricksRemaining <= 0) {
            gameWon = true;
          }
          goto collision_done;
        }
      }
    }
  collision_done: ;

    // Bottom loss
    if (ballY > SCREEN_H) {
      lives--;
      drawHUD();
      if (lives <= 0) {
        gameOver = true;
      } else {
        screen.tft.fillRect((int)prevBallX, (int)prevBallY, BALL_SIZE, BALL_SIZE, ILI9341_BLACK);
        resetPaddleAndBall();
        screen.tft.setTextColor(ILI9341_YELLOW);
        screen.tft.setTextSize(1);
        screen.tft.setCursor(SCREEN_W / 2 - 40, SCREEN_H / 2 + 35);
        screen.tft.print("TAP TO LAUNCH");
      }
    }
  } else if (ballAttached) {
    ballX = paddleX + (PADDLE_W / 2) - (BALL_SIZE / 2);
    ballY = PADDLE_Y - BALL_SIZE - 2;
  }

  // ============================================================================
  // TURBO DELTA RENDERING (Only redraw moved paddle & ball - 60 FPS)
  // ============================================================================
  // Erase and redraw ball
  if ((int)prevBallX != (int)ballX || (int)prevBallY != (int)ballY) {
    screen.tft.fillRect((int)prevBallX, (int)prevBallY, BALL_SIZE, BALL_SIZE, ILI9341_BLACK);
    screen.tft.fillRect((int)ballX, (int)ballY, BALL_SIZE, BALL_SIZE, ILI9341_YELLOW);
    prevBallX = ballX;
    prevBallY = ballY;
  }

  // Erase and redraw paddle
  if ((int)prevPaddleX != (int)paddleX) {
    screen.tft.fillRect((int)prevPaddleX, PADDLE_Y, PADDLE_W, PADDLE_H, ILI9341_BLACK);
    screen.tft.fillRoundRect((int)paddleX, PADDLE_Y, PADDLE_W, PADDLE_H, 3, ILI9341_WHITE);
    prevPaddleX = paddleX;
  }

  // Game Over / Victory Modal
  if (gameOver || gameWon) {
    screen.tft.fillRect(SCREEN_W / 2 - 90, SCREEN_H / 2 - 30, 180, 60, 0x10A2);
    screen.tft.drawRect(SCREEN_W / 2 - 90, SCREEN_H / 2 - 30, 180, 60, ILI9341_WHITE);

    screen.tft.setTextColor(gameWon ? ILI9341_GREEN : ILI9341_RED);
    screen.tft.setTextSize(2);
    screen.tft.setCursor(SCREEN_W / 2 - (gameWon ? 60 : 55), SCREEN_H / 2 - 18);
    screen.tft.print(gameWon ? "VICTORY!" : "GAME OVER");

    screen.tft.setTextColor(ILI9341_WHITE);
    screen.tft.setTextSize(1);
    screen.tft.setCursor(SCREEN_W / 2 - 58, SCREEN_H / 2 + 10);
    screen.tft.print("Tap to Play Again");
  }

  delay(12);
}
