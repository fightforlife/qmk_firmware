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
#include "rgb_matrix.h"
#include "sled1734x_direct.h"
#include "progmem.h"

#ifndef SLED1734X_RGB_MATRIX_COLOR_DEPTH
#    define SLED1734X_RGB_MATRIX_COLOR_DEPTH 24
#endif

/* I2C Address Declarations */
#define SLED1734X_I2C_ADDRESS_GND 0x74
#define SLED1734X_I2C_ADDRESS_SCL 0x75
#define SLED1734X_I2C_ADDRESS_SDA 0x76
#define SLED1734X_I2C_ADDRESS_VDDIO 0x77

#define SLED1734X_OFFSET 0x20
#define SLED1734X_FRAME_OFFSET 0x80

#define I2C_SDA A15
#define I2C_SCL D0
#define I2C_SDB C9 // has to be High

#define I2C_SCL_IN readPin(I2C_SCL)
#define I2C_SDA_IN readPin(I2C_SDA)

#define I2C_SCL_HI             \
    do {                       \
        setPinOutput(I2C_SCL); \
        writePinHigh(I2C_SCL); \
    } while (0)
#define I2C_SCL_LO             \
    do {                       \
        setPinOutput(I2C_SCL); \
        writePinLow(I2C_SCL);  \
    } while (0)
#define I2C_SCL_HIZ               \
    do {                          \
        setPinInputHigh(I2C_SCL); \
    } while (0)

#define I2C_SDA_HI             \
    do {                       \
        setPinOutput(I2C_SDA); \
        writePinHigh(I2C_SDA); \
    } while (0)
#define I2C_SDA_LO             \
    do {                       \
        setPinOutput(I2C_SDA); \
        writePinLow(I2C_SDA);  \
    } while (0)
#define I2C_SDA_HIZ               \
    do {                          \
        setPinInputHigh(I2C_SDA); \
    } while (0)

/*
 * according to the spec, high SCL peroid 0.7us, low SCL peroid 1.3us
 * i2c_delay 1 loop about 7 cycles. Under 48MHz, the actual delay is around 0.9us and 1.5us respectively.
 * Reduced this from 2 to 0 on Womier K66. No problems in testing.
 */
#define I2C_DELAY i2c_delay(1)

void i2c_delay(int delay) {
    for (int32_t i = 0; i < delay; i++)
        __NOP();
}

void i2c_init(void) {
    setPinOutput(I2C_SDB);
    writePinHigh(I2C_SDB);
    I2C_SCL_HI;
    I2C_SDA_HIZ;
    i2c_delay(50000); // very lojng delay for I2C LED chip wakeup min 180us, not calculated
}

void i2c_start(void) {
    /* I2C Start condition, data line goes low when clock is high */
    /* START */
    I2C_SDA_LO;
    I2C_DELAY; // needed in testing!
    I2C_SCL_LO;
    //I2C_DELAY; //not needed in testing
}

void i2c_stop(void) {
    /* I2C Stop condition, clock goes high when data is low */
    I2C_SDA_LO;
    //I2C_DELAY; //not needed in testing

    /* STOP */
    I2C_SCL_HI;
    //I2C_DELAY; //not needed in testing
    I2C_SDA_HIZ;

    //I2C_DELAY; //not needed in testing
    I2C_DELAY;
}

bool i2c_sendByte(uint8_t data) {   // Send a byte over I2C
    for (uint8_t i = 0; i < 8; i++) // 8 bits, so 8 loops
    {
        if ((0x80 >> i) & data)
            I2C_SDA_HI; // Check if i bit to be sent is a 1 or 0  // It's a 1, so send a logic HIGH
        else
            I2C_SDA_LO; // It's a 0, so just pretend we need send a 0 for completion
        // Generate Clock for data bits
        I2C_SCL_HI;
        I2C_DELAY; // needed
        I2C_SCL_LO;
        I2C_DELAY; //(not needed in testing)
    }
    // Tick Signal that we sent the logic
    I2C_SDA_HIZ;
    I2C_SCL_HI;
    I2C_DELAY; // needed
    bool ack = !I2C_SDA_IN;
    I2C_SCL_LO;
    //I2C_DELAY; //not needed in testing
    return ack;
}

void i2c_writeReg(uint8_t adr, uint8_t reg, const uint8_t data) {
    i2c_start();
    i2c_sendByte(adr);
    i2c_sendByte(reg);
    i2c_sendByte(data);
    i2c_stop();
}

