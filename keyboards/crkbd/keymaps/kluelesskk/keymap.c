#include QMK_KEYBOARD_H
#include "bootloader.h"
#ifdef PROTOCOL_LUFA
  #include "lufa.h"
  #include "split_util.h"
#endif
#ifdef OLED_DRIVER_ENABLE
  #include "oled_driver.h"
#endif

extern uint8_t is_master;
extern rgb_config_t rgb_matrix_config;

enum custom_layers {
  _QWERTY,
  _LOWER,
  _RAISE,
  _ADJUST
};

enum custom_keycodes {
  QWERTY = SAFE_RANGE,
  RGBRST,
  RGBMOI,
  RGBMOD
};

enum macro_keycodes {
  KC_SAMPLEMACRO,
};

#define KC______ KC_TRNS
#define KC_XXXXX KC_NO
#define KC_RST   RESET
#define KC_LRST  RGBRST
#define KC_LTOG  RGB_TOG
#define KC_LHUI  RGB_HUI
#define KC_LHUD  RGB_HUD
#define KC_LSAI  RGB_SAI
#define KC_LSAD  RGB_SAD
#define KC_LVAI  RGB_VAI
#define KC_LVAD  RGB_VAD
#define KC_LMOI  RGBMOI
#define KC_LMOD  RGBMOD

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [_QWERTY] = LAYOUT( \
    KC_LALT,KC_Q,KC_W,KC_E,KC_R,KC_T,KC_Y,KC_U,KC_I,KC_O,KC_P,KC_RALT,\
    KC_RCTRL,KC_A,KC_S,KC_D,KC_F,KC_G,KC_H,KC_J,KC_K,KC_L,KC_SCOLON,KC_RCTRL,\
    KC_LSHIFT,KC_Z,KC_X,KC_C,KC_V,KC_B,KC_N,KC_M,KC_COMMA,KC_DOT,KC_SLASH,KC_RSHIFT,\
        OSL(_RAISE),KC_SPACE,LSFT_T(KC_TAB),
        RSFT_T(KC_BSPACE),KC_ENTER,OSL(_LOWER)\
  ),

  [_LOWER] = LAYOUT( \
    KC_TRANSPARENT,LSFT(KC_1),LSFT(KC_2),LSFT(KC_3),LSFT(KC_4),LSFT(KC_5),\
      KC_0,KC_1,KC_2,KC_3,LSFT(KC_RBRACKET),KC_TRANSPARENT,\
    KC_TRANSPARENT,LSFT(KC_MINUS),LSFT(KC_EQUAL),KC_LPRN,KC_RPRN,KC_LBRACKET,\
      KC_E,KC_4,KC_5,KC_6,KC_QUOTE,KC_TRANSPARENT,\
    KC_TRANSPARENT,LSFT(KC_LBRACKET),KC_AMPR,KC_MINUS,KC_EQUAL,KC_TILD,\
      KC_RBRACKET,KC_7,KC_8,KC_9,KC_ASTR,KC_TRANSPARENT,\
        MO(_ADJUST),KC_TRANSPARENT,KC_TRANSPARENT,\
        KC_TRANSPARENT,MO(_ADJUST),KC_TRANSPARENT\
  ),

  [_RAISE] = LAYOUT( \
    KC_TRANSPARENT,KC_TRANSPARENT,KC_MS_BTN2,KC_MS_UP,KC_MS_BTN1,KC_TRANSPARENT,\
      LSFT(KC_6),KC_PGUP,KC_UP,KC_PGDOWN,LSFT(KC_RBRACKET),KC_TRANSPARENT,\
    KC_TRANSPARENT,KC_MEDIA_PREV_TRACK,KC_MS_LEFT,KC_MS_DOWN,KC_MS_RIGHT,KC_MEDIA_NEXT_TRACK,\
      KC_BSLASH,KC_LEFT,KC_DOWN,KC_RIGHT,KC_QUOTE,KC_TRANSPARENT,\
    KC_TRANSPARENT,KC_TRANSPARENT,KC_MEDIA_STOP,KC_MEDIA_PLAY_PAUSE,KC_MS_BTN3,KC_TRANSPARENT,\
      KC_RBRACKET,KC_HOME,KC_PIPE,KC_END,KC_ASTR,KC_TRANSPARENT,\
        KC_TRANSPARENT,MO(_ADJUST),KC_TRANSPARENT,\
        KC_DELETE,KC_ESCAPE,MO(_ADJUST)\
  ),

  [_ADJUST] = LAYOUT_kc( \
    RST, LRST, LTOG, XXXXX, XXXXX, XXXXX, XXXXX, XXXXX, XXXXX, XXXXX, XXXXX, _____,\
    LMOI, LHUI, LSAI, LVAI, XXXXX, XXXXX, XXXXX, XXXXX, XXXXX, XXXXX, XXXXX, _____,\
    LMOD, LHUD, LSAD, LVAD, XXXXX, XXXXX, XXXXX, XXXXX, XXXXX, XXXXX, XXXXX, _____,\
        XXXXX, XXXXX, XXXXX, XXXXX, XXXXX, XXXXX
  )
};

