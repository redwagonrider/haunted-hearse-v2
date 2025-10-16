#include "../include/fpp_control.hpp"
#include "../include/hardware.hpp"
#include "../include/config.hpp"

namespace {
    uint32_t pulse_start = 0;
    bool pulse_active = false;
    void sendPulse(uint8_t pin);
}

namespace FPPControl {
    void init() {
        pinMode(FPP_START_LOOP, OUTPUT);
        pinMode(FPP_START_TRIGGER, OUTPUT);
        pinMode(FPP_START_ATTRACT, OUTPUT);
        pinMode(FPP_STOP_ALL, OUTPUT);
        digitalWrite(FPP_START_LOOP, LOW);
        digitalWrite(FPP_START_TRIGGER, LOW);
        digitalWrite(FPP_START_ATTRACT, LOW);
        digitalWrite(FPP_STOP_ALL, LOW);
    }
    
    void sendPulse(uint8_t pin) {
        digitalWrite(pin, HIGH);
        pulse_start = millis();
        pulse_active = true;
    }
    
    void sendStartLoop() { sendPulse(FPP_START_LOOP); Serial.println(F("FPP: Loop")); }
    void sendStartTrigger() { sendPulse(FPP_START_TRIGGER); Serial.println(F("FPP: Trigger")); }
    void sendStartAttract() { sendPulse(FPP_START_ATTRACT); Serial.println(F("FPP: Attract")); }
    void sendStop() { sendPulse(FPP_STOP_ALL); Serial.println(F("FPP: Stop")); }
    
    void update() {
        if (pulse_active && (millis() - pulse_start >= FPP_PULSE_MS)) {
            digitalWrite(FPP_START_LOOP, LOW);
            digitalWrite(FPP_START_TRIGGER, LOW);
            digitalWrite(FPP_START_ATTRACT, LOW);
            digitalWrite(FPP_STOP_ALL, LOW);
            pulse_active = false;
        }
    }
}