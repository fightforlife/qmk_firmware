/*
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#pragma once

#define LAYER_STATE_8BIT
#define MATRIX_COLS 14
#define MATRIX_ROWS 5


/* sn32fxx.h Definitions for K66v1*/
#define SN32F2XX_LED_OUTPUT_LUMINOSITY_R 1
#define SN32F2XX_LED_OUTPUT_LUMINOSITY_G 1
#define SN32F2XX_LED_OUTPUT_LUMINOSITY_B 1
#define SN32F2XX_LED_COUNT 66
#define SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL SN32F2XX_PWM_OUTPUT_ACTIVE_HIGH
#define SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL SN32F2XX_RGB_OUTPUT_ACTIVE_LOW
#define SN32F2XX_PWM_CONTROL HARDWARE_PWM
#define SN32F2XX_PWM_DIRECTION ROW2COL
#define SN32F2XX_RGB_MATRIX_ROW_CHANNELS 3
#define SN32F2XX_RGB_MATRIX_ROWS MATRIX_ROWS
#define SN32F2XX_RGB_MATRIX_COLS MATRIX_COLS
#define SN32F2XX_RGB_MATRIX_ROW_PINS { A2, A0, A1, A5, A3, A4, A8, A6, A7, A11, A9, A10, A14, A12, A13 }  //RGB order
#define SHARED_MATRIX
#define SN32F2XX_RGB_MATRIX_COL_PINS MATRIX_COL_PINS
#define SN32F2XX_RGB_MATRIX_ROWS_HW (SN32F2XX_RGB_MATRIX_ROWS * SN32F2XX_RGB_MATRIX_ROW_CHANNELS)
#define SN32F2XX_RGB_MATRIX_COLOR_DEPTH 16


/* sn32f2xx.c Definitios for K66v1 */
#define MATRIX_UNSELECT_DRIVE_HIGH //needed for ROW2COL
#define SN32F2XX_ACTIVATE_PWM_CHAN_0 //A0
#define SN32F2XX_ACTIVATE_PWM_CHAN_1 //A1
#define SN32F2XX_ACTIVATE_PWM_CHAN_2 //A2
#define SN32F2XX_ACTIVATE_PWM_CHAN_3 //A3
#define SN32F2XX_ACTIVATE_PWM_CHAN_4 //A4
#define SN32F2XX_ACTIVATE_PWM_CHAN_5 //A5
#define SN32F2XX_ACTIVATE_PWM_CHAN_6 //A6
#define SN32F2XX_ACTIVATE_PWM_CHAN_7 //A7
#define SN32F2XX_ACTIVATE_PWM_CHAN_8 //A8
#define SN32F2XX_ACTIVATE_PWM_CHAN_9 //A9
#define SN32F2XX_ACTIVATE_PWM_CHAN_10 //A10
#define SN32F2XX_ACTIVATE_PWM_CHAN_11 //A11
#define SN32F2XX_ACTIVATE_PWM_CHAN_12 //A12
#define SN32F2XX_ACTIVATE_PWM_CHAN_13 //A13
#define SN32F2XX_ACTIVATE_PWM_CHAN_14 //A14




#define SLED1734X_I2C_ADDRESS_1 SLED1734X_I2C_ADDRESS_GND  //AD is connected to ground (1110100(0)=0xE8) (0x74 << 1)
#define SLED1734X_LED_COUNT 43
#define SLED1734X_RGB_MATRIX_COLOR_DEPTH 16 // 8 and 16 are possible, 24 takes too much RAM

#define I2C1_SCL_PIN D0
#define I2C1_SDA_PIN A15
#define SLED1734X_SDB_PIN C9