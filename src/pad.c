/// PROJECT:        Deltarune MD
/// FILE:           src/pad.c
/// AUTHOR:         RatcheT2497
/// CREATION:       ???
/// MODIFIED:       23/09/21
/// DESCRIPTION:    File containing definitions for the pad module.
/// CHANGELOG:      (23/09/21) Added this file header. -R#
#include "pad.h"

u16 pad_state_current[8] = {0};
u16 pad_state_changed[8] = {0};

void PAD_JoyCallback(u16 joy, u16 changed, u16 state)
{
    pad_state_current[joy] = state;
    pad_state_changed[joy] = changed;
}

/// TODO: learn more about the inline keyword...
inline void PAD_Update(void)
{
    memset(pad_state_changed, 0, sizeof(u16)*8);
}