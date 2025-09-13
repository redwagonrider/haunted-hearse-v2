#pragma once
#include <Arduino.h>

void recorder_begin(uint8_t relayPin);
void recorder_power(bool on);
void recorder_record_for(uint32_t ms);
void recorder_update();