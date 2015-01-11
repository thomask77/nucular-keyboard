#pragma once

#include <unistd.h>
#include <stdint.h>

ssize_t ps2_read(void *buf, size_t n);
ssize_t ps2_write(const void *buf, size_t n);
void    ps2_init(void);
