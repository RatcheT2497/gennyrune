#include "scriptmachine.h"

// source vars
u8 *scr_source_instructions = NULL;
char *scr_source_textbank = NULL;

// runtime vars
u16 scr_pc = 0;
u16 scr_tx = 0;
u16 scr_flags = 0;
u8 scr_instruction_delay = 0;
u8 scr_choice = 0;
u8 scr_current_actor = 0;
u16 scr_current_actor_group = 0;

// API
inline void scriptInterpret(void)
{
    u8 control = scr_source_instructions[scr_pc++];
    u8 instruction = scr_source_instructions[scr_pc++];
    
    u8 class = control & 7;
    u16 offset = scr_class_offset_table[class];

    scr_instruction_delay = control >> 3;
    scr_handlers[instruction + offset]();
}

inline u8 scriptBatchRunning(void)
{
    return  (scr_flags & SCRF_RUNNING) &&
            (scr_instruction_delay == 0) && 
            !(scr_flags & SCRFM_WAIT);
}

void ScriptTick(void)
{
    // handle both instruction delay and ext. delay with the same code
    if (scr_instruction_delay)
    {
        if (!(scr_instruction_delay--)) // decrement delay & check if it's zero
        {
            // if it is, unset the ext. delay bitmask since it uses the same instruction delay variable
            FLAG_UNSET(scr_flags, SCRF_WAIT_EXT_DELAY);
        } else {
            return;
        }
    }

    // no textbox yet lol
    if (scr_flags & SCRF_WAIT_TEXTBOX)
    {
        FLAG_UNSET(scr_flags, SCRF_WAIT_TEXTBOX);
    }

    // handle both actor waits with one loop
    if (scr_flags & (SCRF_WAIT_ACTOR_ANIMATION | SCRF_WAIT_ACTOR_MOVEMENT))
    {
        // don't modify actor group bitmask directly
        u16 bitmask_copy = scr_current_actor_group;
        u8 actor = 0;
        while (bitmask_copy)
        {
            /// TODO: cache actors with finished conditions in a separate global bitmask? 
            /// TODO: also obvious but add the actor code lol
            if (bitmask_copy & 1)
            {
                // movement takes precedence
                if (scr_flags & SCRF_WAIT_ACTOR_MOVEMENT)
                {

                    return;
                }
                if (scr_flags & SCRF_WAIT_ACTOR_ANIMATION)
                {

                    return;
                }
            }
            // update actor id & bitmask data
            actor++;
            bitmask_copy >>= 1;
        }
        FLAG_UNSET(scr_flags, SCRF_WAIT_ACTOR_ANIMATION);
        FLAG_UNSET(scr_flags, SCRF_WAIT_ACTOR_MOVEMENT);
    }

    // execute instructions up to one of the blocking ones
    while ( scriptBatchRunning() ) {
        scriptInterpret();
    }
}