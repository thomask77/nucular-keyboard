#pragma once

#include <string.h>
#include <stdarg.h>

int vsnprintf(char *str, size_t size, const char *fmt, va_list args);
int vsprintf(char *str, const char *fmt, va_list args);
int vprintf(const char *fmt, va_list args);

__attribute__ ((format(printf, 3, 4)))
int snprintf(char *str, size_t size, const char *fmt, ...);

__attribute__ ((format(printf, 2, 3)))
int sprintf(char *str, const char *fmt, ...);

__attribute__ ((format(printf, 1, 2)))
int printf(const char *fmt, ...);

int puts(const char *str);


// putchar has to be provided by the user
//
#undef putchar

extern int putchar(int c);

