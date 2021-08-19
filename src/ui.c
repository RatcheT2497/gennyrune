#include "ui.h"

#include "utils.h"
#include "graphics_allocator.h"

#include <resources.h>
#define UI_BOX_T TILE_ATTR_FULL(PAL3, 0, 0, 0, 4)
#define UI_BOX_L TILE_ATTR_FULL(PAL3, 0, 0, 0, 5)
#define UI_BOX_R TILE_ATTR_FULL(PAL3, 0, 0, 1, 5)
#define UI_BOX_B TILE_ATTR_FULL(PAL3, 0, 1, 0, 4)
#define UI_BOX_F TILE_ATTR_FULL(PAL3, 0, 0, 0, 6)
#define FX(f) ((f) & 1)
#define FY(f) (((f) & 2)>>1)

enum {
    TBF_SHOWN = FLAG(0),
    TBF_END   = FLAG(1)
};
enum {
    FMT_END = 0,
    FMT_SETPOS = 1,
    FMT_SETDELAY = 2,
    FMT_NL = 0x10,
    FMT_MENU = 0xFF
};
static ui_menu_handler_t **ui_menu_handler_array;
static u8 ui_initialized = 0;

static u16 ui_textbox_tileset_idx = 0xFFFF;
static u16 ui_font_top_idx = 0xFFFF;
static u16 ui_font_bottom_idx = 0xFFFF;

static u8 ui_plane = WINDOW;

static u16 ui_text_x = 0;
static u16 ui_text_y = 0;
static u16 ui_text_idx = 0;
static s8 ui_text_delay = 1;

static u8 ui_textbox_flags = 0;
static ui_textbox_t *ui_textbox_current = NULL;

const char menu_test[] = "\x02\x00\x01\x01\x01Hello World\n\xFF Option1\n Option2\n Option3\n Option4\x00";
static u16 ui_menu_x_buffer[6] = {0};
static u16 ui_menu_y_buffer[6] = {0};
static u8 ui_menu_idx = 0;

TileSet *ui_textbox_tilesets[] = {
    &tls_ui_textbox_light,
    &tls_ui_textbox_dark
};
void UI_Init(u8 init_flags)
{
    if (!ui_initialized)
    {
        ui_textbox_tileset_idx = GFX_LoadTileset(ui_textbox_tilesets[IS_FLAG_SET(init_flags, UIIF_DARK)], DMA);
        memcpy(gfx_palette + 48, pal_ui.data, pal_ui.length);

        ui_font_top_idx = GFX_LoadTileset(&tls_fnt_operator_0, DMA);
        ui_font_bottom_idx = GFX_LoadTileset(&tls_fnt_operator_1, DMA);
        ui_initialized = 1;
    }
}

