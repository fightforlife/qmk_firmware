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
#define I2C_SDB C9 //has to be High

#define I2C_SCL_IN readPin(I2C_SCL)
#define I2C_SDA_IN readPin(I2C_SDA)

#define I2C_SCL_HI  do { setPinOutput(I2C_SCL); writePinHigh(I2C_SCL); } while (0)
#define I2C_SCL_LO  do { setPinOutput(I2C_SCL); writePinLow(I2C_SCL); } while (0)
#define I2C_SCL_HIZ do { setPinInputHigh(I2C_SCL); } while (0)

#define I2C_SDA_HI  do { setPinOutput(I2C_SDA); writePinHigh(I2C_SDA); } while (0)
#define I2C_SDA_LO  do { setPinOutput(I2C_SDA); writePinLow(I2C_SDA); } while (0)
#define I2C_SDA_HIZ do { setPinInputHigh(I2C_SDA); } while (0)

/*
 * according to the spec, high SCL peroid 0.7us, low SCL peroid 1.3us
 * i2c_delay 1 loop about 7 cycles. Under 48MHz, the actual delay is around 0.9us and 1.5us respectively.
 * Reduced this from 2 to 0 on Womier K66. No problems in testing.
 */
#define I2C_DELAY           i2c_delay(1)


void i2c_delay(int delay){
    for (int32_t i = 0; i < delay; i++)
        __NOP();
}

void i2c_init(void){
    setPinOutput(I2C_SDB);
    writePinHigh(I2C_SDB);
    I2C_SCL_HI;
    I2C_SDA_HIZ;
    i2c_delay(50000); //very lojng delay for I2C LED chip wakeup min 180us, not calculated
}

void i2c_start(void){
    /* I2C Start condition, data line goes low when clock is high */
    /* START */
    I2C_SDA_LO;
    I2C_DELAY; //needed in testing!
    I2C_SCL_LO;
    //I2C_DELAY; (not needed in testing)
}

void i2c_stop(void)
{
    /* I2C Stop condition, clock goes high when data is low */
    I2C_SDA_LO;
    //I2C_DELAY; (not needed in testing)

    /* STOP */
    I2C_SCL_HI;
    //I2C_DELAY; (not needed in testing)
    I2C_SDA_HIZ;

    //I2C_DELAY; (not needed in testing)
    //I2C_DELAY;
}

bool i2c_sendByte(uint8_t data){       // Send a byte over I2C
    for(uint8_t i = 0; i < 8; i++)          // 8 bits, so 8 loops
    {
        if((0x80 >> i) & data) I2C_SDA_HI;               // Check if i bit to be sent is a 1 or 0  // It's a 1, so send a logic HIGH
        else I2C_SDA_LO;                       // It's a 0, so just pretend we need send a 0 for completion
        //Generate Clock for data bits
        I2C_SCL_HI;
        I2C_DELAY; //needed
        I2C_SCL_LO;
        //I2C_DELAY; (not needed in testing)
    }
    // Tick Signal that we sent the logic
    I2C_SDA_HIZ;
    I2C_SCL_HI;
    I2C_DELAY; //needed
    bool ack = !I2C_SDA_IN;
    I2C_SCL_LO;
    //I2C_DELAY; (not needed in testing)
    return ack;
}

void i2c_writeReg(uint8_t adr, uint8_t reg, const uint8_t data){
    i2c_start();
    i2c_sendByte(adr);
    i2c_sendByte(reg);
    i2c_sendByte(data);
    i2c_stop();
}

void i2c_writeBuf(uint8_t adr, uint8_t reg, const uint8_t *data, uint8_t len){
    i2c_start();
    i2c_sendByte(adr);
    i2c_sendByte(reg);
    for(int i=0; i<len; i++){
        i2c_sendByte(data[i]);
    }
    i2c_stop();
}

//List of Configuration registers
enum{
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

//list of LED registers
enum{
    REG_CONFIGURE_COMMAND       = 0xFD,
    PAGE_FRAME_1                = 0x00,
    PAGE_FRAME_2                = 0x01,
    PAGE_FUNCTION               = 0x0B,
    PAGE_DETECTION              = 0x0C,
    PAGE_VAF                    = 0x0D,

