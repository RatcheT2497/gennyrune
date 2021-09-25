/// PROJECT:        Deltarune MD
/// FILE:           src/editor.c
/// AUTHOR:         RatcheT2497
/// CREATION:       ???
/// MODIFIED:       23/09/21
/// DESCRIPTION:    File containing definitions for the editor module's API, and its functionality. Effectively turned off if NEDITOR is defined in src/config.h
/// CHANGELOG:      (23/09/21) Added this file header. -R#
#include <genesis.h>
#include <resources.h>
#include "editor.h"
#include "utils.h"
#include "pad.h"
#include "config.h"
#include "level.h"
#include "graphics_allocator.h"
#ifdef NEDITOR
void EDT_PreInit(void) {}
void EDT_Init(void) {}
void EDT_Tick(void) {}
#else

#define TIMELINE_START_X (6)
#define TIMELINE_END_X (TILE_SCREEN_WIDTH - 1)
#define TIMELINE_CONTENT_START_X (TIMELINE_START_X+1)
#define TIMELINE_CONTENT_END_X (TIMELINE_END_X-1)

#define TIMELINE_HEIGHT (8)
#define TIMELINE_START_Y (TILE_SCREEN_HEIGHT - TIMELINE_HEIGHT)
typedef void (*_EDT_StateHandler_t)(void);
enum {
    SEL_NONE,
    SEL_REPOSITION
};
enum {
    EDT_MAIN,
    EDT_TIMELINE,
    EDT_SIDEBAR
};

typedef struct {
    u8 *bytecode_buffer;
    u16 bytecode_buffer_used;
    u8 bytecode_page;

    u16 cursor_x, cursor_y, cursor_old_x, cursor_old_y;
    u8 cursor_speed;
    u8 cursor_select_state;

    Sprite *cursor_sprite;
    Sprite *cursor_tl;
    Sprite *cursor_br;

    Sprite *menu_selector;
} EDTState_t;
static EDTState_t *_edt_state = NULL;
static u8 _edt_bytecode_buffer[kB(2)] = {0};
static u16 _edt_bytecode_buffer_used = 16;
static u8 _edt_bytecode_page = 0;
static u8 _edt_state_current = 0;
static u8 cursor_select_state = 0;
static ActorRuntime_t *cursor_selected_actor = NULL;
static Sprite *cursor_sprite = NULL;
static Sprite *cursor_top_left = NULL;
static Sprite *cursor_bottom_right = NULL;

static Sprite *_edt_menu_selector = NULL;

static u16 cursor_x = 0;
static u16 cursor_y = 0;
static u16 cursor_old_x = 0;
static u16 cursor_old_y = 0;
static u8 cursor_speed = 1;
void EDT_PreInit(void) 
{
    // load cursor palette
    /// TODO: just use the regular UI palette (of course have to build a UI module first)
    memcpy(gfx_palette + 48, spr_crosshair.palette->data, spr_crosshair.palette->length*2);

    _edt_state = MEM_alloc(sizeof(EDTState_t));
}
void _EDT_ShowAllActors(void)
{
    for (u8 actor_id = 0; actor_id < 16; actor_id++)
    {
        static ActorRuntime_t *actor = NULL;
        actor = lvl_current.actors + actor_id;
        if ((s16)actor->flags < 0 && IS_FLAG_UNSET(actor->flags, ENT_VISIBLE))
        {
            SPR_setVisibility(actor->sprite, AUTO_SLOW);
        }
    }
}

