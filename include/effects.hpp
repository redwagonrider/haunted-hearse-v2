// effects.hpp
#pragma once
#include <Arduino.h>

// Init
void effects_begin();

// Core scene helpers used across the project
void effects_holdPulseRed(unsigned long elapsed);
void effects_updateCooldown();
void effects_updateArmed();

// RGB tracking for telemetry
void effects_setRGB(uint8_t r, uint8_t g, uint8_t b);
void effects_getRGB(uint8_t& r, uint8_t& g, uint8_t& b);

// Common effects referenced by scenes
void effects_showBlackout();     // all off
void effects_exitStrobe();       // white strobe
void effects_showStandby();      // wrapper used by scene_standby

// Fire room
void effects_fireFlicker();      // red/orange flicker

// Fur room
void effects_furPulse();         // warm pink/magenta pulse

// Graveyard
void effects_mistyGraveyard();   // cyan/green slow fade

// Blood room
void effects_bloodPulse();       // deep red pulse
void effects_bloodDrip();        // random red spikes

// Spider lair
void effects_spiderCrawl();      // quick white blips
void effects_spiderEyes();       // dim red hold
void effects_spiderWebFlash();   // sharp web camera flash

// Mirror room
void effects_mirrorStrobe();     // bright white strobe
void effects_mirrorSweep();      // slower sweep
void effects_mirrorFlash();      // single-call flash helper

// Orca scene
void effects_orcaBlueFade();     // deep blue breathe
void effects_orcaWave();         // blue to teal wave
void effects_orcaSplash();       // short bright splash

// Intro
void effects_introFade();        // soft white fade in

// Secret
void effects_secretGlow();       // purple breathe
void effects_secretReveal();     // bright reveal pulse

// Standby
void effects_standbyIdle();      // dim amber idle