#pragma once

#include <stdint.h>


struct kb_in_report {
    // Usage Page 0x07 Generic Desktop
    //
    unsigned _e0_keyboard_left_control  : 1;
    unsigned _e1_keyboard_left_shift    : 1;
    unsigned _e2_keyboard_left_alt      : 1;
    unsigned _e3_keyboard_left_gui      : 1;
    unsigned _e4_keyboard_right_control : 1;
    unsigned _e5_keyboard_right_shift   : 1;
    unsigned _e6_keyboard_rigt_alt      : 1;
    unsigned _e7_keyboard_right_gui     : 1;

    uint8_t reserved;
    uint8_t keycode[6];
};


struct kb_out_report {
    // Usage Page 0x08 LEDs
    //
    unsigned _01_num_lock   : 1;
    unsigned _02_caps_lock  : 1;
    unsigned _03_scroll_lock: 1;
    unsigned _04_compose    : 1;
    unsigned _05_kana       : 1;
    unsigned reserved       : 3;
};


struct kb_sysctrl_report {
    uint8_t report_id_01;

    // Usage Page 0x01 Generic Desktop
    //
    unsigned _81_system_power_down  : 1;
    unsigned _82_system_sleep       : 1;
    unsigned _83_system_wake_up     : 1;
    unsigned _a8_system_hibernate   : 1;
    unsigned reserved               : 4;
};


struct kb_consumer_report {
    uint8_t report_id_02;

    // Usage Page 0x0C Consumer Devices
    //
    unsigned _e9_volume_increment   : 1;
    unsigned _ea_volume_decrement   : 1;
    unsigned _e2_mute               : 1;
    unsigned _cd_play_pause         : 1;
    unsigned _b5_scan_next_track    : 1;
    unsigned _b6_scan_previous_track: 1;
    unsigned _b7_stop               : 1;
    unsigned _b8_eject              : 1;

    unsigned _18a_al_email_reader   : 1;
    unsigned _221_ac_search         : 1;
    unsigned _22a_ac_bookmarks      : 1;
    unsigned _223_ac_home           : 1;
    unsigned _224_ac_back           : 1;
    unsigned _225_ac_forward        : 1;
    unsigned _226_ac_stop           : 1;
    unsigned _227_ac_refresh        : 1;

    unsigned _183_al_consumer_control_configuration : 1;
    unsigned _196_al_internet_browser               : 1;
    unsigned _192_al_calculator                     : 1;
    unsigned _19e_al_terminal_lock_screensaver      : 1;
    unsigned _194_al_local_machine_browser          : 1;
    unsigned _206_ac_minimize                       : 1;
    unsigned _6f_brightness_increment               : 1;
    unsigned _70_brightness_decrement               : 1;
};


struct kb_misc_keys {
    // Usage Page 0xFF00 Vendor-defined
    //
    unsigned    _01_thinklight_up   : 1;
    unsigned    _02_thinklight_down : 1;
};


extern struct  kb_in_report         kb_in_report;

extern struct  kb_sysctrl_report    kb_sysctrl_report;
extern struct  kb_consumer_report   kb_consumer_report;

void kb_set_leds(const struct kb_out_report *report);

void kb_update(void);
