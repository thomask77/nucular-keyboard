#pragma once

#include <stdint.h>

#define ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))
#define FIELD_SIZEOF(t, f)  (sizeof(((t*)0)->f))

#define STATIC_ASSERT(x)    static_assert(x, #x)

#define STRINGIFY_(x)       #x
#define STRINGIFY(x)        STRINGIFY_(x)


#define clamp(x, min, max)      \
( { typeof (x) _x   = (x);      \
    typeof (x) _min = (min);    \
    typeof (x) _max = (max);    \
    _x < _min ? _min : (_x > _max ? _max : _x); \
} )


#define deadband(x, band)       \
( { typeof (x) _x    = (x);     \
    typeof (x) _band = (band);  \
    _x < - _band                \
    ? _x + _band                \
    : _x > _band                \
      ? _x - _band              \
      : 0;                      \
} )


char *strnbar(char *s, int len, float value, float min, float max);
void hexdump(const void *addr, unsigned len);
void enter_bootloader(void);
