/// PROJECT:        Deltarune MD
/// FILE:           src/level.c
/// AUTHOR:         RatcheT2497
/// CREATION:       ???
/// MODIFIED:       25/09/21
/// DESCRIPTION:    File containing definitions for the level module and its API. 
///                 Also includes some internal things such as player movement, but I hope to refactor that out later.
/// CHANGELOG:      (23/09/21) Added this file header. -R#
///                 (24/09/21) Added basic collision detection to player movement. -R#
///                            Cut down on data copying by storing a pointer to the current level definition. -R#
///                 (25/09/21) Added room transition trigger code to the player movement. -R#
///                            Partially untied player position and direction from the level data. -R# (NOTE: should maybe think of untying it completely/making a separate struct for it...)
#include <genesis.h>
#include <resources.h>
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

static inline bool _LVL_CharIsOnTile(CharacterRuntime_t *character, u8 tile, u16 *ox, u16 *oy)
{
    s16 x1, y1, x2, y2, x, y;
    Vect2D_s16 offset = lvl_character_offsets_lw[character->type];
    Vect2D_s16 size = lvl_character_bounds_lw[character->type];
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
            //KLog_U3("TILEFUNC x ", x1, " y ", y1, " t ", _tile);
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
static inline bool _LVL_CharCanPass(CharacterRuntime_t *character, fix16 dx, fix16 dy)
{
    s16 x1, y1, x2, y2, x, y;
    Vect2D_s16 offset = lvl_character_offsets_lw[character->type];
    Vect2D_s16 size = lvl_character_bounds_lw[character->type];
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
                u16 room = trigger->button;
                u8 node_id = (u8)trigger->script;
                LevelActorNode_t *node;

                // fade out 
                VDP_waitVBlank(TRUE);
                PAL_fadeOutAll(16, FALSE);

                // initialize level and new player coordinates
                LVL_Init(levels[room]);
                node = lvl_current_definition->actor_nodes + node_id;
                lvl_player_x = node->x;
                lvl_player_y = node->y;
                _LVL_FocusCamera();
            } else {
                // script trigger
                /// TODO: button filter
                /// TODO: testing
                scr_pc = trigger->script;
                scr_flags |= SCRF_RUNNING;
            }
            lvl_current_trigger = i;
        }
    }
}
/// TODO: make this into its own module maybe? feels weird having it as a level function
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
static inline void _LVL_FocusCamera(void)
{
    lvl_current.cam_x = lvl_player_x - FIX16(PIXEL_SCREEN_WIDTH/2);
    lvl_current.cam_y = lvl_player_y - FIX16(PIXEL_SCREEN_HEIGHT/2);
}
static inline void _LVL_UpdatePlayer(void)
{
    /// TODO: move static variable to proper character variable
    static fix16 current_speed = PLR_WALK_SPEED;
    bool running = FALSE;
    fix16 dx = 0, dy = 0;
    u16 tx, ty;
    u8 lead_dir = lvl_current.character_lead->direction;

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
    if (!noclip && !_LVL_CharCanPass(lvl_current.character_lead, dx, 0))
        dx = 0;
    if (!noclip && !_LVL_CharCanPass(lvl_current.character_lead, 0, dy))
        dy = 0;

    // character movement
    lvl_player_x += dx;
    lvl_player_y += dy;

    // position and direction stored as copies so they can be modified/kept as is before level deletion
    lvl_current.character_lead->x = lvl_player_x;
    lvl_current.character_lead->y = lvl_player_y;

    if (_LVL_CharIsOnTile(lvl_current.character_lead, 0xF, &tx, &ty))
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
    lvl_current.character_lead->direction = lvl_player_direction;
    if (dx == 0 && dy == 0)
    {
        // still
        lvl_current.character_lead->animation = lead_dir;
        lvl_current.character_lead->frame_timer = 0;
        lvl_current.character_lead->frame = 0;
    } else {
        // walking
        lvl_current.character_lead->animation = lead_dir + 4;
        lvl_current.character_lead->frame_timer++;
        if (running)
            lvl_current.character_lead->frame_timer++;
        if (lvl_current.character_lead->frame_timer >= 12)
        {
            lvl_current.character_lead->frame = (lvl_current.character_lead->frame + 1) & 3;
            lvl_current.character_lead->frame_timer = 0;
        }
    }
}

static inline void _LVL_UpdateEntity(BaseEntityRuntime_t *entity)
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
    for (u8 i = 0; i < 4; i++)
    {
        if (lvl_current_party & (1 << i))
        {
            CharacterRuntime_t *character = lvl_current.characters + i;
            _LVL_UpdateEntity((BaseEntityRuntime_t*) character);
        }
    }
}

