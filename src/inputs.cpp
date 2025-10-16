#include "../include/inputs.hpp"
#include "../include/hardware.hpp"

namespace {
    bool beam_states[5] = {false};
    bool beam_prev_states[5] = {false};
}

namespace Inputs {
    void init() {
        pinMode(22, INPUT_PULLUP);
        pinMode(23, INPUT_PULLUP);
        pinMode(24, INPUT_PULLUP);
        pinMode(25, INPUT_PULLUP);
        pinMode(26, INPUT_PULLUP);
    }
    
    void update() {
        for (uint8_t i = 0; i < 5; i++) {
            beam_prev_states[i] = beam_states[i];
            beam_states[i] = (digitalRead(22 + i) == LOW);
        }
    }
    
    bool isBeamBroken(uint8_t beam_index) {
        return beam_index < 5 ? beam_states[beam_index] : false;
    }
    
    bool beamJustBroken(uint8_t beam_index) {
        return beam_index < 5 ? (beam_states[beam_index] && !beam_prev_states[beam_index]) : false;
    }
}