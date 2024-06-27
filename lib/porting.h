
#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#if defined(_MSC_VER)

// prevent collisions with unpaper' Rectangle type:
#define Rectangle Win32Rectangle
#include <windows.h>
#undef Rectangle

#include <string.h>

#define __attribute__(x)			/* x */

#define __typeof__(a)         decltype(a)

#define strcasecmp(a, b)      stricmp(a, b)
#define strncasecmp(a, b)     strnicmp(a, b)

#else
#include <strings.h>
#endif

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

