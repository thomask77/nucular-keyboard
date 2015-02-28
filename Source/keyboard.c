/**
 * Nucular Keyboard - USB Keyboard Report
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
#include "keyboard.h"
#include "kb_driver.h"
#include "util.h"
#include "stm32l0xx_hal.h"
#include <stdio.h>
#include <string.h>

#define T_POWER_DELAY   2000    // 2s

// HID reports
//
struct kb_in_report         kb_in_report;
struct kb_sysctrl_report    kb_sysctrl_report;
struct kb_consumer_report   kb_consumer_report;
static struct kb_misc_keys  kb_misc_keys;

static int kb_in_report_num_keys;


// TODO
//                                                  Implemented     Tested
// Mute                 10/4                        x               x
// Volume -             10/3                        x               x
// Volume +             10/2                        x               x
// Mic Mute             10/6
// ThinkVantage         10/5                        x               x
// Page backward        11/6                        x               x
// Page forward         11/7                        x               x
// Power                own signal                  x               x
//
// --- FN Keys ---
// Lock                 2/0     (F2)
// Battery              2/3     (F3)
// Sleep                2/5     (F4)                x               x
// Radio toggle         8/5     (F5)
// Webcam/Headphones    5/5     (F6)
// Screen/Projector     6/3     (F7)
// Mouse/Trackpoint     6/0     (F8)
// Suspend to Disk      9/1     (F12)               x               x (>= Win 8.1 ?)
// Brightness Up        12/0    (Home)              x               x (>= Win 8.1)
// Brightness Down      12/1    (End)               x               x (>= Win 8.1)
// Thinklight           11/0    (Page up)           x               x
// Thinklight           11/1    (Page down)         x               x

// Zoom                 8/7     (Space)
// Stop                 12/5    (Cursor Up)         x               x
// Play/Pause           10/7    (Cursor Down)       x               x
// Backward             12/7    (Cursor Left)       x               x
// Forward              9/7     (Cursor Right)      x               x
//
// Special FN functions:
// * Keypad Keys & Keypad Enter
// * Left-GUI sends Right GUI
//

// Keyboard matrix for Lenovo T420 keyboards
//
static const uint32_t usage_tab[16][8] = {
/*          0         1         2         3         4         5         6         7       */
/*  0 */  { 0x070035, 0x07001e, 0x070014, 0x07002b, 0x070004, 0x070029, 0x07001d, 0x000000 },
/*  1 */  { 0x07003a, 0x07001f, 0x07001a, 0x070039, 0x070016, 0x070064, 0x07001b, 0x000000 },
/*  2 */  { 0x07003b, 0x070020, 0x070008, 0x07003c, 0x070007, 0x07003d, 0x070006, 0x000000 },
/*  3 */  { 0x070022, 0x070021, 0x070015, 0x070017, 0x070009, 0x07000a, 0x070019, 0x070005 },
/*  4 */  { 0x070023, 0x070024, 0x070018, 0x07001c, 0x07000d, 0x07000b, 0x070010, 0x070011 },
/*  5 */  { 0x07002e, 0x070025, 0x07000c, 0x070030, 0x07000e, 0x07003f, 0x070036, 0x000000 },
/*  6 */  { 0x070041, 0x070026, 0x070012, 0x070040, 0x07000f, 0x000000, 0x070037, 0x000000 },
/*  7 */  { 0x07002d, 0x070027, 0x070013, 0x07002f, 0x070033, 0x070034, 0x070032, 0x070038 },
/*  8 */  { 0x070042, 0x070043, 0x000000, 0x07002a, 0x070031, 0x07003e, 0x070028, 0x07002c },
/*  9 */  { 0x070049, 0x070045, 0x000000, 0x0700e3, 0x000000, 0x000000, 0x000000, 0x07004f },
/* 10 */  { 0x07004c, 0x070044, 0x0c00e9, 0x0c00ea, 0x0c00e2, 0x0c0192, 0x000000, 0x070051 },
/* 11 */  { 0x07004b, 0x07004e, 0x000000, 0x000000, 0x070065, 0x000000, 0x0c0224, 0x0c0225 },
/* 12 */  { 0x07004a, 0x07004d, 0x000000, 0x000000, 0x000000, 0x070052, 0x070048, 0x070050 },
/* 13 */  { 0x000000, 0x070046, 0x070047, 0x000000, 0x000000, 0x0700e2, 0x000000, 0x0700e6 },
/* 14 */  { 0x000000, 0x000000, 0x000000, 0x0700e1, 0x000000, 0x000000, 0x0700e5, 0x000000 },
/* 15 */  { 0x0700e0, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x0700e4, 0x000000 }
};


