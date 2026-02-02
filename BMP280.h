#ifndef BMP280_H
#define BMP280_H

#include <Adafruit_SH110X.h>
#include <Adafruit_BMP280.h>

void bmp280Init(Adafruit_SH1106G* disp);
bool bmp280Loop();  // return true → exit mode

#endif
