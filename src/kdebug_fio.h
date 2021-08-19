#ifndef _KDEBUG_FIO_H_
#define _KDEBUG_FIO_H_
#include <genesis.h>
/*
    writes:
        control register
        - 00000000 - KDebug Halt
        - 00000001:011111111 - disable latch
        - 1nnnnfff - enable latch & give command
        - .0000... - select current file
        - .0001... - open file for writing - goto state 1
        - .0010... -- open file for reading
        - .0011... - seek in file
        - .0100... -- tell cursor position
        - .0101... -- get file status
        - .1110... - enable latch and ignore file id
        - .1111... - close file - goto state 0
        data register
        - latch disabled:
        -- KDebug Logging
        - latch enabled:
        -- state 0: invalid
        -- state 1: fetch 12 bytes of filename - goto state 2
        -- state 2: fetch byte to write to file - goto state 2
    reads:
        vdp control port
        f.nnnn.. ........

*/
void FIO_FileOpenWrite(u8 id, const char *filename);
void FIO_FileWrite(u8 id, u32 size, void *buffer);
void FIO_FileSeek(u8 id, u32 offset, u8 mode);
void FIO_FileClose(u8 id);
u32 FIO_ReadValue(void);
#endif