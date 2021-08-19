
#ifndef _UTILS_H_
#define _UTILS_H_

#define FLAG(n) (1 << (n))
#define IS_FLAG_SET(v, f) (((v) & (f)))
#define IS_FLAG_UNSET(v, f) (((v) & (f)) == FALSE)
#define FLAG_SET(v, f) v |= (f)
#define FLAG_UNSET(v, f) v &= (0xFF ^ (f))
#define FLAG_FLIP(v, f) v ^= (f)

#define METATILE_TO_TILE(v) ((v) << 1)
#define METATILE_TO_PIXEL(v) ((v) << 4)
#define TILE_TO_PIXEL(v) ((v) << 3)

#define TILE_TO_METATILE(v) ((v) >> 1)
#define PIXEL_TO_METATILE(v) ((v) >> 4)
#define PIXEL_TO_TILE(v) ((v) >> 3)

#define PIXEL_SCREEN_WIDTH (320)
#define PIXEL_SCREEN_HEIGHT (224)
#define TILE_SCREEN_WIDTH (PIXEL_TO_TILE(PIXEL_SCREEN_WIDTH))
#define TILE_SCREEN_HEIGHT (PIXEL_TO_TILE(PIXEL_SCREEN_HEIGHT))

#define countof(a) (sizeof((a)) / sizeof((a)[0]))
#define kB(v) ((v) * 1024)
#define mB(v) ((v) * 1048576)

#endif