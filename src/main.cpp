#include <Arduino.h>
#include "../include/settings.hpp"
#include "../include/keypad_input.hpp"
#include "../include/fpp_control.hpp"
#include "../include/director.hpp"
#include "../include/effects.hpp"
#include "../include/display.hpp"
#include "../include/inputs.hpp"

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("HH V5 Complete"));
    SettingsManager::init();
    Effects::init();
    Display::init();
    Inputs::init();
    KeypadInput::init();
    FPPControl::init();
    Director::init();
    Serial.println(F("Ready - Type 'help'"));
}

void loop() {
    Inputs::update();
    
    char key = KeypadInput::getKey();
    if (key != NO_KEY) {
        Serial.print(F("Key: "));
        Serial.println(key);
        
        KeyEvent event = mapKeyToEvent(key);
        if (event == KeyEvent::STOP) Director::handleEvent(DirectorEvent::STOP);
        else if (event == KeyEvent::ATTRACT) Director::handleEvent(DirectorEvent::START_ATTRACT);
        else if (event == KeyEvent::SKIP) Director::handleEvent(DirectorEvent::SKIP);
        else if (event == KeyEvent::LOOP_MODE) Director::handleEvent(DirectorEvent::START_LOOP);
        else if (event == KeyEvent::TRIGGER_MODE) Director::handleEvent(DirectorEvent::START_TRIGGER);
    }
    
    Director::update();
    FPPControl::update();
}