#include "Breakout.h"
#include <math.h>

// ===== Buttons =====
#define BTN_LEFT  25
#define BTN_RIGHT 33

// ===== Display Pointer =====
static Adafruit_SH1106G* display;

// ===== Game Objects =====
static int paddleX;
static const int paddleWidth = 30;
static const int paddleHeight = 3;
static const int paddleSpeed = 5;

static const int brickRows = 4;
static const int brickCols = 6;
static const int brickWidth = 128 / brickCols;
static const int brickHeight = 5;
static bool bricks[brickRows][brickCols];

static float ballX, ballY;
static float ballDX, ballDY;
static float ballSpeed;
static int hitCount;

static bool gameOver;
static bool gameWin;

// ===== Internal Functions =====
static void resetGame();
static void normalizeBallSpeed();
static void drawGame();
static void showEndScreen();

// ===== Init =====
void breakoutInit(Adafruit_SH1106G* disp) {
  display = disp;

  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  randomSeed(esp_random());
  resetGame();
}

// ===== Game Loop =====
bool breakoutLoop() {

  // Exit game if BOTH buttons held
  if (digitalRead(BTN_LEFT) == LOW && digitalRead(BTN_RIGHT) == LOW) {
    delay(300);
    return true;
  }

  if (gameOver || gameWin) {
    showEndScreen();
    return false;
  }

  // Paddle movement
  if (digitalRead(BTN_LEFT) == LOW)  paddleX -= paddleSpeed;
  if (digitalRead(BTN_RIGHT) == LOW) paddleX += paddleSpeed;
  paddleX = constrain(paddleX, 0, 128 - paddleWidth);

  // Ball movement
  int oldX = ballX;
  int oldY = ballY;
  ballX += ballDX;
  ballY += ballDY;

  if (ballX <= 0 || ballX >= 126) ballDX = -ballDX;
  if (ballY <= 0) ballDY = -ballDY;

  // Paddle collision
  if (ballY >= 64 - paddleHeight - 1 &&
      ballX >= paddleX && ballX <= paddleX + paddleWidth) {
    ballDY = -ballDY;
    ballSpeed = min(6.0f, ballSpeed + 0.3f);
    normalizeBallSpeed();
  }

  // Brick collision
  int remaining = 0;
  for (int i = 0; i < brickRows; i++) {
    for (int j = 0; j < brickCols; j++) {
      if (bricks[i][j]) {
        remaining++;
        int bx = j * brickWidth;
        int by = i * brickHeight;

        if (ballX >= bx && ballX <= bx + brickWidth &&
            ballY >= by && ballY <= by + brickHeight) {
          bricks[i][j] = false;
          ballDY = -ballDY;
          normalizeBallSpeed();
        }
      }
    }
  }

  if (remaining == 0) gameWin = true;
  if (ballY > 64) gameOver = true;

  drawGame();
  delay(10);
  return false;
}

// ===== Helpers =====
static void resetGame() {
  paddleX = 64 - paddleWidth / 2;
  ballX = 64;
  ballY = 40;

  ballSpeed = 3.0;
  hitCount = 0;
  gameOver = false;
  gameWin = false;

  float angle = random(30, 150);
  ballDX = ballSpeed * cos(radians(angle));
  ballDY = -ballSpeed * sin(radians(angle));

  for (int i = 0; i < brickRows; i++)
    for (int j = 0; j < brickCols; j++)
      bricks[i][j] = true;
}

static void normalizeBallSpeed() {
  float mag = sqrt(ballDX * ballDX + ballDY * ballDY);
  ballDX = (ballDX / mag) * ballSpeed;
  ballDY = (ballDY / mag) * ballSpeed;
}

static void drawGame() {
  display->clearDisplay();

  display->fillRect(paddleX, 64 - paddleHeight, paddleWidth, paddleHeight, SH110X_WHITE);
  display->fillCircle(ballX, ballY, 2, SH110X_WHITE);

  for (int i = 0; i < brickRows; i++)
    for (int j = 0; j < brickCols; j++)
      if (bricks[i][j])
        display->fillRect(j * brickWidth, i * brickHeight,
                          brickWidth - 1, brickHeight - 1, SH110X_WHITE);

  display->display();
}

static void showEndScreen() {
  display->clearDisplay();
  display->setTextSize(2);
  display->setCursor(10, 25);
  display->println(gameWin ? "YOU WIN" : "GAME OVER");
  display->display();

  delay(1500);
  resetGame();
}
