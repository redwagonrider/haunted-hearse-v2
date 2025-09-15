#pragma once

// Frankenphones Lab public API

// Call once at boot to init pins and idle display
void frankenphone_init();

// One-shot: start the Frankenphone scene (HOLD -> COOLDOWN)
void scene_frankenphone();

// Tick this from loop() to advance the scene state machine
void frankenphone_update();

// New: globally mute or unmute the Frankenphone modem sound
// true  = mute immediately and keep muted
// false = allow sound per current scene phase
void frankenphone_set_mute(bool muted);