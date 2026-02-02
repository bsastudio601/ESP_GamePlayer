#ifndef BREAKOUT_H
#define BREAKOUT_H

#include <Adafruit_SH110X.h>

void breakoutInit(Adafruit_SH1106G* disp);
bool breakoutLoop();   // return true → exit game

#endif
