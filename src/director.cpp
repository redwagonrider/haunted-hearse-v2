#include "../include/director.hpp"
#include "../include/settings.hpp"
#include "../include/effects.hpp"
#include "../include/display.hpp"
#include "../include/fpp_control.hpp"
#include "../include/scene_intro.hpp"
#include "../include/scene_blood.hpp"
#include "../include/scene_frankenphone.hpp"
#include "../include/scene_mirror.hpp"
#include "../include/scene_exit.hpp"

namespace {
    DirectorState state = DirectorState::OPERATOR_IDLE;
    Scene current_scene = Scene::NONE;
    uint32_t scene_start_time = 0;
}

namespace Director {
    void init() {
        state = DirectorState::OPERATOR_IDLE;
        Effects::setOperatorLED(0, 255, 0, 0);
        Display::setStatus("RDY");
    }
    
    void update() {
        if (state == DirectorState::LOOP_MODE || state == DirectorState::ATTRACT_MODE) {
            if (current_scene == Scene::INTRO) SceneIntro::update();
            else if (current_scene == Scene::BLOOD) SceneBlood::update();
            else if (current_scene == Scene::FRANKENPHONE) SceneFrankenphone::update();
            else if (current_scene == Scene::MIRROR) SceneMirror::update();
            else if (current_scene == Scene::EXIT) SceneExit::update();
        }
    }
    
    void handleEvent(DirectorEvent event) {
        if (event == DirectorEvent::START_LOOP) {
            state = DirectorState::LOOP_MODE;
            Effects::setOperatorLED(255, 255, 0, 0);
            FPPControl::sendStartLoop();
            current_scene = Scene::INTRO;
            SceneIntro::enter();
        } else if (event == DirectorEvent::STOP) {
            state = DirectorState::OPERATOR_IDLE;
            Effects::setOperatorLED(0, 255, 0, 0);
            FPPControl::sendStop();
} else if (event == DirectorEvent::START_TRIGGER) {
    state = DirectorState::TRIGGER_MODE;
    Effects::setOperatorLED(255, 0, 0, 0);  // Red LED = Armed
    Display::setStatus("ARMD");              // Display shows "ARMED"
    current_scene = Scene::INTRO;            // Ready for intro beam
    Serial.println("Director: Armed for trigger mode");
    // Don't start FPP or scene yet - wait for beam sensor!
        } else if (event == DirectorEvent::START_ATTRACT) {
            state = DirectorState::ATTRACT_MODE;
            Effects::setOperatorLED(0, 0, 255, 0);
            FPPControl::sendStartAttract();
        }
    }
}