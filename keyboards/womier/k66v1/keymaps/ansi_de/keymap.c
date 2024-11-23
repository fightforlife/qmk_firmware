// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "keymap_german.h"

// Each layer gets a name for readability, which is then used in the keymap matrix below.
// The underscores don't mean anything - you can have a layer called STUFF or any other name.
// Layer names don't all need to be of the same length, obviously, and you can also skip them
// entirely and just use numbers.
enum layer_names {
    _BASE = 0,
    _FN1 = 1,
};


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [_BASE] = LAYOUT_66_ansi(
    KC_ESC  , DE_1    , DE_2     , DE_3    , DE_4    , DE_5    , DE_6    , DE_7    , DE_8    , DE_9    , DE_0    , DE_SS   , DE_ACUT , KC_BSPC , KC_DEL   ,
    KC_TAB  , DE_Q    , DE_W     , DE_E    , DE_R    , DE_T    , DE_Z    , DE_U    , DE_I    , DE_O    , DE_P    , DE_UDIA , DE_PLUS , DE_HASH , DE_LABK  ,
    KC_CAPS , DE_A    , DE_S     , DE_D    , DE_F    , DE_G    , DE_H    , DE_J    , DE_K    , DE_L    , DE_ODIA , DE_ADIA , KC_ENT  ,
    KC_LSFT , DE_Y    , DE_X     , DE_C    , DE_V    , DE_B    , DE_N    , DE_M    , DE_COMM , DE_DOT  , DE_MINS , KC_RSFT , KC_UP   ,
    KC_LCTL , KC_LGUI , KC_LALT  ,                     KC_SPC  ,                     KC_RALT , MO(_FN1), KC_RCTL , KC_LEFT , KC_DOWN , KC_RGHT
    )       ,
  [_FN1] = LAYOUT_66_ansi(
    DE_CIRC , KC_F1   , KC_F2    , KC_F3   , KC_F4   , KC_F5   , KC_F6   , KC_F7   , KC_F8   , KC_F9   , KC_F10  , KC_F11  , KC_F12  , QK_BOOT , KC_PGUP ,
    _______ , _______ , _______  , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , QK_RBT  , KC_PGDN ,
    _______ , _______ , _______  , _______ , _______ , _______ , _______ , _______ , RGB_VAI , RGB_SPI , RGB_HUI , RGB_SAI , _______ ,
    RGB_RMOD, _______ , _______  , _______ , _______ , _______ , _______ , RGB_VAD , RGB_SPD , RGB_HUD , RGB_SAD , _______ , KC_VOLU ,
    RGB_RMOD, _______ , _______  ,                     RGB_TOG ,                     _______ , _______ , KC_MPLY , KC_MPRV , KC_VOLD , KC_MNXT
    )
};

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    if (host_keyboard_led_state().caps_lock) {
        rgb_matrix_set_color(15, RGB_WHITE);
    }
    return false;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case RGB_TOG:
            if (record->event.pressed) {
                switch (rgb_matrix_get_flags()) {
                    case LED_FLAG_ALL: {
                        rgb_matrix_set_flags(LED_FLAG_NONE);
                        rgb_matrix_set_color_all(0, 0, 0);
                    } break;
                    default: {
                        rgb_matrix_set_flags(LED_FLAG_ALL);
                    } break;
                }
            }
            return false;
    }
    return true;
}