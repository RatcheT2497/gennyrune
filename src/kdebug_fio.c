#include "kdebug_fio.h"
enum {
    FIO_CTL_SELECT = 0x9D80,
    FIO_CTL_OPEN_WRITE = 0x9D88,
    FIO_CTL_CLOSE = 0x9DF8,
    FIO_CTL_ENABLE_LATCH = 0x9DF0,
    FIO_CTL_DISABLE_LATCH = 0x9D01,
    FIO_WRT = 0x9E00,
};
enum {
    FIO_SEEK_SET,
    FIO_SEEK_CUR,
    FIO_SEEK_END
};

volatile u16 *const GFX_CTRL = (u16*) GFX_CTRL_PORT;
void FIO_FileOpenWrite(u8 id, const char *filename)
{
    u8 i = 0;
    *GFX_CTRL = FIO_CTL_OPEN_WRITE | (id & 7);
    while (i < 12)
    {
        if (!filename[i])
            break;

        *GFX_CTRL = FIO_WRT | filename[i];
        i++;
    }
    while (i < 12)
    {
        *GFX_CTRL = FIO_WRT | 0;
        i++;
    }
    *GFX_CTRL = FIO_CTL_DISABLE_LATCH;
}
void FIO_FileWrite(u8 id, u32 size, void *buffer)
{
    *GFX_CTRL = FIO_CTL_SELECT | (id & 7);
    for (u32 i = 0; i < size; i++)
    {
        *GFX_CTRL = FIO_WRT | ((u8*)buffer)[i];
    }
    *GFX_CTRL = FIO_CTL_DISABLE_LATCH;
}
void FIO_FileSeek(u8 id, u32 offset, u8 mode)
{
    *GFX_CTRL = FIO_CTL_SELECT | (id & 7);
    *GFX_CTRL = FIO_WRT | (mode & 0xFF);

    *GFX_CTRL = FIO_WRT | (((offset) >> 0) & 0xFF);
    *GFX_CTRL = FIO_WRT | (((offset) >> 8) & 0xFF);
    *GFX_CTRL = FIO_WRT | (((offset) >> 16) & 0xFF);
    *GFX_CTRL = FIO_WRT | (((offset) >> 24) & 0xFF);

    *GFX_CTRL = FIO_CTL_DISABLE_LATCH;
}
void FIO_FileClose(u8 id)
{
    *GFX_CTRL = FIO_CTL_CLOSE | (id & 7);
    *GFX_CTRL = FIO_CTL_DISABLE_LATCH;
}

// https://codereview.stackexchange.com/questions/151049/endianness-conversion-in-c
// couldn't be bothered to do it myself lol
inline uint32_t Reverse32(uint32_t value) 
{
    return (((value & 0x000000FF) << 24) |
            ((value & 0x0000FF00) <<  8) |
            ((value & 0x00FF0000) >>  8) |
            ((value & 0xFF000000) >> 24));
}

u32 FIO_ReadValue(void)
{
    u32 value = 0;
    u16 ctrl_val = 0;
    *GFX_CTRL = FIO_CTL_ENABLE_LATCH;
    do {
        value <<= 4;

        ctrl_val = *GFX_CTRL;
        value |= (ctrl_val >> 10) & 0x0F;
    } while(!(ctrl_val & 0x8000));
    
    *GFX_CTRL = FIO_CTL_DISABLE_LATCH;
    return value;
}