#ifndef _CONTENT_BASE_H_
#define _CONTENT_BASE_H_
#include <genesis.h>
#include "utils.h"
enum {
    MAPF_DARK = FLAG(0),
};
    
typedef u8 *CutsceneData_t;

typedef struct FontHeader {
    u8 size;
    TileSet *top;
    TileSet *bottom;
} FontHeader_t;

typedef struct MapActorDef {
    u16 flags, type;
    fix16 x, y;
    s16 depth;
    u16 animation;
    SpriteDefinition *sprite_def;
} MapActorDef_t;
typedef struct MapBackground {
    Palette *palette;
    TileSet *tileset;
    MapDefinition *definition;
} MapHeader_t;
//typedef struct MapHeader {
//    u16 flags;
//
//    u16 metatile_width;
//    u16 metatile_height;
//    
//    Palette         *map_palette;
//    TileSet         *map_tileset;
//    MapDefinition   *map_definition;
//    
//    u8              *map_collision;
//
//    Palette         *actor_palette;
//    u8               actor_count;
//    MapActorDef_t   *actor_defs;
//
//    u8              *script_instructions;
//    char            *script_textbank;
//} MapHeader_t;

extern Map* map_current;
extern MapHeader_t* map_current_header;

extern s16 map_camera_pixel_x, map_camera_pixel_y;
void MapLoad(u8 index);
void MapUnload(void);
void MapUpdate(void);
#endif