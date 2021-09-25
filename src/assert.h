/// PROJECT:        Deltarune MD
/// FILE:           src/assert.h
/// AUTHOR:         RatcheT2497
/// CREATION:       ???
/// MODIFIED:       23/09/21
/// DESCRIPTION:    File containing declarations for a runtime and a compile time assert.
///                 Compile time assert requires a single symbol as a name. (very neat hack, copied off of stack overflow)
/// CHANGELOG:      (23/09/21) Added this file header. -R#
#ifndef _ASSERT_H_
#define _ASSERT_H_

#include <genesis.h>
#define STATIC_ASSERT(name, expr) typedef char assert_ ## name [(!!(expr))*2 - 1];
#ifndef NDEBUG
void __AssertFailed(const char* filename, u32 linenum, const char* message);
#define ASSERT(cond, msg) if (!(cond)) { __AssertFailed(__FILE__, __LINE__, msg); }
#else
#define ASSERT(cond, msg) ((void*)0)
#endif

#endif