    PAGE_FRAME_SPLIT            = 0x03, //not specifed, needed for LEDs where not all colors are in the same frame
};

//List of enabled LEDs for RAM map, not PWM!
const uint8_t PROGMEM state_frame1[16] = {
    0b11111000, 0b11111111, //CA1 R
    0b11111000, 0b11111111, //CA2 B
    0b11111000, 0b11111111, //CA3 G

    0b11000111, 0b11111111, //CA4 R
    0b11000111, 0b11111111, //CA5 B
    0b11000111, 0b11111111, //CA6 G

    0b00111111, 0b11111110, //CA7 R
    0b00111111, 0b11111110  //CA8 B
};
const uint8_t PROGMEM state_frame2[8] = {
    0b00111111, 0b11111110, //CA9 G

    0b00000000, 0b01110001, //CB1 R
    0b00000000, 0b01110001, //CB2 B
    0b00000000, 0b01110001, //CB3 G
};

//Mapping of underglow RBG channels to RAM-Map of I2C LED driver
const uint8_t PROGMEM led_map[SLED1734X_LED_COUNT][4] = {
    /*  R     B    G       Frame       */
    { 0x25, 0x35, 0x45, PAGE_FRAME_1}, //1
    { 0x24, 0x34, 0x44, PAGE_FRAME_1},
    { 0x23, 0x33, 0x43, PAGE_FRAME_1},
    { 0x5F, 0x6F, 0x7F, PAGE_FRAME_1},
    { 0x5E, 0x6E, 0x7E, PAGE_FRAME_1}, //5
    { 0x5D, 0x6D, 0x7D, PAGE_FRAME_1},
    { 0x5C, 0x6C, 0x7C, PAGE_FRAME_1},
    { 0x5B, 0x6B, 0x7B, PAGE_FRAME_1},
    { 0x5A, 0x6A, 0x7A, PAGE_FRAME_1},
    { 0x59, 0x69, 0x79, PAGE_FRAME_1}, //10
    { 0x58, 0x68, 0x78, PAGE_FRAME_1},
    { 0x57, 0x67, 0x77, PAGE_FRAME_1},
    { 0x56, 0x66, 0x76, PAGE_FRAME_1},
    { 0x52, 0x62, 0x72, PAGE_FRAME_1},
    { 0x51, 0x61, 0x71, PAGE_FRAME_1}, //15
    { 0x50, 0x60, 0x70, PAGE_FRAME_1},
    { 0x8F, 0x9F, 0x2F, PAGE_FRAME_SPLIT},
    { 0x8E, 0x9E, 0x2E, PAGE_FRAME_SPLIT},
    { 0x8D, 0x9D, 0x2D, PAGE_FRAME_SPLIT},
    { 0x8C, 0x9C, 0x2C, PAGE_FRAME_SPLIT}, //20
    { 0x8B, 0x9B, 0x2B, PAGE_FRAME_SPLIT},
    { 0x8A, 0x9A, 0x2A, PAGE_FRAME_SPLIT},
    { 0x89, 0x99, 0x29, PAGE_FRAME_SPLIT},
    { 0x85, 0x95, 0x25, PAGE_FRAME_SPLIT},
    { 0x84, 0x94, 0x24, PAGE_FRAME_SPLIT}, //25
    { 0x83, 0x93, 0x23, PAGE_FRAME_SPLIT},
    { 0x82, 0x92, 0x22, PAGE_FRAME_SPLIT},
    { 0x81, 0x91, 0x21, PAGE_FRAME_SPLIT},
    { 0x80, 0x90, 0x20, PAGE_FRAME_SPLIT},
    { 0x38 + SLED1734X_FRAME_OFFSET, 0x48, 0x58, PAGE_FRAME_2}, //30
    { 0x3E + SLED1734X_FRAME_OFFSET, 0x4E, 0x5E, PAGE_FRAME_2},
    { 0x3D + SLED1734X_FRAME_OFFSET, 0x4D, 0x5D, PAGE_FRAME_2},
    { 0x3C + SLED1734X_FRAME_OFFSET, 0x4C, 0x5C, PAGE_FRAME_2},
    { 0x2F, 0x3F, 0x4F, PAGE_FRAME_1},
    { 0x2E, 0x3E, 0x4E, PAGE_FRAME_1}, //35
    { 0x2D, 0x3D, 0x4D, PAGE_FRAME_1},
    { 0x2C, 0x3C, 0x4C, PAGE_FRAME_1},
    { 0x2B, 0x3B, 0x4B, PAGE_FRAME_1},
    { 0x2A, 0x3A, 0x4A, PAGE_FRAME_1},
    { 0x29, 0x39, 0x49, PAGE_FRAME_1}, //40
    { 0x28, 0x38, 0x48, PAGE_FRAME_1},
    { 0x27, 0x37, 0x47, PAGE_FRAME_1},
    { 0x26, 0x36, 0x46, PAGE_FRAME_1}
};


#if (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 24)
uint8_t pwm_buffer[192] = {0};
#elif (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 16)
uint16_t pwm_buffer[64] = {0};
#elif (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 8)
uint8_t pwm_buffer[64] = {0};
#endif


bool pwm_buffer_dirty = false; 



void sled1734x_init_drivers(){
    // initialise I2C
    i2c_init();
    //write config Registers as described in SLED1734 pdf (Matrix type3), using writeReg func since performance is not important. (page 82, middle, 86-)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND,       PAGE_FUNCTION);
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_CONFIGURATION,      0x00); //SYNC High Impedance 00, ADC disable 0, PWM enable 0 (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_PICTURE_DISPLAY,    0x10); //Matrix Type 3, (00010000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_DISPLAY_OPTION,     0x00); //Blinking Off (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_AUDIO_SYNC,         0x00); //Audio Off (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_BREATH_CONTROL_1,   0x00); //Disable (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_BREATH_CONTROL_2,   0x00); //Disable (00000000)
    //REG_FUNC_SHUTDOWN (10)will set at the end
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_AUDIO_GAIN_CONTROL, 0x00); //Disable (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_STAGGERED_DELAY,    0x00); //Disable (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_SLEW_RATE_CONTROL,  0x01); //Enable (00000001)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_CURRENT_CONTROL,    0xB0); //Default Disabled (00110001,0x31,8mA), Max (10111111,0xBF,39.5mA), Choosen(10110000,0x30,32mA)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_OPEN_SHORT_TEST_1,  0x00); //Default Disable (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_OPEN_SHORT_TEST_2,  0x00); //Default Disable (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_ADC_OUTPUT,         0x00); //Default Disable (00000000)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_VAF_1,              0x44); //Default (01000100)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_VAF_2,              0x04); //Default (00000100)
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_FUNC_SHUTDOWN,           0x01); //wakeup

    //All LEDs ON in Frame1, not PWM
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND,       PAGE_FRAME_1);
    i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, 0x00, state_frame1, 16);

    //All LEDs ON Frame2, not PWM
    i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND,       PAGE_FRAME_2);
    i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, 0x00, state_frame2, 8);    
}

