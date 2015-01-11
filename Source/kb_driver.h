#pragma once

#include <stdint.h>

int   kb_scan_matrix(uint8_t *matrix);

int   kb_get_fn_key(void);
int   kb_get_power_key(void);
int   kb_get_id(void);
void  kb_set_brightness(uint8_t level);

void  kb_init(void);
