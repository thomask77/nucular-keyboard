/**
 * Nucular Keyboard - Microsecond Timer
 * Copyright (C)2015 Thomas Kindler <mail_nucular@t-kindler.de>
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

#include "ustime.h"
#include "stm32l0xx_hal.h"


/**
 * Get time count in microseconds.
 *
 * \note
 *   this function must be called at least
 *   once every 65ms to work correctly.
 *
 * \todo: Use a 32 bit timer.
 *
 */
uint64_t get_us_time64(void)
{
    static uint16_t t0;
    static uint64_t tickcount;

    uint16_t  t = TIM6->CNT;
    if (t < t0)
        tickcount += t + 0x10000 - t0;
    else
        tickcount += t - t0;

    t0 = t;

    return tickcount;
}


uint32_t get_us_time32(void)
{
    return get_us_time64();
}


/**
 * Perform a microsecond delay
 *
 * \param  us  number of microseconds to wait.
 * \note   The actual delay will last between us and (us-1) microseconds.
 *         To wait _at_least_ 1 us, you should use delay_us(2).
 */
void delay_us(int us)
{
    uint16_t  t0 = TIM6->CNT;

    while (us > 0) {
        uint16_t dt = TIM6->CNT - t0;
        us -= dt;
        t0 += dt;
    }
}


/**
 * Perform a millisecond delay
 *
 * \param  ms  number of milliseconds to wait.
 */
void delay_ms(int ms)
{
    delay_us(ms * 1000);
}


/**
 * Set up TIM6 as a 16bit microsecond-timer.
 *
 */
void init_us_timer(void)
{
    // Enable peripheral clocks
    //
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

    TIM6->PSC = (HAL_RCC_GetPCLK1Freq() / 1000000) - 1;
    TIM6->ARR = 0xFFFF;
    TIM6->CR1 = TIM_CR1_CEN;
}

