#pragma once

#include <stdint.h>

struct tp_mouse_report {
    uint8_t buttons;
    int8_t  dx;
    int8_t  dy;
    int8_t  dwheel;
    int8_t  dpan;
};


extern struct tp_mouse_report tp_mouse_report;


void tp_clear_mouse_report(void);

void tp_update(void);
void tp_init(void);
