/**
 * Utility functions
 *
 * Copyright (c) Thomas Kindler <mail@t-kindler.de>
 *
 * 2005-11-30: tk, improved hexdump with ascii output
 * 2002-01-10: tk, added strnbar function for progress bars
 * 2000-04-24: tk, initial implementation
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

#include "util.h"
#include "stm32l0xx.h"
#include <stdio.h>
#include <ctype.h>
#include <math.h>

/**
 * Make a text-mode bargraph string.
 *
 * \param  s      pointer to string buffer
 * \param  len    length of string buffer
 * \param  value  bargraph value
 * \param  min    left edge of bargraph range
 * \param  max    right edge of bargraph range
 *
 * \return  pointer to the string buffer
 */
char *strnbar(char *s, int len, float value, float min, float max)
{
    const int n = floorf( ((value - min) * len) / (max-min) );

    for (int i=0; i<len-1; i++)
        s[i] = i < n ? '=' : ' ';

    if (n < 0)
        s[0] = '<';
    else if (n > len-1)
        s[len-2] = '>';

    s[len-1] = '\0';

    return s;
}


/**
 * Generate a nice hexdump of a memory area.
 *
 * \param  mem     pointer to memory to dump
 * \param  length  how many bytes to dump
 */
void hexdump(const void *mem, unsigned length)
{
    puts("       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    0123456789ABCDEF");

    const unsigned char *src = (const unsigned char*)mem;
    for (unsigned i=0; i<length; i += 16, src += 16) {
        char  line[80], *t = line;

        t += sprintf(t, "%04x:  ", i);
        for (unsigned j=0; j<16; j++) {
            if (i+j < length)
                t += sprintf(t, "%02X", src[j]);
            else
                t += sprintf(t, "  ");
            *t++ = j & 1 ? ' ' : '-';
        }

        t += sprintf(t, "  ");
        for (unsigned j=0; j<16; j++) {
            if (i+j < length)
                *t++ = isprint(src[j]) ? src[j] : '.';
            else
                *t++ = ' ';
        }
        *t++ = 0;
        puts(line);
    }
}


void enter_bootloader(void)
{
    RCC->APB1ENR |=  RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_DBP;
    RTC->BKP0R = 0xB00710AD;
    PWR->CR &= ~PWR_CR_DBP;

    NVIC_SystemReset();
}
