#include "scriptmachine.h"

// tables
#define I(name) static void ins##name(void)
// textbox instructions
I(TextboxShowText)
{

}
I(TextboxContinueText)
{
    
}

I(TextboxShowChoice2)
{

}
I(TextboxShowChoice4)
{

}
I(TextboxShowChoiceFaux2)
{

}
// actor instructions
I(ActorSetCurrent)
{
    scr_current_actor = scr_source_instructions[scr_pc++];
}
I(ActorSetMvtX)
{

}
I(ActorSetMvtY)
{

}
I(ActorSetMvtFrames)
{

}
I(ActorSetAnimIndex)
{

}
I(ActorSetAnimSpeed)
{

}
I(ActorGroupSet)
{
    scr_current_actor_group = *(u16*)(scr_source_instructions + (scr_pc++));
}
I(ActorGroupFlagSet)
{
    u8 bitmask_copy = scr_current_actor_group;
    u8 actor = 0;
    while (bitmask_copy)
    {
        if (bitmask_copy & 1)
        {
        
        }
        actor++;
        bitmask_copy >>= 1;
    }
}
I(ActorGroupFlagUnset)
{

}
// system instructions
I(SystemWaitExtendedDelay)
{
    FLAG_SET(scr_flags, SCRF_WAIT_EXT_DELAY);
    scr_instruction_delay = scr_source_instructions[scr_pc++];
}
I(SystemWaitTextboxClose)
{
    FLAG_SET(scr_flags, SCRF_WAIT_TEXTBOX);
}
I(SystemWaitActorsStopMovement)
{
    FLAG_SET(scr_flags, SCRF_WAIT_ACTOR_MOVEMENT);
}
I(SystemWaitActorsStopAnimations)
{
    FLAG_SET(scr_flags, SCRF_WAIT_ACTOR_ANIMATION);
}
I(SystemBranch)
{
    // 4 bytes, yowza but makes switches a bit easier to do (mul. by 4 should be faster than mul. by 3)
    s16 offset = *((s16*)(scr_source_instructions + (scr_pc++)));
    scr_pc += offset;
}
I(SystemSwitchBranchOnChoice)
{
    // 3 bytes
    u8 max_choice = scr_source_instructions[scr_pc++];
    if (scr_choice <= max_choice)
    {
        scr_pc += scr_choice * 4;
    }
}
I(SystemExit)
{
    FLAG_UNSET(scr_flags, SCRF_RUNNING);
}
#undef I

#define SCR_CLASS_TEXTBOX_OFFSET (0)
#define SCR_CLASS_ACTOR_OFFSET   (5)
#define SCR_CLASS_WORLD_OFFSET   (0)
#define SCR_CLASS_SYSTEM_OFFSET  (14)
/// TODO: rethink this
const u16 scr_class_offset_table[] = { 
    SCR_CLASS_TEXTBOX_OFFSET, 
    SCR_CLASS_ACTOR_OFFSET, 
    SCR_CLASS_WORLD_OFFSET, 
    0, 
    0,
    0,
    SCR_CLASS_SYSTEM_OFFSET,
};
/*const*/ script_handler_fn_t *scr_handlers[] = {
    insTextboxShowText,
    insTextboxContinueText,
    insTextboxShowChoice2,
    insTextboxShowChoice4,
    insTextboxShowChoiceFaux2,

    insActorSetCurrent,
    insActorSetMvtX,
    insActorSetMvtY,
    insActorSetMvtFrames,
    insActorSetAnimIndex,
    insActorSetAnimSpeed,
    insActorGroupSet,
    insActorGroupFlagSet,
    insActorGroupFlagUnset,

    insSystemWaitExtendedDelay,
    insSystemWaitActorsStopMovement,
    insSystemWaitActorsStopAnimations,
    insSystemWaitTextboxClose,
    insSystemBranch,
    insSystemSwitchBranchOnChoice,
    insSystemExit
};
