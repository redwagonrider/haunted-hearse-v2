#include "mapping.hpp"
#include "pins.hpp"

// If you have these headers, you can include them.
// They are not required for the simple defaults here.
// #include "settings.hpp"
// #include "console.hpp"
extern void scene_blood();
extern void scene_graveyard();
extern void scene_fur();
extern void scene_orca();
extern void scene_frankenphone();
extern void scene_mirror();

// Default pin map pulled from your README
uint8_t BEAM_PINS[6] = {
  PIN_BEAM_0, PIN_BEAM_1, PIN_BEAM_2, PIN_BEAM_3, PIN_BEAM_4, PIN_BEAM_5
};

// Default scene map. Adjust as you like. This just makes the arrays valid.
SceneFn BEAM_SCENE[6] = {
  scene_frankenphone, // Beam 0
  scene_blood,        // Beam 1
  scene_graveyard,    // Beam 2
  scene_fur,          // Beam 3
  scene_orca,         // Beam 4
  scene_mirror        // Beam 5
};

// If you keep a settings structure with beam pins and codes, wire it here.
// For now we keep safe defaults so other modules can link cleanly.
void apply_mapping_from_settings() {
  // Example if you later add settings:
  // const auto& S = settings_get();
  // for (uint8_t i = 0; i < 6; i++) {
  //   BEAM_PINS[i] = S.beam_pins[i];
  //   BEAM_SCENE[i] = scene_by_code(S.beam_scene[i]);
  // }
}