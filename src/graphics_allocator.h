#ifndef _H_GRAPHICS_ALLOCATOR_
#define _H_GRAPHICS_ALLOCATOR_
#include <genesis.h>
extern u16 gfx_allocated_tile_index;
extern u16 gfx_palette[];
u16 GFX_LoadTileset(const TileSet *tileset, TransferMethod transferMethod);
u16 GFX_LoadTilesetAt(const TileSet *tileset, TransferMethod transferMethod, u16 index);
u16 GFX_DrawImage(const Image *image, u16 x, u16 y, VDPPlane plane, u16 tileAttributes, bool loadPalette);
void GFX_Free(u16 size);
#endif // _H_GRAPHICS_ALLOCATOR_