void i2c_writeBuf(uint8_t adr, uint8_t reg, const uint8_t *data, uint8_t len) {
    i2c_start();
    i2c_sendByte(adr);
    i2c_sendByte(reg);
    for (int i = 0; i < len; i++) {
        i2c_sendByte(data[i]);
    }
    i2c_stop();
}

#if (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 24)
uint8_t colorBuffer[SLED1734X_LED_COUNT][3]; // RBG
#elif (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 16)
uint16_t colorBuffer[SLED1734X_LED_COUNT];
#elif (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 8)
uint8_t colorBuffer[SLED1734X_LED_COUNT];
#endif

uint16_t stateTracker;
/*
0x8000 is_writing
0x4000
0x2000
0x1000
0x0800 Group dirty
0x0400 Group dirty
0x0200 Group dirty
0x0100 Group dirty
0x0080 Group dirty
0x0040 Group dirty
0x0020 Group dirty
0x0010 Group dirty
0x0008 Group dirty
0x0004 Group dirty
0x0002 Group dirty
0x0001 Group dirty
*/

void i2c_writeGroup(uint8_t adr, uint8_t reg, const uint8_t *data, uint8_t len, uint16_t dirtyMaker) {
    if (stateTracker & dirtyMaker) {
        if (!(stateTracker & 0x8000)) {
            i2c_start();
            i2c_sendByte(adr);
            i2c_sendByte(reg);
            stateTracker |= 0x8000;
        }

        for (int i = 0; i < len; i++) {
            i2c_sendByte(data[i]);
        }

        stateTracker &= ~dirtyMaker;
    } else {
        if (stateTracker & 0x8000) {
            i2c_stop();
            stateTracker &= ~0x8000;
        }
    }
}

// List of Configuration registers
enum {
    REG_FUNC_CONFIGURATION      = 0x00,
    REG_FUNC_PICTURE_DISPLAY    = 0x01,
    REG_FUNC_DISPLAY_OPTION     = 0x05,
    REG_FUNC_AUDIO_SYNC         = 0x06,
    REG_FUNC_BREATH_CONTROL_1   = 0x08,
    REG_FUNC_BREATH_CONTROL_2   = 0x09,
    REG_FUNC_SHUTDOWN           = 0x0A,
    REG_FUNC_AUDIO_GAIN_CONTROL = 0x0B,
    REG_FUNC_STAGGERED_DELAY    = 0x0D,
    REG_FUNC_SLEW_RATE_CONTROL  = 0x0E,
    REG_FUNC_CURRENT_CONTROL    = 0x0F,
    REG_FUNC_OPEN_SHORT_TEST_1  = 0x10,
    REG_FUNC_OPEN_SHORT_TEST_2  = 0x11,
    REG_FUNC_ADC_OUTPUT         = 0x13,
    REG_FUNC_VAF_1              = 0x14,
    REG_FUNC_VAF_2              = 0x15,
    REG_FUNC_THERMAL_DETECTION  = 0x17,
};

// list of LED registers
enum {
    REG_CONFIGURE_COMMAND = 0xFD,
    PAGE_FRAME_1          = 0x00,
    PAGE_FRAME_2          = 0x01,
    PAGE_FUNCTION         = 0x0B,
    PAGE_DETECTION        = 0x0C,
    PAGE_VAF              = 0x0D,

    PAGE_FRAME_SPLIT = 0x03, // not specifed, needed for LEDs where not all colors are in the same frame
};

// List of enabled LEDs for RAM map, not PWM!
const uint8_t PROGMEM state_frame1[16] = {
    0b11111000, 0b11111111, // CA1 R
    0b11111000, 0b11111111, // CA2 B
    0b11111000, 0b11111111, // CA3 G

    0b11000111, 0b11111111, // CA4 R
    0b11000111, 0b11111111, // CA5 B
    0b11000111, 0b11111111, // CA6 G

    0b00111111, 0b11111110, // CA7 R
    0b00111111, 0b11111110  // CA8 B
};
const uint8_t PROGMEM state_frame2[8] = {
    0b00111111, 0b11111110, // CA9 G

    0b00000000, 0b01110001, // CB1 R
    0b00000000, 0b01110001, // CB2 B
    0b00000000, 0b01110001, // CB3 G
};

