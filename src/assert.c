#include "assert.h"
void __AssertFailed(const char* filename, u32 linenum, const char* message)
{
    char buffer[256];
    intToStr(linenum, buffer, 1);
    strcat(buffer, ":");
    strcat(buffer, filename);
    strcat(buffer, ":");
    strcat(buffer, message);
    SYS_die(buffer);
}