void LVL_Init(const LevelDefinition_t *definition)
{
    u8 i;
    // wait for vblank
    VDP_waitVBlank(FALSE);
    // store definition pointer
    lvl_current_definition = definition;

    // reset system state
    SPR_reset();
    MEM_free(lvl_current.map);
    memset(&lvl_current, 0, sizeof(LevelRuntime_t));

    // reset camera & scroll
    lvl_current.cam_x = 0;
    lvl_current.cam_y = 0;
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_B, 0);

    // allocate background tileset graphics
    gfx_allocated_tile_index = TILE_USERINDEX;
    GFX_LoadTileset(definition->background_tileset, DMA); // tileset
    memcpy(gfx_palette, definition->background_palette->data, definition->background_palette->length); // palette
    
    /// TODO: compress map data    
    // create scrolling map
    lvl_current.map = MAP_create(definition->background_map, BG_B, TILE_ATTR_FULL(PAL_WORLD, FALSE, FALSE, FALSE, TILE_USERINDEX));
    
    /// TODO: compress collision data
    
    // initialize entities 
    memcpy(gfx_palette + 16, definition->actor_palette->data, definition->actor_palette->length); // entity palette
    for (i = 0; i < definition->entity_count; i++)
    {
        EntityDefinition_t *def = definition->entities + i;
        if (def->type == ENT_ACTOR)
        {
            ActorRuntime_t *actor = lvl_current.actors + def->index;
            actor->x = def->x;
            actor->y = def->y;
            actor->flags = 0x8000 | def->flags;
            actor->type = def->type;
            if (lvl_entity_sprite_definitions[actor->type])
            {
                actor->sprite = SPR_addSprite(lvl_entity_sprite_definitions[actor->type], fix16ToInt(def->x), fix16ToInt(def->y), TILE_ATTR(PAL_ACTORS, FALSE, FALSE, FALSE));
                SPR_setVisibility(actor->sprite, IS_FLAG_SET(actor->flags, ENF_HIDDEN) ? HIDDEN : AUTO_FAST);
            }
        }
    }
    // load characters & set leading character
    memcpy(gfx_palette + 32, pal_main_party.data, pal_main_party.length);
    lvl_current.character_lead = lvl_current.characters;
    for (i = 0; i < 4; i++)
    {
        if ( ((lvl_current_party & (1 << i)) == (definition->flags & (1 << i))) && 
             ((lvl_current_party & (1 << i)) != 0) )
        {
            CharacterRuntime_t temp = {0};
            temp.type = i;
            temp.x = lvl_player_x;
            temp.y = lvl_player_y;
            temp.direction = lvl_player_direction;
            temp.sprite = SPR_addSprite(lvl_character_sprites_lw[i], fix16ToInt(temp.x), fix16ToInt(temp.y), TILE_ATTR(PAL_CHARACTERS, FALSE, FALSE, FALSE));
            SPR_setVisibility(temp.sprite, AUTO_FAST);
            lvl_current.characters[i] = temp;
        }
    }

    // scripting engine initialization
    scr_source_instructions = definition->cutscene_bank;
    scr_source_textbank = definition->text_bank;

    /// TODO: leave palette update to scripting engine
    PAL_setColors(0, gfx_palette, 4*16, DMA_QUEUE);
    /// TODO: figure out if this does anything because it sure as heck doesn't scroll
    MAP_scrollTo(lvl_current.map, 0, 0);
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
    for (i = 0; i < ACTOR_COUNT; i++)
    {
        ActorRuntime_t *actor = lvl_current.actors + i;
        if (((s16)actor->flags) < 0)
        {
            if (actor->mvt_timer != 0)
            {
                actor->x += actor->mvt_dx;
                actor->y += actor->mvt_dy;
                actor->mvt_timer--;
            }
            _LVL_UpdateEntity((BaseEntityRuntime_t*) actor);
        }
    }

    // camera bounds
    lvl_current.cam_x = clamp(lvl_current.cam_x, 0, FIX16(lvl_current_definition->width - PIXEL_SCREEN_WIDTH));
    lvl_current.cam_y = clamp(lvl_current.cam_y, 0, FIX16(lvl_current_definition->height - PIXEL_SCREEN_HEIGHT));
    
    // scroll map
    if (lvl_current.map)
    {
        MAP_scrollTo(lvl_current.map, fix16ToInt(lvl_current.cam_x), fix16ToInt(lvl_current.cam_y));
    }
}