/*
         0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
CA1 0x20                 R00  R01  R02  R03  R04  R05  R06  R07  R08  R09  R10  R11  R12
CA2 0x30                 B00  B01  B02  B03  B04  B05  B06  B07  B08  B09  B10  B11  B12
CA3 0x40                 G00  G01  G02  G03  G04  G05  G06  G07  G08  G09  G10  G11  G12
CA4 0x50  R13  R14  R15                 R16  R17  R18  R19  R20  R21  R22  R23  R24  R25
CA5 0x60  B13  B14  B15                 B16  B17  B18  B19  B20  B21  B22  B23  B24  B25
CA6 0x70  G13  G14  G15                 G16  G17  G18  G19  G20  G21  G22  G23  G24  G25
CA7 0x80  R26  R27  R28  R29  R30  R31                 R32  R33  R34  R35  R36  R37  R38
CA8 0x90  B26  B27  B28  B29  B30  B31                 B32  B33  B34  B35  B36  B37  B38

CA9 0x20  G26  G27  G28  G29  G30  G31                 G32  G33  G34  G35  G36  G37  G38
CB1 0x30                                          R39                 R40  R41  R42
CB2 0x40                                          B39                 B40  B41  B42
CB3 0x50                                          G39                 G40  G41  G42
CB4 0x60
CB5 0x70
CB6 0x80
CB7 0x90
*/

void sled1734x_init_drivers(void) {
    // initialise I2C
    i2c_init();
    // write config Registers as described in SLED1734 pdf (Matrix type3), using writeReg func since performance is not important. (page 82, middle, 86-)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND, PAGE_FUNCTION);
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_CONFIGURATION, 0x00);    // SYNC High Impedance 00, ADC disable 0, PWM enable 0 (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_PICTURE_DISPLAY, 0x10);  // Matrix Type 3, (00010000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_DISPLAY_OPTION, 0x00);   // Blinking Off (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_AUDIO_SYNC, 0x00);       // Audio Off (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_BREATH_CONTROL_1, 0x00); // Disable (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_BREATH_CONTROL_2, 0x00); // Disable (00000000)
    // REG_FUNC_SHUTDOWN (10)will set at the end
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_AUDIO_GAIN_CONTROL, 0x00); // Disable (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_STAGGERED_DELAY, 0x00);    // Disable (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_SLEW_RATE_CONTROL, 0x01);  // Enable (00000001)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_CURRENT_CONTROL, 0xB0);    // Default Disabled (00110001,0x31,8mA), Max (10111111,0xBF,39.5mA), Choosen(10110000,0x30,32mA)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_OPEN_SHORT_TEST_1, 0x00);  // Default Disable (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_OPEN_SHORT_TEST_2, 0x00);  // Default Disable (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_ADC_OUTPUT, 0x00);         // Default Disable (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_VAF_1, 0x44);              // Default (01000100)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_VAF_2, 0x04);              // Default (00000100)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_SHUTDOWN, 0x01);           // wakeup

    // All LEDs ON in Frame1, not PWM
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND, PAGE_FRAME_1);
    i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, 0x00, state_frame1, 16);

    // All LEDs ON Frame2, not PWM
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND, PAGE_FRAME_2);
    i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, 0x00, state_frame2, 8);
}

