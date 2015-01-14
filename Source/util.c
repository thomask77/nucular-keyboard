/**
 * Nucular Keyboard - Utility functions
 * Copyright (C)2015 Thomas Kindler <mail_nucular@t-kindler.de>
 *
 * 2005-11-30: tk, improved hexdump with ascii output
 * 2002-01-10: tk, added strnbar function for progress bars
 * 2000-04-24: tk, initial implementation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
