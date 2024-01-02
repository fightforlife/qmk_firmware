#include <string.h>
#include "matrix.h"
#include "rgb_matrix.h"
#include "sn32f2xx.h"

#define ROWS_PER_HAND (MATRIX_ROWS)

#if !defined(MATRIX_IO_DELAY)
#    define MATRIX_IO_DELAY 30
#endif

#define SN32_PWM_OUTPUT_ACTIVE_HIGH PWM_OUTPUT_ACTIVE_HIGH
#define SN32_PWM_OUTPUT_ACTIVE_LOW PWM_OUTPUT_ACTIVE_LOW

#define SN32_RGB_OUTPUT_ACTIVE_HIGH PWM_OUTPUT_ACTIVE_HIGH
#define SN32_RGB_OUTPUT_ACTIVE_LOW PWM_OUTPUT_ACTIVE_LOW

#define HARDWARE_PWM 0
#define SOFTWARE_PWM 1

/*
    Default configuration example

    COLS key / led
    SS8050 transistors NPN driven low
    base      - GPIO
    collector - LED Col pins
    emitter   - VDD

    VDD     GPIO
    (E)     (B)
     |  PNP  |
     |_______|
         |
         |
        (C)
        LED

    ROWS RGB
    SS8550 transistors PNP driven high
    base      - GPIO
    collector - LED RGB row pins
    emitter   - GND

        LED
        (C)
         |
         |
      _______
     |  NPN  |
     |       |
    (B)     (E)
    GPIO    GND
*/
#if (SN32_PWM_DIRECTION == COL2ROW)
static uint8_t chan_col_order[SN32_RGB_MATRIX_COLS] = {0}; // track the channel col order
static uint8_t current_row                          = 0;   // LED row scan counter
static uint8_t current_key_row                      = 0;   // key row scan counter
#    if (SN32_PWM_CONTROL == SOFTWARE_PWM)
static uint8_t led_duty_cycle[SN32_RGB_MATRIX_COLS] = {0}; // track the channel duty cycle
#    endif
#elif (SN32_PWM_DIRECTION == ROW2COL)
/* make sure to `#define MATRIX_UNSELECT_DRIVE_HIGH` in this configuration*/
static uint8_t chan_row_order[SN32_RGB_MATRIX_ROWS_HW] = {0}; // track the channel row order
static uint8_t current_key_col                         = 0;   // key col scan counter
static uint8_t last_key_col                            = 0;   // key col scan counter
#    if (SN32_PWM_CONTROL == SOFTWARE_PWM)
static uint8_t led_duty_cycle[SN32_RGB_MATRIX_ROWS_HW] = {0}; // track the channel duty cycle
#    endif
#endif
#if (DIODE_DIRECTION == ROW2COL)
static matrix_row_t row_shifter = MATRIX_ROW_SHIFTER;
#endif
#if defined(SHARED_MATRIX)
extern matrix_row_t   raw_matrix[MATRIX_ROWS];                       // raw values
extern matrix_row_t   matrix[MATRIX_ROWS];                           // debounced values
static matrix_row_t   shared_matrix[MATRIX_ROWS];                    // scan values
static volatile bool  matrix_locked                         = false; // matrix update check
static volatile bool  matrix_scanned                        = false;
#endif // SHARED MATRIX
static const uint32_t periodticks                           = RGB_MATRIX_MAXIMUM_BRIGHTNESS;
static const uint32_t freq                                  = (RGB_MATRIX_HUE_STEP * RGB_MATRIX_SAT_STEP * RGB_MATRIX_VAL_STEP * RGB_MATRIX_SPD_STEP * RGB_MATRIX_LED_PROCESS_LIMIT);
static const pin_t    led_row_pins[SN32_RGB_MATRIX_ROWS_HW] = SN32_RGB_MATRIX_ROW_PINS; // We expect a R,B,G order here
static const pin_t    led_col_pins[SN32_RGB_MATRIX_COLS]    = SN32_RGB_MATRIX_COL_PINS;
static RGB            led_state[SN32F2XX_LED_COUNT];     // led state buffer
static RGB            led_state_buf[SN32F2XX_LED_COUNT]; // led state buffer
bool                  led_state_buf_update_required = false;
#ifdef UNDERGLOW_RBG // handle underglow with flipped B,G channels
static const uint8_t underglow_leds[UNDERGLOW_LEDS] = UNDERGLOW_IDX;
#endif