void _UI_DrawChar(s8 c, u16 x, u16 y)
{
    VDP_setTileMapXY(ui_plane, TILE_ATTR(PAL3, 0, 0, 0) + ui_font_top_idx + c, x, y);
    VDP_setTileMapXY(ui_plane, TILE_ATTR(PAL3, 0, 0, 0) + ui_font_bottom_idx + c, x, y+1);
}
s8 _UI_ParseText(char *text)
{
    static u8 flags = 0;
    u8 c = text[ui_text_idx++];
    switch(c)
    {
        case FMT_END: {
            // end
            if (flags & 1) 
            {
                ui_menu_x_buffer[ui_menu_idx] = ui_text_x;
                ui_menu_y_buffer[ui_menu_idx] = ui_text_y;
                ui_menu_idx++;
            }

            flags = 0;
        } return -1;
        
        case FMT_SETPOS: {
            // set cursor pos
            ui_text_x = text[ui_text_idx++];
            ui_text_y = text[ui_text_idx++];
        } return 0;
        
        case FMT_MENU: {
            // begin/end menu options definition
            flags ^= 1;
            ui_menu_x_buffer[ui_menu_idx] = ui_text_x;
            ui_menu_y_buffer[ui_menu_idx] = ui_text_y;
            ui_menu_idx++;
        } return 0;
        
        case FMT_NL: {
            // newline
            if (flags & 1) 
            {
                ui_menu_x_buffer[ui_menu_idx] = ui_text_x;
                ui_menu_y_buffer[ui_menu_idx] = ui_text_y;
                ui_menu_idx++;
            }
        } return 0;
        
        case FMT_SETDELAY: {
            ui_text_delay = text[ui_text_idx++];
        } return 0;

        default: {
            // text
            _UI_DrawChar(c, ui_text_x, ui_text_y);
        } return ui_text_delay;
    }

    // deadcode but kept just in case
    return 0;
}
void _UI_DrawCorner(u8 plane, u8 flags, u16 x, u16 y)
{
    VDP_setTileMapXY(plane, TILE_ATTR_FULL(PAL3, 0, FY(flags), FX(flags), 0) + ui_textbox_tileset_idx, x + (FX(flags) ? 1 : 0), y + (FY(flags) ? 1 : 0));
    VDP_setTileMapXY(plane, TILE_ATTR_FULL(PAL3, 0, FY(flags), FX(flags), 1) + ui_textbox_tileset_idx, x + (FX(flags) ? 0 : 1), y + (FY(flags) ? 1 : 0));
    VDP_setTileMapXY(plane, TILE_ATTR_FULL(PAL3, 0, FY(flags), FX(flags), 2) + ui_textbox_tileset_idx, x + (FX(flags) ? 1 : 0), y + (FY(flags) ? 0 : 1));
    VDP_setTileMapXY(plane, TILE_ATTR_FULL(PAL3, 0, FY(flags), FX(flags), 3) + ui_textbox_tileset_idx, x + (FX(flags) ? 0 : 1), y + (FY(flags) ? 0 : 1));
}
void UI_DrawBox(ui_box_t box)
{
    for (s16 y = box.y; y < box.y + box.height; y++)
    {
        if (y < 0 || y > PIXEL_TO_TILE(PIXEL_SCREEN_HEIGHT)) continue;
        for (s16 x = box.x; x < box.x + box.width; x++)
        {
            if (x < 0 || x > PIXEL_TO_TILE(PIXEL_SCREEN_WIDTH)) continue;
            VDP_setTileMapXY(ui_plane, UI_BOX_F + ui_textbox_tileset_idx, x, y);
            if (x == box.x)
            {
                VDP_setTileMapXY(ui_plane, UI_BOX_L + ui_textbox_tileset_idx, x, y);
            } 
            else if (x == box.x + box.width - 1)
            {
                VDP_setTileMapXY(ui_plane, UI_BOX_R + ui_textbox_tileset_idx, x, y);
            } 
            else if (y == box.y)
            {
                VDP_setTileMapXY(ui_plane, UI_BOX_T + ui_textbox_tileset_idx, x, y);
            } 
            else if (y == box.y + box.height - 1)
            {
                VDP_setTileMapXY(ui_plane, UI_BOX_B + ui_textbox_tileset_idx, x, y);
            } 
        }
    }
    _UI_DrawCorner(ui_plane, 0, box.x, box.y);
    _UI_DrawCorner(ui_plane, 1, box.x + box.width - 2, box.y);
    _UI_DrawCorner(ui_plane, 2, box.x, box.y + box.height - 2);
    _UI_DrawCorner(ui_plane, 3, box.x + box.width - 2, box.y + box.height - 2);
}
inline void _UI_TextboxClear(void)
{
    /// TODO: proper clear
    VDP_clearPlane(ui_plane, TRUE);
}
void UI_TextBoxShow(ui_textbox_t *textbox)
{
    if (!ui_initialized)
        return;
    UI_DrawBox(textbox->box);
    ui_textbox_current = textbox;
    ui_text_x = textbox->text_x;
    ui_text_y = textbox->text_y;
    ui_text_idx = 0;
    ui_textbox_flags = TBF_SHOWN;
    _UI_TextboxClear();
}
void UI_TextBoxContinue(u8 n)
{
    if (!ui_initialized)
        return;
    if (IS_FLAG_SET(ui_textbox_flags, TBF_END))
        return;
    while (n--)
    {
        _UI_DrawChar(ui_textbox_current->text[ui_text_idx], ui_text_x, ui_text_y);

        ui_text_x++;
        if (ui_text_x >= ui_textbox_current->box.x+ui_textbox_current->box.width)
        {
            ui_text_x = ui_textbox_current->text_x;
            ui_text_y += 2;
        }
        if (ui_text_y > ui_textbox_current->box.y+ui_textbox_current->box.height)
        {
            ui_text_x = 0;
            ui_text_y = 0;
            FLAG_SET(ui_textbox_flags, TBF_END);
            break;
        }
        ui_text_idx++;
    }
}
void UI_TextBoxClose(void)
{
    if (IS_FLAG_SET(ui_textbox_flags, TBF_SHOWN))
    {
        _UI_TextboxClear();
        FLAG_UNSET(ui_textbox_flags, TBF_SHOWN);
    }
}
void UI_Quit(void)
{
    UI_TextBoxClose();
    
}
