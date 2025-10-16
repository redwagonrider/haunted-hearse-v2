#include "../include/display.hpp"
#include "../include/hardware.hpp"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

namespace {
    LiquidCrystal_I2C lcd(0x27, 16, 2);
    bool display_available = false;
}

namespace Display {
    void init() {
        Wire.begin();
        Wire.beginTransmission(0x27);
        if (Wire.endTransmission() != 0) {
            display_available = false;
            return;
        }
        lcd.init();
        lcd.backlight();
        display_available = true;
    }
    
    void setStatus(const char* status) {
        if (!display_available) return;
        lcd.setCursor(0, 0);
        lcd.print("Status: ");
        lcd.print(status);
    }
    
    void setScene(const char* scene) {
        if (!display_available) return;
        lcd.setCursor(0, 1);
        lcd.print("Scene: ");
        lcd.print(scene);
    }
    
    bool isAvailable() { return display_available; }
}