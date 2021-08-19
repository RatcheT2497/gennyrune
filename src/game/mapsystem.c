#include "game/mapsystem.h"
#include "graphics_allocator.h"
#include "utils.h"
#include "scriptmachine.h"
//#include "content/maps.h"
//#include "resources.h"
#define SCREEN_TILE_WIDTH (40)
#define SCREEN_TILE_HEIGHT (28)
s16 map_camera_pixel_x = 0, map_camera_pixel_y = 0;
Map* map_current = NULL;
MapHeader_t* map_current_header = NULL;

// cutscene-modifiable:
//   4 party actors  
//   4 cutscene actors
//   8 map actors
Actor_t map_actors[16];

typedef void ActorUpdate_t(Actor_t *actor, u8 id);
void do_nothing(Actor_t *actor, u8 id) { }
void updateActorCutscene(Actor_t *actor, u8 id)
{
    CutsceneActorData_t *data = (CutsceneActorData_t*) &actor->data;
    if (data->target_framecount)
    {
        actor->x += data->dx;
        actor->y += data->dy;
        data->target_framecount--;
    }
}
/*const*/ ActorUpdate_t *actor_update_table[] = {
    do_nothing, // player
    do_nothing, // follower
    updateActorCutscene, // cutscene actor
};
/*
    PAL0 - background palette
    PAL1 - playable character palette
    PAL2 - background actor palette
    PAL3 - UI palette
*/
void MapLoad(u8 index)
{
#if 0
    KLog(" before map unload");
    MapUnload();
    KLog(" after map unload / before map header set");
    /// TODO: content pipeline
    //map_current_header = &(content_maps[index]); 
    KLog(" after map header set / before tileset load");
    
    // install map - tileset, palette & scrolling tilemap
    GfxLoadTileset(map_current_header->map_tileset, CPU);
    KLog(" after tileset load / before palette copy");
    memcpy(gfx_palette, map_current_header->map_palette->data, map_current_header->map_palette->length);
    /// TODO: content pipeline
    //memcpy(gfx_palette + 16, pal_main_party.data, pal_main_party.length);
    memcpy(gfx_palette + 32, map_current_header->actor_palette->data, map_current_header->actor_palette->length);
    KLog(" after palette copy / before map creation");
    map_current = MAP_create(map_current_header->map_definition, BG_B, TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, TILE_USERINDEX));
    
    KLog(" after map creation / before map scroll to");
    MAP_scrollTo(map_current, map_camera_pixel_x, map_camera_pixel_y);
    KLog(" after map scroll to / before actor array zero");
    
    // zero out dynamic actors array
    memset(map_actors, 0, sizeof(map_actors));
    KLog_U1(" before actor load - count: ", map_current_header->actor_count);
    
    // install actors
    for (u8 i = 0; i < map_current_header->actor_count; i++)
    {
        MapActorDef_t *src = &map_current_header->actor_defs[i];
        Actor_t *dest = &map_actors[i + (IS_FLAG_SET(map_current_header->flags, MAPF_PARTY_START) ? 0 : 4) ];
        
        // create a single sprite for each actor
        KLog_U1("before actor sprite creation. id: ", i);
        dest->sprite = SPR_addSprite(src->sprite_def, fix16ToInt(src->x), fix16ToInt(src->y), TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
        if (!dest->sprite)
        {
            char buffer[128];
            sprintf(buffer, "could not add actor sprite for actor %d!", i);
            SYS_die(buffer);
        }

        KLog("before actor sprite frame set");
        SPR_setAnimAndFrame(dest->sprite, src->animation, 0);

        KLog("before actor sprite visibility set");
        if (IS_FLAG_SET(src->flags, ACTF_VISIBLE))
        {
            SPR_setVisibility(dest->sprite, AUTO_FAST);
        } else {
            SPR_setVisibility(dest->sprite, HIDDEN);
        }
        KLog_S1("before actor sprite depth set ", src->depth);
        SPR_setDepth(dest->sprite, src->depth);
        KLog("after actor sprite depth set");

        // copy/set actor data (could probably be done wtih memcpy?)
        dest->animation_timer = dest->animation_speed = 0;
        dest->frame = 0;
        dest->type = src->type;
        dest->x = src->x;
        dest->y = src->y;
        dest->flags = src->flags | ACTF_ENABLED;
    }

    // set up script sources
    scr_source_instructions = map_current_header->script_instructions;
    scr_source_textbank = map_current_header->script_textbank;
    scr_pc = 0;
    FLAG_SET(scr_flags, SCRF_RUNNING); // run script - first instruction can be exit if there's nothing to run
#endif
}

void MapUpdate(void)
{
    #if 0
    // clamp camera values to edges of map
    if (map_camera_pixel_x > METATILE_TO_PIXEL(map_current_header->metatile_width) - TILE_TO_PIXEL(SCREEN_TILE_WIDTH))
        map_camera_pixel_x = METATILE_TO_PIXEL(map_current_header->metatile_width) - TILE_TO_PIXEL(SCREEN_TILE_WIDTH);
    if (map_camera_pixel_y > METATILE_TO_PIXEL(map_current_header->metatile_height) - TILE_TO_PIXEL(SCREEN_TILE_HEIGHT) )
        map_camera_pixel_y = METATILE_TO_PIXEL(map_current_header->metatile_height) - TILE_TO_PIXEL(SCREEN_TILE_HEIGHT);
    if (map_camera_pixel_x < 0)
        map_camera_pixel_x = 0;
    if (map_camera_pixel_y < 0)
        map_camera_pixel_y = 0;

    // update actors
    for (u8 i = 0; i < countof(map_actors); i++)
    {
        Actor_t *actor = &map_actors[i];
        if (IS_FLAG_SET(actor->flags, ACTF_ENABLED))
        {
            if (actor->type != 0xFFFF)
            {
                ActorUpdate_t *func = actor_update_table[actor->type];
                func(actor, i);
            }

            if (actor->sprite)
            {
                // update animation
                actor->animation_timer++;
                if (actor->animation_timer == actor->animation_speed)
                {
                    actor->frame++;
                    if ( (actor->sprite->animation->numFrame) != 0)
                        actor->frame = actor->frame % (actor->sprite->animation->numFrame);
                    else actor->frame = 0;
                    
                    actor->animation_timer = 0;
                }
            }
            
            if (IS_FLAG_SET(actor->flags, ACTF_VISIBLE))
            {
                SPR_setFrame(actor->sprite, actor->frame);
                if (actor->sprite->visibility != AUTO_FAST)
                    SPR_setVisibility(actor->sprite, AUTO_FAST);
            } else {
                if (actor->sprite->visibility != HIDDEN)
                    SPR_setVisibility(actor->sprite, HIDDEN);
            }
            SPR_setPosition(actor->sprite, fix16ToInt(actor->x) - map_camera_pixel_x, fix16ToInt(actor->y) - map_camera_pixel_y);
        }
    }
    
    // update tilemap
    MAP_scrollTo(map_current, map_camera_pixel_x, map_camera_pixel_y);
    #endif
}
void MapUnload(void)
{
    #if 0
    if (map_current)
    {
        for (u8 i = 0; i < map_current_header->actor_count; i++)
        {
            Actor_t *dest = &map_actors[i+4];
            if (dest->sprite)
                SPR_releaseSprite(dest->sprite);

            memset(dest, 0, sizeof(Actor_t));
        }
        MEM_free(map_current);
    }
    #endif
}