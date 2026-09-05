#include "settings_screen.h"
#include "./cjk_20.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static lv_obj_t *create_settings_screen(settings_screen_state_t *state);

static void copy_setting(char *destination, size_t destination_size, const char *source) {
    strncpy(destination, source, destination_size - 1);
    destination[destination_size - 1] = '\0';
}

void settings_screen_context_init(settings_screen_context_t *context) {
    if (context == NULL)
        return;

    memset(context, 0, sizeof(*context));
    context->automatic_oxygen = true;
    context->start_threshold  = 20.0f;
    context->stop_threshold   = 20.0f;
}

void settings_screen_set_save_callback(settings_screen_context_t      *context,
                                       settings_screen_save_callback_t callback,
                                       void                           *user_data) {
    if (context == NULL)
        return;

    context->save_callback  = callback;
    context->save_user_data = user_data;
}

settings_screen_state_t *settings_screen_state_create(settings_screen_context_t *context) {
    if (context == NULL)
        return NULL;

    settings_screen_state_t *state = lv_malloc(sizeof(*state));
    if (state == NULL)
        return NULL;

    memset(state, 0, sizeof(*state));
    state->context = context;
    state->screen  = create_settings_screen(state);
    if (state->screen == NULL) {
        lv_free(state);
        return NULL;
    }
    return state;
}

void settings_screen_state_destroy(settings_screen_state_t *state) {
    if (state != NULL) {
        if (state->screen != NULL) {
            lv_obj_del(state->screen);
        }
        lv_free(state);
    }
}

