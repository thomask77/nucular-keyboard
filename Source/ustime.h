#pragma once

#include <stdint.h>

uint64_t get_us_time64(void);
uint32_t get_us_time32(void);

void     delay_us(int us);
void     delay_ms(int ms);

void     init_us_timer(void);