ActorRuntime_t *_edt_highlighted_actors[16];
bool _EDT_FindHighlightedActorList(void)
{
    static ActorRuntime_t *temp;
    static u8 i = 0;
    for (u8 actor_id = 0; actor_id < 16; actor_id++)    
    {
        temp = lvl_current.actors + actor_id;
        if ((s16)temp->flags < 0)
        {
            if (cursor_x >= fix16ToInt(temp->x) && cursor_y >= fix16ToInt(temp->y) &&
                cursor_x <= fix16ToInt(temp->x) + lvl_entity_bounds[temp->type].width && cursor_y <= fix16ToInt(temp->y) + lvl_entity_bounds[temp->type].height)
            {
                _edt_highlighted_actors[i++] = temp;
            }
        }
    }
    return (i > 0);
}
bool _EDT_FindHighlightedActor(ActorRuntime_t **dest)
{
    static ActorRuntime_t *temp;
    for (u8 actor_id = 0; actor_id < 16; actor_id++)    
    {
        temp = lvl_current.actors + actor_id;
        if ((s16)temp->flags < 0)
        {
            if (cursor_x >= fix16ToInt(temp->x) && cursor_y >= fix16ToInt(temp->y) &&
                cursor_x <= fix16ToInt(temp->x) + lvl_entity_bounds[temp->type].width && cursor_y <= fix16ToInt(temp->y) + lvl_entity_bounds[temp->type].height)
            {
                *dest = temp;
                return TRUE;
            }
        }
    }
    return FALSE;
}
void _EDT_MoveCursor(void)
{
    // cursor movement
    if (PAD_BTN_HELD(JOY_1, BUTTON_C))
        cursor_speed = 2;
    else
        cursor_speed = 1;
    cursor_old_x = cursor_x;
    cursor_old_y = cursor_y;
    if (PAD_BTN_HELD(JOY_1, BUTTON_RIGHT))
        cursor_x += cursor_speed;
    if (PAD_BTN_HELD(JOY_1, BUTTON_LEFT))
        cursor_x -= cursor_speed;
    if (PAD_BTN_HELD(JOY_1, BUTTON_DOWN))
        cursor_y += cursor_speed;
    if (PAD_BTN_HELD(JOY_1, BUTTON_UP))
        cursor_y -= cursor_speed;
    SPR_setPosition(cursor_sprite, cursor_x, cursor_y);
}
void _EDT_PrepareTimeline(void)
{
    VDP_waitVBlank(TRUE);
    {
        VDP_fillTileMapRect(WINDOW, TILE_ATTR_FULL(PAL3, TRUE, FALSE, FALSE, 1), 0, TIMELINE_START_Y, TILE_SCREEN_WIDTH, TIMELINE_HEIGHT);
        VDP_setTextPriority(TRUE);
        VDP_drawText("TIMELINE", 5, TIMELINE_START_Y);
        VDP_drawText("SAVE", 1, TIMELINE_START_Y + 1);
        VDP_drawText("LOAD", 1, TIMELINE_START_Y + 2);
        VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL3, TRUE, FALSE, FALSE, TILE_FONTINDEX + '<' - ' '), TIMELINE_START_X, TILE_SCREEN_HEIGHT - 4);
        VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL3, TRUE, FALSE, FALSE, TILE_FONTINDEX + '>' - ' '), TIMELINE_END_X, TILE_SCREEN_HEIGHT - 4);
        
        for (u8 i = 0; i < 16; i++)
        {
            if (i + (_edt_bytecode_page << 4) >= _edt_bytecode_buffer_used)
                break;
            VDP_drawText("@ ", TIMELINE_CONTENT_START_X + i + i, TIMELINE_START_Y+1);
        }
    }
    VDP_setWindowVPos(TRUE, TIMELINE_START_Y);
    
    SPR_setVisibility(_edt_menu_selector, AUTO_FAST);
    SPR_setPosition(_edt_menu_selector, 0, (TILE_SCREEN_HEIGHT-7)*8);

}
void _EDT_ShutdownTimeline(void)
{
    VDP_waitVBlank(TRUE);
    VDP_setWindowVPos(FALSE, 0);
    SPR_setVisibility(_edt_menu_selector, HIDDEN);
}
s16 cursor_actor_depth = 0;
void _EDT_UpdateMainState(void)
{
    _EDT_MoveCursor();
    
    bool actor_found = FALSE;
    switch (cursor_select_state)
    {
        case SEL_NONE: {
            if (_EDT_FindHighlightedActor(&cursor_selected_actor))
            {
                SPR_setVisibility(cursor_top_left, AUTO_FAST);
                SPR_setVisibility(cursor_bottom_right, AUTO_FAST);
                SPR_setPosition(cursor_top_left, fix16ToInt(cursor_selected_actor->x), fix16ToInt(cursor_selected_actor->y));
                SPR_setPosition(cursor_bottom_right, fix16ToInt(cursor_selected_actor->x) + lvl_entity_bounds[cursor_selected_actor->type].width, fix16ToInt(cursor_selected_actor->y) + lvl_entity_bounds[cursor_selected_actor->type].height);
                actor_found = TRUE;
            } else {
                SPR_setVisibility(cursor_top_left, HIDDEN);
                SPR_setVisibility(cursor_bottom_right, HIDDEN);
            }
            if (PAD_BTN_PRESSED(JOY_1, BUTTON_A) && actor_found)
            {
                SPR_setVisibility(cursor_sprite, HIDDEN);
                cursor_select_state = SEL_REPOSITION;
                cursor_actor_depth = cursor_selected_actor->sprite->depth;
                SPR_setDepth(cursor_selected_actor->sprite, SPR_MAX_DEPTH);
            }
            if (PAD_BTN_PRESSED(JOY_1, BUTTON_START))
            {
                _EDT_PrepareTimeline();
                _edt_state_current = EDT_TIMELINE;
            }
        } break;
        case SEL_REPOSITION: {
            if (PAD_BTN_PRESSED(JOY_1, BUTTON_A))
            {
                SPR_setDepth(cursor_selected_actor->sprite, cursor_actor_depth);
                SPR_setVisibility(cursor_sprite, AUTO_FAST);
                cursor_select_state = SEL_NONE;
            }
            cursor_selected_actor->x += FIX16(cursor_x - cursor_old_x);
            cursor_selected_actor->y += FIX16(cursor_y - cursor_old_y);
            SPR_setPosition(cursor_top_left, fix16ToInt(cursor_selected_actor->x), fix16ToInt(cursor_selected_actor->y));
            SPR_setPosition(cursor_bottom_right, fix16ToInt(cursor_selected_actor->x) + lvl_entity_bounds[cursor_selected_actor->type].width, fix16ToInt(cursor_selected_actor->y) + lvl_entity_bounds[cursor_selected_actor->type].height);
        } break;
    }
}
void _EDT_UpdateTimelineState(void)
{
    if (PAD_BTN_PRESSED(JOY_1, BUTTON_START))
    {
        _EDT_ShutdownTimeline();
        _edt_state_current = EDT_MAIN;
    }
}
void _EDT_UpdateSidebarState(void)
{

}

