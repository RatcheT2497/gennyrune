#ifndef _SCRIPTMACHINE_H_
#define _SCRIPTMACHINE_H_
#include <genesis.h>
#include "utils.h"

enum {
    SCRF_WAIT_EXT_DELAY       = FLAG(0),
    SCRF_WAIT_TEXTBOX         = FLAG(1),
    SCRF_WAIT_ACTOR_MOVEMENT  = FLAG(2),
    SCRF_WAIT_ACTOR_ANIMATION = FLAG(3),
    SCRFM_WAIT                = FLAG(0) | FLAG(1) | FLAG(2) | FLAG(3),
    
    SCRF_RUNNING              = FLAG(15)
};

extern const u16 scr_class_offset_table[];

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
extern u8 scr_choice;
extern u8 scr_current_actor;
extern u16 scr_current_actor_group;
void ScriptTick(void);

#endif