const char *read_layer_state(void) {
  switch (biton32(layer_state)) {
    case _QWERTY:
      return PSTR("Crkbd\n");
    case _RAISE:
      return PSTR("Raise\n");
    case _LOWER:
      return PSTR("Lower\n");
    case _ADJUST:
      return PSTR("Adjst\n");
    default:
      return PSTR("Unknown\n");
  }
}

uint8_t read_modifier_state(const char** out) {
  uint8_t i = 0;
  uint8_t leds = host_keyboard_leds();
  uint8_t modifiers = get_mods() | get_oneshot_mods();

  if (leds & (1 << USB_LED_NUM_LOCK)) out[i++] = PSTR("NUM");
  if (leds & (1 << USB_LED_CAPS_LOCK)) out[i++] = PSTR("CAPS");
  if (leds & (1 << USB_LED_SCROLL_LOCK)) out[i++] = PSTR("SCRL");
  if (modifiers & MOD_MASK_CTRL) out[i++] = PSTR("CTRL");
  if (modifiers & MOD_MASK_GUI) out[i++] = PSTR("GUI");
  if (modifiers & MOD_MASK_ALT) out[i++] = PSTR("ALT");
  if (modifiers & MOD_MASK_SHIFT) out[i++] = PSTR("SHFT");
  return i;
}

const char *read_rgb_light_state(void) {
  static char rgb_light_state_str[24];
  if (rgb_matrix_config.enable) {
    snprintf(rgb_light_state_str, sizeof(rgb_light_state_str),
      "M:%3dH:%3dS:%3dV:%3d",
      rgb_matrix_config.mode,
      rgb_matrix_config.hue,
      rgb_matrix_config.sat,
      rgb_matrix_config.val);
  } else {
    strcpy(rgb_light_state_str, "M:off\n\n");
  }
  return rgb_light_state_str;
}

void matrix_init_user(void) {
    // OLED init, make sure to add #define OLED_DRIVER_ENABLE in config.h
    #ifdef OLED_DRIVER_ENABLE
        oled_init(OLED_ROTATION_270);   // turns on the display
    #endif
}

// OLED update loop, make sure to add #define OLED_DRIVER_ENABLE in config.h
#ifdef OLED_DRIVER_ENABLE
void matrix_scan_user(void) {
   oled_task();
}

void matrix_render_user(void) {
  if (is_master) {
    const char* mods[7];
    uint8_t mods_count = read_modifier_state(mods);
    oled_write_ln_P(read_layer_state(), false);
    for (uint8_t i = 0; i < 7; ++i)
      oled_write_ln_P(i < mods_count ? mods[i] : PSTR(""), true);
    oled_write_ln(read_rgb_light_state(), false);
  } else {
    //oled_write(matrixead_logo(), false);
  }
}

void oled_task_user(void) {
  matrix_render_user();
}
#endif//OLED_DRIVER_ENABLE

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case RGBMOI:
        if (record->event.pressed) {
          rgblight_step();
        }
        return false;
    case RGBMOD:
        if (record->event.pressed) {
          rgblight_step_reverse();
        }
        return false;
    case RGBRST:
        if (record->event.pressed) {
          rgblight_mode(1);
          rgblight_sethsv(0, 255, RGBLIGHT_LIMIT_VAL);
          rgblight_enable();
        }
        return false;
  }
  return true;
}
