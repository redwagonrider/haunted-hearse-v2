#pragma once
#include <Arduino.h>

namespace FPPControl {
    void init();
    void sendStartLoop();
    void sendStartTrigger();
    void sendStartAttract();
    void sendStop();
    void update();
}