void sled1734x_flush(void) {
    if (!(stateTracker & 0x0FFF)) return;

#if (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 24)

    if (stateTracker & 0b0000000011111111) {
        /* PAGE FRAME 1 */
        i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND, PAGE_FRAME_1);
        /* Group 0x01 - CA1 */
        i2c_writeGroup(SLED1734X_I2C_ADDRESS_1 << 1, 0x20, (uint8_t[]){0x00, 0x00, 0x00, colorBuffer[0][0], colorBuffer[1][0], colorBuffer[2][0], colorBuffer[3][0], colorBuffer[4][0], colorBuffer[5][0], colorBuffer[6][0], colorBuffer[7][0], colorBuffer[8][0], colorBuffer[9][0], colorBuffer[10][0], colorBuffer[11][0], colorBuffer[12][0]}, 16, 0x01);
        /* Group 0x02 - CA2 */
        i2c_writeGroup(SLED1734X_I2C_ADDRESS_1 << 1, 0x30, (uint8_t[]){0x00, 0x00, 0x00, colorBuffer[0][1], colorBuffer[1][1], colorBuffer[2][1], colorBuffer[3][1], colorBuffer[4][1], colorBuffer[5][1], colorBuffer[6][1], colorBuffer[7][1], colorBuffer[8][1], colorBuffer[9][1], colorBuffer[10][1], colorBuffer[11][1], colorBuffer[12][1]}, 16, 0x02);
        /* Group 0x04 - CA3 */
        i2c_writeGroup(SLED1734X_I2C_ADDRESS_1 << 1, 0x40, (uint8_t[]){0x00, 0x00, 0x00, colorBuffer[0][2], colorBuffer[1][2], colorBuffer[2][2], colorBuffer[3][2], colorBuffer[4][2], colorBuffer[5][2], colorBuffer[6][2], colorBuffer[7][2], colorBuffer[8][2], colorBuffer[9][2], colorBuffer[10][2], colorBuffer[11][2], colorBuffer[12][2]}, 16, 0x04);
        /* Group 0x08 - CA4 */
        i2c_writeGroup(SLED1734X_I2C_ADDRESS_1 << 1, 0x50, (uint8_t[]){colorBuffer[13][0], colorBuffer[14][0], colorBuffer[15][0], 0x00, 0x00, 0x00, colorBuffer[16][0], colorBuffer[17][0], colorBuffer[18][0], colorBuffer[19][0], colorBuffer[20][0], colorBuffer[21][0], colorBuffer[22][0], colorBuffer[23][0], colorBuffer[24][0], colorBuffer[25][0]}, 16, 0x08);
        /* Group 0x10 - CA5 */
        i2c_writeGroup(SLED1734X_I2C_ADDRESS_1 << 1, 0x60, (uint8_t[]){colorBuffer[13][1], colorBuffer[14][1], colorBuffer[15][1], 0x00, 0x00, 0x00, colorBuffer[16][1], colorBuffer[17][1], colorBuffer[18][1], colorBuffer[19][1], colorBuffer[20][1], colorBuffer[21][1], colorBuffer[22][1], colorBuffer[23][1], colorBuffer[24][1], colorBuffer[25][1]}, 16, 0x10);
        /* Group 0x20 - CA6 */
        i2c_writeGroup(SLED1734X_I2C_ADDRESS_1 << 1, 0x70, (uint8_t[]){colorBuffer[13][2], colorBuffer[14][2], colorBuffer[15][2], 0x00, 0x00, 0x00, colorBuffer[16][2], colorBuffer[17][2], colorBuffer[18][2], colorBuffer[19][2], colorBuffer[20][2], colorBuffer[21][2], colorBuffer[22][2], colorBuffer[23][2], colorBuffer[24][2], colorBuffer[25][2]}, 16, 0x20);
        /* Group 0x40 - CA7 */
        i2c_writeGroup(SLED1734X_I2C_ADDRESS_1 << 1, 0x80, (uint8_t[]){colorBuffer[26][0], colorBuffer[27][0], colorBuffer[28][0], colorBuffer[29][0], colorBuffer[30][0], colorBuffer[31][0], 0x00, 0x00, 0x00, colorBuffer[32][0], colorBuffer[33][0], colorBuffer[34][0], colorBuffer[35][0], colorBuffer[36][0], colorBuffer[37][0], colorBuffer[38][0]}, 16, 0x40);
        /* Group 0x80 - CA8 */
        i2c_writeGroup(SLED1734X_I2C_ADDRESS_1 << 1, 0x90, (uint8_t[]){colorBuffer[26][1], colorBuffer[27][1], colorBuffer[28][1], colorBuffer[29][1], colorBuffer[30][1], colorBuffer[31][1], 0x00, 0x00, 0x00, colorBuffer[32][1], colorBuffer[33][1], colorBuffer[34][1], colorBuffer[35][1], colorBuffer[36][1], colorBuffer[37][1], colorBuffer[38][1]}, 16, 0x80);

        if (stateTracker & 0x8000) {
            i2c_stop();
            stateTracker &= ~0x8000;
        }
    }
    if (stateTracker & 0b0000111100000000) {
        i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND, PAGE_FRAME_2);

        /* Group 0x100 - CA9 */
        i2c_writeGroup(SLED1734X_I2C_ADDRESS_1 << 1, 0x20, (uint8_t[]){colorBuffer[26][2], colorBuffer[27][2], colorBuffer[28][2], colorBuffer[29][2], colorBuffer[30][2], colorBuffer[31][2], 0x00, 0x00, 0x00, colorBuffer[32][2], colorBuffer[33][2], colorBuffer[34][2], colorBuffer[35][2], colorBuffer[36][2], colorBuffer[37][2], colorBuffer[38][2]}, 16, 0x100);
        /* Group 0x200 - CB1 */
        i2c_writeGroup(SLED1734X_I2C_ADDRESS_1 << 1, 0x30, (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, colorBuffer[39][0], 0x00, 0x00, 0x00, colorBuffer[40][0], colorBuffer[41][0], colorBuffer[42][0], 0x00}, 16, 0x200);
        /* Group 0x400 - CB2 */
        i2c_writeGroup(SLED1734X_I2C_ADDRESS_1 << 1, 0x40, (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, colorBuffer[39][1], 0x00, 0x00, 0x00, colorBuffer[40][1], colorBuffer[41][1], colorBuffer[42][1], 0x00}, 16, 0x400);
        /* Group 0x800 - CB3 */
        i2c_writeGroup(SLED1734X_I2C_ADDRESS_1 << 1, 0x50, (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, colorBuffer[39][2], 0x00, 0x00, 0x00, colorBuffer[40][2], colorBuffer[41][2], colorBuffer[42][2], 0x00}, 16, 0x800);

        if (stateTracker & 0x8000) i2c_stop();
        if (stateTracker & 0x8000) {
            i2c_stop();
            stateTracker &= ~0x8000;
        }
    }

