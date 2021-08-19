#ifndef _UI_H_
#define _UI_H_
#include <genesis.h>
#include "utils.h"
typedef void ui_menu_handler_t(void);
typedef struct ui_box {
    u8 x;
    u8 y;
    u8 width;
    u8 height;
} ui_box_t;

typedef struct ui_textbox {
    ui_box_t box;
    u8       text_x;
    u8       text_y;
    u16      text_char_count;
    char    *text;
} ui_textbox_t;

enum {
    UIIF_DARK = FLAG(0)
};
void UI_Init(u8 init_flags);
void UI_DrawBox(ui_box_t box);
void UI_TextBoxShow(ui_textbox_t *textbox);
void UI_TextBoxContinue(u8 n);
void UI_TextBoxClose(void);

void UI_Quit(void);

#endif