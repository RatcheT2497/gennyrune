/// PROJECT:        Deltarune MD
/// FILE:           src/textbox.c
/// AUTHOR:         RatcheT2497
/// CREATION:       25/09/21
/// MODIFIED:       04/10/21
/// DESCRIPTION:    File containing definitions for the textbox API.
/// CHANGELOG:      (25/09/21) Added this file header alongside the basic skeleton API functions. -R#
///                 (04/10/21) Added basic windowing code to the show/hide functions. -R#
#include "textbox.h"
#include <resources.h>
#include "level.h"
#include "graphics_allocator.h"
u16 _txb_tileset_address = 0xFFFF;
void TXB_Init(void)
{
    if (_txb_tileset_address != 0xFFFF)
    {
        //_txb_tileset_address = GFX_LoadTileset(NULL, DMA_QUEUE);
        //memcpy(gfx_palette + PAL_GUI, NULL, 16*2);
    }
}
void TXB_Show(void)
{
    VDP_setWindowVPos(TRUE, TILE_SCREEN_HEIGHT - 16);
}
void TXB_Hide(void)
{
    VDP_setWindowVPos(FALSE, 0);
}
void TXB_Update(void)
{

}