#if defined(SHARED_MATRIX)
void matrix_output_unselect_delay(uint8_t line, bool key_pressed) {
    for (int i = 0; i < TIME_US2I(MATRIX_IO_DELAY); ++i) {
        __asm__ volatile("" ::: "memory");
    }
}
bool matrix_can_read(void) {
    return matrix_scanned;
}
#endif // SHARED_MATRIX

/* PWM configuration structure. We use timer CT16B1 with 24 channels. */
static PWMConfig pwmcfg = {
    freq,        /* PWM clock frequency. */
    periodticks, /* PWM period (in ticks) 1S (1/10kHz=0.1mS 0.1ms*10000 ticks=1S) */
    NULL,        /* RGB Callback */
    {
        /* Default all channels to disabled - Channels will be configured during init */
        [0 ... PWM_CHANNELS-1] = {PWM_OUTPUT_DISABLED, NULL, 0},
    },
    0 /* HW dependent part.*/
};

static void rgb_ch_ctrl(PWMConfig *cfg) {
    /* Enable PWM function, IOs and select the PWM modes for the LED pins */
#if (SN32_PWM_DIRECTION == COL2ROW)
    for (uint8_t i = 0; i < SN32_RGB_MATRIX_COLS; i++) {
#    if (SN32_PWM_CONTROL == HARDWARE_PWM)
        // Only P0.0 to P2.15 can be used as pwm output
        if (led_col_pins[i] > C15) continue;
#    endif // SN32_PWM_CONTROL
        /* We use a tricky here, accordint to pfpa table of sn32f240b datasheet,
           pwm channel and pfpa of pin Px.y can be calculated as below:
             channel = (x*16+y)%24
             pfpa = 1, when (x*16+y)>23
        */
        uint8_t pio_value = ((uint32_t)(PAL_PORT(led_col_pins[i])) - (uint32_t)(PAL_PORT(A0))) / ((uint32_t)(PAL_PORT(B0)) - (uint32_t)(PAL_PORT(A0))) * PAL_IOPORTS_WIDTH + PAL_PAD(led_col_pins[i]);
        uint8_t ch_idx    = pio_value % PWM_CHANNELS;
        chan_col_order[i] = ch_idx;
#elif (SN32_PWM_DIRECTION == ROW2COL)
    for (uint8_t i = 0; i < SN32_RGB_MATRIX_ROWS_HW; i++) {
#    if (SN32_PWM_CONTROL == HARDWARE_PWM)
        // Only P0.0 to P2.15 can be used as pwm output
        if (led_row_pins[i] > C15) continue;
#    endif // SN32_PWM_CONTROL
        /* We use a tricky here, accordint to pfpa table of sn32f240b datasheet,
           pwm channel and pfpa of pin Px.y can be calculated as below:
             channel = (x*16+y)%24
             pfpa = 1, when (x*16+y)>23
        */
        uint8_t pio_value = ((uint32_t)(PAL_PORT(led_row_pins[i])) - (uint32_t)(PAL_PORT(A0))) / ((uint32_t)(PAL_PORT(B0)) - (uint32_t)(PAL_PORT(A0))) * PAL_IOPORTS_WIDTH + PAL_PAD(led_row_pins[i]);
        uint8_t ch_idx    = pio_value % PWM_CHANNELS;
        chan_row_order[i] = ch_idx;
#endif
#if (SN32_PWM_CONTROL == HARDWARE_PWM)
        cfg->channels[ch_idx].pfpamsk = pio_value > (PWM_CHANNELS -1);
        cfg->channels[ch_idx].mode    = SN32_PWM_OUTPUT_ACTIVE_LEVEL;
#endif // SN32_PWM_CONTROL
    }
}
static void rgb_callback(PWMDriver *pwmp);

static void shared_matrix_rgb_enable(void) {
    pwmcfg.callback = rgb_callback;
    pwmEnablePeriodicNotification(&PWMD1);
}

#if defined(SHARED_MATRIX)
static void shared_matrix_scan_keys(matrix_row_t current_matrix[], uint8_t current_key, uint8_t last_key) {
    // Scan the key matrix row or col, depending on DIODE_DIRECTION
    static uint8_t first_scanned;
    if (!matrix_scanned) {
        if (!matrix_locked) {
            matrix_locked = true;
            first_scanned = current_key;
        } else {
            if ((last_key != current_key) && (current_key == first_scanned)) {
                matrix_locked = false;
            }
        }
        if (matrix_locked) {
#if (DIODE_DIRECTION == COL2ROW)
#    if (SN32_PWM_DIRECTION == DIODE_DIRECTION)
            matrix_read_cols_on_row(current_matrix, current_key);
#    else
            // For each row...
            for (uint8_t row_index = 0; row_index < ROWS_PER_HAND; row_index++) {
                matrix_read_cols_on_row(current_matrix, row_index);
            }

#    endif // DIODE_DIRECTION == SN32_PWM_DIRECTION
#elif (DIODE_DIRECTION == ROW2COL)
#    if (SN32_PWM_DIRECTION == DIODE_DIRECTION)
            matrix_read_rows_on_col(current_matrix, current_key, row_shifter);
#    else
            // For each col...
            matrix_row_t row_shifter = MATRIX_ROW_SHIFTER;
            for (uint8_t col_index = 0; col_index < MATRIX_COLS; col_index++, row_shifter <<= 1) {
                matrix_read_rows_on_col(current_matrix, current_key, row_shifter);
            }
#    endif // SN32_PWM_DIRECTION
#endif     // DIODE_DIRECTION
            matrix_scanned = true;
        }
    }
}
#endif // SHARED_MATRIX

#if (SN32_PWM_DIRECTION == COL2ROW)

static void shared_matrix_rgb_disable_output(void) {
    // Disable PWM outputs on column pins
    for (uint8_t y = 0; y < SN32_RGB_MATRIX_COLS; y++) {
#    if (SN32_PWM_CONTROL == HARDWARE_PWM)
        pwmDisableChannel(&PWMD1, chan_col_order[y]);
#    elif (SN32_PWM_CONTROL == SOFTWARE_PWM)
        setPinInput(led_col_pins[y]);
#    endif // SN32_PWM_CONTROL
    }
    // Disable LED outputs on RGB channel pins
    for (uint8_t x = 0; x < SN32_RGB_MATRIX_ROWS_HW; x++) {
#    if (SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_HIGH)
        writePinLow(led_row_pins[x]);
#    elif (SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_LOW)
        writePinHigh(led_row_pins[x]);
#    endif // SN32_RGB_OUTPUT_ACTIVE_LEVEL
    }
}

static void update_pwm_channels(PWMDriver *pwmp) {
    // Advance to the next LED RGB channels
    current_row++;
    /* Check if counter has wrapped around, reset before the next pass */
    if (current_row == SN32_RGB_MATRIX_ROWS_HW) current_row = 0;
#   if defined(SHARED_MATRIX)
    uint8_t last_key_row = current_key_row;
#   endif // SHARED_MATRIX
    // Advance to the next key matrix row
#    if (SN32_PWM_CONTROL == HARDWARE_PWM)
    if (current_row % SN32_RGB_MATRIX_ROW_CHANNELS == 2) current_key_row++;
#    elif (SN32_PWM_CONTROL == SOFTWARE_PWM)
    if (current_row % SN32_RGB_MATRIX_ROW_CHANNELS == 0) current_key_row++;
#    endif // SN32_PWM_CONTROL
    /* Check if counter has wrapped around, reset before the next pass */
    if (current_key_row == SN32_RGB_MATRIX_ROWS) current_key_row = 0;
    // Disable LED output before scanning the key matrix
    if (current_key_row < ROWS_PER_HAND) {
        shared_matrix_rgb_disable_output();
#   if defined(SHARED_MATRIX)
        shared_matrix_scan_keys(shared_matrix, current_key_row, last_key_row);
#   endif // SHARED_MATRIX
    }
    bool enable_pwm_output = false;
    for (uint8_t current_key_col = 0; current_key_col < SN32_RGB_MATRIX_COLS; current_key_col++) {
        uint8_t led_index = g_led_config.matrix_co[current_key_row][current_key_col];
#    if (SN32_PWM_CONTROL == SOFTWARE_PWM)
        if (led_index >= SN32F2XX_LED_COUNT) continue;
#    endif // SN32_PWM_CONTROL
        // Check if we need to enable RGB output
        if (led_state[led_index].b > 0) enable_pwm_output |= true;
        if (led_state[led_index].g > 0) enable_pwm_output |= true;
        if (led_state[led_index].r > 0) enable_pwm_output |= true;
            // Update matching RGB channel PWM configuration
#    if (SN32_PWM_CONTROL == HARDWARE_PWM)
        switch (current_row % SN32_RGB_MATRIX_ROW_CHANNELS) {
            case 0:
                pwmEnableChannel(pwmp, chan_col_order[current_key_col], led_state[led_index].b);
                break;
            case 1:
                pwmEnableChannel(pwmp, chan_col_order[current_key_col], led_state[led_index].g);
                break;
            case 2:
                pwmEnableChannel(pwmp, chan_col_order[current_key_col], led_state[led_index].r);
                break;
            default:;
        }
#    elif (SN32_PWM_CONTROL == SOFTWARE_PWM)
        switch (current_row % SN32_RGB_MATRIX_ROW_CHANNELS) {
            case 0:
                led_duty_cycle[current_key_col] = led_state[led_index].r;
#        if defined(EVISION_BOTCHED_RED_CHANNEL) // some keyboards have a 151k resistor value tied to the R channel instead of a 10k, as the rest.
                /* Boost the output for that channel maximizing the current draw by disabling other sinks */
#            if (SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_HIGH)
                writePinLow(led_row_pins[current_row + 1]);
                writePinLow(led_row_pins[current_row + 2]);
#            elif (SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_LOW)
                writePinHigh(led_row_pins[current_row + 1]);
                writePinHigh(led_row_pins[current_row + 2]);
#            endif // SN32_RGB_OUTPUT_ACTIVE_LEVEL
#        endif     // EVISION_BOTCHED_RED_CHANNEL
                break;
            case 1:
                led_duty_cycle[current_key_col] = led_state[led_index].b;
                break;
            case 2:
                led_duty_cycle[current_key_col] = led_state[led_index].g;
                break;
            default:;
        }
#    endif
    }
    // Enable RGB output
    if (enable_pwm_output) {
#    if (SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_HIGH)
        writePinHigh(led_row_pins[current_row]);
#    elif (SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_LOW)
        writePinLow(led_row_pins[current_row]);
#    endif
    }
}
#elif (SN32_PWM_DIRECTION == ROW2COL)

static void shared_matrix_rgb_disable_output(void) {
    // Disable LED outputs on RGB channel pins
    for (uint8_t x = 0; x < SN32_RGB_MATRIX_COLS; x++) {
#    if (DIODE_DIRECTION != SN32_PWM_DIRECTION)
        setPinInput(led_col_pins[x]);
#    endif // DIODE_DIRECTION != SN32_PWM_DIRECTION
        // Unselect all columns before scanning the key matrix
#    if (SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_LOW || defined(MATRIX_UNSELECT_DRIVE_HIGH))
        writePinHigh(led_col_pins[x]);
#    elif (SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_HIGH)
        writePinLow(led_col_pins[x]);
#    endif
    }
#    if (DIODE_DIRECTION != SN32_PWM_DIRECTION)
    // Disable PWM outputs on row pins
    for (uint8_t x = 0; x < SN32_RGB_MATRIX_ROWS_HW; x++) {
#        if (SN32_PWM_CONTROL == HARDWARE_PWM)
        pwmDisableChannel(&PWMD1, chan_row_order[x]);
#        elif (SN32_PWM_CONTROL == SOFTWARE_PWM)
        setPinInput(led_row_pins[x]);
#        endif // SN32_PWM_CONTROL
    }
#    endif     // DIODE_DIRECTION != SN32_PWM_DIRECTION
}

static void update_pwm_channels(PWMDriver *pwmp) {
    /* Advance to the next LED RGB channel and get ready for the next pass */
    last_key_col = current_key_col;
    current_key_col++;
    /* Check if counter has wrapped around, reset before the next pass */
#    if (DIODE_DIRECTION == ROW2COL)
    if (current_key_col < MATRIX_COLS) row_shifter <<= 1;
    if (current_key_col == MATRIX_COLS) row_shifter = MATRIX_ROW_SHIFTER;
#    endif // DIODE_DIRECTION == ROW2COL
    if (current_key_col == SN32_RGB_MATRIX_COLS) current_key_col = 0;
    // Disable LED output before scanning the key matrix
    if (current_key_col < MATRIX_COLS) {
        shared_matrix_rgb_disable_output();
#   if defined(SHARED_MATRIX)
        shared_matrix_scan_keys(shared_matrix, current_key_col, last_key_col);
#   endif // SHARED_MATRIX
    }

    for (uint8_t x = 0; x < SN32_RGB_MATRIX_COLS; x++) {
#    if (DIODE_DIRECTION != SN32_PWM_DIRECTION)
        setPinOutput(led_col_pins[x]);
#    endif // DIODE_DIRECTION != SN32_PWM_DIRECTION
        // Disable all RGB columns before turning on PWM in case matrix read unselect high
#    if (SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_HIGH && defined(MATRIX_UNSELECT_DRIVE_HIGH))
        writePinLow(led_col_pins[x]);
#    endif // SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_HIGH && defined(MATRIX_UNSELECT_DRIVE_HIGH)
    }

    bool enable_pwm_output = false;
    for (uint8_t current_key_row = 0; current_key_row < MATRIX_ROWS; current_key_row++) {
        uint8_t led_index = g_led_config.matrix_co[current_key_row][current_key_col];
#    if (SN32_PWM_CONTROL == SOFTWARE_PWM)
        if (led_index >= SN32F2XX_LED_COUNT) continue;
#    endif
        uint8_t led_row_id = (current_key_row * SN32_RGB_MATRIX_ROW_CHANNELS);
        // Check if we need to enable RGB output
        if (led_state[led_index].b > 0) enable_pwm_output |= true;
        if (led_state[led_index].g > 0) enable_pwm_output |= true;
        if (led_state[led_index].r > 0) enable_pwm_output |= true;
            // Update matching RGB channel PWM configuration
#    if (SN32_PWM_CONTROL == HARDWARE_PWM)
        pwmEnableChannelI(pwmp, chan_row_order[(led_row_id + 0)], led_state[led_index].r);
        pwmEnableChannelI(pwmp, chan_row_order[(led_row_id + 1)], led_state[led_index].b);
        pwmEnableChannelI(pwmp, chan_row_order[(led_row_id + 2)], led_state[led_index].g);
    }
    // Enable RGB output
    if (enable_pwm_output) {
#        if (SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_HIGH)
        writePinHigh(led_col_pins[last_key_col]);
#        elif (SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_LOW)
        writePinLow(led_col_pins[last_key_col]);
#        endif // SN32_RGB_OUTPUT_ACTIVE_LEVEL
    }
#    elif (SN32_PWM_CONTROL == SOFTWARE_PWM)
        led_duty_cycle[(led_row_id + 0)] = led_state[led_index].r;
        led_duty_cycle[(led_row_id + 1)] = led_state[led_index].b;
        led_duty_cycle[(led_row_id + 2)] = led_state[led_index].g;
    }
    // Enable RGB output
    if (enable_pwm_output) {
#        if (SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_HIGH)
        writePinHigh(led_col_pins[current_key_col]);
#        elif (SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_LOW)
        writePinLow(led_col_pins[current_key_col]);
#        endif // SN32_RGB_OUTPUT_ACTIVE_LEVEL
    }
#    endif     // SN32_PWM_CONTROL
}
#endif         // SN32_PWM_DIRECTION == ROW2COL

