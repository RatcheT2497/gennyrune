#include "state_titlescreen.h"
#include "game/statemachine.h"
#include <genesis.h>
#include "pad.h"
#include "kdebug_fio.h"
struct __attribute__((packed)) file_io_dma {
    uint32_t datalen;
    char *fname;
    uint8_t *data;
};
const u8 testbuffer[] = {0xDE, 0xAD, 0xBE, 0xEF};
void StateTitlescreen(void)
{
    static u16 option = 0;
    PAD_Install();

    SYS_disableInts();
        VDP_setScreenWidth320();
        VDP_setScreenHeight224();
    SYS_enableInts();

    VDP_drawText("Deltarune", 2, 4);
    VDP_drawText("BUT SLIGHTLY WORSE", 2, 5);
    VDP_drawText("> Start", 4, 16);
    KLog("bruh\n");
    //FIO_FileOpenWrite(0, "hello.bin");
    //FIO_FileWrite(0, sizeof(testbuffer), testbuffer);
    //FIO_FileClose(0);
    while (!option)
    {
        if (PAD_BTN_RELEASED(JOY_1, BUTTON_START))
        {
            option = STATE_GAME;
        }
        SYS_doVBlankProcess();
    }
    game_state_current = option;
}