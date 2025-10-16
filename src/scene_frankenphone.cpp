#include "../include/scene_frankenphone.hpp"
#include "../include/effects.hpp"

namespace {
    uint32_t phase_start = 0;
    bool in_hold_phase = true;
}

namespace SceneFrankenphone {
    void enter() {
        phase_start = millis();
        in_hold_phase = true;
        Effects::setMagnet(true);
    }
    
    void update() {
        uint32_t elapsed = millis() - phase_start;
        if (in_hold_phase) {
            if (elapsed < 8000) {
                Effects::setSceneLED(0, 200);
                Effects::setSceneLED(1, 200);
            } else {
                in_hold_phase = false;
                Effects::setMagnet(false);
            }
        } else {
            uint8_t b = Effects::pulse(2000);
            Effects::setSceneLED(0, b);
            Effects::setSceneLED(1, b);
        }
    }
    
    void exit() {
        Effects::setMagnet(false);
        Effects::setSceneLED(0, 0);
        Effects::setSceneLED(1, 0);
    }
}