#pragma once
#include <Arduino.h>

namespace Effects {
    void init();
    void setSceneLED(uint8_t led, uint8_t brightness);
    void setOperatorLED(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    void setMagnet(bool on);
    void setBuzzer(bool on);
    uint8_t pulse(uint32_t period_ms, uint8_t min_val = 0, uint8_t max_val = 255);
    uint8_t sin8(uint8_t theta);
}