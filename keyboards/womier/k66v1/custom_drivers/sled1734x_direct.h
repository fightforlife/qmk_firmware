#pragma once

#include <stdint.h>
#include <stdbool.h>

void sled1734x_init_drivers(void);
void sled1734x_flush(void);
void sled1734x_set_color(int index, uint8_t r, uint8_t g, uint8_t b);
void sled1734x_set_color_all(uint8_t r, uint8_t g, uint8_t b);