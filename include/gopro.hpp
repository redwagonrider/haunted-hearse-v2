#pragma once
#include <Arduino.h>

void gopro_begin(uint8_t relayPin);
void gopro_power(bool on);
void gopro_record_for(uint32_t ms);
void gopro_update();