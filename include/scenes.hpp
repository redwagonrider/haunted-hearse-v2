#pragma once
#include <Arduino.h>

// High-level scenes (expand as you implement)
enum class Scene : uint8_t {
  PhoneLoading,
  IntroCue,
  BloodRoom,
  Graveyard,
  FurRoom,
  OrcaDino,
  FrankenLab,   // your current working scene
  MirrorRoom,
  ExitHole,
  Standby
};

// Public API
void scenes_begin();                   // call in setup()
void scenes_set(Scene s);              // immediate jump to scene (manual or via trigger)
void scenes_next();                    // advance to next scene (optional helper)
Scene scenes_current();                // query

// Must be called from loop()
void scenes_update();                  // calls current scene's update()

// Optional: timing helpers if a scene runs by duration
void scenes_startTimer(uint32_t ms);   // start/replace a scene-local timer
bool scenes_timerExpired();            // true when timer hits 0
uint32_t scenes_elapsed();             // ms since scene enter

// Optional hooks to PI/Falcon (stubs you can wire later)
void scenes_notify_audio(const __FlashStringHelper* msg);
void scenes_notify_video(const __FlashStringHelper* msg);
void scenes_notify_pixels(const __FlashStringHelper* msg);

// Console helpers (optional): dump current scene name
const __FlashStringHelper* scenes_name(Scene s);