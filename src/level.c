/// PROJECT:        Deltarune MD
/// FILE:           src/level.c
/// AUTHOR:         RatcheT2497
/// CREATION:       ???
/// MODIFIED:       12/10/21
/// DESCRIPTION:    File containing definitions for the level module and its API. 
///                 Also includes some internal things such as player movement, but I hope to refactor that out later.
/// CHANGELOG:      (23/09/21) Added this file header. -R#
///                 (24/09/21) Added basic collision detection to player movement.
///                            Cut down on data copying by storing a pointer to the current level definition. -R#
///                 (25/09/21) Added room transition trigger code to the player movement.
///                            Partially untied player position and direction from the level data. (NOTE: should maybe think of untying it completely/making a separate struct for it...)
///                            Began working on custom palette management for room transitions. -R#
///                 (11/10/21) Moved palette handling stuff to the GAME module. -R#
///                 (12/10/21) Defined helper functions for GAME module interoperability. -R#
#include <genesis.h>
#include <resources.h>
#include "game.h"
#include "level.h"
#include "graphics_allocator.h"
#include "scriptmachine.h"
#include "pad.h"
#define PLR_WALK_SPEED (FIX16(1.5f))
#define PLR_RUN_SPEED (FIX16(3.0f))
extern const LevelDefinition_t const* levels[];
LevelRuntime_t lvl_current = {0};
LevelDefinition_t *lvl_current_definition = NULL;
u8 lvl_current_party = CHAR_KRIS;
u8 lvl_current_trigger = 0xFF;

fix16 lvl_player_x = 0, lvl_player_y = 0;
u8 lvl_player_direction = 0;

