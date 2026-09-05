#include "main_screen.h"
#include "images.h"
#include <stdio.h>
#include <string.h>

#define MAIN_SCREEN_LONG_PRESS_REPEAT_COUNT 26

extern lv_font_t cjk_20;

struct main_screen_state {
    lv_obj_t *screen;
    lv_obj_t *mode_image;
    lv_obj_t *network_image;
    lv_obj_t *settings_button;
    lv_obj_t *altitude_value;
    lv_obj_t *humidity_value;
    lv_obj_t *temperature_value;
    lv_obj_t *carbon_dioxide_value;
    lv_obj_t *oxygen_value;
    main_screen_mode_t mode;
    bool network_connected;
    bool settings_long_pressed;
    uint16_t settings_repeat_count;
    main_screen_mode_callback_t mode_callback;
    void *mode_callback_user_data;
    main_screen_settings_callback_t settings_click_callback;
    main_screen_settings_callback_t settings_long_press_callback;
    void *settings_callback_user_data;
};

static void mode_image_event(lv_event_t *event) {
    main_screen_state_t *state = lv_event_get_user_data(event);
    main_screen_set_mode(state, state->mode == MAIN_SCREEN_MODE_WORK
                                  ? MAIN_SCREEN_MODE_STOP
                                  : MAIN_SCREEN_MODE_WORK);
}

static void settings_button_event(lv_event_t *event) {
    main_screen_state_t *state = lv_event_get_user_data(event);
    lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_PRESSED) {
        state->settings_long_pressed = false;
        state->settings_repeat_count = 0;
    }
    else if(code == LV_EVENT_LONG_PRESSED_REPEAT) {
        if(state->settings_repeat_count < MAIN_SCREEN_LONG_PRESS_REPEAT_COUNT) {
            state->settings_repeat_count++;
        }
        if(state->settings_repeat_count >= MAIN_SCREEN_LONG_PRESS_REPEAT_COUNT &&
           !state->settings_long_pressed) {
            state->settings_long_pressed = true;
            if(state->settings_long_press_callback != NULL) {
                state->settings_long_press_callback(state, state->settings_callback_user_data);
            }
        }
    }
    else if(code == LV_EVENT_CLICKED) {
        if(state->settings_long_pressed) {
            state->settings_long_pressed = false;
            state->settings_repeat_count = 0;
            return;
        }
        if(state->settings_click_callback != NULL) {
            state->settings_click_callback(state, state->settings_callback_user_data);
        }
        state->settings_repeat_count = 0;
    }
}

static lv_obj_t *create_data_value(lv_obj_t *parent, int32_t x, int32_t y) {
    lv_obj_t *value_label = lv_label_create(parent);
    lv_label_set_text(value_label, "--");
    lv_obj_set_size(value_label, 150, 42);
    lv_obj_set_pos(value_label, x, y);
    lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(value_label, lv_color_hex(0x00B89C), 0);
    lv_obj_set_style_text_font(value_label, &cjk_20, 0);
    return value_label;
}

