#include "rgb_matrix.h"
#include "drivers/led/sn32f2xx.c"
//#include "drivers/rgb_matrix_sled1734x.c"
#include "drivers/led/sled1734x.c"

void custom_init(void) {
    sn32f2xx_init();
    //SLED1734X_init(UNDERGLOW_I2C_ADR);
    sled1734x_init_drivers();

}

void custom_set_color(int index, uint8_t r, uint8_t g, uint8_t b) {
     //if key matrix LEDs
     if(index < SN32F2XX_LED_COUNT) 
     {     
        sn32f2xx_set_color(index, r, g, b);
     }
     //if underglow LEDs
     else if (index < RGB_MATRIX_LED_COUNT)
     {
        //SLED1734X_set_color(index, r, g, b);
        sled1734x_set_color(index, r, g, b);
     }
}

void custom_set_color_all(uint8_t r, uint8_t g, uint8_t b) {
    for (int i=0; i<RGB_MATRIX_LED_COUNT; i++) {
        custom_set_color(i, r, g, b);
    }
}


void custom_flush(void) {
    sn32f2xx_flush();
    sled1734x_flush();
    
}

const rgb_matrix_driver_t rgb_matrix_driver = {
    .init          = custom_init,
    .flush         = custom_flush,
    .set_color     = custom_set_color,
    .set_color_all = custom_set_color_all,
};