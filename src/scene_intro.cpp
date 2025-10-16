#include "../include/scene_intro.hpp"
#include "../include/effects.hpp"

namespace SceneIntro {
    void enter() { Effects::setMagnet(true); }
    void update() {
        uint8_t b = Effects::pulse(2000);
        Effects::setSceneLED(0, b);
    }
    void exit() {
        Effects::setMagnet(false);
        Effects::setSceneLED(0, 0);
    }
}