#include "pad.h"

u16 pad_state_current[8]   = {0};
u16 pad_state_changed[8] = {0};

void PAD_JoyCallback(u16 joy, u16 changed, u16 state)
{
    pad_state_current[joy] = state;
    pad_state_changed[joy] = changed;
}

inline void PAD_Install(void)
{
    JOY_setEventHandler(PAD_JoyCallback);
}