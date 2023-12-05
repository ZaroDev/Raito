#pragma once
#include <assert.h>

#ifndef DIST
#define ENABLE_ASSERTS
#else
#undef ENABLE_ASSERTS
#endif

#ifdef ENABLE_ASSERTS
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif
