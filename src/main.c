/// PROJECT:        Deltarune MD
/// FILE:           src/main.c
/// AUTHOR:         RatcheT2497
/// CREATION:       ???
/// MODIFIED:       23/09/21
/// DESCRIPTION:    Main project file. Contains the entry point for the program, initialization and main game loop.
///                 Also includes some content which I hope to refactor out later.
/// CHANGELOG:      (23/09/21) Added this file header. -R#

#include <genesis.h>
#include "utils.h"
#include <genesis.h>
#include <resources.h>
#include "config.h"
#include "graphics_allocator.h"
#include "level.h"
#include "scriptmachine.h"
#include "pad.h"
//#include "keyboard.h"
#include "editor.h"
typedef struct {
    u8 flags;
    u8 frames;
    u16 node;
} ActorAnimationFrame_t;
/// TODO: refactor content out into its own file
const EntityDefinition_t test_level_entities[] = {
    { 
        .index = 0,
        .flags = 0,
        .type = ENT_ACTOR,
        .x = FIX16(16),
        .y = FIX16(16)
    },
    { 
        .index = 1,
        .flags = 0,
        .type = ENT_ACTOR,
        .x = FIX16(64),
        .y = FIX16(48)
    },
};
const LevelDefinition_t test_level = {
    .actor_palette = &pal_placeholder_spr,

    .background_map = &map_placeholder,
    .background_tileset = &tls_placeholder_map,
    .background_palette = &pal_placeholder_map,

    .cutscene_bank = cut_test,
    .text_bank = NULL,

    .trigger_count = 0,
    .triggers = NULL,
    
    .entity_count = 2,
    .entities = test_level_entities
};
const LevelTrigger_t triggers_torihouse_krisroom[] = {
    {
        .flags = 0x8000,
        .x = 9,
        .y = 12,
        .width = 2,
        .height = 2,
        
        .button = 2, // room
        .script = 0  // node
    }
};
const LevelActorNode_t nodes_torihouse_krisroom[] = {
    { .x = FIX16(160), .y = FIX16(112) }
};
const LevelDefinition_t level_torihouse_krisroom = {
    .flags = CHAR_KRIS,

    .actor_palette = &pal_placeholder_spr,
    .background_map = &map_torihouse_krisroom,
    .background_tileset = &tls_torihouse_krisroom,
    .background_palette = &pal_torihouse_krisroom,

    .width = 320,
    .height = 224,
    .collision_map = col_torihouse_krisroom,

    .cutscene_bank = NULL,
    .text_bank = NULL,

    .actor_node_count = 1,
    .actor_nodes = nodes_torihouse_krisroom,
    
    .trigger_count = 1,
    .triggers = triggers_torihouse_krisroom,

    .entity_count = 0,
    .entities = NULL
};
const LevelActorNode_t nodes_torihouse_hallway[] = {
    { .x = FIX16(280), .y = FIX16(128) }
};

const LevelTrigger_t triggers_torihouse_hallway[] = {
    {
        .flags = 0x8000,
        .x = 16,
        .y = 6,
        .width = 3,
        .height = 2,
        
        .button = 1, // room
        .script = 0  // node
    }
};
const LevelDefinition_t level_torihouse_hallway = {
    .flags = CHAR_KRIS,

    .actor_palette = &pal_placeholder_spr,
    .background_map = &map_torihouse_hallway,
    .background_tileset = &tls_torihouse_hallway,
    .background_palette = &pal_torihouse_hallway,

    .width = 512,
    .height = 224,
    .collision_map = col_torihouse_hallway,

    .cutscene_bank = NULL,
    .text_bank = NULL,

    .actor_node_count = 1,
    .actor_nodes = nodes_torihouse_hallway,

    .trigger_count = 1,
    .triggers = triggers_torihouse_hallway,

    .entity_count = 0,
    .entities = NULL
};
const LevelDefinition_t level_torihouse_lobby = {
    .actor_palette = &pal_placeholder_spr,
    .background_map = &map_torihouse_lobby,
    .background_tileset = &tls_torihouse_lobby,
    .background_palette = &pal_torihouse_lobby,

    .cutscene_bank = NULL,
    .text_bank = NULL,

    .trigger_count = 0,
    .triggers = NULL,

    .entity_count = 0,
    .entities = NULL
};
const LevelDefinition_t level_torihouse_bathroom = {
    .actor_palette = &pal_placeholder_spr,
    .background_map = &map_torihouse_bathroom,
    .background_tileset = &tls_torihouse_bathroom,
    .background_palette = &pal_torihouse_bathroom,

    .cutscene_bank = NULL,
    .text_bank = NULL,

    .trigger_count = 0,
    .triggers = NULL,

    .entity_count = 0,
    .entities = NULL
};
const LevelDefinition_t const* levels[] = {
    &test_level,
    &level_torihouse_krisroom,
    &level_torihouse_hallway,
    &level_torihouse_lobby,
    &level_torihouse_bathroom
};

void callback(char c, u16 modifier_keys)
{
    static char buffer[16] = {0};
    sprintf(buffer, "char: %c", c);
    KLog(buffer);
}
int main(void)
{
    /// INFO: compiler requires -fms-extensions for the nested structures used in level.h
    SYS_disableInts();
        VDP_setPlaneSize(64, 64, TRUE);
        VDP_setScreenWidth320();
        VDP_setScreenHeight224();
        SPR_init();
        JOY_setEventHandler(&PAD_JoyCallback);
//          SYS_setVIntCallback(&KBD_VIntCallback);
    SYS_enableInts();
    //KBD_SetCharWriteCallback(callback);

    // clear palette
    memsetU16(gfx_palette, 0xffff, 16*4);

    //EDT_PreInit();
    LVL_Init(levels[1]);
    VDP_waitVBlank(TRUE);
    PAL_fadeInAll(gfx_palette, 16, FALSE);
    //EDT_Init();
    
    while (1)
    {
        //EDT_Tick();
        LVL_Update();
        SPR_update();
        PAD_Update();
        SYS_doVBlankProcess();
    }
}
