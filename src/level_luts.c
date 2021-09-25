/// PROJECT:        Deltarune MD
/// FILE:           src/level_luts.c
/// AUTHOR:         RatcheT2497
/// CREATION:       ???
/// MODIFIED:       25/09/21
/// DESCRIPTION:    File containing definitions for a few entity & character specific LUTs.
/// CHANGELOG:      (23/09/21) Added this file header. -R#
///                 (24/09/21) Added character collision offset & size data. -R#
#include <genesis.h>
#include "level.h"
#include "resources.h"
const SpriteDefinition *lvl_entity_sprite_definitions[] = { &spr_placeholder };
const EntitySize_t lvl_entity_bounds[] = { 
    {2*8, 5*8} 
};

const SpriteDefinition const* lvl_character_sprites_lw[4] = {
    &spr_kris,
    NULL,
    NULL,
    NULL
};
const Vect2D_s16 lvl_character_bounds_lw[] = {
    { .x = 16, .y = 16 },
    { .x = 0, .y = 0 },
    { .x = 0, .y = 0 },
    { .x = 0, .y = 0 }
};
const Vect2D_s16 lvl_character_offsets_lw[] = {
    { .x = 8, .y = 24 },
    { .x = 0, .y = 0 },
    { .x = 0, .y = 0 },
    { .x = 0, .y = 0 }
};