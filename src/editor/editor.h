#ifndef _EDITOR_H_
#define _EDITOR_H_
#include <genesis.h>
#include "config.h"

#if defined(NEDITOR)
#define EditorInit() 
#define EditorQuit()
#define EditorTick() (1)
#else
void EditorInit(void);
void EditorQuit(void);
u16 EditorTick(void);
#endif

#endif