void sled1734x_flush(void) {
    if (pwm_buffer_dirty) {

#if (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 16)
        uint8_t temp[16] = {0};

        i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND, PAGE_FRAME_1);

        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i] & 0b1111100000000000) >> 11) * 8;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x00, temp, 16);
        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i] & 0b0000011111100000) >> 5) * 4;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x10, temp, 16);
        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i] & 0b0000000000011111) >> 0) * 8;           
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x20, temp, 16);

        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 16] & 0b1111100000000000) >> 11) * 8;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x30, temp, 16);
        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 16] & 0b0000011111100000) >> 5) * 4;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x40, temp, 16);
        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 16] & 0b0000000000011111) >> 0) * 8;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x50, temp, 16);

        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 32] & 0b1111100000000000) >> 11) * 8;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x60, temp, 16);
        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 32] & 0b0000011111100000) >> 5) * 4;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x70, temp, 16);



        i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND, PAGE_FRAME_2);

        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 32] & 0b0000000000011111) >> 0) * 8;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x00, temp, 16);

        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 48] & 0b1111100000000000) >> 11) * 8;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x10, temp, 16);
        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 48] & 0b0000011111100000) >> 5) * 4;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x20, temp, 16);
        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 48] & 0b0000000000011111) >> 0) * 8;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x30, temp, 16);

