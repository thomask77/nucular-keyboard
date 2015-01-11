/**
 * Small, integer-only printf functions.
 *
 * Copyright (c)2012 Thomas Kindler <mail@t-kindler.de>
 *
 * 2014-07-20: tk, added puts() function.
 * 2012-11-18: tk, added snprintf() function.
 * 2012-11-11: tk, initial implementation.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "small_printf.h"
#include <string.h>
#include <limits.h>
#include <stdarg.h>

#define FLAG_LEFT   1
#define FLAG_ZERO   2


struct spec {
    char *out;
    int  max_length;
    int  length;

    int  width;
    int  flags;
};


static void print_char(struct spec *spec, char c)
{
    if (spec->out) {
        if (spec->length < spec->max_length-1)
            *spec->out++ = c;
    }
    else {
        putchar(c);
    }

    spec->length++;
}


static void print_string(struct spec *spec, const char *string)
{
    int w;

    if (!string)
        string = "(null)";

    w = spec->width - strlen(string);

    while (!(spec->flags & FLAG_LEFT) && w-- > 0)
        print_char(spec, (spec->flags & FLAG_ZERO) ? '0' : ' ');

    while (*string)
        print_char(spec, *string++);

    while (w-- > 0)
        print_char(spec, ' ');
}


static void print_number(struct spec *spec, int i, int base, int sign, char alpha)
{
    int neg = 0;
    unsigned int u = i;

    /* Enough for 2^32 in base 8 + '\0' */
    char buf[12], *s = &buf[11];

    if (sign && i < 0) {
        neg =  1;
        u   = -i;
    }

    *s = 0;
    do {
        int d = u % base;
        *--s = (d < 10) ? d + '0' : d - 10 + alpha;
    } while (u /= base);

    if (neg) {
        if (spec->width && (spec->flags & FLAG_ZERO)) {
            print_char(spec, '-');
            spec->width--;
        }
        else {
            *--s = '-';
        }
    }
    print_string(spec, s);
}


static int print_format(struct spec *spec, const char *fmt, va_list args)
{
    enum {
        TEXT, FLAGS, WIDTH, /* DOT, PRECISION */ LENGTH, TYPE
    } state = TEXT;

    spec->length = 0;

    while (*fmt) {
        switch (state) {
        case TEXT:
            spec->width = 0;
            spec->flags = 0;

            switch (*fmt) {
            case '%': state = FLAGS;  break;
            default:  print_char(spec, *fmt);  break;
            }
            fmt++;
            break;

        case FLAGS:
            switch (*fmt) {
            case '-': spec->flags |= FLAG_LEFT;  fmt++;  break;
            case '0': spec->flags |= FLAG_ZERO;  fmt++;  break;
            default:  state = WIDTH;  break;
            }
            break;

        case WIDTH:
            if (*fmt >= '0' && *fmt <= '9')
                spec->width = spec->width * 10 + *fmt++ - '0';
            else
                state = LENGTH;
            break;

        case LENGTH:
            /* not implemented. just consume "l", "ll", "h" and "hh" */
            switch (*fmt) {
            case 'l': fmt++; break;
            case 'h': fmt++; break;
            default:
                state = TYPE;
            }
            break;

        case TYPE:
            switch (*fmt) {
            case 'i': /* fall-through */
            case 'd': print_number(spec, va_arg(args, int), 10, 1, 'a');  break;
            case 'u': print_number(spec, va_arg(args, int), 10, 0, 'a');  break;
            case 'x': print_number(spec, va_arg(args, int), 16, 0, 'a');  break;
            case 'X': print_number(spec, va_arg(args, int), 16, 0, 'A');  break;
            case 's': print_string(spec, va_arg(args, char*));  break;
            case 'c': {
                char s[2] = { va_arg(args, int), 0 };
                print_string(spec, s);
                break;
            }
            case '%': print_string(spec, "%");  break;
            default:  print_string(spec, "?");  break;
            }
            fmt++;
            state = TEXT;
            break;
        }
    }

    if (spec->out)
        *spec->out++ = '\0';

    return spec->length;
}


int vsnprintf(char *str, size_t size, const char *fmt, va_list args)
{
    struct spec spec = { str, size };

    if (!str || !size) {
        static char dummy;
        spec.out = &dummy;
        spec.max_length = 1;
    }

    return print_format(&spec, fmt, args);
}


int vsprintf(char *str, const char *fmt, va_list args)
{
    return vsnprintf(str, INT_MAX, fmt, args);
}


int vprintf(const char *fmt, va_list args)
{
    struct spec spec = { NULL, 0 };

    return print_format(&spec, fmt, args);
}


int snprintf(char *str, size_t size, const char *fmt, ...)
{
    int ret;
    va_list args;

    va_start(args, fmt);
    ret = vsnprintf(str, size, fmt, args);
    va_end(args);
    
    return ret;
}


int sprintf(char *str, const char *fmt, ...)
{
    int ret;
    va_list args;

    va_start(args, fmt);
    ret = vsprintf(str, fmt, args);
    va_end(args);
    
    return ret;
}


int printf(const char *fmt, ...)
{
    int ret;
    va_list args;

    va_start(args, fmt);
    ret = vprintf(fmt, args);
    va_end(args);
    
    return ret;
}


int puts(const char *str)
{
    int ret = 0;
    while (*str) {
        putchar(*str++);
        ret++;
    }

    putchar('\n');
    ret++;

    return ret;
}

