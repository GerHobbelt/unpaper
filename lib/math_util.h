// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include "lib/porting.h"

#undef min
#undef max

#if 0

#define max(a, b)                                                              \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a > _b ? _a : _b;                                                         \
  })

#define min(a, b)                                                              \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a < _b ? _a : _b;                                                         \
  })

#define max3(a, b, c)                                                          \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    __typeof__(c) _c = (c);                                                    \
    (_a > _b ? (_a > _c ? _a : _c) : (_b > _c ? _b : _c));                     \
  })

#define min3(a, b, c)                                                          \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    __typeof__(c) _c = (c);                                                    \
    (_a < _b ? (_a < _c ? _a : _c) : (_b < _c ? _b : _c));                     \
  })

#else

#include <stdint.h>

static inline int64_t max(const int64_t _a, const int64_t _b)
{                                                                           
  return _a > _b ? _a : _b;                                                         
}

static inline int64_t min(const int64_t _a, const int64_t _b)
{                                                                           
  return _a < _b ? _a : _b;                                                         
}

static inline int64_t max3(const int64_t _a, const int64_t _b, const int64_t _c)
{                                                                           
  return (_a > _b ? (_a > _c ? _a : _c) : (_b > _c ? _b : _c));                     
}

static inline int64_t min3(const int64_t _a, const int64_t _b, const int64_t _c)
{                                                                           
  return (_a < _b ? (_a < _c ? _a : _c) : (_b < _c ? _b : _c));                     
}

#endif
