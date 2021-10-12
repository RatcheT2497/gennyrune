/// PROJECT:        Deltarune MD
/// FILE:           src/game.c
/// AUTHOR:         RatcheT2497
/// CREATION:       11/10/21
/// MODIFIED:       11/10/21
/// DESCRIPTION:    File containing definitions for the game module.
/// CHANGELOG:      (23/09/21) Created this file. -R#
///                            Implemented switch level function. -R#
#include "game.h"
#include "graphics_allocator.h"
#include <genesis.h>
void GAME_Init(void)
{

}
void GAME_SwitchLevel(u16 index)
{
    // just in case of sgdk shenanigans, wait for vblank before fades & other stuff
    SYS_doVBlankProcess();
    PAL_fadeOutAll(16, FALSE);
    LVL_Init(levels[index]);

    SYS_doVBlankProcess();
    LVL_FocusCamera();
    LVL_Update();
    SPR_update();

    SYS_doVBlankProcess();
    PAL_fadeInAll(gfx_palette, 16, FALSE);
}
