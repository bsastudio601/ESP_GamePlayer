#include "SpaceInvaders.h"
#include <Arduino.h>  // for digitalRead, millis, delay, random, etc

// ===== Buttons =====
#define BTN_LEFT   25
#define BTN_RIGHT  33
#define BTN_SHOOT  27

// ===== Display =====
static Adafruit_SH1106G* display;

// ===== Screen =====
#define SCREEN_W 128
#define SCREEN_H 64

// ===== Player =====
#define PLAYER_W 10
#define PLAYER_H 4

// ===== Invaders =====
#define INV_W 8
#define INV_H 6
#define INV_ROWS 3
#define INV_COLS 6
static int invX[INV_ROWS*INV_COLS];
static int invY[INV_ROWS*INV_COLS];
static bool invAlive[INV_ROWS*INV_COLS];
static int invDir;
static int invDrop;
static unsigned long lastInvMove;
static unsigned long invDelay;

// ===== Bullet =====
static bool bulletActive;
static int bulletX, bulletY;
#define BULLET_H 4

// ===== Game state =====
static int playerX;
static unsigned long lastFrame;
static bool gameOver;
static int score;
static int level;

// ===== Internal functions =====
static void resetInvaders();
static void resetGame();
static void updateGame();
static void drawGame();
static bool allDead();

// ===== Init =====
void spaceInvadersInit(Adafruit_SH1106G* disp) {
    display = disp;

    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_SHOOT, INPUT_PULLUP);

    randomSeed(esp_random());

    resetGame();
}

// ===== Loop =====
bool spaceInvadersLoop() {

    // Exit if BOTH buttons held
    if (digitalRead(BTN_LEFT) == LOW && digitalRead(BTN_RIGHT) == LOW) {
        delay(300);
        return true;
    }

    if (millis() - lastFrame < 40) return false;
    lastFrame = millis();

    if (!gameOver) {
        updateGame();
        drawGame();
    } else {
        display->clearDisplay();
        display->setTextSize(2);
        display->setCursor(10, 20);
        display->println("GAME OVER");
        display->setTextSize(1);
        display->setCursor(30, 50);
        display->print("Score: ");
        display->println(score);
        display->display();

        // Wait for shoot button to restart
        if (digitalRead(BTN_SHOOT) == LOW) {
            delay(300);
            resetGame();
        }
    }

    return false;
}

// ===== Logic =====
static void updateGame() {
    // Player movement
    if (digitalRead(BTN_LEFT) == LOW && playerX > 0)
        playerX -= 2;
    if (digitalRead(BTN_RIGHT) == LOW && playerX < SCREEN_W - PLAYER_W)
        playerX += 2;

    // Shoot bullet
    if (!bulletActive && digitalRead(BTN_SHOOT) == LOW) {
        bulletActive = true;
        bulletX = playerX + PLAYER_W / 2;
        bulletY = SCREEN_H - PLAYER_H - 4;
    }

    // Bullet movement
    if (bulletActive) {
        bulletY -= 4;
        if (bulletY < 0) bulletActive = false;

        for (int i = 0; i < INV_ROWS*INV_COLS; i++) {
            if (invAlive[i] &&
                bulletX >= invX[i] && bulletX <= invX[i]+INV_W &&
                bulletY >= invY[i] && bulletY <= invY[i]+INV_H) {
                invAlive[i] = false;
                bulletActive = false;
                score += 10;
            }
        }
    }

    // Invader movement
    if (millis() - lastInvMove >= invDelay) {
        bool hitEdge = false;

        // check for hitting screen edge
        for (int i = 0; i < INV_ROWS*INV_COLS; i++) {
            if (!invAlive[i]) continue;
            if ((invDir>0 && invX[i]+INV_W >= SCREEN_W) ||
                (invDir<0 && invX[i]<=0)) hitEdge = true;
        }

        for (int i = 0; i < INV_ROWS*INV_COLS; i++) {
            if (!invAlive[i]) continue;
            if (hitEdge)
                invY[i] += invDrop; // drop
            else
                invX[i] += invDir;  // horizontal move

            // Check if invader reached player
            if (invY[i]+INV_H >= SCREEN_H-PLAYER_H)
                gameOver = true;
        }

        if (hitEdge) invDir = -invDir; // reverse direction
        lastInvMove = millis();
    }

    // Level up
    if (allDead()) {
        level++;
        invDelay = max(100UL, invDelay - 50); // faster each level
        invDrop = min(8, invDrop+1);          // drop more each level
        resetInvaders();
    }
}

// ===== Draw =====
static void drawGame() {
    display->clearDisplay();

    // Player
    display->fillRect(playerX, SCREEN_H-PLAYER_H, PLAYER_W, PLAYER_H, SH110X_WHITE);

    // Invaders
    for (int i=0;i<INV_ROWS*INV_COLS;i++) {
        if (invAlive[i])
            display->drawRect(invX[i], invY[i], INV_W, INV_H, SH110X_WHITE);
    }

    // Bullet
    if (bulletActive)
        display->drawFastVLine(bulletX, bulletY, BULLET_H, SH110X_WHITE);

    // Score & level
    display->setTextSize(1);
    display->setCursor(0,0);
    display->print("Score:");
    display->print(score);
    display->setCursor(70,0);
    display->print("Lvl:");
    display->print(level);

    display->display();
}

// ===== Helpers =====
static bool allDead() {
    for (int i=0;i<INV_ROWS*INV_COLS;i++)
        if (invAlive[i]) return false;
    return true;
}

static void resetInvaders() {
    int idx=0;
    for (int row=0; row<INV_ROWS; row++) {
        for (int col=0; col<INV_COLS; col++) {
            invX[idx] = 10 + col*18;
            invY[idx] = 12 + row*12;
            invAlive[idx] = true;
            idx++;
        }
    }
    invDir = 1;
    lastInvMove = millis();
    invDrop = 4;
}

static void resetGame() {
    playerX = SCREEN_W/2 - PLAYER_W/2;
    bulletActive = false;
    invDelay = 200;
    score = 0;
    level = 1;
    gameOver = false;
    resetInvaders();
}
