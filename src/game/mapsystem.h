#ifndef _MAP_SYSTEM_H_
#define _MAP_SYSTEM_H_
#include <genesis.h>
#include "content_base.h"

extern Map* map_current;
extern MapHeader_t* map_current_header;
enum {
    MAPF_PARTY_START = FLAG(0)
};
enum {
    ACTF_ENABLED = FLAG(0),
    ACTF_VISIBLE = FLAG(1),
};
enum {
    ACTOR_PLAYER,
    ACTOR_FOLLOWER,
    ACTOR_CUTSCENE
};
typedef struct __attribute__((packed)) CutsceneActorData {
    s16 dx;
    s16 dy;
    u16 target_framecount;
} CutsceneActorData_t;

typedef struct  __attribute__((packed)) Actor {
    u16 flags; 
    u16 type;
    fix16 x; 
    fix16 y;
    u8 animation_speed, animation_timer, frame;
    u8 data[8];
    Sprite *sprite;
} Actor_t;
extern Actor_t map_actors[];

extern s16 map_camera_pixel_x, map_camera_pixel_y;
void MapLoad(u8 index);
void MapUnload(void);
void MapUpdate(void);
#endif