#elif (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 16)
    uint8_t temp[16];

    /* PAGE FRAME 1 */
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND, PAGE_FRAME_1);

    /* Group 0: pwm_buffer[0..15] -> offsets 0x00,0x10,0x20 */
    if (pwm_group_dirty & 0x01) {
        for (int i = 0; i < 16; i++)
            temp[i] = ((pwm_buffer[i] & 0b1111100000000000) >> 11) * 8;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x00, temp, 16);
        for (int i = 0; i < 16; i++)
            temp[i] = ((pwm_buffer[i] & 0b0000011111100000) >> 5) * 4;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x10, temp, 16);
        for (int i = 0; i < 16; i++)
            temp[i] = ((pwm_buffer[i] & 0b0000000000011111) >> 0) * 8;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x20, temp, 16);
    }

    /* Group 1: pwm_buffer[16..31] -> offsets 0x30,0x40,0x50 */
    if (pwm_group_dirty & 0x02) {
        int base = 16;
        for (int i = 0; i < 16; i++)
            temp[i] = ((pwm_buffer[base + i] & 0b1111100000000000) >> 11) * 8;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x30, temp, 16);
        for (int i = 0; i < 16; i++)
            temp[i] = ((pwm_buffer[base + i] & 0b0000011111100000) >> 5) * 4;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x40, temp, 16);
        for (int i = 0; i < 16; i++)
            temp[i] = ((pwm_buffer[base + i] & 0b0000000000011111) >> 0) * 8;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x50, temp, 16);
    }

    /* Group 2: pwm_buffer[32..47] -> offsets 0x60,0x70 (frame1) and 0x00 (frame2 B part) */
    if (pwm_group_dirty & 0x04) {
        int base = 32;
        for (int i = 0; i < 16; i++)
            temp[i] = ((pwm_buffer[base + i] & 0b1111100000000000) >> 11) * 8;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x60, temp, 16);
        for (int i = 0; i < 16; i++)
            temp[i] = ((pwm_buffer[base + i] & 0b0000011111100000) >> 5) * 4;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x70, temp, 16);
    }

    /* PAGE FRAME 2 */
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND, PAGE_FRAME_2);

    /* Group 2 B-part */
    if (pwm_group_dirty & 0x04) {
        int base = 32;
        for (int i = 0; i < 16; i++)
            temp[i] = ((pwm_buffer[base + i] & 0b0000000000011111) >> 0) * 8;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x00, temp, 16);
    }

    /* Group 3: pwm_buffer[48..63] -> frame2 offsets 0x10,0x20,0x30 */
    if (pwm_group_dirty & 0x08) {
        int base = 48;
        for (int i = 0; i < 16; i++)
            temp[i] = ((pwm_buffer[base + i] & 0b1111100000000000) >> 11) * 8;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x10, temp, 16);
        for (int i = 0; i < 16; i++)
            temp[i] = ((pwm_buffer[base + i] & 0b0000011111100000) >> 5) * 4;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x20, temp, 16);
        for (int i = 0; i < 16; i++)
            temp[i] = ((pwm_buffer[base + i] & 0b0000000000011111) >> 0) * 8;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x30, temp, 16);
    }

    /* Clear dirty mask */
    pwm_group_dirty  = 0;
    pwm_buffer_dirty = false;

