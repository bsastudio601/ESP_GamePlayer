#include "Tetris.h"
#include <Arduino.h>

// ================== Buttons ==================
#define BTN_LEFT   25
#define BTN_RIGHT  33
#define BTN_DOWN   26
#define BTN_ROTATE 27

// ================== Display ==================
static Adafruit_SH1106G* display;

// ================== Grid ==================
#define BLOCK   4
#define GRID_W  10
#define GRID_H  16

// Centered playfield
#define FIELD_X  30
#define FIELD_Y  0

// Right panel
#define PANEL_X  78
#define PANEL_Y  10

static byte grid[GRID_H][GRID_W];

// ================== Timing ==================
static unsigned long lastDrop;
static const int dropDelay = 500;

// Input rate limit
static unsigned long lastMove;
static const int moveDelay = 120;

// ================== Score / State ==================
static int score;
static bool gameOver;

// ================== Piece ==================
struct Piece {
  byte shape[4][4];
  int x, y;
};

static Piece current;
static Piece nextPiece;

// ================== Shapes ==================
static const byte SHAPES[4][4][4] = {
  { {1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} }, // I
  { {1,1,0,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0} }, // O
  { {0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} }, // T
  { {1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} }  // S
};

// ================== Internals ==================
static void generateNext();
static void spawnPiece();
static bool collision(int nx, int ny, Piece& p);
static void lockPiece();
static void clearLines();
static void rotatePiece();
static void drawBlock(int x, int y);
static void drawPlayfieldBorder();
static void drawGame();
static void resetGame();

// ================== Init ==================
void tetrisInit(Adafruit_SH1106G* disp) {
  display = disp;

  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_ROTATE, INPUT_PULLUP);

  randomSeed(millis());
  resetGame();
}

// ================== Loop ==================
bool tetrisLoop() {

  // Exit to menu
  if (digitalRead(BTN_LEFT) == LOW &&
      digitalRead(BTN_RIGHT) == LOW) {
    delay(300);
    return true;
  }

  // ---------- GAME OVER ----------
  if (gameOver) {
    display->clearDisplay();

    display->setTextSize(2);
    display->setCursor(18, 18);
    display->print("GAME");

    display->setCursor(18, 36);
    display->print("OVER");

    display->setTextSize(1);
    display->setCursor(28, 54);
    display->print("SCORE:");
    display->print(score);

    display->display();
    delay(2000);
    resetGame();
    return false;
  }

  unsigned long now = millis();

  // ---------- Horizontal movement ----------
  if (now - lastMove > moveDelay) {
    if (digitalRead(BTN_LEFT) == LOW &&
        !collision(current.x - 1, current.y, current)) {
      current.x--;
      lastMove = now;
    }

    if (digitalRead(BTN_RIGHT) == LOW &&
        !collision(current.x + 1, current.y, current)) {
      current.x++;
      lastMove = now;
    }
  }

  // Soft drop
  if (digitalRead(BTN_DOWN) == LOW &&
      !collision(current.x, current.y + 1, current)) {
    current.y++;
  }

  // Rotate
  if (digitalRead(BTN_ROTATE) == LOW) {
    rotatePiece();
    delay(160);
  }

  // ---------- Gravity ----------
  if (now - lastDrop > dropDelay) {
    lastDrop = now;
    if (!collision(current.x, current.y + 1, current))
      current.y++;
    else
      lockPiece();
  }

  drawGame();
  return false;
}

// ================== Logic ==================
static void generateNext() {
  memset(nextPiece.shape, 0, sizeof(nextPiece.shape));
  int r = random(0, 4);
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      nextPiece.shape[y][x] = SHAPES[r][y][x];
}

static void spawnPiece() {
  current = nextPiece;
  current.x = GRID_W / 2 - 2;
  current.y = 0;
  generateNext();

  if (collision(current.x, current.y, current))
    gameOver = true;
}

static bool collision(int nx, int ny, Piece& p) {
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++) {
      if (!p.shape[y][x]) continue;
      int gx = nx + x;
      int gy = ny + y;
      if (gx < 0 || gx >= GRID_W || gy >= GRID_H) return true;
      if (gy >= 0 && grid[gy][gx]) return true;
    }
  return false;
}

static void lockPiece() {
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      if (current.shape[y][x])
        grid[current.y + y][current.x + x] = 1;

  clearLines();
  spawnPiece();
}

static void clearLines() {
  for (int y = 0; y < GRID_H; y++) {
    bool full = true;
    for (int x = 0; x < GRID_W; x++)
      if (!grid[y][x]) full = false;

    if (full) {
      score += 100;
      for (int yy = y; yy > 0; yy--)
        memcpy(grid[yy], grid[yy - 1], GRID_W);
      memset(grid[0], 0, GRID_W);
    }
  }
}

static void rotatePiece() {
  byte tmp[4][4] = {0};
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      tmp[x][3 - y] = current.shape[y][x];

  Piece test = current;
  memcpy(test.shape, tmp, sizeof(tmp));
  if (!collision(test.x, test.y, test))
    memcpy(current.shape, tmp, sizeof(tmp));
}

// ================== Drawing ==================
static void drawBlock(int x, int y) {
  display->drawRect(
    FIELD_X + x * BLOCK,
    FIELD_Y + y * BLOCK,
    BLOCK,
    BLOCK,
    SH110X_WHITE
  );
}

static void drawPlayfieldBorder() {
  display->drawRect(
    FIELD_X - 1,
    FIELD_Y,
    GRID_W * BLOCK + 2,
    GRID_H * BLOCK,
    SH110X_WHITE
  );
}

static void drawGame() {
  display->clearDisplay();

  // Playfield border ONLY during gameplay
  drawPlayfieldBorder();

  // Grid
  for (int y = 0; y < GRID_H; y++)
    for (int x = 0; x < GRID_W; x++)
      if (grid[y][x]) drawBlock(x, y);

  // Current piece
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      if (current.shape[y][x])
        drawBlock(current.x + x, current.y + y);

  // Next piece
  display->setCursor(PANEL_X, 0);
  display->print("NEXT");

  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      if (nextPiece.shape[y][x])
        display->drawRect(
          PANEL_X + x * BLOCK,
          PANEL_Y + y * BLOCK,
          BLOCK,
          BLOCK,
          SH110X_WHITE
        );

  // Score
  display->setCursor(PANEL_X, 40);
  display->print("SCORE");
  display->setCursor(PANEL_X, 50);
  display->print(score);

  display->display();
}

// ================== Reset ==================
static void resetGame() {
  memset(grid, 0, sizeof(grid));
  score = 0;
  gameOver = false;
  lastDrop = millis();
  lastMove = millis();
  generateNext();
  spawnPiece();
}