static void rgb_callback(PWMDriver *pwmp) {
    // Disable the interrupt
    pwmDisablePeriodicNotification(pwmp);
#if ((SN32_PWM_CONTROL == SOFTWARE_PWM) && (SN32_PWM_DIRECTION == COL2ROW))
    for (uint8_t pwm_cnt = 0; pwm_cnt < (SN32_RGB_MATRIX_COLS * RGB_MATRIX_HUE_STEP); pwm_cnt++) {
        uint8_t pwm_index = (pwm_cnt % SN32_RGB_MATRIX_COLS);
        if (((uint16_t)(pwmp->ct->TC) < ((uint16_t)(led_duty_cycle[pwm_index] + periodticks))) && (led_duty_cycle[pwm_index] > 0)) {
            setPinOutput(led_col_pins[pwm_index]);
#    if (SN32_PWM_OUTPUT_ACTIVE_LEVEL == SN32_PWM_OUTPUT_ACTIVE_LOW)
            writePinLow(led_col_pins[pwm_index]);
        } else {
            setPinInputHigh(led_col_pins[pwm_index]);
#    elif (SN32_PWM_OUTPUT_ACTIVE_LEVEL == SN32_PWM_OUTPUT_ACTIVE_HIGH)
            writePinHigh(led_col_pins[pwm_index]);
        } else {
            setPinInputLow(led_col_pins[pwm_index]);
#    endif // SN32_PWM_OUTPUT_ACTIVE_LEVEL
        }
    }
#elif ((SN32_PWM_CONTROL == SOFTWARE_PWM) && (SN32_PWM_DIRECTION == ROW2COL))
    for (uint8_t pwm_cnt = 0; pwm_cnt < (SN32_RGB_MATRIX_ROWS_HW * RGB_MATRIX_HUE_STEP); pwm_cnt++) {
        uint8_t pwm_index = (pwm_cnt % SN32_RGB_MATRIX_ROWS_HW);
        if (((uint16_t)(pwmp->ct->TC) < ((uint16_t)(led_duty_cycle[pwm_index] + periodticks))) && (led_duty_cycle[pwm_index] > 0)) {
#    if (DIODE_DIRECTION != SN32_PWM_DIRECTION)
            setPinOutput(led_row_pins[pwm_index]);
#    endif // DIODE_DIRECTION != SN32_PWM_DIRECTION

#    if (SN32_PWM_OUTPUT_ACTIVE_LEVEL == SN32_PWM_OUTPUT_ACTIVE_LOW)
            writePinLow(led_row_pins[pwm_index]);
        } else {
            writePinHigh(led_row_pins[pwm_index]);
#    elif (SN32_PWM_OUTPUT_ACTIVE_LEVEL == SN32_PWM_OUTPUT_ACTIVE_HIGH)
            writePinHigh(led_row_pins[pwm_index]);
        } else {
            writePinLow(led_row_pins[pwm_index]);
#    endif // SN32_PWM_OUTPUT_ACTIVE_LEVEL
        }
    }
#endif
    // Scan the rgb and key matrix
    update_pwm_channels(pwmp);
    chSysLockFromISR();
    // Advance the timer to just before the wrap-around, that will start a new PWM cycle
    pwm_lld_change_counter(pwmp, 0xFFFF);
    // Enable the interrupt
    pwmEnablePeriodicNotificationI(pwmp);
    chSysUnlockFromISR();
}

