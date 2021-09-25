/// PROJECT:        Deltarune MD
/// FILE:           src/assert.c
/// AUTHOR:         RatcheT2497
/// CREATION:       ???
/// MODIFIED:       23/09/21
/// DESCRIPTION:    File containing the definition for the assertion failed function.
/// CHANGELOG:      (23/09/21) Added this file header. -R#

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