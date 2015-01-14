/**
 * Nucular Keyboard - PS/2 host controller functions
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

/*
     see http://www.computer-engineering.org/ps2protocol/
     see https://github.com/tmk/tmk_keyboard
     TODO: receive/send buffer
     TODO: inhibit on overflow
     TODO: handle time outs
*/

#include "ps2_host.h"
#include "ustime.h"
#include "stm32l0xx_hal.h"
#include <stdbool.h>

#define PIN_CLK     GPIO_PIN_3
#define PIN_DATA    GPIO_PIN_4

// PS/2 Receive state
//
static volatile int rx_frame;
static volatile int rx_frame_pos;

/*static*/ volatile int rx_buf = -1;

static volatile int rx_bytes;
static volatile int rx_errors;

// PS/2 Transmit state
//
static volatile int tx_frame;
static volatile int tx_frame_pos;

static volatile int tx_buf = -1;


volatile bool foo;


static bool check_frame(int frame)
{
    if (frame & (1<<0))         // start bit
        return false;

    if (!(frame & (1<<10)))     // stop bit
        return false;

    int parity = 0;             // parity check
    for (int b=1; b<=9; b++)
        parity ^= !!(frame & (1 << b));

    if (parity != 1)
        return false;

    return true;
}


static void handle_clk_edge(int data)
{
    if (rx_frame_pos == 0 && data != 0) {
        // invalid start bit.. try to resync
        //
        return;
    }

    // add bit to receive register
    //
    rx_frame |= !!data << rx_frame_pos++;

    if (rx_frame_pos >= 11) {
        // start + 8 bits + parity + stop received
        //
        if (check_frame(rx_frame)) {
            rx_buf = (rx_frame >> 1) & 255;

            GPIOB->BRR = PIN_CLK;   // Inhibit clock
            rx_bytes++;
        }
        else {
            rx_errors++;
        }

        rx_frame = 0;
        rx_frame_pos = 0;
    }
}


void EXTI2_3_IRQHandler(void)
{
    uint32_t exti_pr = EXTI->PR;

    if (exti_pr & EXTI_PR_PR3) {
        if (!foo)
            handle_clk_edge(GPIOB->IDR & PIN_DATA);
    }

    EXTI->PR = exti_pr;     // clear interrupts
    EXTI->PR;               // dummy read to avoid glitches
}


static void ps2_send_byte(uint8_t data)
{
    int parity = 0;
    for (int b=0; b<=7; b++)
        parity ^= !!(data & (1 << b));

    // start + data + parity + stop
    //
    int frame = (data << 1) | (!parity << 9) | (1 << 10);

    foo = 1;

    GPIOB->BSRR = (PIN_CLK << 16);  // clk lo
    delay_us(100);

    for (int i=0; i<11; i++) {
        if (frame & (1<<i))
            GPIOB->BSRR = PIN_DATA;
        else
            GPIOB->BRR = PIN_DATA;

        delay_us(15);

        GPIOB->BSRR = PIN_CLK;  // clk hi

        while (!(GPIOB->IDR & PIN_CLK));
        while (GPIOB->IDR & PIN_CLK);
    }

    foo = 0;
}


ssize_t ps2_read(void *buf, size_t n)
{
    uint8_t *d = buf;

    for (int i=0; i<n; i++) {
        while (rx_buf == -1);
        *d++ = rx_buf;

        rx_buf = -1;
        GPIOB->BSRR = PIN_CLK;   // Idle
    }

    return n;
}


ssize_t ps2_write(const void *buf, size_t n)
{
    const char *s = buf;

    for (int i=0; i<n; i++)
        ps2_send_byte(*s++);

    return n;
}


void ps2_init(void)
{
    // PB3      TP4_CLK
    // PB4      TP4_DATA
    //
    GPIOB->BSRR = GPIO_PIN_3;   // Idle
    GPIOB->BSRR = GPIO_PIN_4;   // Idle

    // Set up EXTI3 Interrupt.. dämliche HAL-Library.
    // EXTI und OUTPUT_OD gleichzeitig geht nicht mehr m(
    //
    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef) {
        .Pin   = GPIO_PIN_3,
        .Mode  = GPIO_MODE_IT_FALLING,
    } );

    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef) {
        .Pin   = GPIO_PIN_4 | GPIO_PIN_3,
        .Mode  = GPIO_MODE_OUTPUT_OD,
        .Pull  = GPIO_PULLUP,
        .Speed = GPIO_SPEED_LOW
    } );

    NVIC_SetPriority(EXTI2_3_IRQn, 15);
    NVIC_EnableIRQ(EXTI2_3_IRQn);
}

