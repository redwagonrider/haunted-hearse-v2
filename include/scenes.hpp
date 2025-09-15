#pragma once
#include <Arduino.h>

// Function pointer used for scenes
typedef void (*SceneFn)();

// Canonical scene enum (matches your README codes)
enum class Scene : uint8_t {
  Standby       = 0,
  FrankenLab    = 1,   // alias also 16
  MirrorRoom    = 2,   // alias also 17
  PhoneLoading  = 10,
  IntroCue      = 11,
  BloodRoom     = 12,
  Graveyard     = 13,
  FurRoom       = 14,
  OrcaDino      = 15,
  FrankenLabAlt = 16,  // maps to FrankenLab
  MirrorRoomAlt = 17,  // maps to MirrorRoom
  ExitHole      = 18
};

// Resolve a numeric code to a scene function
SceneFn scene_by_code(uint8_t code);

// Resolve an enum to a scene function
SceneFn scene_by_enum(Scene s);

// Human readable name for UI/logs
const char* scene_name(Scene s);

// Set the current scene by enum
void scenes_set(Scene s);

// Expose current chosen function for callers that want to run it directly
extern SceneFn g_currentScene;

// Declarations for all scene entry points implemented in your repo
// Each is a "kickoff" that sets its own timers and state.
// The function bodies live in src/scenes/scene_*.cpp
void scene_standby();
void scene_frankenphone();  // FrankenLab
void scene_mirror();
void scene_phoneLoading();
void scene_intro();
void scene_blood();
void scene_graveyard();
void scene_fur();
void scene_orca();
void scene_exit();
void scene_blackout();
void scene_secret();
void scene_spider();
void scene_spiders();
void scene_fire();