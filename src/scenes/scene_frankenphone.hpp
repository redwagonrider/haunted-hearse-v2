#pragma once
#include <Arduino.h>

// Public API for the Frankenphone scene
void frankenphone_init();
void scene_frankenphone();   // start the scene (enters HOLD)
void frankenphone_update();  // call each loop

// Helper for dashboards
const char* frankenphone_phase_name();