static const _EDT_StateHandler_t edt_state_update[] = {
    _EDT_UpdateMainState,
    _EDT_UpdateTimelineState,
    _EDT_UpdateSidebarState,
};

void EDT_Init(void) 
{
    // load editor font
    VDP_loadFont(&tls_editor_font, DMA);
    VDP_setTextPalette(PAL3);
    VDP_setTextPlane(WINDOW);
    VDP_setTextPriority(TRUE);
    // create cursor sprite
    cursor_sprite = SPR_addSprite(&spr_crosshair, cursor_x, cursor_y, TILE_ATTR(PAL3, FALSE, FALSE, FALSE));
    SPR_setDepth(cursor_sprite, SPR_MIN_DEPTH);

    // create actor selection "box" sprites
    cursor_top_left = SPR_addSprite(&spr_crosshair, 0, 0, TILE_ATTR(PAL3, FALSE, FALSE, FALSE));
    cursor_bottom_right = SPR_addSprite(&spr_crosshair, 0, 0, TILE_ATTR(PAL3, FALSE, FALSE, FALSE));
    SPR_setDepth(cursor_top_left, SPR_MIN_DEPTH);
    SPR_setDepth(cursor_bottom_right, SPR_MIN_DEPTH);

    // create menu selector sprites
    _edt_menu_selector = SPR_addSprite(&spr_selector_small, 0, 0, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
    SPR_setDepth(_edt_menu_selector, SPR_MIN_DEPTH);
    SPR_setVisibility(_edt_menu_selector, HIDDEN);

    // initialize variables
    cursor_selected_actor = NULL;
    _edt_state_current = 0;
    
    _EDT_ShowAllActors();
}
void EDT_Tick(void) 
{
    edt_state_update[_edt_state_current]();
}

#endif