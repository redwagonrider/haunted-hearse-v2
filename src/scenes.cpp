#include "scenes.hpp"
#include "effects.hpp"
#include "display.hpp"
#include "settings.hpp"
#include "recorder.hpp"
#include "gopro.hpp"

namespace {
  Scene currentScene = Scene::Standby;
  unsigned long sceneStart = 0;

  // --- Scene entry helpers ---

  static void enterStandby(){
    effects_showStandby();
    display_idle("OBEY");
    effects_rearmCue();  // brief solid-green flash when re-armed
    currentScene = Scene::Standby;
    sceneStart   = millis();
  }

  static void enterFrankenHold(){
    effects_startHold();
    display_hold_init();

    // Power external devices if wired
    recorder_power(true);  // Tascam 5V relay on
    gopro_power(true);     // GoPro 5V relay on
    // If you prefer timed recording instead of just power, you can use:
    // recorder_record_for(settings_ref().hold_ms + 500);
    // gopro_record_for(settings_ref().hold_ms + 500);

    currentScene = Scene::FrankenLab;
    sceneStart   = millis();
  }

  static void enterFrankenCooldown(){
    effects_endHold();
    effects_startCooldown();
    display_cooldown_init();

    // Power down external devices during cooldown
    recorder_power(false);
    gopro_power(false);

    currentScene = Scene::Cooldown;
    sceneStart   = millis();
  }
}

// --- Public API ---

void scenes_begin(){
  enterStandby();
}

void scenes_set(Scene s){
  switch(s){
    case Scene::Standby:      enterStandby();         break;
    case Scene::FrankenLab:   enterFrankenHold();     break;
    case Scene::Cooldown:     enterFrankenCooldown(); break;
    default: /* others TBD */ enterStandby();         break;
  }
}

void scenes_update(){
  unsigned long now = millis();
  auto& S = settings_ref();

  switch(currentScene){
    case Scene::Standby:
      effects_showStandby();
      break;

    case Scene::FrankenLab: {
      effects_updateHold();
      display_hold_update();
      if (now - sceneStart >= S.hold_ms){
        scenes_set(Scene::Cooldown);
      }
    } break;

    case Scene::Cooldown: {
      effects_updateCooldown();
      display_cooldown_update();
      if (now - sceneStart >= S.cooldown_ms){
        scenes_set(Scene::Standby);
      }
    } break;

    default:
      break;
  }
}

Scene scenes_current(){
  return currentScene;
}

const char* scenes_name(Scene s){
  switch(s){
    case Scene::Standby:     return "Standby";
    case Scene::IntroCue:    return "IntroCue";
    case Scene::BloodRoom:   return "BloodRoom";
    case Scene::Graveyard:   return "Graveyard";
    case Scene::FurRoom:     return "FurRoom";
    case Scene::OrcaDino:    return "OrcaDino";
    case Scene::FrankenLab:  return "FrankenLab";
    case Scene::MirrorRoom:  return "MirrorRoom";
    case Scene::ExitHole:    return "ExitHole";
    case Scene::PhoneLoading:return "PhoneLoading";
    case Scene::Cooldown:    return "Cooldown";
    default:                 return "Unknown";
  }
}

bool scenes_accepts_triggers(){
  return (scenes_current() == Scene::Standby);
}