#pragma once
#include <Arduino.h>

// Beams and reed initialization and per-loop update
void inputs_init();
void inputs_update();

// Print current beam -> scene mapping and pins to Serial
void inputs_print_map();