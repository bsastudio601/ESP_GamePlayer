#ifndef SNAKE_H
#define SNAKE_H

#include <Adafruit_SH110X.h>

void snakeInit(Adafruit_SH1106G* disp);
bool snakeLoop();   // return true to exit to menu

#endif
