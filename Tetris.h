#ifndef TETRIS_H
#define TETRIS_H

#pragma once
#include <Adafruit_SH110X.h>

void tetrisInit(Adafruit_SH1106G* disp);
bool tetrisLoop();   // return true to exit game

#endif
