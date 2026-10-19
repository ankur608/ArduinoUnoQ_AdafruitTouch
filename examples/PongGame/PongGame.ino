/*
  PongGame.ino - Retro Arcade Ping-Pong (Player vs CPU)
  Part of the ArduinoUnoQ_AdafruitTouch library.
  
  Controls:
  - Drag your finger vertically anywhere on the right half of the screen
    to position your paddle with instant 1:1 capacitive tracking!
  - Tap screen to serve or restart after game over.
  
  Board: Arduino UNO Q (STM32U585)
  Shield: Adafruit 2.8" Capacitive TFT (PID 1947)
*/

#include <ArduinoUnoQ_AdafruitTouch.h>

UnoQ_AdafruitTouch screen;

#define SCREEN_W 320
#define SCREEN_H 240

// Paddle dimensions
#define PADDLE_W   8
#define PADDLE_H  42
#define PADDLE_MARGIN 12

// Ball dimensions
#define BALL_SIZE 6

// State variables
float playerY, prevPlayerY;
float aiY, prevAiY;

float ballX, ballY;
float prevBallX, prevBallY;
float ballVx, ballVy;
float ballSpeed;

int playerScore = 0;
int aiScore = 0;
const int WIN_SCORE = 7;

bool inPlay = false;
bool gameOver = false;
String winnerText = "";

void resetBall(bool serveToPlayer) {
  ballX = SCREEN_W / 2;
  ballY = SCREEN_H / 2;
  prevBallX = ballX;
  prevBallY = ballY;

  ballSpeed = 3.2;
  float angle = random(-35, 36) * 0.01745; // -35 to +35 degrees in radians
  float dir = serveToPlayer ? 1.0 : -1.0;

  ballVx = cos(angle) * ballSpeed * dir;
  ballVy = sin(angle) * ballSpeed;
  inPlay = false;
}

void drawCourt() {
  screen.tft.fillScreen(ILI9341_BLACK);

  // Top & bottom field borders
  screen.tft.fillRect(0, 0, SCREEN_W, 3, ILI9341_WHITE);
  screen.tft.fillRect(0, SCREEN_H - 3, SCREEN_W, 3, ILI9341_WHITE);

  // Dashed center line
  for (int y = 6; y < SCREEN_H - 6; y += 14) {
    screen.tft.fillRect(SCREEN_W / 2 - 1, y, 2, 8, 0x4208);
  }

  // Draw Scores
  drawScores();
}

void drawScores() {
  screen.tft.setTextSize(3);
  screen.tft.setTextColor(0x7BEF, ILI9341_BLACK); // Dim gray
  screen.tft.setCursor(SCREEN_W / 2 - 45, 12);
  screen.tft.print(aiScore);
  screen.tft.setCursor(SCREEN_W / 2 + 25, 12);
  screen.tft.print(playerScore);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  screen.begin();
  randomSeed(analogRead(0) + millis());

  playerY = (SCREEN_H - PADDLE_H) / 2;
  prevPlayerY = playerY;
  aiY = playerY;
  prevAiY = aiY;

  drawCourt();
  resetBall(true);

  // Ready prompt
  screen.tft.setTextColor(ILI9341_YELLOW);
  screen.tft.setTextSize(2);
  screen.tft.setCursor(SCREEN_W / 2 - 70, SCREEN_H / 2 + 30);
  screen.tft.print("TAP TO SERVE");
}

