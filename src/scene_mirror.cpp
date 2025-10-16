#include "../include/scene_mirror.hpp"
#include "../include/effects.hpp"

namespace {
    uint32_t last_strobe = 0;
    bool strobe_state = false;
}

namespace SceneMirror {
    void enter() { last_strobe = millis(); }
    
    void update() {
        if (millis() - last_strobe >= 200) {
            strobe_state = !strobe_state;
            uint8_t b = strobe_state ? 255 : 0;
            for (uint8_t i = 0; i < 3; i++) {
                Effects::setSceneLED(i, b);
            }
            last_strobe = millis();
        }
    }
    
    void exit() {
        for (uint8_t i = 0; i < 3; i++) {
            Effects::setSceneLED(i, 0);
        }
    }
}