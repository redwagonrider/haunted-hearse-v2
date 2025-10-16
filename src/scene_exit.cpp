#include "../include/scene_exit.hpp"
#include "../include/effects.hpp"

namespace SceneExit {
    void enter() {}
    void update() {
        uint8_t b = Effects::pulse(1500);
        for (uint8_t i = 0; i < 3; i++) {
            Effects::setSceneLED(i, b);
        }
    }
    void exit() {
        for (uint8_t i = 0; i < 3; i++) {
            Effects::setSceneLED(i, 0);
        }
    }
}