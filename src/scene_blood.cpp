#include "../include/scene_blood.hpp"
#include "../include/effects.hpp"

namespace {
    uint32_t last_drip = 0;
    uint32_t buzzer_start = 0;
    bool buzzer_active = false;
}

namespace SceneBlood {
    void enter() { last_drip = millis(); buzzer_active = false; }
    
    void update() {
        if (!buzzer_active && (millis() - last_drip >= 2000)) {
            Effects::setBuzzer(true);
            buzzer_start = millis();
            buzzer_active = true;
            last_drip = millis();
        }
        if (buzzer_active && (millis() - buzzer_start >= 50)) {
            Effects::setBuzzer(false);
            buzzer_active = false;
        }
        uint8_t b = Effects::pulse(3000, 50, 255);
        Effects::setSceneLED(0, b);
    }
    
    void exit() {
        Effects::setBuzzer(false);
        Effects::setSceneLED(0, 0);
    }
}