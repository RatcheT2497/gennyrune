#include "graphics_allocator.h"
u16 gfx_palette[64] = {0};
u16 gfx_allocated_tile_index = TILE_USERINDEX;
u16 GFX_LoadTileset(const TileSet *tileset, TransferMethod transferMethod)
{
    u16 ret = VDP_loadTileSet(tileset, gfx_allocated_tile_index, transferMethod);
    if (ret == FALSE)
        return 0;
        
    gfx_allocated_tile_index += tileset->numTile;
    return gfx_allocated_tile_index - tileset->numTile;
}
u16 GFX_LoadTilesetData(const u32 *data, u16 len, TransferMethod transferMethod)
{
    VDP_loadTileData(data, gfx_allocated_tile_index, len, transferMethod);
        
    gfx_allocated_tile_index += len;
    return gfx_allocated_tile_index - len;
}
u16 GFX_LoadTilesetAt(const TileSet *tileset, TransferMethod transferMethod, u16 index)
{
    u16 ret;
    u16 saved_index;
    
    saved_index = gfx_allocated_tile_index;
    gfx_allocated_tile_index = index;
    ret = GfxLoadTileset(tileset, transferMethod);
    gfx_allocated_tile_index = saved_index;
    return ret;
}
u16 GFX_DrawImage(const Image *image, u16 x, u16 y, VDPPlane plane, u16 tileAttributes, bool loadPalette)
{
    u16 ret = VDP_drawImageEx(plane, image, tileAttributes + gfx_allocated_tile_index, x, y, loadPalette, TRUE);
    if (ret == FALSE)
        return 0;
    gfx_allocated_tile_index += image->tileset->numTile;
    return gfx_allocated_tile_index - image->tileset->numTile;
}
void GFX_Free(u16 size)
{
    gfx_allocated_tile_index -= size;
}
