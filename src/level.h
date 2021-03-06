/// PROJECT:        Deltarune MD
/// FILE:           src/level.h
/// AUTHOR:         RatcheT2497
/// CREATION:       ???
/// MODIFIED:       12/10/21
/// DESCRIPTION:    File containing declarations for the level module, including definition, runtime structs and functions. The builk of the game lies here, I'd say.
/// CHANGELOG:      (23/09/21) Added this file header. -R#
///                 (24/09/21) Added declaration for character collision offset/size. -R#
///                            Inlined the visual/logical structs into the actual level definition -R#
///                            Added level tile legend -R#
///                 (25/09/21) Replaced custom LevelActorNode_t struct with simple Vect2D_f16. -R#
///                 (09/10/21) Consolidated actor/character LUTS; renamed to just actor_... -R#
///                 (12/10/21) Added some helper functions for interoperability with GAME module. -R#

#ifndef _LEVEL_H_
#define _LEVEL_H_
#include <genesis.h>
#include "utils.h"

#define ACTOR_COUNT (16)
#define CHARACTER_COUNT (4)
#define DECOR_COUNT (8)

#define PAL_WORLD (PAL0)
#define PAL_ACTORS (PAL1)
#define PAL_CHARACTERS (PAL2)
#define PAL_GUI (PAL3)

#define PARTY_MAX_MEMBERS (4)
#define PARTY_FOLLOW_OFFSET (16)
#define PARTY_MAX_FOLLOW_QUEUE (PARTY_FOLLOW_OFFSET * (PARTY_MAX_MEMBERS-1))

/*
    COLLISION TILES:
    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F (trigger)
    .. ## O/ \O \   / .. .. .. .. .. .. .. .. .. ~~
    .. ## /   \ O\ /O .. .. .. .. .. .. .. .. .. ~~ 
*/
enum {
    CHAR_KRIS = FLAG(0),
    CHAR_RALSEI = FLAG(1),
    CHAR_SUSIE = FLAG(2)
};
/// TODO: remove this & figure out a better actor/entity type system
enum {
    ENT_ACTOR = 0,
};
enum {
    ENF_HIDDEN = FLAG(0),
};
/// TODO: make all of these structures simpler somehow
typedef struct { 
    u8      index;
    u8      type;
    u8      flags;
    u8      padding;
    fix16   x;
    fix16   y;
} EntityDefinition_t;

/// TODO: remove this and replace with vector
typedef struct { u8 width; u8 height; } EntitySize_t;

typedef struct {
    u8 type;
    u8 flags;
    fix16 x;
    fix16 y;
    u8 direction;
    u8 animation;
    u8 frame;
    s8 frame_timer; // if positive, counts down. else next animation frame. if negative, then frozen

    // for scripting system
    s8 mvt_node; // vvvvvvtt - type 0: level node; type 1: level mvt. sequence; type 2: ???; type 3: none;
    u8 mvt_speed;

    Sprite *sprite;
} ActorRuntimeData_t; // 16 bytes
typedef struct {
    u8 actor_id;
} CharacterRuntimeData_t;

// Logic portion of a level definition 
typedef struct {
    u16 flags;

    u16 target;
    u16 condition;
    
    u16 x;
    u16 y;
    u8  width; 
    u8  height;
} LevelTrigger_t;
typedef struct {
    u16                 flags;

    // logic data
    u16                 width;
    u16                 height;
    u8*                 collision_map;

    char*               text_bank;
    u8*                 cutscene_bank;

    u8                  actor_node_count;
    u8                  entity_count;
    u8                  trigger_count;
    /// TODO: make use of this byte
    u8                  padding; 

    Vect2D_f16*         actor_nodes;
    EntityDefinition_t* entities;
    LevelTrigger_t*     triggers;

    // visual data
    MapDefinition*      background_map;
    TileSet*            background_tileset;
    Palette*            background_palette;
    
    Palette*            actor_palette;

} LevelDefinition_t; // 52

typedef struct {
    /// TODO: dynamic actor count + unified actor system
    // character data
    CharacterRuntimeData_t      characters[CHARACTER_COUNT];
    s8                          character_follow_queue_rear;
    fix16                       character_follow_queue_x[PARTY_MAX_FOLLOW_QUEUE];
    fix16                       character_follow_queue_y[PARTY_MAX_FOLLOW_QUEUE];
    u8                          character_follow_queue_dir[PARTY_MAX_FOLLOW_QUEUE];

    /// TODO: add back room decor, possibly only as SGDK Sprites
    ActorRuntimeData_t         *actors;//[ACTOR_COUNT];
    ActorRuntimeData_t         *lead_character_actor;
    
    Map*                        map;
    fix16                       cam_x; 
    fix16                       cam_y;
} LevelRuntime_t; // 280 bytes

// defined in file generated from resources.gen
extern const SpriteDefinition *lvl_actor_sprite_definitions[];

extern const Vect2D_s16 lvl_actor_offsets[];
extern const Vect2D_s16 lvl_actor_bounds[];

extern LevelRuntime_t lvl_current;
extern LevelDefinition_t *lvl_current_definition;
extern u8 lvl_current_party;

extern fix16 lvl_player_x, lvl_player_y;
extern u8 lvl_player_direction;

void LVL_Init(const LevelDefinition_t *definition);

void LVL_FocusCamera(void);
void LVL_Scroll(void);

void LVL_Update(void);
#endif