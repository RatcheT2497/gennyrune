#include "editor.h"
#include "resources.h"
#include "graphics_allocator.h"
#include "utils.h"
#include "ui.h"
enum {
    EDITOR_NONE,
    EDITOR_TILEMAP,
    EDITOR_CUTSCENE,
};
static u8 editor_state = EDITOR_CUTSCENE;
static u8 editor_flags = 0;
static u16 editor_game_run = 0;

const ui_box_t editor_cutscene_box = {.x = 0, .y = TILE_SCREEN_HEIGHT - 10, .width = TILE_SCREEN_WIDTH, .height = 10};
void editorInitState(void)
{
    switch(editor_state)
    {
        case EDITOR_CUTSCENE: {
            UI_DrawBox(editor_cutscene_box);
            VDP_setWindowVPos(TRUE, TILE_SCREEN_HEIGHT - 10);
        } break;
        default:
            break;
    }
}
void EditorInit(void)
{
    JOY_setSupport(PORT_2, JOY_SUPPORT_MOUSE);
    UI_Init(0);
    editorInitState();
}
void EditorQuit(void)
{
    UI_Quit();
}
u16 EditorTick(void)
{
    s16 x = JOY_readJoypadX(JOY_2);
    s16 y = JOY_readJoypadY(JOY_2);
    return editor_game_run;
}