static inline bool _LVL_CharIsOnTile(ActorRuntimeData_t *character, u8 tile, u16 *ox, u16 *oy)
{
    s16 x1, y1, x2, y2, x, y;
    Vect2D_s16 offset = lvl_actor_offsets[character->type];
    Vect2D_s16 size = lvl_actor_bounds[character->type];
    x1 = (fix16ToInt(character->x) + offset.x) / 16;
    y1 = (fix16ToInt(character->y) + offset.y) / 16;
    x2 = (fix16ToInt(character->x) + offset.x + size.x) / 16;
    y2 = (fix16ToInt(character->y) + offset.y + size.y) / 16;

    x1 = clamp(x1, 0, lvl_current_definition->width / 16);
    x2 = clamp(x2, 0, lvl_current_definition->width / 16);
    y1 = clamp(y1, 0, lvl_current_definition->height / 16);
    y2 = clamp(y2, 0, lvl_current_definition->height / 16);
    for (y = y1; y <= y2; y++)
    {
        for (x = x1; x <= x2; x++)
        {
            u8 _tile = ((lvl_current_definition->collision_map[(x/2) + y * lvl_current_definition->width/32] >> (( 1 ^ (x & 1)) * 4)) & 0xF);
            if (_tile == tile)
            {
                *ox = x;
                *oy = y;
                return TRUE;
            }
        }
    }
    return FALSE;
}
static inline bool _LVL_CharCanPass(ActorRuntimeData_t *character, fix16 dx, fix16 dy)
{
    s16 x1, y1, x2, y2, x, y;
    Vect2D_s16 offset = lvl_actor_offsets[character->type];
    Vect2D_s16 size = lvl_actor_bounds[character->type];
    x1 = (fix16ToInt(character->x + dx) + offset.x) / 16;
    y1 = (fix16ToInt(character->y + dy) + offset.y) / 16;
    x2 = (fix16ToInt(character->x + dx) + offset.x + size.x) / 16;
    y2 = (fix16ToInt(character->y + dy) + offset.y + size.y) / 16;

    x1 = clamp(x1, 0, lvl_current_definition->width / 16);
    x2 = clamp(x2, 0, lvl_current_definition->width / 16);
    y1 = clamp(y1, 0, lvl_current_definition->height / 16);
    y2 = clamp(y2, 0, lvl_current_definition->height / 16);

    for (y = y1; y <= y2; y++)
    {
        for (x = x1; x <= x2; x++)
        {
            if (((lvl_current_definition->collision_map[(x/2) + y * lvl_current_definition->width/32] >> (( 1 ^ (x & 1)) * 4)) & 0xF) == 1)
                return FALSE;
        }
    }
    return TRUE;
}
static inline void _LVL_ExecuteTrigger(u16 x, u16 y)
{
    for (u8 i = 0; i < lvl_current_definition->trigger_count; i++)
    {
        LevelTrigger_t *trigger = lvl_current_definition->triggers + i;
        if (x >= trigger->x && y >= trigger->y && x < trigger->x + trigger->width && y < trigger->y + trigger->height)
        {
            // already in this trigger, do nothing
            if ( lvl_current_trigger == i )
                return;

            if ((s16)(trigger->flags) < 0)
            {
                // teleport trigger
                u16 levelid = trigger->condition;
                u8 node_id = (u8)trigger->target;
                Vect2D_f16 *node;
                LevelDefinition_t *newlevel = levels[levelid];

                // fade out 
                //while(gfx_palette_dirty); // wait for enough vblanks
                //SYS_disableInts();
                //    gfx_palette_dirty = GFX_PALETTE_FADEOUT | TRUE;
                    //gfx_palette_fade_time = 16;
                //SYS_enableInts();
                //while(gfx_palette_dirty); // wait for enough vblanks
                // initialize level and new player coordinates
                node = newlevel->actor_nodes + node_id;
                lvl_player_x = node->x;
                lvl_player_y = node->y;
                GAME_SwitchLevel(levelid);
                //LVL_Init(newlevel);
            } else {
                // script trigger
                /// TODO: button filter
                /// TODO: testing
                scr_pc = trigger->target;
                scr_flags |= SCRF_RUNNING;
            }
            lvl_current_trigger = i;
        }
    }
}
/// TODO: make this into its own module maybe? feels weird having it as level functions
void LVL_PartyFollowQueueDequeue()
{
    u8 i;
    ASSERT(lvl_current.character_follow_queue_rear == 0, "LVLPFQ underflow");
    for (i = 0; i < lvl_current.character_follow_queue_rear - 1; i++)
    {
        lvl_current.character_follow_queue_x[i] = lvl_current.character_follow_queue_x[i + 1];
        lvl_current.character_follow_queue_y[i] = lvl_current.character_follow_queue_y[i + 1];
        lvl_current.character_follow_queue_dir[i] = lvl_current.character_follow_queue_dir[i + 1];
    }
    lvl_current.character_follow_queue_rear--;
}
void LVL_PartyFollowQueueEnqueue(fix16 x, fix16 y, u8 dir)
{
    if (lvl_current.character_follow_queue_rear == PARTY_MAX_FOLLOW_QUEUE)
        LVL_PartyFollowQueueDequeue(); // circular tape kinda dealio, removes one element on overflow

    lvl_current.character_follow_queue_x[lvl_current.character_follow_queue_rear] = x;
    lvl_current.character_follow_queue_y[lvl_current.character_follow_queue_rear] = y;
    lvl_current.character_follow_queue_dir[lvl_current.character_follow_queue_rear] = dir;
    lvl_current.character_follow_queue_rear++;
}
static inline void _LVL_UpdatePlayer(void)
{
    /// TODO: move static variable to proper character variable
    static fix16 current_speed = PLR_WALK_SPEED;
    bool running = FALSE;
    fix16 dx = 0, dy = 0;
    u16 tx, ty;
    u8 lead_dir = lvl_current.lead_character_actor->direction;

    /// TODO: slow buildup to running speed
    if (PAD_BTN_HELD(JOY_1, BUTTON_B))
    {
        current_speed = PLR_RUN_SPEED;
        running = TRUE;
    } else {
        current_speed = PLR_WALK_SPEED;
        running = FALSE;
    }

    // movement
    if (PAD_BTN_HELD(JOY_1, BUTTON_DOWN))
    {
        lead_dir = 0;
        dy += current_speed;
    }
    if (PAD_BTN_HELD(JOY_1, BUTTON_UP))
    {
        lead_dir = 2;
        dy += -current_speed;
    }
    if (PAD_BTN_HELD(JOY_1, BUTTON_LEFT))
    {
        lead_dir = 3;
        dx += -current_speed;
    }
    if (PAD_BTN_HELD(JOY_1, BUTTON_RIGHT))
    {
        lead_dir = 1;
        dx += current_speed;
    }
    /// TODO: add support for slopes
    // basic collision detection
    bool noclip = PAD_BTN_HELD(JOY_1, BUTTON_C);
    if (!noclip && !_LVL_CharCanPass(lvl_current.lead_character_actor, dx, 0))
        dx = 0;
    if (!noclip && !_LVL_CharCanPass(lvl_current.lead_character_actor, 0, dy))
        dy = 0;

    // character movement
    lvl_player_x += dx;
    lvl_player_y += dy;

    // position and direction stored as copies so they can be modified/kept as is before level deletion
    lvl_current.lead_character_actor->x = lvl_player_x;
    lvl_current.lead_character_actor->y = lvl_player_y;

    if (_LVL_CharIsOnTile(lvl_current.lead_character_actor, 0xF, &tx, &ty))
    {
        // is inside trigger
        _LVL_ExecuteTrigger(tx, ty);
    } else {
        lvl_current_trigger = 0xFF;
    }

    // scroll screen
    if ( ((dx > 0) && (lvl_player_x - lvl_current.cam_x > FIX16(PIXEL_SCREEN_WIDTH/2))) ||
         ((dx < 0) && (lvl_player_x - lvl_current.cam_x < FIX16(PIXEL_SCREEN_WIDTH/2))) )
    {
        lvl_current.cam_x += dx;
    }
    if ( ((dy > 0) && (lvl_player_y - lvl_current.cam_y > FIX16(PIXEL_SCREEN_HEIGHT/2))) || 
         ((dy < 0) && (lvl_player_y - lvl_current.cam_y < FIX16(PIXEL_SCREEN_HEIGHT/2))) )
    {
        lvl_current.cam_y += dy;
    }

    // animate
    lvl_player_direction = lead_dir;
    lvl_current.lead_character_actor->direction = lvl_player_direction;
    if (dx == 0 && dy == 0)
    {
        // still
        lvl_current.lead_character_actor->animation = lead_dir;
        lvl_current.lead_character_actor->frame_timer = 0;
        lvl_current.lead_character_actor->frame = 0;
    } else {
        // walking
        lvl_current.lead_character_actor->animation = lead_dir + 4;
        lvl_current.lead_character_actor->frame_timer++;
        if (running)
            lvl_current.lead_character_actor->frame_timer++;
        if (lvl_current.lead_character_actor->frame_timer >= 12)
        {
            lvl_current.lead_character_actor->frame = (lvl_current.lead_character_actor->frame + 1) & 3;
            lvl_current.lead_character_actor->frame_timer = 0;
        }
    }
}
static inline void _LVL_UpdateActor(ActorRuntimeData_t *entity)
{
    if (entity->sprite) 
    {
        s16 screen_x = fix16ToInt(entity->x - lvl_current.cam_x);
        s16 screen_y = fix16ToInt(entity->y - lvl_current.cam_y);

        // set depth of sprite
        SPR_setAnimAndFrame(entity->sprite, entity->animation, entity->frame);
        SPR_setZ(entity->sprite, -screen_y);
        SPR_setPosition(entity->sprite, screen_x, screen_y);
    }
}
static inline void _LVL_UpdateCharacters(void)
{
    /// TODO: WIP
}

