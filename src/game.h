/// PROJECT:        Deltarune MD
/// FILE:           src/game.h
/// AUTHOR:         RatcheT2497
/// CREATION:       11/10/21
/// MODIFIED:       11/10/21
/// DESCRIPTION:    File containing declarations for the game module.
/// CHANGELOG:      (23/09/21) Created this file. -R#
#ifndef _GAME_H_
#define _GAME_H_
#include <genesis.h>
#include "level.h"
extern const LevelDefinition_t const* levels[];
void GAME_Init(void);
void GAME_SwitchLevel(u16 index);

#endif