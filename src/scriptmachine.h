/// PROJECT:        Deltarune MD
/// FILE:           src/scriptmachine.h
/// AUTHOR:         RatcheT2497
/// CREATION:       ???
/// MODIFIED:       23/09/21
/// DESCRIPTION:    File containing declarations for functions, variables and constants related to the script machine module. Ties together with the level module.
/// CHANGELOG:      (23/09/21) Added this file header. -R#
#ifndef _SCRIPTMACHINE_H_
#define _SCRIPTMACHINE_H_
#include <genesis.h>
#include "utils.h"
#include "level.h"
#define REG_CHOICE (0)
#define REG_FADE_DATA (1)

enum {
    SCRF_WAIT_EXT_DELAY       = FLAG(0),
    SCRF_WAIT_TEXTBOX         = FLAG(1),
    SCRF_WAIT_ACTOR_MOVEMENT  = FLAG(2),
    SCRF_WAIT_ACTOR_ANIMATION = FLAG(3),
    SCRFM_WAIT                = FLAG(0) | FLAG(1) | FLAG(2) | FLAG(3),
    SCRF_CONDITION            = FLAG(4),
    
    SCRF_RUNNING              = FLAG(15)
};

typedef void script_handler_fn_t(void);
extern /*const*/ script_handler_fn_t *scr_handlers[];

// source
extern u8 *scr_source_instructions;
extern char *scr_source_textbank;

// runtime
extern u16 scr_pc;
extern u16 scr_tx;
extern u16 scr_flags;
extern u8 scr_instruction_delay;
extern u8 scr_registers[];
extern u8 scr_register_slots[];
extern u8 scr_current_actor;
extern u16 scr_current_actor_group;
void SCR_Tick(void);

#endif