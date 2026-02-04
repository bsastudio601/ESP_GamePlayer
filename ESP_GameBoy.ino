#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include "Breakout.h"
#include "BMP280.h"
#include "SpaceInvaders.h"
#include "Tetris.h"
#include "Snake.h"

// ===== Display =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define I2C_ADDR 0x3C

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== Buttons =====
#define BTN_UP     25
#define BTN_DOWN   26
#define BTN_SELECT 27

// ===== Menu =====
const char* menuItems[] = {
  "Breakout",
  "hPa & Temp Sensor",
  "Space Invaders",
  "Tetris",
  "Snake"
};

const int menuCount = 5;
int menuIndex = 0;
bool inApp = false;

// ===== Draw Menu =====
void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  display.setCursor(0, 0);
  display.println("== ESP GameBoy ==");

  display.drawLine(0, 10, SCREEN_WIDTH, 10, SH110X_WHITE);

  for (int i = 0; i < menuCount; i++) {
    display.setCursor(0, 14 + i * 10);
    display.print(i == menuIndex ? "> " : "  ");
    display.println(menuItems[i]);
  }

  display.display();
}

// ===== Setup =====
void setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);

  Wire.begin(21, 22);

  if (!display.begin(I2C_ADDR, true)) {
    while (1);
  }

  drawMenu();
}

// ===== Main Loop =====
void loop() {

  // ===== MENU MODE =====
  if (!inApp) {
    drawMenu();

    if (digitalRead(BTN_UP) == LOW) {
      menuIndex--;
      if (menuIndex < 0) menuIndex = menuCount - 1;
      delay(200);
    }

    if (digitalRead(BTN_DOWN) == LOW) {
      menuIndex++;
      if (menuIndex >= menuCount) menuIndex = 0;
      delay(200);
    }

    if (digitalRead(BTN_SELECT) == LOW) {
      delay(250);

      if (menuIndex == 0) {          // Breakout
        breakoutInit(&display);
        inApp = true;
      }
      else if (menuIndex == 1) {     // BMP280
        bmp280Init(&display);
        inApp = true;
      }
      else if (menuIndex == 2) {     // Space Invaders
        spaceInvadersInit(&display);
        inApp = true;
      }
      else if (menuIndex == 3) {     // Tetris
        tetrisInit(&display);
        inApp = true;
      }
      else if (menuIndex == 4) {     // Snake ✅
        snakeInit(&display);
        inApp = true;
      }
    }
  }

  // ===== APP MODE =====
  else {
    if (menuIndex == 0) {
      if (breakoutLoop()) inApp = false;
    }
    else if (menuIndex == 1) {
      if (bmp280Loop()) inApp = false;
    }
    else if (menuIndex == 2) {
      if (spaceInvadersLoop()) inApp = false;
    }
    else if (menuIndex == 3) {
      if (tetrisLoop()) inApp = false;
    }
    else if (menuIndex == 4) {
      if (snakeLoop()) inApp = false;   // ✅
    }
  }
}
