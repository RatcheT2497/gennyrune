#include <genesis.h>
#include "level.h"
#include "resources.h"
const SpriteDefinition const* lvl_actor_sprite_definitions[] = {
    &spr_kris,
    &spr_kris,
    &spr_kris,
    &spr_kris,
    &spr_placeholder,
};
const Vect2D_s16 lvl_actor_bounds[] = {
    {.x = 14, .y = 14 },
    {.x = 14, .y = 14 },
    {.x = 14, .y = 14 },
    {.x = 14, .y = 14 },
    {.x = 16, .y = 32 },
};
const Vect2D_s16 lvl_actor_offsets[] = {
    {.x = 10, .y = 26 },
    {.x = 10, .y = 26 },
    {.x = 10, .y = 26 },
    {.x = 10, .y = 26 },
    {.x = 0, .y = 0 },
};