void loop() {
  // 1. Touch Input for Player Paddle
  if (screen.touched()) {
    TS_Point p = screen.getPoint();

    if (!inPlay) {
      if (gameOver) {
        playerScore = 0;
        aiScore = 0;
        gameOver = false;
        drawCourt();
      }
      inPlay = true;
      // Erase tap message
      screen.tft.fillRect(SCREEN_W / 2 - 80, SCREEN_H / 2 + 25, 160, 30, ILI9341_BLACK);
      // Redraw center line
      for (int y = 6; y < SCREEN_H - 6; y += 14) {
        screen.tft.fillRect(SCREEN_W / 2 - 1, y, 2, 8, 0x4208);
      }
    }

    // Direct analog touch tracking (center paddle on touch Y)
    playerY = p.y - (PADDLE_H / 2);
    if (playerY < 4) playerY = 4;
    if (playerY > SCREEN_H - PADDLE_H - 4) playerY = SCREEN_H - PADDLE_H - 4;
  }

  if (inPlay && !gameOver) {
    // 2. AI Opponent Paddle Logic
    float targetAiY = ballY - (PADDLE_H / 2);
    float aiSpeed = 2.4 + (ballSpeed * 0.15); // Scales with game tempo
    if (aiY < targetAiY - 4) aiY += aiSpeed;
    else if (aiY > targetAiY + 4) aiY -= aiSpeed;

    if (aiY < 4) aiY = 4;
    if (aiY > SCREEN_H - PADDLE_H - 4) aiY = SCREEN_H - PADDLE_H - 4;

    // 3. Move Ball
    ballX += ballVx;
    ballY += ballVy;

    // Bounce off Top/Bottom walls
    if (ballY <= 4) {
      ballY = 4;
      ballVy = -ballVy;
    } else if (ballY >= SCREEN_H - BALL_SIZE - 4) {
      ballY = SCREEN_H - BALL_SIZE - 4;
      ballVy = -ballVy;
    }

    // Collision with Player Paddle (Right Side)
    float playerX = SCREEN_W - PADDLE_MARGIN - PADDLE_W;
    if (ballX + BALL_SIZE >= playerX && ballX <= playerX + PADDLE_W) {
      if (ballY + BALL_SIZE >= playerY && ballY <= playerY + PADDLE_H) {
        ballX = playerX - BALL_SIZE;
        // Impact factor: -1.0 (top edge) to +1.0 (bottom edge)
        float hitOffset = ((ballY + BALL_SIZE / 2.0) - (playerY + PADDLE_H / 2.0)) / (PADDLE_H / 2.0);
        ballSpeed += 0.25; // Increase speed with each hit!
        if (ballSpeed > 7.0) ballSpeed = 7.0;

        float bounceAngle = hitOffset * 0.85; // Max ~50 degrees
        ballVx = -cos(bounceAngle) * ballSpeed;
        ballVy = sin(bounceAngle) * ballSpeed;
      }
    }

    // Collision with AI Paddle (Left Side)
    float aiX = PADDLE_MARGIN;
    if (ballX <= aiX + PADDLE_W && ballX + BALL_SIZE >= aiX) {
      if (ballY + BALL_SIZE >= aiY && ballY <= aiY + PADDLE_H) {
        ballX = aiX + PADDLE_W;
        float hitOffset = ((ballY + BALL_SIZE / 2.0) - (aiY + PADDLE_H / 2.0)) / (PADDLE_H / 2.0);
        ballSpeed += 0.25;
        if (ballSpeed > 7.0) ballSpeed = 7.0;

        float bounceAngle = hitOffset * 0.85;
        ballVx = cos(bounceAngle) * ballSpeed;
        ballVy = sin(bounceAngle) * ballSpeed;
      }
    }

    // Scoring conditions
    if (ballX < 0) {
      // Player scores!
      playerScore++;
      drawScores();
      if (playerScore >= WIN_SCORE) {
        gameOver = true;
        winnerText = "YOU WIN!";
      } else {
        resetBall(true);
      }
    } else if (ballX > SCREEN_W) {
      // AI scores!
      aiScore++;
      drawScores();
      if (aiScore >= WIN_SCORE) {
        gameOver = true;
        winnerText = "CPU WINS!";
      } else {
        resetBall(false);
      }
    }
  }

  // ============================================================================
  // TURBO DELTA RENDERING (Only redraw moved objects - zero flicker 60 FPS!)
  // ============================================================================
  // Erase previous ball
  if ((int)prevBallX != (int)ballX || (int)prevBallY != (int)ballY) {
    screen.tft.fillRect((int)prevBallX, (int)prevBallY, BALL_SIZE, BALL_SIZE, ILI9341_BLACK);
    // Restore dashed line if ball passed through center
    if (prevBallX >= SCREEN_W / 2 - 4 && prevBallX <= SCREEN_W / 2 + 4) {
      for (int y = 6; y < SCREEN_H - 6; y += 14) {
        screen.tft.fillRect(SCREEN_W / 2 - 1, y, 2, 8, 0x4208);
      }
    }
  }
  // Draw new ball
  screen.tft.fillRect((int)ballX, (int)ballY, BALL_SIZE, BALL_SIZE, ILI9341_YELLOW);
  prevBallX = ballX;
  prevBallY = ballY;

  // Erase and redraw AI Paddle
  if ((int)prevAiY != (int)aiY) {
    screen.tft.fillRect(PADDLE_MARGIN, (int)prevAiY, PADDLE_W, PADDLE_H, ILI9341_BLACK);
    screen.tft.fillRect(PADDLE_MARGIN, (int)aiY, PADDLE_W, PADDLE_H, ILI9341_RED);
    prevAiY = aiY;
  } else {
    screen.tft.fillRect(PADDLE_MARGIN, (int)aiY, PADDLE_W, PADDLE_H, ILI9341_RED);
  }

  // Erase and redraw Player Paddle
  int playerX = SCREEN_W - PADDLE_MARGIN - PADDLE_W;
  if ((int)prevPlayerY != (int)playerY) {
    screen.tft.fillRect(playerX, (int)prevPlayerY, PADDLE_W, PADDLE_H, ILI9341_BLACK);
    screen.tft.fillRect(playerX, (int)playerY, PADDLE_W, PADDLE_H, ILI9341_CYAN);
    prevPlayerY = playerY;
  } else {
    screen.tft.fillRect(playerX, (int)playerY, PADDLE_W, PADDLE_H, ILI9341_CYAN);
  }

  // Game Over Banner
  if (gameOver) {
    screen.tft.fillRect(SCREEN_W / 2 - 85, SCREEN_H / 2 - 25, 170, 50, 0x18C3);
    screen.tft.drawRect(SCREEN_W / 2 - 85, SCREEN_H / 2 - 25, 170, 50, ILI9341_WHITE);
    screen.tft.setTextColor(playerScore >= WIN_SCORE ? ILI9341_GREEN : ILI9341_RED);
    screen.tft.setTextSize(2);
    screen.tft.setCursor(SCREEN_W / 2 - 50, SCREEN_H / 2 - 16);
    screen.tft.print(winnerText);

    screen.tft.setTextColor(ILI9341_WHITE);
    screen.tft.setTextSize(1);
    screen.tft.setCursor(SCREEN_W / 2 - 58, SCREEN_H / 2 + 10);
    screen.tft.print("Tap to Play Again");
  }

  delay(12); // ~60 FPS
}
