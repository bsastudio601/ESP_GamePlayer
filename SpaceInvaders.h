#ifndef SPACE_INVADERS_H
#define SPACE_INVADERS_H

#include <Adafruit_SH110X.h>

void spaceInvadersInit(Adafruit_SH1106G* disp);
bool spaceInvadersLoop();   // return true → exit to menu

#endif
