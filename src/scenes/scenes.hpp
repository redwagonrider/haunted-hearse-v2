#ifndef SCENES_HPP
#define SCENES_HPP

#include "scene_common.hpp"  // Provides typedef: void (*Scenefn)()

// Declare all scene functions here
void scene_intro();           // Phone loads in / darkness
void scene_blood();           // Blood room or splatter effect
void scene_spiders();         // Spider animatronic + webs
void scene_spider();         // Spider animatronic + webs
void scene_graveyard();       // Graveyard scene
void scene_orca();            // Orca or dinosaur splash scene
void scene_fur();             // Fur room, breathing walls
void scene_frankenphone();    // Frankenphone Lab — electromagnet, modem, display
void scene_mirror();          // Mirror room
void scene_fire();            // Fire illusion or effect
void scene_exit();            // Exit-or-die hole
void scene_secret();          // Hidden final room

// Optional utility scenes
void scene_blackout();        // Full blackout, used as a soft reset
void scene_standby();         // Idle fallback

#endif  // SCENES_HPP