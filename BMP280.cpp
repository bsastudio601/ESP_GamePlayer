#include "BMP280.h"

static Adafruit_SH1106G* display;
static Adafruit_BMP280 bmp;

#define BTN_LEFT 25
#define BTN_RIGHT 33

void bmp280Init(Adafruit_SH1106G* disp) {
    display = disp;
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SH110X_WHITE);

    if (!bmp.begin(0x76)) {  // check your BMP280 address
        display->setCursor(0, 0);
        display->println("BMP280 ERR");
        display->display();
    }
}

bool bmp280Loop() {
    // exit if both buttons pressed
    if (digitalRead(BTN_LEFT) == LOW && digitalRead(BTN_RIGHT) == LOW) {
        delay(300);
        return true;
    }

    display->clearDisplay();
    display->setCursor(0, 0);

    float temp = bmp.readTemperature();
    float pres = bmp.readPressure() / 100.0F;  // hPa

    display->print("Temp: "); display->print(temp); display->println(" C");
    display->print("Pres: "); display->print(pres); display->println(" hPa");
    display->display();

    delay(500);
    return false;
}
