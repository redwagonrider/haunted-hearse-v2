#ifndef SCENES_HPP
#define SCENES_HPP

// Function pointer type for scenes
typedef void (*Scene)();

// Scene declarations
void scene_blackout();
void scene_spider();
void scene_graveyard();
void scene_orca();
void scene_frankenphone();  // ← Triggers Frankenphone FSM
void scene_fire();
void scene_exit();

// External scene list and pins
extern const uint8_t NUM_BEAMS;
extern uint8_t BEAM_PINS[];
extern Scene   BEAM_SCENE[];

#endif