static const uint32_t usage_tab_fn[16][8] = {
/*            0           1         2         3         4         5         6         7       */
/*  0 */  {   0x000000,   0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000 },
/*  1 */  {   0x000000,   0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000 },
/*  2 */  {   0x000000,   0x000000, 0x000000, 0x000000, 0x000000, 0x010082, 0x000000, 0x000000 },
/*  3 */  {   0x000000,   0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000 },
/*  4 */  {   0x000000,   0x07005f, 0x07005c, 0x000000, 0x070059, 0x000000, 0x070062, 0x000000 },
/*  5 */  {   0x000000,   0x070060, 0x07005d, 0x000000, 0x07005a, 0x000000, 0x000000, 0x000000 },
/*  6 */  {   0x000000,   0x070061, 0x07005e, 0x000000, 0x07005b, 0x000000, 0x070063, 0x000000 },
/*  7 */  {   0x000000,   0x070054, 0x070055, 0x000000, 0x070056, 0x000000, 0x000000, 0x070057 },
/*  8 */  {   0x000000,   0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x070058, 0x000000 },
/*  9 */  {   0x000000,   0x0100A8, 0x000000, 0x0700e7, 0x000000, 0x000000, 0x000000, 0x0c00b5 },
/* 10 */  {   0x000000,   0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x0c00cd },
/* 11 */  { 0xff000001, 0xff000002, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000 },
/* 12 */  {   0x0c006f,   0x0c0070, 0x000000, 0x000000, 0x000000, 0x0c00b7, 0x000000, 0x0c00b6 },
/* 13 */  {   0x000000,   0x000000, 0x070053, 0x000000, 0x000000, 0x0700e2, 0x000000, 0x0700e6 },
/* 14 */  {   0x000000,   0x000000, 0x000000, 0x0700e1, 0x000000, 0x000000, 0x0700e5, 0x000000 },
/* 15 */  {   0x0700e0,   0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x0700e4, 0x000000 }
};


static void key_down(uint32_t usage)
{
    uint16_t usage_page = usage >> 16;
    uint16_t usage_id   = usage & 0xFFFF;

    switch (usage_page) {
    case 0x01:
        // Generic Desktop Page (0x01)
        //
        switch (usage_id) {
        case 0x81: kb_sysctrl_report._81_system_power_down = 1; break;
        case 0x82: kb_sysctrl_report._82_system_sleep      = 1; break;
        case 0x83: kb_sysctrl_report._83_system_wake_up    = 1; break;
        case 0xA8: kb_sysctrl_report._a8_system_hibernate  = 1; break;
        default: goto unknown_usage;
        }
        break;

    case 0x07:
        // Keyboard/Keypad Page (0x07)
        //
        switch (usage_id) {

        case 0x01: goto error_roll_over;    // special handling for ghost key..

        case 0xE0: kb_in_report._e0_keyboard_left_control  = 1; break;
        case 0xE1: kb_in_report._e1_keyboard_left_shift    = 1; break;
        case 0xE2: kb_in_report._e2_keyboard_left_alt      = 1; break;
        case 0xE3: kb_in_report._e3_keyboard_left_gui      = 1; break;
        case 0xE4: kb_in_report._e4_keyboard_right_control = 1; break;
        case 0xE5: kb_in_report._e5_keyboard_right_shift   = 1; break;
        case 0xE6: kb_in_report._e6_keyboard_rigt_alt      = 1; break;
        case 0xE7: kb_in_report._e7_keyboard_right_gui     = 1; break;

        default:
            if (usage_id > 255)
                goto unknown_usage;

            if (kb_in_report_num_keys < 6)
                kb_in_report.keycode[kb_in_report_num_keys++] = usage_id;
            else
                goto error_roll_over;

            break;
        }
        break;

    case 0x0C:
        // Usage Page 0x0C Consumer Devices
        //
        switch (usage_id) {
        case 0xe9:  kb_consumer_report._e9_volume_increment                    = 1; break;
        case 0xea:  kb_consumer_report._ea_volume_decrement                    = 1; break;
        case 0xe2:  kb_consumer_report._e2_mute                                = 1; break;
        case 0xcd:  kb_consumer_report._cd_play_pause                          = 1; break;
        case 0xb5:  kb_consumer_report._b5_scan_next_track                     = 1; break;
        case 0xb6:  kb_consumer_report._b6_scan_previous_track                 = 1; break;
        case 0xb7:  kb_consumer_report._b7_stop                                = 1; break;
        case 0xb8:  kb_consumer_report._b8_eject                               = 1; break;
        case 0x18a: kb_consumer_report._18a_al_email_reader                    = 1; break;
        case 0x221: kb_consumer_report._221_ac_search                          = 1; break;
        case 0x22a: kb_consumer_report._22a_ac_bookmarks                       = 1; break;
        case 0x223: kb_consumer_report._223_ac_home                            = 1; break;
        case 0x224: kb_consumer_report._224_ac_back                            = 1; break;
        case 0x225: kb_consumer_report._225_ac_forward                         = 1; break;
        case 0x226: kb_consumer_report._226_ac_stop                            = 1; break;
        case 0x227: kb_consumer_report._227_ac_refresh                         = 1; break;
        case 0x183: kb_consumer_report._183_al_consumer_control_configuration  = 1; break;
        case 0x196: kb_consumer_report._196_al_internet_browser                = 1; break;
        case 0x192: kb_consumer_report._192_al_calculator                      = 1; break;
        case 0x19e: kb_consumer_report._19e_al_terminal_lock_screensaver       = 1; break;
        case 0x194: kb_consumer_report._194_al_local_machine_browser           = 1; break;
        case 0x206: kb_consumer_report._206_ac_minimize                        = 1; break;
        case 0x6f:  kb_consumer_report._6f_brightness_increment                = 1; break;
        case 0x70:  kb_consumer_report._70_brightness_decrement                = 1; break;
        default: goto unknown_usage;
        }
        break;

    case 0xFF00:
        // Usage Page 0xFF00 Vendor-defined
        //
        switch (usage_id) {
        case 0x0001: kb_misc_keys._01_thinklight_up = 1;     break;
        case 0x0002: kb_misc_keys._02_thinklight_down = 1;   break;
        default: goto unknown_usage;
        }
    }
    return;


unknown_usage:
    printf("unknown usage %08lx\n", usage);
    return;


error_roll_over:
    // The keyboard must report a phantom state indexing Usage(ErrorRollOver) in
    // all array fields whenever the number of keys pressed exceeds the Report
    // Count. The limit is six non-modifier keys when using the keyboard descriptor
    // in Appendix B. Additionally, a keyboard may report the phantom condition
    // when an invalid or unrecognizable combination of keys is pressed.
    //
    if (kb_in_report.keycode[0] != 0x01) {
        for (int i=0; i<6; i++)
            kb_in_report.keycode[i] = 0x01;

        kb_in_report_num_keys = 6;
    }
    return;
}


