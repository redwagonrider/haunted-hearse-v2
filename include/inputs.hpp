#pragma once
#include <Arduino.h>

namespace Inputs {
    void init();
    void update();
    bool isBeamBroken(uint8_t beam_index);
    bool beamJustBroken(uint8_t beam_index);
}