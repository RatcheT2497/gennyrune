/// PROJECT:        Deltarune MD
/// FILE:           src/graphics_allocator.h
/// AUTHOR:         RatcheT2497
/// CREATION:       ???
/// MODIFIED:       25/09/21
/// DESCRIPTION:    File containing declarations for the graphics allocation module. 
///                 Doesn't actually do any proper allocation - basically just loads tiles and moves the pointer forward.
/// CHANGELOG:      (23/09/21) Added this file header. -R#
///                 (25/09/21) Added variables for manual vblank palette handling. -R#
#ifndef _GRAPHICS_ALLOCATOR_H_
#define _GRAPHICS_ALLOCATOR_H_
#include <genesis.h>
#define GFX_PALETTE_FADEOUT (0b00000000)
#define GFX_PALETTE_FADEIN (0b00000010)

extern u16 gfx_allocated_tile_index;
extern u16 gfx_last_allocated_tile_index;
extern volatile u8 gfx_palette_dirty;
extern volatile u8 gfx_palette_fade_time;
extern u16 gfx_palette[];
u16 GFX_LoadTileset(const TileSet *tileset, TransferMethod transferMethod);
u16 GFX_LoadTilesetAt(const TileSet *tileset, TransferMethod transferMethod, u16 index);
u16 GFX_DrawImage(const Image *image, u16 x, u16 y, VDPPlane plane, u16 tileAttributes, bool loadPalette);
void GFX_Free(u16 size);
#endif
