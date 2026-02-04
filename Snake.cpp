#include "Snake.h"
#include <Arduino.h>

// ========== Buttons ==========
#define BTN_LEFT   27
#define BTN_RIGHT  33
#define BTN_UP     25
#define BTN_DOWN   26

// ========== Display ==========
static Adafruit_SH1106G* display;

// ========== Grid ==========
#define CELL 4
#define GRID_W 20
#define GRID_H 12   // reduced so bottom never clips

#define OFFSET_X 24
#define OFFSET_Y 14  // moved down to leave space for score

// ========== Snake ==========
#define MAX_LEN 80
static int snakeX[MAX_LEN];
static int snakeY[MAX_LEN];
static int snakeLen;

static int dirX, dirY;

// ========== Food ==========
static int foodX, foodY;

// ========== Game ==========
static unsigned long lastMove;
static int moveDelay = 180;
static int score;
static bool gameOver;
static unsigned long gameOverTime;

// ========== Internal ==========
static void resetGame();
static void spawnFood();
static void drawCell(int x, int y);
static void drawBorder();
static void drawGame();
static bool hitsSelf(int x, int y);

// ========== Init ==========
void snakeInit(Adafruit_SH1106G* disp) {
  display = disp;

  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);

  randomSeed(millis());
  resetGame();
}

// ========== Loop ==========
bool snakeLoop() {

  // Exit to menu (LEFT + RIGHT)
  if (digitalRead(BTN_LEFT) == LOW &&
      digitalRead(BTN_RIGHT) == LOW) {
    delay(300);
    return true;
  }

  // Restart automatically after game over
  if (gameOver && millis() - gameOverTime > 2000) {
    resetGame();
  }

  // Controls
  if (!gameOver) {
    if (digitalRead(BTN_LEFT) == LOW && dirX == 0) {
      dirX = -1; dirY = 0;
    }
    if (digitalRead(BTN_RIGHT) == LOW && dirX == 0) {
      dirX = 1; dirY = 0;
    }
    if (digitalRead(BTN_UP) == LOW && dirY == 0) {
      dirX = 0; dirY = -1;
    }
    if (digitalRead(BTN_DOWN) == LOW && dirY == 0) {
      dirX = 0; dirY = 1;
    }
  }

  // Movement
  if (!gameOver && millis() - lastMove > moveDelay) {
    lastMove = millis();

    int newX = snakeX[0] + dirX;
    int newY = snakeY[0] + dirY;

    // Collision
    if (newX < 0 || newX >= GRID_W ||
        newY < 0 || newY >= GRID_H ||
        hitsSelf(newX, newY)) {
      gameOver = true;
      gameOverTime = millis();
    } else {
      for (int i = snakeLen; i > 0; i--) {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
      }

      snakeX[0] = newX;
      snakeY[0] = newY;

      if (newX == foodX && newY == foodY) {
        snakeLen++;
        score += 10;
        spawnFood();
        if (moveDelay > 90) moveDelay -= 5;
      }
    }
  }

  drawGame();
  return false;
}

// ========== Logic ==========
static void resetGame() {
  snakeLen = 3;

  snakeX[0] = GRID_W / 2;
  snakeY[0] = GRID_H / 2;
  snakeX[1] = snakeX[0] - 1;
  snakeY[1] = snakeY[0];
  snakeX[2] = snakeX[1] - 1;
  snakeY[2] = snakeY[1];

  dirX = 1;
  dirY = 0;

  score = 0;
  moveDelay = 180;
  gameOver = false;

  spawnFood();
  lastMove = millis();
}

static void spawnFood() {
  while (true) {
    foodX = random(0, GRID_W);
    foodY = random(0, GRID_H);

    bool ok = true;
    for (int i = 0; i < snakeLen; i++) {
      if (snakeX[i] == foodX && snakeY[i] == foodY) {
        ok = false;
        break;
      }
    }
    if (ok) break;
  }
}

static bool hitsSelf(int x, int y) {
  for (int i = 0; i < snakeLen; i++)
    if (snakeX[i] == x && snakeY[i] == y)
      return true;
  return false;
}

// ========== Drawing ==========
static void drawCell(int x, int y) {
  display->fillRect(
    OFFSET_X + x * CELL,
    OFFSET_Y + y * CELL,
    CELL - 1,
    CELL - 1,
    SH110X_WHITE
  );
}

static void drawBorder() {
  display->drawRect(
    OFFSET_X - 2,
    OFFSET_Y - 2,
    GRID_W * CELL + 3,
    GRID_H * CELL + 3,
    SH110X_WHITE
  );
}

static void drawGame() {
  display->clearDisplay();

  // ===== GAME OVER SCREEN =====
  if (gameOver) {
    display->setTextSize(2);
    display->setCursor(14, 20);
    display->println("GAME OVER");

    display->setTextSize(1);
    display->setCursor(40, 46);
    display->print("Score: ");
    display->print(score);

    display->display();
    return;
  }

  // ===== GAME SCREEN =====
  drawBorder();

  for (int i = 0; i < snakeLen; i++)
    drawCell(snakeX[i], snakeY[i]);

  drawCell(foodX, foodY);

  // Score (top, outside playfield)
  display->setTextSize(1);
  display->setCursor(0, 0);
  display->print("S:");
  display->print(score);

  display->display();
}