void settings_screen_state_load(settings_screen_state_t *state) {
    if (state != NULL && state->screen != NULL) {
        char                       altitude_text[32];
        char                       start_threshold_text[16];
        char                       stop_threshold_text[16];
        settings_screen_context_t *context = state->context;

        snprintf(altitude_text, sizeof(altitude_text), "%.1f", context->altitude);
        snprintf(start_threshold_text, sizeof(start_threshold_text), "%.1f", context->start_threshold);
        snprintf(stop_threshold_text, sizeof(stop_threshold_text), "%.1f", context->stop_threshold);
        lv_textarea_set_text(state->altitude, altitude_text);
        lv_obj_set_state(state->oxygen_switch, LV_STATE_CHECKED, context->automatic_oxygen);
        lv_textarea_set_text(state->start_input, start_threshold_text);
        lv_textarea_set_text(state->stop_input, stop_threshold_text);
        lv_textarea_set_text(state->mqtt, context->mqtt_server);

        lv_keyboard_set_textarea(state->keyboard, NULL);
        lv_obj_add_flag(state->keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_center(lv_obj_get_parent(lv_obj_get_parent(state->mqtt)));
        lv_scr_load(state->screen);
    }
}

static void set_text_style(lv_obj_t *obj, lv_color_t color, int32_t size) {
    lv_obj_set_style_text_color(obj, color, 0);
    lv_obj_set_style_text_font(obj, size == 22 ? &cjk_20 : &cjk_20, 0);
}

static lv_obj_t *create_numeric_input(lv_obj_t *parent, const char *value) {
    lv_obj_t *input = lv_textarea_create(parent);
    lv_textarea_set_one_line(input, true);
    lv_textarea_set_text(input, value);
    lv_textarea_set_accepted_chars(input, "0123456789.");
    lv_obj_set_width(input, LV_PCT(20));
    lv_obj_set_style_radius(input, 10, 0);
    lv_obj_set_style_bg_color(input, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(input, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(input, 2, 0);
    lv_obj_set_style_border_color(input, lv_color_hex(0xC9E7F7), 0);
    lv_obj_set_style_text_color(input, lv_color_hex(0x173454), 0);
    lv_obj_set_style_text_font(input, &cjk_20, 0);
    lv_obj_set_style_pad_left(input, 18, 0);
    lv_obj_set_style_pad_right(input, 12, 0);
    return input;
}

static void save_button_event(lv_event_t *event) {
    settings_screen_state_t   *state   = lv_event_get_user_data(event);
    settings_screen_context_t *context = state->context;

    context->altitude         = strtof(lv_textarea_get_text(state->altitude), NULL);
    context->automatic_oxygen = lv_obj_has_state(state->oxygen_switch, LV_STATE_CHECKED);
    context->start_threshold  = strtof(lv_textarea_get_text(state->start_input), NULL);
    context->stop_threshold   = strtof(lv_textarea_get_text(state->stop_input), NULL);
    copy_setting(context->mqtt_server, sizeof(context->mqtt_server), lv_textarea_get_text(state->mqtt));

    if (context->save_callback != NULL) {
        context->save_callback(context, context->save_user_data);
    }
}

static void numeric_input_event(lv_event_t *event) {
    lv_obj_t       *input    = lv_event_get_target(event);
    lv_obj_t       *keyboard = lv_event_get_user_data(event);
    lv_event_code_t code     = lv_event_get_code(event);

    if (code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED) {
        lv_keyboard_set_textarea(keyboard, input);
        lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_NUMBER);
        lv_obj_center(lv_obj_get_parent(lv_obj_get_parent(input)));
        lv_obj_remove_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    } else if (code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(keyboard, NULL);
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_center(lv_obj_get_parent(lv_obj_get_parent(input)));
    }
}

static void mqtt_event(lv_event_t *event) {
    lv_obj_t       *mqtt     = lv_event_get_target(event);
    lv_obj_t       *keyboard = lv_event_get_user_data(event);
    lv_event_code_t code     = lv_event_get_code(event);

    if (code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED) {
        lv_keyboard_set_textarea(keyboard, mqtt);
        lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
        lv_obj_set_y(lv_obj_get_parent(lv_obj_get_parent(mqtt)), -50);
        lv_obj_remove_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    } else if (code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(keyboard, NULL);
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_center(lv_obj_get_parent(lv_obj_get_parent(mqtt)));
    }
}

static void keyboard_event(lv_event_t *event) {
    lv_obj_t *keyboard = lv_event_get_target(event);
    lv_obj_t *textarea = lv_keyboard_get_textarea(keyboard);

    if (lv_event_get_code(event) == LV_EVENT_READY || lv_event_get_code(event) == LV_EVENT_CANCEL) {
        lv_keyboard_set_textarea(keyboard, NULL);
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        if (textarea != NULL) {
            lv_obj_center(lv_obj_get_parent(lv_obj_get_parent(textarea)));
        }
    }
}

static lv_obj_t *create_div(lv_obj_t *parent, int32_t height) {
    lv_obj_t *div = lv_obj_create(parent);
    lv_obj_set_style_bg_opa(div, LV_OPA_TRANSP, 0);
    lv_obj_set_width(div, LV_PCT(100));
    lv_obj_set_height(div, height);
    lv_obj_set_style_pad_all(div, 0, 0);
    lv_obj_clear_flag(div, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(div, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(div, 0, 0);
    lv_obj_set_flex_align(div, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    return div;
}

static lv_obj_t *create_threshold_group(lv_obj_t *parent, const char *label_text, const char *value,
                                        lv_obj_t *keyboard) {
    lv_obj_t *group = create_div(parent, 60);
    lv_obj_set_width(group, LV_PCT(50));

    lv_obj_t *label = lv_label_create(group);
    lv_label_set_text(label, label_text);
    set_text_style(label, lv_color_hex(0x2586B0), 22);

    lv_obj_t *input = create_numeric_input(group, value);
    lv_obj_set_width(input, 72);
    lv_obj_add_event_cb(input, numeric_input_event, LV_EVENT_ALL, keyboard);

    lv_obj_t *percent = lv_label_create(group);
    lv_label_set_text(percent, "%");
    lv_obj_set_style_text_color(percent, lv_color_hex(0x2586B0), 0);
    lv_obj_set_style_text_font(percent, &cjk_20, 0);

    return input;
}

static lv_obj_t *create_settings_screen(settings_screen_state_t *state) {
    if (state == NULL || state->context == NULL)
        return NULL;

    settings_screen_context_t *context = state->context;

    char altitude_text[32];
    char start_threshold_text[16];
    char stop_threshold_text[16];
    snprintf(altitude_text, sizeof(altitude_text), "%.1f", context->altitude);
    snprintf(start_threshold_text, sizeof(start_threshold_text), "%.1f", context->start_threshold);
    snprintf(stop_threshold_text, sizeof(stop_threshold_text), "%.1f", context->stop_threshold);

    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xF2F8FA), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);

    lv_obj_t *panel = lv_obj_create(screen);
    lv_obj_set_size(panel, 440, 440);
    lv_obj_center(panel);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(panel, 40, 0);
    lv_obj_set_style_bg_color(panel, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0xE3ECEF), 0);
    lv_obj_set_style_shadow_width(panel, 18, 0);
    lv_obj_set_style_shadow_opa(panel, LV_OPA_20, 0);
    lv_obj_set_style_shadow_color(panel, lv_color_hex(0x8EA8B0), 0);
    lv_obj_set_style_pad_all(panel, 24, 0);

    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t *title = lv_label_create(panel);
    lv_label_set_text(title, "参数配置");
    lv_obj_set_width(title, LV_PCT(100));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x173454), 0);
    lv_obj_set_style_text_font(title, &cjk_20, 0);

    lv_obj_t *altitude_div   = create_div(panel, 60);
    lv_obj_t *altitude_label = lv_label_create(altitude_div);
    lv_label_set_text(altitude_label, "1、当前海拔");
    set_text_style(altitude_label, lv_color_hex(0x173454), 24);

    lv_obj_t *altitude = lv_textarea_create(altitude_div);
    lv_textarea_set_one_line(altitude, true);
    lv_textarea_set_text(altitude, altitude_text);
    lv_obj_set_width(altitude, LV_PCT(20));
    lv_obj_set_style_radius(altitude, 5, 0);
    lv_obj_set_style_border_width(altitude, 2, 0);
    lv_obj_set_style_border_color(altitude, lv_color_hex(0xC9E7F7), 0);
    lv_obj_set_style_text_color(altitude, lv_color_hex(0x173454), 0);
    lv_obj_set_style_text_font(altitude, &cjk_20, 0);
    lv_obj_set_style_pad_left(altitude, 15, 0);
    lv_textarea_set_accepted_chars(altitude, "0123456789.");

    lv_obj_t *keyboard = lv_keyboard_create(screen);
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_NUMBER);
    lv_keyboard_set_textarea(keyboard, NULL);
    lv_obj_set_size(keyboard, 440, 170);
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(altitude, numeric_input_event, LV_EVENT_ALL, keyboard);
    lv_obj_add_event_cb(keyboard, keyboard_event, LV_EVENT_ALL, NULL);

    lv_obj_t *meter = lv_label_create(altitude_div);
    lv_label_set_text(meter, "米");
    set_text_style(meter, lv_color_hex(0x2586B0), 22);

    lv_obj_t *oxygen_div   = create_div(panel, 40);
    lv_obj_t *oxygen_label = lv_label_create(oxygen_div);
    lv_label_set_text(oxygen_label, "2、自动补氧功能");
    set_text_style(oxygen_label, lv_color_hex(0x173454), 22);

    lv_obj_t *oxygen_switch = lv_switch_create(oxygen_div);
    if (context->automatic_oxygen) {
        lv_obj_add_state(oxygen_switch, LV_STATE_CHECKED);
    }
    lv_obj_set_style_bg_color(oxygen_switch, lv_color_hex(0x666666), LV_PART_KNOB);
    lv_obj_set_style_pad_top(oxygen_switch, -2, LV_PART_KNOB);
    lv_obj_set_style_pad_bottom(oxygen_switch, -2, LV_PART_KNOB);
    lv_obj_set_style_radius(oxygen_switch, LV_RADIUS_CIRCLE, LV_PART_KNOB);

    lv_obj_t *threshold_div = create_div(panel, 60);
    lv_obj_set_flex_flow(threshold_div, LV_FLEX_FLOW_ROW);
    lv_obj_t *start_input = create_threshold_group(threshold_div, "启动阈值", start_threshold_text, keyboard);
    lv_obj_t *stop_input  = create_threshold_group(threshold_div, "停止阈值", stop_threshold_text, keyboard);

    lv_obj_t *mqtt_label_div = create_div(panel, 30);
    lv_obj_t *mqtt_label     = lv_label_create(mqtt_label_div);
    lv_label_set_text(mqtt_label, "3、MQTT 服务器");
    set_text_style(mqtt_label, lv_color_hex(0x173454), 22);

    lv_obj_t *mqtt_url_div = create_div(panel, 60);
    lv_obj_t *mqtt         = lv_textarea_create(mqtt_url_div);
    lv_textarea_set_one_line(mqtt, true);
    lv_textarea_set_text(mqtt, context->mqtt_server);
    lv_textarea_set_placeholder_text(mqtt, "请输入服务器地址");
    lv_obj_set_width(mqtt, LV_PCT(80));
    lv_obj_set_style_radius(mqtt, 5, 0);
    lv_obj_set_style_border_width(mqtt, 2, 0);
    lv_obj_set_style_border_color(mqtt, lv_color_hex(0xC9E7F7), 0);
    lv_obj_set_style_text_color(mqtt, lv_color_hex(0x173454), 0);
    lv_obj_set_style_text_font(mqtt, &cjk_20, 0);
    lv_obj_set_style_text_color(mqtt, lv_color_hex(0xA9C4D4), LV_PART_TEXTAREA_PLACEHOLDER);
    lv_obj_add_event_cb(mqtt, mqtt_event, LV_EVENT_ALL, keyboard);

    lv_obj_t *save_div = create_div(panel, 60);
    lv_obj_set_flex_align(save_div, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t *save = lv_btn_create(save_div);
    lv_obj_set_size(save, 80, 40);
    lv_obj_set_style_radius(save, 5, 0);
    lv_obj_set_style_bg_color(save, lv_color_hex(0x459BDA), 0);
    lv_obj_set_style_bg_grad_color(save, lv_color_hex(0x58C2A2), 0);
    lv_obj_set_style_bg_grad_dir(save, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_shadow_width(save, 8, 0);
    lv_obj_set_style_shadow_opa(save, LV_OPA_20, 0);
    lv_obj_add_event_cb(save, save_button_event, LV_EVENT_CLICKED, state);

    lv_obj_t *save_text = lv_label_create(save);
    lv_label_set_text(save_text, "保存");
    lv_obj_center(save_text);
    lv_obj_set_style_text_color(save_text, lv_color_white(), 0);
    lv_obj_set_style_text_font(save_text, &cjk_20, 0);

    state->altitude      = altitude;
    state->keyboard      = keyboard;
    state->oxygen_switch = oxygen_switch;
    state->start_input   = start_input;
    state->stop_input    = stop_input;
    state->mqtt          = mqtt;
    lv_obj_set_user_data(screen, state);

    state->screen = screen;
    return screen;
}