inline void LVL_FocusCamera(void)
{
    lvl_current.cam_x = lvl_player_x - FIX16(PIXEL_SCREEN_WIDTH/2);
    lvl_current.cam_y = lvl_player_y - FIX16(PIXEL_SCREEN_HEIGHT/2);
}
inline void LVL_Scroll(void)
{
    // camera bounds
    lvl_current.cam_x = clamp(lvl_current.cam_x, 0, FIX16(lvl_current_definition->width - PIXEL_SCREEN_WIDTH));
    lvl_current.cam_y = clamp(lvl_current.cam_y, 0, FIX16(lvl_current_definition->height - PIXEL_SCREEN_HEIGHT));
    
    // scroll map
    if (lvl_current.map)
    {
        MAP_scrollTo(lvl_current.map, fix16ToInt(lvl_current.cam_x), fix16ToInt(lvl_current.cam_y));
    }
}
void LVL_Init(const LevelDefinition_t *definition)
{
    u8 i;
    //SYS_disableInts();
    // store definition pointer
    lvl_current_definition = definition;

    // reset system state
    SPR_reset();
    MEM_free(lvl_current.map);
    memset(&lvl_current, 0, sizeof(LevelRuntime_t));

    // reset camera & scroll
    LVL_FocusCamera();
    //lvl_current.cam_x = 0;
    //lvl_current.cam_y = 0;
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_B, 0);

    // allocate background tileset graphics
    gfx_allocated_tile_index = TILE_USERINDEX;
    GFX_LoadTileset(definition->background_tileset, DMA_QUEUE); // tileset
    memcpy(gfx_palette, definition->background_palette->data, definition->background_palette->length); // palette
    
    /// TODO: compress map data    
    // create scrolling map
    lvl_current.map = MAP_create(definition->background_map, BG_B, TILE_ATTR_FULL(PAL_WORLD, FALSE, FALSE, FALSE, TILE_USERINDEX));
    
    /// TODO: compress collision data
    // set player position
    if (definition->actor_node_count)
    {
        lvl_player_x = definition->actor_nodes[0].x;
        lvl_player_y = definition->actor_nodes[0].y;
    } else {
        lvl_player_x = 0;
        lvl_player_y = 0;
    }
    // initialize entities 
    if (lvl_current.actors) MEM_free(lvl_current.actors);
    u16 actor_size = sizeof(ActorRuntimeData_t) * (definition->entity_count + CHARACTER_COUNT);
    lvl_current.actors = MEM_alloc(actor_size);
    memset(lvl_current.actors, 0, actor_size);

    memcpy(gfx_palette + 16, definition->actor_palette->data, definition->actor_palette->length); // entity palette
    for (i = 0; i < definition->entity_count; i++)
    {
        EntityDefinition_t *def = definition->entities + i;
        if (def->type == ENT_ACTOR)
        {
            ActorRuntimeData_t *actor = lvl_current.actors + def->index + CHARACTER_COUNT;
            actor->x = def->x;
            actor->y = def->y;
            actor->flags = 0x80 | def->flags;
            actor->type = def->type;
            if (lvl_actor_sprite_definitions[actor->type])
            {
                actor->sprite = SPR_addSprite(lvl_actor_sprite_definitions[actor->type], fix16ToInt(def->x - lvl_current.cam_x), fix16ToInt(def->y - lvl_current.cam_y), TILE_ATTR(PAL_ACTORS, FALSE, FALSE, FALSE));
                SPR_setVisibility(actor->sprite, IS_FLAG_SET(actor->flags, ENF_HIDDEN) ? HIDDEN : AUTO_FAST);
            }
        }
    }
    // load characters & set leading character
    memcpy(gfx_palette + 32, pal_main_party.data, pal_main_party.length);
    lvl_current.lead_character_actor = lvl_current.actors;
    for (i = 0; i < CHARACTER_COUNT; i++)
    {
        // if the party member exists and is allowed in the level 
        if ( ((lvl_current_party & (1 << i)) == (definition->flags & (1 << i))) && 
             ((lvl_current_party & (1 << i)) != 0) )
        {
            ActorRuntimeData_t *actor = lvl_current.actors + i;
            lvl_current.characters[i].actor_id = i;

            actor->flags = 0x80; // alive
            actor->type = i;
            actor->x = lvl_player_x;
            actor->y = lvl_player_y;
            actor->direction = lvl_player_direction;
            actor->sprite = SPR_addSprite(lvl_actor_sprite_definitions[i], fix16ToInt(actor->x), fix16ToInt(actor->y), TILE_ATTR(PAL_CHARACTERS, FALSE, FALSE, FALSE));
            SPR_setVisibility(actor->sprite, AUTO_FAST);
        }
    }

    // scripting engine initialization
    scr_source_instructions = definition->cutscene_bank;
    scr_source_textbank = definition->text_bank;

    /// TODO: leave palette update to scripting engine
    //PAL_setColors(0, gfx_palette, 4*16, DMA_QUEUE);
    //gfx_palette_dirty = GFX_PALETTE_FADEIN | TRUE;
    //gfx_palette_fade_time = 16;
    //SYS_enableInts();
    
    /// TODO: figure out if this does anything because it sure as heck doesn't scroll
    MAP_scrollTo(lvl_current.map, 0, 0);
    //while(gfx_palette_dirty); // wait for enough vblanks to load palette
}
void LVL_Update(void)
{
    static u8 i = 0;
    // update interpreter
    SCR_Tick();

    // update player (movement, input)
    _LVL_UpdatePlayer();

    // update characters (party movement, following, etc)
    _LVL_UpdateCharacters();

    // update actors
    for (i = 0; i < lvl_current_definition->entity_count + 4; i++)
    {
        ActorRuntimeData_t *actor = lvl_current.actors + i;
        if (((s8)actor->flags) < 0)
        {
            /// TODO: actor movement code
            u8 node_type = actor->mvt_node & 3;
            u8 node_value = actor->mvt_node >> 2;
            if (node_type == 0)
            {
                // actor node
            } 
            else if (node_type == 1)
            {
                // movement sequence
            }
            _LVL_UpdateActor(actor);
        }
    }
    LVL_Scroll();
}
