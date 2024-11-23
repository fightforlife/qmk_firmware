#pragma once

#include <stdint.h>
#include <stdbool.h>

void SLED1734X_init(uint8_t devid);
void SLED1734X_flush(void);
void SLED1734X_set_color(int index, uint8_t r, uint8_t g, uint8_t b);
void SLED1734X_set_color_all(uint8_t r, uint8_t g, uint8_t b);