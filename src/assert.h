#ifndef _ASSERT_H_
#define _ASSERT_H_

#include <genesis.h>
void __AssertFailed(const char* filename, u32 linenum, const char* message);
#ifndef NDEBUG
#define ASSERT(cond, msg) if (!(cond)) { __AssertFailed(__FILE__, __LINE__, msg); }
#else
#define ASSERT(cond, msg) ((void*)0)
#endif

#endif
