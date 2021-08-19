#include "config.h"
#include <genesis.h>
#include "game/statemachine.h"

int main()
{
    while (1)
    {
        //char buf[256];
        //intToHex((game_states), buf, 16);
        //SYS_die(buf);
        game_states[game_state_current]();
        // state change
        SYS_doVBlankProcess();
    }
}