static void clear_reports(void)
{
    memset(&kb_sysctrl_report,  0, sizeof(kb_sysctrl_report));
    memset(&kb_consumer_report, 0, sizeof(kb_consumer_report));
    memset(&kb_in_report,       0, sizeof(kb_in_report));
    memset(&kb_misc_keys,       0, sizeof(kb_misc_keys));

    kb_sysctrl_report.report_id_01 = 1;
    kb_consumer_report.report_id_02 = 2;
    kb_in_report_num_keys = 0;
}


static void kb_update_reports(void)
{
    static uint8_t  matrix_old[16];
    static uint8_t  fn_state[16];

    uint8_t matrix[16];

    if (!kb_scan_matrix(matrix)) {
        // Set ErrorRollOver
        //
        key_down(0x070001);
        return;
    }

    clear_reports();

    uint8_t fn_mask = kb_get_fn_key() ? 255 : 0;

    for (int d=0; d<16; d++) {
        uint8_t down    = ~matrix_old[d] &  matrix[d];
        uint8_t up      =  matrix_old[d] & ~matrix[d];
        uint8_t pressed =  matrix_old[d] &  matrix[d];

        // remember fn key on key-down, clear on key-up
        //
        fn_state[d] |= fn_mask & down;
        fn_state[d] &= ~up;

        matrix_old[d] = matrix[d];

        if (!pressed)
            continue;

        for (int s=0; s<8; s++) {
            if (pressed & (1<<s)) {
                int fn = !!(fn_state[d] & (1<<s));
                uint32_t usage = fn ? usage_tab_fn[d][s] : usage_tab[d][s];

                if (usage != 0)
                    key_down(usage);
                else
                    printf("Unknown key: drv=%d, sense=%d, fn=%d\n", d, s, fn);
            }
        }
    }
}


static void kb_update_power(void)
{
    static uint32_t power_t0;

    if (kb_get_power_key()) {

        if (kb_get_fn_key())
            enter_bootloader();

        if (power_t0 == 0)
            power_t0 = HAL_GetTick();

        if (HAL_GetTick() - power_t0 >= T_POWER_DELAY) {
            // System power down
            //
            key_down(0x010081);
        }
    }
    else {
        power_t0 = 0;
    }
}


static void kb_update_thinklight(void)
{
    static int      level;
    static struct   kb_misc_keys  old_keys;
    static uint32_t t_last;

    uint32_t t = HAL_GetTick();
    int repeat = t - t_last > 1;

    if (kb_misc_keys._01_thinklight_up) {
        if (!old_keys._01_thinklight_up || repeat) {
            if (level < 255)
                level++;
            t_last = t;
        }
    }

    if (kb_misc_keys._02_thinklight_down) {
        if (!old_keys._02_thinklight_down || repeat) {
            if (level > 0)
                level--;
            t_last = t;
        }
    }

    old_keys = kb_misc_keys;
    kb_set_thinklight(level);
}


void kb_update(void)
{
    kb_update_reports();
    kb_update_power();
    kb_update_thinklight();
}