#elif (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 8)
    uint8_t temp8[16];

    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND, PAGE_FRAME_1);

    if (pwm_group_dirty & 0x01) {
        for (int i = 0; i < 16; i++)
            temp8[i] = ((pwm_buffer[i] & 0b11100000) >> 5) * 32;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x00, temp8, 16);
        for (int i = 0; i < 16; i++)
            temp8[i] = ((pwm_buffer[i] & 0b00011100) >> 2) * 32;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x10, temp8, 16);
        for (int i = 0; i < 16; i++)
            temp8[i] = ((pwm_buffer[i] & 0b00000011) >> 0) * 64;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x20, temp8, 16);
    }

    if (pwm_group_dirty & 0x02) {
        int base = 16;
        for (int i = 0; i < 16; i++)
            temp8[i] = ((pwm_buffer[base + i] & 0b11100000) >> 5) * 32;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x30, temp8, 16);
        for (int i = 0; i < 16; i++)
            temp8[i] = ((pwm_buffer[base + i] & 0b00011100) >> 2) * 32;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x40, temp8, 16);
        for (int i = 0; i < 16; i++)
            temp8[i] = ((pwm_buffer[base + i] & 0b00000011) >> 0) * 64;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x50, temp8, 16);
    }

    if (pwm_group_dirty & 0x04) {
        int base = 32;
        for (int i = 0; i < 16; i++)
            temp8[i] = ((pwm_buffer[base + i] & 0b11100000) >> 5) * 32;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x60, temp8, 16);
        for (int i = 0; i < 16; i++)
            temp8[i] = ((pwm_buffer[base + i] & 0b00011100) >> 2) * 32;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x70, temp8, 16);
    }

    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND, PAGE_FRAME_2);

    if (pwm_group_dirty & 0x04) {
        int base = 32;
        for (int i = 0; i < 16; i++)
            temp8[i] = ((pwm_buffer[base + i] & 0b00000011) >> 0) * 64;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x00, temp8, 16);
    }

    if (pwm_group_dirty & 0x08) {
        int base = 48;
        for (int i = 0; i < 16; i++)
            temp8[i] = ((pwm_buffer[base + i] & 0b11100000) >> 5) * 32;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x10, temp8, 16);
        for (int i = 0; i < 16; i++)
            temp8[i] = ((pwm_buffer[base + i] & 0b00011100) >> 2) * 32;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x20, temp8, 16);
        for (int i = 0; i < 16; i++)
            temp8[i] = ((pwm_buffer[base + i] & 0b00000011) >> 0) * 64;
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x30, temp8, 16);
    }

    pwm_group_dirty  = 0;
    pwm_buffer_dirty = false;

#endif // SLED1734X_RGB_MATRIX_COLOR_DEPTH
}

void sled1734x_set_color(int index, uint8_t r, uint8_t g, uint8_t b) {
#if (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 24)

    if (colorBuffer[index][0] != r) {
        colorBuffer[index][0] = r;
        if (index >= 0 && index < 13) stateTracker |= 0b0000000000000001;
        if (index >= 13 && index < 26) stateTracker |= 0b0000000000001000;
        if (index >= 26 && index < 39) stateTracker |= 0b0000000001000000;
        if (index >= 39 && index < 43) stateTracker |= 0b0000001000000000;
    };
    if (colorBuffer[index][1] != g) {
        colorBuffer[index][1] = g;
        if (index >= 0 && index < 13) stateTracker |= 0b0000000000000010;
        if (index >= 13 && index < 26) stateTracker |= 0b0000000000010000;
        if (index >= 26 && index < 39) stateTracker |= 0b0000000010000000;
        if (index >= 39 && index < 43) stateTracker |= 0b0000010000000000;
    };
    if (colorBuffer[index][2] != b) {
        colorBuffer[index][2] = b;
        if (index >= 0 && index < 13) stateTracker |= 0b0000000000000100;
        if (index >= 13 && index < 26) stateTracker |= 0b0000000000100000;
        if (index >= 26 && index < 39) stateTracker |= 0b0000000100000000;
        if (index >= 39 && index < 43) stateTracker |= 0b0000100000000000;
    };

#elif (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 16)

    colorBuffer[index] = ((r * 31 / 255) << 11) + ((g * 63 / 255) << 5) + ((b * 31 / 255) << 0);

#elif (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 8)

    colorBuffer[index] = ((r * 7 / 255) << 5) + ((g * 7 / 255) << 2) + (b * 3 / 255);

#endif // SLED1734X_RGB_MATRIX_COLOR_DEPTH
}
