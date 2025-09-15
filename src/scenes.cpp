#include "scenes.hpp"

// Forward declarations to avoid needing every scene header here
extern void scene_standby();
extern void scene_frankenphone();  // FrankenLab
extern void scene_mirror();
extern void scene_phoneLoading();
extern void scene_intro();
extern void scene_blood();
extern void scene_graveyard();
extern void scene_fur();
extern void scene_orca();
extern void scene_exit();
extern void scene_blackout();
extern void scene_secret();
extern void scene_spider();
extern void scene_spiders();
extern void scene_fire();

SceneFn g_currentScene = nullptr;

static SceneFn map_code_to_fn(uint8_t code) {
  switch (code) {
    case 0:  return scene_standby;
    case 1:  return scene_frankenphone; // FrankenLab
    case 2:  return scene_mirror;
    case 10: return scene_phoneLoading;
    case 11: return scene_intro;
    case 12: return scene_blood;
    case 13: return scene_graveyard;
    case 14: return scene_fur;
    case 15: return scene_orca;        // Orca/Dino
    case 16: return scene_frankenphone; // FrankenLab alias
    case 17: return scene_mirror;       // MirrorRoom alias
    case 18: return scene_exit;
    default: return scene_standby;
  }
}

SceneFn scene_by_code(uint8_t code) {
  return map_code_to_fn(code);
}

SceneFn scene_by_enum(Scene s) {
  return map_code_to_fn(static_cast<uint8_t>(s));
}

const char* scene_name(Scene s) {
  switch (s) {
    case Scene::Standby:       return "Standby";
    case Scene::FrankenLab:    return "FrankenLab";
    case Scene::MirrorRoom:    return "MirrorRoom";
    case Scene::PhoneLoading:  return "PhoneLoading";
    case Scene::IntroCue:      return "IntroCue";
    case Scene::BloodRoom:     return "BloodRoom";
    case Scene::Graveyard:     return "Graveyard";
    case Scene::FurRoom:       return "FurRoom";
    case Scene::OrcaDino:      return "OrcaDino";
    case Scene::FrankenLabAlt: return "FrankenLab";
    case Scene::MirrorRoomAlt: return "MirrorRoom";
    case Scene::ExitHole:      return "ExitHole";
    default:                   return "Unknown";
  }
}

void scenes_set(Scene s) {
  g_currentScene = scene_by_enum(s);
  if (!g_currentScene) {
    g_currentScene = scene_standby;
  }
  // Kick off the chosen scene immediately
  g_currentScene();
}