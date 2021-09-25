/// PROJECT:        Deltarune MD
/// FILE:           src/graphics_allocator.h
/// AUTHOR:         RatcheT2497
/// CREATION:       ???
/// MODIFIED:       23/09/21
/// DESCRIPTION:    File containing declarations for the graphics allocation module. 
///                 Doesn't actually do any proper allocation - basically just loads tiles and moves the pointer forward.
/// CHANGELOG:      (23/09/21) Added this file header. -R#
#ifndef _GRAPHICS_ALLOCATOR_H_
#define _GRAPHICS_ALLOCATOR_H_
#include <genesis.h>
extern u16 gfx_allocated_tile_index;
extern u16 gfx_last_allocated_tile_index;

extern u16 gfx_palette[];
u16 GFX_LoadTileset(const TileSet *tileset, TransferMethod transferMethod);
u16 GFX_LoadTilesetAt(const TileSet *tileset, TransferMethod transferMethod, u16 index);
u16 GFX_DrawImage(const Image *image, u16 x, u16 y, VDPPlane plane, u16 tileAttributes, bool loadPalette);
void GFX_Free(u16 size);
#endif
