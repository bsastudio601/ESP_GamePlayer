#ifndef WIFI_TEXT_H
#define WIFI_TEXT_H

#include <Adafruit_SH110X.h>

void wifiTextInit(Adafruit_SH1106G* disp);
bool wifiTextLoop();   // return true to exit app

#endif