#elif (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 8)
        uint8_t temp[16] = {0};

        i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND, PAGE_FRAME_1);

        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i] & 0b11100000) >> 5) * 32;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x00, temp, 16);
        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i] & 0b00011100) >> 2) * 32;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x10, temp, 16);
        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i] & 0b00000011) >> 0) * 64;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x20, temp, 16);

        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 16] & 0b11100000) >> 5) * 32;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x30, temp, 16);
        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 16] & 0b00011100) >> 2) * 32;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x40, temp, 16);
        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 16] & 0b00000011) >> 0) * 64;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x50, temp, 16);

        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 32] & 0b11100000) >> 5) * 32;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x60, temp, 16);
        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 32] & 0b00011100) >> 2) * 32;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x70, temp, 16);



        i2c_writeReg(SLED1734X_I2C_ADDRESS_1 << 1, REG_CONFIGURE_COMMAND, PAGE_FRAME_2);

        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 32] & 0b00000011) >> 0) * 64;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x00, temp, 16);

        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 48] & 0b11100000) >> 5) * 32;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x10, temp, 16);
        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 48] & 0b00011100) >> 2) * 32;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x20, temp, 16);
        for (int i = 0; i < 16; i++) {
            temp[i] = ((pwm_buffer[i + 48] & 0b00000011) >> 0) * 64;
        }
        i2c_writeBuf(SLED1734X_I2C_ADDRESS_1 << 1, SLED1734X_OFFSET + 0x30, temp, 16);

#endif // SLED1734X_RGB_MATRIX_COLOR_DEPTH
    }
}

void sled1734x_set_color(int index, uint8_t r, uint8_t g, uint8_t b){



#if (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 16)
    /*Calculate the byte location for all three colors example
    R=CA7_M 0x6C   /   G=CA9_M 0x8C   /   B=CA8_M 0x7C
    Target is byte 44 0x2C
    base 0x06  /  remainder C
    location = 0x2C = 44
    */
    uint8_t base      = ((led_map[index][0]-SLED1734X_OFFSET) / 0x10); // 9
    uint8_t remainder = ((led_map[index][0]-SLED1734X_OFFSET) % 0x10); // D
    uint8_t location  = ((base * 0x10) / 0x03) + remainder; // 0x3D
    //uint8_t location  = ((((led_map[index][0]-SLED1734X_OFFSET) / 0x10) * 0x10) / 0x03) + ((led_map[index][0]-SLED1734X_OFFSET) % 0x10); // 0x3D

    if (pwm_buffer[location] == ((r * 31 / 255) << 11) + ((g * 63 / 255) << 5) + ((b * 31 / 255) << 0)) {
        return;
    }
    pwm_buffer[location] = ((r * 31 / 255) << 11) + ((g * 63 / 255) << 5) + ((b * 31 / 255) << 0);
    pwm_buffer_dirty     = true;

#elif (SLED1734X_RGB_MATRIX_COLOR_DEPTH == 8)
    /*Calculate the byte location for all three colors example
    R=CA7_M 0x6C   /   G=CA9_M 0x8C   /   B=CA8_M 0x7C
    Target is byte 44 0x2C
    base 0x06  /  remainder C
    location = 0x2C = 44
    */
    uint8_t base      = (led_map[index][0]-SLED1734X_OFFSET) / 0x10; // 9
    uint8_t remainder = (led_map[index][0]-SLED1734X_OFFSET) % 0x10; // D
    uint8_t location  = ((base * 0x10) / 0x03) + remainder; // 0x3D

    if (pwm_buffer[location] == ((r * 7 / 255) << 5) + ((g * 7 / 255) << 2) + (b * 3 / 255)) {
        return;
    }
    pwm_buffer[location] = ((r * 7 / 255) << 5) + ((g * 7 / 255) << 2) + (b * 3 / 255);
    pwm_buffer_dirty     = true;
#endif // SLED1734X_RGB_MATRIX_COLOR_DEPTH

}

void sled1734x_set_color_all(uint8_t r, uint8_t g, uint8_t b){
    for (int i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        sled1734x_set_color(i, r, g, b);
    }
}