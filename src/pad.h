#ifndef _PAD_H_
#define _PAD_H_
#include <genesis.h>

#define PAD_BTN_HELD(joy, btn) (pad_state_current[(joy)] & (btn))
#define PAD_BTN_PRESSED(joy, btn) ((pad_state_changed[(joy)] & (btn)) && (pad_state_current[(joy)] & (btn)))
#define PAD_BTN_RELEASED(joy, btn) ((pad_state_changed[(joy)] & (btn)) && (!(pad_state_current[(joy)] & (btn))))

extern u16 pad_state_current[];
extern u16 pad_state_changed[];

void PAD_JoyCallback(u16 joy, u16 changed, u16 state);
void PAD_Install(void);
#endif