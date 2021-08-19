#include <genesis.h>
#include "utils.h"
#include "graphics_allocator.h"

#include "editor/editor.h"
#include "game/statemachine.h"

#include "game/scriptmachine.h"
#include "game/mapsystem.h"

#include "state_game.h"

/*
    physical cutscene triggers:
    struct { u8 x; u8 y; u8 width; u8 height; u16 button_press; u16 script_index; }
    - on kris enter with conditional button press

    map cutscene triggers:
    struct { ... u16 load_script_index; u16 unload_script_index; }
    - on map load
    - on map unload
*/

void StateGame(void)
{
    u8 option = 0;
    
    SYS_disableInts();
        VDP_setScreenWidth320();
        VDP_setScreenHeight224();
        VDP_clearPlane(BG_A, TRUE);
        VDP_clearPlane(BG_B, TRUE);
    SYS_enableInts();
    EditorInit();
    PAL_setColors(0, gfx_palette, 64);
    //VDP_drawText("in game", 4, 4);
    
    while (!option)
    {

        if (EditorTick())
        {
            ScriptTick();
            //MapUpdate();
        }
        SYS_doVBlankProcess();
    }
    game_state_current = option;
    EditorQuit();
}
