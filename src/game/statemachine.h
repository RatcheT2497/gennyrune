#ifndef _MAIN_STATEMACHINE_H_
#define _MAIN_STATEMACHINE_H_
#include <types.h>
// ugly but works

extern u16 game_state_current;

typedef void (*GameStateFuncPtr_t)(void);
extern const GameStateFuncPtr_t game_states[];

#include "states.inc"

#define _STATE_START STATE_TITLESCREEN

#define _DEFINE_STATE_LIST_ENUM(v, n) STATE_ ## n,
enum {
    _STATE_LIST(_DEFINE_STATE_LIST_ENUM)
};

#ifdef _MAIN_STATEMACHINE_IMPL_
#define _DEFINE_STATE_LIST_ITEMS(v, n) (v),
const GameStateFuncPtr_t game_states[] = {
    _STATE_LIST(_DEFINE_STATE_LIST_ITEMS)
};

u16 game_state_current = _STATE_START;
#endif
#endif