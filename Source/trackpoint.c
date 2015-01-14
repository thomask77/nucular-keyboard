/**
 * Nucular Keyboard - PS/2 TrackPoint to USB Report conversion
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
#include "kb_driver.h"
#include "trackpoint.h"
#include "ps2_host.h"
#include "ustime.h"
#include "util.h"
#include "stm32l0xx_hal.h"
#include <stdio.h>
#include <stdbool.h>

#define PIN_RESET       GPIO_PIN_5
#define PIN_PWR         GPIO_PIN_8

// Fractional integrators for wheel/pan
//
#define WHEEL_SPEED     100
#define WHEEL_SCALE     1024

#define WHEEL_DEADBAND  1

static int32_t  dwheel_frac;
static int32_t  dpan_frac;

// HID report
//
struct tp_mouse_report  tp_mouse_report;


static void write_ram(int addr, int data)
{
    char ack;

    ps2_write((char[]){ 0xe2 }, 1);
    ps2_read(&ack, 1);
    ps2_write((char[]){ 0x81 }, 1);
    ps2_read(&ack, 1);
    ps2_write((char[]){ addr }, 1);
    ps2_read(&ack, 1);
    ps2_write((char[]){ data }, 1);
    ps2_read(&ack, 1);
}


static int read_ram(int addr)
{
    char ack;

    ps2_write((char[]){ 0xe2 }, 1);
    ps2_read(&ack, 1);
    ps2_write((char[]){ 0x80 }, 1);
    ps2_read(&ack, 1);
    ps2_write((char[]){ addr }, 1);
    ps2_read(&ack, 1);

    return ps2_read(&ack, 1);
}


void set_sensitivity_factor(int sensitivity_factor)
{
    write_ram(0x4a, sensitivity_factor);
}


int get_sensitivity_factor(void)
{
    return read_ram(0x4a);
}


void tp_reset(void)
{
    printf("tp_reset: ");

    GPIOB->BSRR = PIN_RESET;
    delay_ms(1);
    GPIOB->BSRR = PIN_RESET << 16;

    uint8_t res[2];

    int n = ps2_read(res, 2);
    if (n == 2) {
        // AA 00 - everything ok
        // FC 00 - something went wrong
        //
        printf("0x%02x 0x%02x\n", res[0], res[1]);
    }
    else {
        printf("failed.\n");
    }
}


void tp_clear_mouse_report(void)
{
    tp_mouse_report.dx     = 0;
    tp_mouse_report.dy     = 0;
    tp_mouse_report.dwheel = 0;
    tp_mouse_report.dpan   = 0;

    // clear integer parts only
    //
    dwheel_frac -= dwheel_frac / WHEEL_SCALE * WHEEL_SCALE;
    dpan_frac   -= dpan_frac   / WHEEL_SCALE * WHEEL_SCALE;
}


void tp_update(void)
{
    extern volatile int rx_buf;
    if (rx_buf == -1)
        return;

    struct {
        uint8_t status;
        int8_t  dx;
        int8_t  dy;
    } buf;

    ps2_read(&buf, sizeof(buf));

    // TODO: Check bit 3 for errors!!

    tp_mouse_report.buttons = buf.status & 7;

    if (!kb_get_fn_key()) {
        // normal mouse movement, reset wheel/pan integrators.
        //
        dwheel_frac = 0;
        dpan_frac   = 0;

        tp_mouse_report.dx = clamp(tp_mouse_report.dx + buf.dx, -127, 127);
        tp_mouse_report.dy = clamp(tp_mouse_report.dy - buf.dy, -127, 127);
        tp_mouse_report.dwheel = 0;
        tp_mouse_report.dpan = 0;
    }
    else {
        // wheel/pan mode
        //
        int dwheel = deadband(buf.dy, WHEEL_DEADBAND) * WHEEL_SPEED;
        int dpan   = deadband(buf.dx, WHEEL_DEADBAND) * WHEEL_SPEED;

        // reset integrators when inside deadband
        //
        dwheel_frac = dwheel ? dwheel_frac + dwheel : 0;
        dpan_frac   = dpan   ? dpan_frac   + dpan   : 0;

        tp_mouse_report.dx = 0;
        tp_mouse_report.dy = 0;
        tp_mouse_report.dwheel = dwheel_frac / WHEEL_SCALE;
        tp_mouse_report.dpan = dpan_frac / WHEEL_SCALE;
    }
}


void tp_init(void)
{
    // PB5      TP4_RESET
    // PB8      +5V_TP_ON
    //
    GPIOB->BSRR = GPIO_PIN_8 << 16;     // power on
    GPIOB->BSRR = GPIO_PIN_5;           // activate reset

    HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef) {
        .Pin   = GPIO_PIN_8 | GPIO_PIN_5,
        .Mode  = GPIO_MODE_OUTPUT_OD,
        .Pull  = GPIO_PULLUP,
        .Speed = GPIO_SPEED_LOW
    } );

    ps2_init();
    tp_reset();

    uint8_t ack;

    // tp_set_sensitivity_factor(2);
    // printf("sens: %d\n", tp_get_sensitivity_factor());

    ps2_write((char[]){ 0xF4 }, 1);     // enable
    ps2_read(&ack, 1);                  // get response

    // printf("response: %02x\n", ack);
}
