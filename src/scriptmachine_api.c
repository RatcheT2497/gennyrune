/// PROJECT:        Deltarune MD
/// FILE:           src/scriptmachine_api.c
/// AUTHOR:         RatcheT2497
/// CREATION:       ???
/// MODIFIED:       23/09/21
/// DESCRIPTION:    File containing definitions for the scriptmachine interpreter's main API, including its variables, registers and memory.
/// CHANGELOG:      (23/09/21) Added this file header. -R#
#include "scriptmachine.h"

// source vars
u8 *scr_source_instructions = NULL;
char *scr_source_textbank = NULL;

// runtime vars/registers
u16 scr_pc = 0;
u16 scr_tx = 0;
u16 scr_flags = 0;
u8 scr_instruction_delay = 0;
u8 scr_registers[8] = {0};
u8 scr_register_slots[32] = {0};
u8 scr_current_actor = 0;
u16 scr_current_actor_group = 0;

inline void scriptInterpret(void)
{
    u8 instruction = scr_source_instructions[scr_pc++];
    scr_instruction_delay = instruction & 3;
    scr_handlers[instruction >> 2]();
}

inline u8 scriptBatchRunning(void)
{
    return  (scr_flags & SCRF_RUNNING) &&
            (scr_instruction_delay == 0) && 
            !(scr_flags & SCRFM_WAIT);
}

// API
void SCR_Tick(void)
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
        register u16 bitmask_copy = scr_current_actor_group;
        u8 actor_id = 0;
        while (bitmask_copy)
        {
            KLog_U1("bitmask: ", bitmask_copy);
            /// TODO: cache actors with finished conditions in a separate global bitmask? 
            /// TODO: add actor animation code
            if (bitmask_copy & 1)
            {
                ActorRuntime_t *actor = lvl_current.actors + actor_id;
                // movement takes precedence
                if (scr_flags & SCRF_WAIT_ACTOR_MOVEMENT)
                {
                    if (actor->mvt_timer != 0)
                        return;
                }
                if (scr_flags & SCRF_WAIT_ACTOR_ANIMATION)
                {
                    SYS_die("unimplemented actor animation system");
                    return;
                }
            }
            // update actor id & bitmask data
            actor_id++;
            bitmask_copy >>= 1;
        }
        FLAG_UNSET(scr_flags, SCRF_WAIT_ACTOR_MOVEMENT);
        FLAG_UNSET(scr_flags, SCRF_WAIT_ACTOR_ANIMATION);
    }

    // execute instructions up to one of the blocking ones
    while ( scriptBatchRunning() )
    {
        scriptInterpret();
    }
}