main_screen_state_t *main_screen_create(void) {
    main_screen_state_t *state = lv_malloc_zeroed(sizeof(*state));
    if(state == NULL) return NULL;

    state->mode = MAIN_SCREEN_MODE_STOP;
    state->screen = lv_obj_create(NULL);
    if(state->screen == NULL) {
        lv_free(state);
        return NULL;
    }

    lv_obj_clear_flag(state->screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(state->screen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(state->screen, lv_color_black(), 0);
    lv_obj_set_style_pad_all(state->screen, 0, 0);

    lv_obj_t *background_image = lv_image_create(state->screen);
    lv_image_set_src(background_image, &background);
    lv_obj_set_size(background_image, 480, 480);
    lv_obj_align(background_image, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(background_image, LV_OBJ_FLAG_CLICKABLE);

    state->altitude_value = create_data_value(state->screen, 20, 142);
    state->humidity_value = create_data_value(state->screen, 350, 142);
    state->temperature_value = create_data_value(state->screen, 20, 268);
    state->carbon_dioxide_value = create_data_value(state->screen, 350, 268);
    state->oxygen_value = create_data_value(state->screen, 160, 187);

    state->settings_button = lv_btn_create(state->screen);
    lv_obj_set_size(state->settings_button, 180, 80);
    lv_obj_align(state->settings_button, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_opa(state->settings_button, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(state->settings_button, 0, 0);
    lv_obj_set_style_shadow_width(state->settings_button, 0, 0);
    lv_obj_add_event_cb(state->settings_button, settings_button_event, LV_EVENT_ALL, state);

    state->network_image = lv_image_create(state->screen);
    lv_image_set_src(state->network_image, &network_connect);
    lv_obj_align(state->network_image, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_add_flag(state->network_image, LV_OBJ_FLAG_HIDDEN);

    state->mode_image = lv_image_create(state->screen);
    lv_image_set_src(state->mode_image, &btn_stop);
    lv_obj_add_flag(state->mode_image, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(state->mode_image, mode_image_event, LV_EVENT_CLICKED, state);
    lv_obj_align(state->mode_image, LV_ALIGN_BOTTOM_MID, 0, 0);
    main_screen_set_mode(state, state->mode);

    return state;
}

void main_screen_load(main_screen_state_t *state) {
    if(state != NULL && state->screen != NULL) {
        lv_scr_load(state->screen);
    }
}

void main_screen_destroy(main_screen_state_t *state) {
    if(state == NULL) return;
    if(state->screen != NULL) lv_obj_del(state->screen);
    lv_free(state);
}

void main_screen_set_mode(main_screen_state_t *state, main_screen_mode_t mode) {
    if(state == NULL || state->mode_image == NULL) return;
    if(state->mode == mode) return;
    state->mode = mode;
    lv_image_set_src(state->mode_image,
                     mode == MAIN_SCREEN_MODE_WORK ? &btn_work : &btn_stop);
    if(state->mode_callback != NULL) {
        state->mode_callback(state, mode, state->mode_callback_user_data);
    }
}

main_screen_mode_t main_screen_get_mode(const main_screen_state_t *state) {
    return state != NULL ? state->mode : MAIN_SCREEN_MODE_STOP;
}

void main_screen_set_mode_callback(main_screen_state_t *state,
                                   main_screen_mode_callback_t callback,
                                   void *user_data) {
    if(state == NULL) return;
    state->mode_callback = callback;
    state->mode_callback_user_data = user_data;
}

void main_screen_set_settings_callbacks(main_screen_state_t *state,
                                        main_screen_settings_callback_t click_callback,
                                        main_screen_settings_callback_t long_press_callback,
                                        void *user_data) {
    if(state == NULL) return;
    state->settings_click_callback = click_callback;
    state->settings_long_press_callback = long_press_callback;
    state->settings_callback_user_data = user_data;
}

void main_screen_set_network_connected(main_screen_state_t *state, bool connected) {
    if(state == NULL || state->network_image == NULL) return;
    state->network_connected = connected;
    if(connected) lv_obj_remove_flag(state->network_image, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(state->network_image, LV_OBJ_FLAG_HIDDEN);
}

bool main_screen_get_network_connected(const main_screen_state_t *state) {
    return state != NULL && state->network_connected;
}

void main_screen_set_data(main_screen_state_t *state, const main_screen_data_t *data) {
    if(state == NULL || data == NULL) return;

    char value[32];
    snprintf(value, sizeof(value), "%.1fm", data->equivalent_altitude);
    lv_label_set_text(state->altitude_value, value);
    snprintf(value, sizeof(value), "%.1f%%", data->humidity);
    lv_label_set_text(state->humidity_value, value);
    snprintf(value, sizeof(value), "%.1f\xE2\x84\x83", data->temperature);
    lv_label_set_text(state->temperature_value, value);
    snprintf(value, sizeof(value), "%.0fppm", data->carbon_dioxide);
    lv_label_set_text(state->carbon_dioxide_value, value);
    snprintf(value, sizeof(value), "%.1f%%", data->oxygen);
    lv_label_set_text(state->oxygen_value, value);
}
