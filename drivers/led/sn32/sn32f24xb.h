#pragma once

#include <stdint.h>
#include <stdbool.h>

#if defined(RGB_MATRIX_SN32F24XB)
#    define SN32F24XB_LED_COUNT RGB_MATRIX_LED_COUNT
#endif

#if !defined(SN32_PWM_OUTPUT_ACTIVE_LEVEL)
#    define SN32_PWM_OUTPUT_ACTIVE_LEVEL SN32_PWM_OUTPUT_ACTIVE_LOW
#endif

#if !defined(SN32_RGB_OUTPUT_ACTIVE_LEVEL)
#    define SN32_RGB_OUTPUT_ACTIVE_LEVEL SN32_RGB_OUTPUT_ACTIVE_HIGH
#endif

#if !defined(SN32_PWM_CONTROL)
#    define SN32_PWM_CONTROL HARDWARE_PWM
#endif

#if !defined(SN32_PWM_DIRECTION)
#    define SN32_PWM_DIRECTION DIODE_DIRECTION
#endif

#if !defined(SN32_RGB_MATRIX_ROW_CHANNELS)
#    define SN32_RGB_MATRIX_ROW_CHANNELS 3
#endif

#if !defined(SN32_RGB_MATRIX_ROWS)
#    define SN32_RGB_MATRIX_ROWS MATRIX_ROWS
#endif

#if !defined(SN32_RGB_MATRIX_COLS)
#    define SN32_RGB_MATRIX_COLS MATRIX_COLS
#endif

#if !defined(SN32_RGB_MATRIX_ROW_PINS)
#    error Missing definition for SN32_RGB_MATRIX_ROW_PINS
#endif

#if !defined(SN32_RGB_MATRIX_COL_PINS)
#    define SHARED_MATRIX
#    define SN32_RGB_MATRIX_COL_PINS MATRIX_COL_PINS
#endif

#if !defined(SN32_RGB_MATRIX_ROWS_HW)
#    define SN32_RGB_MATRIX_ROWS_HW (SN32_RGB_MATRIX_ROWS * SN32_RGB_MATRIX_ROW_CHANNELS)
#endif

void sn32f24xb_init(void);
void sn32f24xb_flush(void);
void sn32f24xb_set_color(int index, uint8_t r, uint8_t g, uint8_t b);
void sn32f24xb_set_color_all(uint8_t r, uint8_t g, uint8_t b);