void sn32f2xx_init(void) {
    for (uint8_t x = 0; x < SN32_RGB_MATRIX_ROWS_HW; x++) {
        setPinOutput(led_row_pins[x]);
#    if (SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_HIGH)
        writePinLow(led_row_pins[x]);
#    elif (SN32_RGB_OUTPUT_ACTIVE_LEVEL == SN32_RGB_OUTPUT_ACTIVE_LOW)
        writePinHigh(led_row_pins[x]);
#    endif // SN32_RGB_OUTPUT_ACTIVE_LEVEL
    }
    // Determine which PWM channels we need to control
    rgb_ch_ctrl(&pwmcfg);

#   if defined(SHARED_MATRIX)
    // initialize matrix state: all keys off
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        shared_matrix[i] = 0;
    }
#   endif // SHARED_MATRIX

    pwmStart(&PWMD1, &pwmcfg);
    shared_matrix_rgb_enable();
}

void sn32f2xx_flush(void) {
    if (led_state_buf_update_required) {
        memcpy(led_state, led_state_buf, sizeof(RGB) * SN32F2XX_LED_COUNT);
        led_state_buf_update_required = false;
    }
}

void sn32f2xx_set_color(int index, uint8_t r, uint8_t g, uint8_t b) {
#ifdef UNDERGLOW_RBG
    bool flip_gb = false;
    for (uint8_t led_id = 0; led_id < UNDERGLOW_LEDS; led_id++) {
        if (underglow_leds[led_id] == index) {
            flip_gb = true;
        }
    }
    if (flip_gb) {
        if (led_state_buf[index].r == r && led_state_buf[index].b == g && led_state_buf[index].g == b) {
            return;
        }

        led_state_buf[index].r        = r;
        led_state_buf[index].b        = g;
        led_state_buf[index].g        = b;
        led_state_buf_update_required = true;
    } else {
#endif // UNDERGLOW_RBG
        if (led_state_buf[index].r == r && led_state_buf[index].b == b && led_state_buf[index].g == g) {
            return;
        }

        led_state_buf[index].r        = r;
        led_state_buf[index].b        = b;
        led_state_buf[index].g        = g;
        led_state_buf_update_required = true;
#ifdef UNDERGLOW_RBG
    }
#endif // UNDERGLOW_RBG
}

void sn32f2xx_set_color_all(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < SN32F2XX_LED_COUNT; i++) {
        sn32f2xx_set_color(i, r, g, b);
    }
}

#if defined(SHARED_MATRIX)
bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    if (!matrix_scanned) return false; // Nothing to process until we have the matrix scanned

    bool changed = memcmp(raw_matrix, shared_matrix, sizeof(shared_matrix)) != 0;
    if (changed) memcpy(raw_matrix, shared_matrix, sizeof(shared_matrix));

    matrix_scanned = false;

    return changed;
}
#endif // SHARED_MATRIX