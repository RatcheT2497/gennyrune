/// PROJECT:        Deltarune MD
/// FILE:           src/scriptmachine_instructions.c
/// AUTHOR:         RatcheT2497
/// CREATION:       ???
/// MODIFIED:       23/09/21
/// DESCRIPTION:    File containing definitions for the scriptmachine interpreter's instructions, alongside a lookup table at the bottom of the file.
/// CHANGELOG:      (23/09/21) Added this file header. -R#
#include "scriptmachine.h"

// tables
#define I(name) static void run##name(void); static void ins##name(void) { KLog(#name); run##name(); } static void run##name(void)
#define IBYTE (scr_source_instructions[scr_pc++])
#define IWORD ((scr_source_instructions[scr_pc++] << 8) | scr_source_instructions[scr_pc++])

// textbox instructions
I(TextboxShowText)
{
    SYS_die(__FUNCTION__);
    u16 text_offset = IWORD;
    scr_tx = text_offset;
    /// TODO: implement textbox
}
I(TextboxContinueText)
{
    SYS_die(__FUNCTION__);
    /// TODO: implement textbox
}

// actor instructions
I(ActorSetCurrent)
{
    scr_current_actor = IBYTE;
}
I(ActorSetMvtVec)
{
    u8 x = IBYTE;
    u8 y = IBYTE;
    lvl_current.actors[scr_current_actor].mvt_dx = FIX16(x);
    lvl_current.actors[scr_current_actor].mvt_dy = FIX16(y);
}
I(ActorSetMvtFrames)
{
    u8 f = IBYTE;
    lvl_current.actors[scr_current_actor].mvt_timer = f;
}
I(ActorSetAnim)
{
    SYS_die(__FUNCTION__);
}
I(ActorGroupSet)
{
    scr_current_actor_group = IWORD;
}
I(ActorGroupFlagSet)
{
    SYS_die(__FUNCTION__);
    u16 bitmask_copy = scr_current_actor_group;
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
I(ActorGroupShow)
{
    register u16 bitmask_copy = scr_current_actor_group;
    u8 actor_id = 0;
    while (bitmask_copy)
    {
        if (bitmask_copy & 1)
        {
            ActorRuntime_t *actor = lvl_current.actors + actor_id;
            if ((s16)actor->flags < 0)
            {
                if (actor->sprite)
                {
                    SPR_setVisibility(actor->sprite, AUTO_FAST);
                    FLAG_UNSET(actor->flags, ENF_HIDDEN);
                }
            }
        }
        actor_id++;
        bitmask_copy >>= 1;
    }
}
I(ActorGroupHide)
{
    SYS_die(__FUNCTION__);
    register u16 bitmask_copy = scr_current_actor_group;
    u8 actor_id = 0;
    while (bitmask_copy)
    {
        if (bitmask_copy & 1)
        {
            ActorRuntime_t *actor = lvl_current.actors + actor_id;
            if ((s16)actor->flags < 0)
            {
                if (actor->sprite)
                {
                    SPR_setVisibility(actor->sprite, HIDDEN);
                    FLAG_SET(actor->flags, ENF_HIDDEN);
                }
            }
        }
        actor_id++;
        bitmask_copy >>= 1;
    }
}
I(ActorGroupFlagIsSet)
{
    SYS_die(__FUNCTION__);
}
I(ActorGroupFlagUnset)
{
    SYS_die(__FUNCTION__);
}
// system instructions
I(SystemWaitExtendedDelay)
{
    FLAG_SET(scr_flags, SCRF_WAIT_EXT_DELAY);
    scr_instruction_delay = IBYTE;
}
I(SystemWaitTextboxFinish)
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
    s8 offset = IBYTE;

    // highest bit set means it's a conditional branch
    if (offset < 0)
    {
        // get rid of conditional flag and sign... extend? what's the opposite of extend
        // if the number isn't negative it removes the conditional flag, otherwise it's fine
        if (!(offset & 0x4000))
            offset &= ~0x8000;
        if (IS_FLAG_SET(scr_flags, SCRF_CONDITION))
        {
            scr_pc += offset;
        }
    } else {
        // sign extend
        // proper one this time
        if (offset & 0x4000)
            offset |= 0x8000;
        scr_pc += offset & ~0x8000;
    }
}
I(SystemSwitchRegister)
{
    /// TODO: figure out if 0x10000 is enough space for cutscene data
    u8 control = IBYTE;
    u8 reg = control & 0b00000111;
    u8 max_choice = control >> 3;

    if (scr_registers[reg] <= max_choice)
    {
        scr_pc = IWORD;
    } else {
        scr_pc += max_choice * 2;
    }
}
I(SystemLoadRegisterImmediate)
{
    // immediate is 5 bits
    u8 imm = IBYTE;
    u8 reg = imm & 7;
    imm >>= 3;
    scr_registers[reg] = imm;
}
I(SystemLoadRegister)
{
    // 32 slots
    u8 slot = IBYTE;
    u8 reg = slot & 7;
    slot >>= 3;

    scr_registers[reg] = scr_register_slots[slot];
}
I(SystemAddRegisterImmediate)
{
    u8 imm = IBYTE;
    u8 reg = imm & 7;
    imm >>= 3;

    scr_registers[reg] += imm;
}
I(SystemSubRegisterImmediate)
{
    u8 imm = IBYTE;
    u8 reg = imm & 7;
    imm >>= 3;

    scr_registers[reg] -= imm;
}
I(SystemSaveRegister)
{
    u8 imm = IBYTE;
    u8 reg = imm & 7;
    imm >>= 3;

    scr_register_slots[imm] = scr_registers[reg];
}
I(SystemCompareRegisterImmediate)
{
    u8 imm = IBYTE;
    u8 reg = imm & 7;
    imm >>= 3;

    if (scr_registers[reg] == imm)
    {
        FLAG_SET(scr_flags, SCRF_CONDITION);
    } else {
        FLAG_UNSET(scr_flags, SCRF_CONDITION);
    }
}
I(SystemReturn)
{
    /// TODO: actual return logic instead of plain ol' exit
    FLAG_UNSET(scr_flags, SCRF_RUNNING);
}
I(SystemScreenFadeOut) 
{
    SYS_die(__FUNCTION__);
}
I(SystemScreenFadeIn) 
{
    SYS_die(__FUNCTION__);
}
I(SystemMapLoad) 
{
    SYS_die(__FUNCTION__);
}
I(SystemPlayAudio)
{
    SYS_die(__FUNCTION__);
}
#undef I
/*
    requirements:
    - async. textbox actions/actor movement
    - timed actions
*/
/*const*/ script_handler_fn_t *scr_handlers[] = {
    /* 
        x - not started
        u - untested
        V - finished
    */
    /* [FIN] [MNEMONIC]        [OPCODE] [BYTE 1] [BYTE 2]              [HEX] */
    /*   x   txb-show          ......dd iiiiiiii iiiiiiii              0x00  */ insTextboxShowText,
    /*   x   txb-cont          ......dd                                0x04  */ insTextboxContinueText,
    /*   u   act-cur           ......dd aaaaaaaa                       0x08  */ insActorSetCurrent,
    /*   u   act-set-mvt-vec   ......dd xxxxxxxx yyyyyyyy              0x0C  */ insActorSetMvtVec,
    /*   u   act-set-mvt-cnt   ......dd ffffffff                       0x10  */ insActorSetMvtFrames,
    /*   x   act-set-mvt-type  ......dd tttttttt                       0x14  */ NULL,
    /*   u   act-set-anim      ......dd iiiiiiii ffffffff              0x18  */ insActorSetAnim,
    /*   u   agr-cur           ......dd abcdefgh ijklmnop              0x1C  */ insActorGroupSet,
    /*   x   agr-set-flag      ......dd ....ffff                       0x20  */ insActorGroupFlagSet,
    /*   x   agr-unset-flag    ......dd ....ffff                       0x24  */ insActorGroupFlagUnset,
    /*   x   agr-is-flag-set   ......dd ....ffff                       0x28  */ insActorGroupFlagIsSet,
    /*   x   agr-show          ......dd                                0x2C  */ insActorGroupShow,
    /*   x   agr-hide          ......dd                                0x30  */ insActorGroupHide,
    /*   x   scr-fade-out      ......dd                                0x34  */ insSystemScreenFadeOut,
    /*   x   scr-fade-in       ......dd                                0x38  */ insSystemScreenFadeIn,
    /*   x   map-load          ......dd iiiiiiii                       0x3C  */ insSystemMapLoad,
    /*   x   snd-play          ......dd iiiiiiii                       0x40  */ insSystemPlayAudio,
    /*   u   wait-frames       ......dd dddddddd                       0x44  */ insSystemWaitExtendedDelay,
    /*   u   wait-agr-movement ......dd                                0x48  */ insSystemWaitActorsStopMovement,
    /*   u   wait-agr-anim     ......dd                                0x4C  */ insSystemWaitActorsStopAnimations,
    /*   u   wait-txb-finish   ......dd                                0x50  */ insSystemWaitTextboxFinish,
    /*   u   reg-load-imm      ......dd iiiiirrr                       0x54  */ insSystemLoadRegisterImmediate,
    /*   u   reg-load          ......dd sssssrrr                       0x58  */ insSystemLoadRegister,
    /*   u   reg-save          ......dd sssssrrr                       0x5C  */ insSystemSaveRegister,
    /*   u   reg-cmp-imm       ......dd iiiiirrr                       0x60  */ insSystemCompareRegisterImmediate,
    /*   u   reg-add-imm       ......dd iiiiirrr                       0x64  */ insSystemAddRegisterImmediate,
    /*   u   reg-sub-imm       ......dd iiiiirrr                       0x68  */ insSystemSubRegisterImmediate,
    /*   u   reg-switch        ......dd nnnnnrrr [iiiiiiii iiiiiiii]*n 0x6C  */ insSystemSwitchRegister,
    /*   u   sys-branch        ......dd ciiiiiii                       0x70  */ insSystemBranch,
    /*   x   return            ......dd                                0x74  */ insSystemReturn,
    /*   x                   UNIMPLEMENTED INSTRUCTION                       */ insSystemReturn,
    /*   x                   UNIMPLEMENTED INSTRUCTION                       */ insSystemReturn
    /*  instructions with high bits set - unnecessary to add into rom        */
};
