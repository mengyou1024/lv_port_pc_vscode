#include "network_settings_screen.h"
#include <stdio.h>
#include <string.h>

extern lv_font_t cjk_20;

struct network_settings_state {
    network_settings_context_t *context;
    lv_obj_t                   *screen;
    lv_obj_t                   *panel;
    lv_obj_t                   *ssid;
    lv_obj_t                   *password;
    lv_obj_t                   *keyboard;
};

static void copy_text(char *destination, size_t size, const char *source) {
    strncpy(destination, source, size - 1);
    destination[size - 1] = '\0';
}

void network_settings_context_init(network_settings_context_t *context) {
    if (context == NULL)
        return;
    memset(context, 0, sizeof(*context));
}

void network_settings_set_ssid_list(network_settings_context_t *context,
                                    const char *const           ssids[],
                                    size_t                      count) {
    if (context == NULL)
        return;

    context->ssid_count = 0;
    if (ssids == NULL)
        return;

    if (count > NETWORK_SETTINGS_MAX_SSIDS)
        count = NETWORK_SETTINGS_MAX_SSIDS;

    for (size_t index = 0; index < count; index++) {
        if (ssids[index] == NULL)
            continue;
        copy_text(context->ssid_list[context->ssid_count],
                  sizeof(context->ssid_list[context->ssid_count]), ssids[index]);
        context->ssid_count++;
    }
}

void network_settings_set_callbacks(network_settings_context_t         *context,
                                    network_settings_scan_callback_t    scan_callback,
                                    network_settings_connect_callback_t connect_callback,
                                    void                               *user_data) {
    if (context == NULL)
        return;
    context->scan_callback    = scan_callback;
    context->connect_callback = connect_callback;
    context->user_data        = user_data;
}

static void set_text_style(lv_obj_t *object, lv_color_t color) {
    lv_obj_set_style_text_color(object, color, 0);
    lv_obj_set_style_text_font(object, &cjk_20, 0);
}

static lv_obj_t *create_input(lv_obj_t *parent, const char *placeholder) {
    lv_obj_t *input = lv_textarea_create(parent);
    lv_textarea_set_one_line(input, true);
    lv_textarea_set_placeholder_text(input, placeholder);
    lv_obj_set_size(input, 240, 50);
    lv_obj_set_style_radius(input, 12, 0);
    lv_obj_set_style_border_width(input, 1, 0);
    lv_obj_set_style_border_color(input, lv_color_hex(0xC9E7F7), 0);
    lv_obj_set_style_text_color(input, lv_color_hex(0x173454), 0);
    lv_obj_set_style_text_font(input, &cjk_20, 0);
    lv_obj_set_style_text_color(input, lv_color_hex(0xA9C4D4), LV_PART_TEXTAREA_PLACEHOLDER);
    return input;
}

static lv_obj_t *create_ssid_dropdown(lv_obj_t *parent, const network_settings_context_t *context) {
    lv_obj_t *dropdown                                                                   = lv_dropdown_create(parent);
    char      options[(NETWORK_SETTINGS_MAX_SSIDS * NETWORK_SETTINGS_SSID_MAX_LEN) + 32] = {0};
    size_t    offset                                                                     = 0;

    for (size_t index = 0; index < context->ssid_count && offset < sizeof(options); index++) {
        offset += (size_t)snprintf(options + offset, sizeof(options) - offset,
                                   "%s%s", index == 0 ? "" : "\n", context->ssid_list[index]);
    }

    lv_dropdown_set_options(dropdown, options);
    uint32_t selected = 0;
    for (size_t index = 0; index < context->ssid_count; index++) {
        if (strcmp(context->ssid, context->ssid_list[index]) == 0) {
            selected = (uint32_t)index;
            break;
        }
    }
    lv_dropdown_set_selected(dropdown, selected);
    lv_obj_set_size(dropdown, 240, 50);
    lv_obj_set_style_radius(dropdown, 12, 0);
    lv_obj_set_style_border_width(dropdown, 1, 0);
    lv_obj_set_style_border_color(dropdown, lv_color_hex(0xC9E7F7), 0);
    lv_obj_set_style_text_color(dropdown, lv_color_hex(0x173454), 0);
    lv_obj_set_style_text_font(dropdown, &cjk_20, 0);
    lv_obj_set_style_pad_left(dropdown, 14, 0);
    lv_obj_set_style_pad_right(dropdown, 12, 0);
    lv_obj_set_style_text_font(dropdown, &lv_font_montserrat_20, LV_PART_INDICATOR);
    return dropdown;
}

static void keyboard_event(lv_event_t *event) {
    network_settings_state_t *state = lv_event_get_user_data(event);
    lv_event_code_t           code  = lv_event_get_code(event);
    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_keyboard_set_textarea(state->keyboard, NULL);
        lv_obj_add_flag(state->keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_center(state->panel);
    }
}

static void input_event(lv_event_t *event) {
    network_settings_state_t *state = lv_event_get_user_data(event);
    lv_obj_t                 *input = lv_event_get_target(event);
    lv_event_code_t           code  = lv_event_get_code(event);
    if (code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED) {
        lv_keyboard_set_textarea(state->keyboard, input);
        lv_keyboard_set_mode(state->keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
        lv_obj_set_y(state->panel, -65);
        lv_obj_remove_flag(state->keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

static void scan_event(lv_event_t *event) {
    network_settings_state_t *state = lv_event_get_user_data(event);
    if (state->context->scan_callback != NULL) {
        state->context->scan_callback(state->context, state->context->user_data);
    }
}

static void connect_event(lv_event_t *event) {
    network_settings_state_t *state = lv_event_get_user_data(event);
    char                      selected_ssid[NETWORK_SETTINGS_SSID_MAX_LEN];
    lv_dropdown_get_selected_str(state->ssid, selected_ssid, sizeof(selected_ssid));
    copy_text(state->context->ssid, sizeof(state->context->ssid), selected_ssid);
    copy_text(state->context->password, sizeof(state->context->password), lv_textarea_get_text(state->password));
    if (state->context->connect_callback != NULL) {
        state->context->connect_callback(state->context, state->context->user_data);
    }
}

static lv_obj_t *create_network_screen(network_settings_state_t *state) {
    static const int32_t grid_columns[] = {60, LV_GRID_FR(1), 58, LV_GRID_TEMPLATE_LAST};
    static const int32_t grid_rows[]    = {56, 56, LV_GRID_TEMPLATE_LAST};
    lv_obj_t            *screen         = lv_obj_create(NULL);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xF2F8FA), 0);
    lv_obj_set_style_pad_all(screen, 0, 0);

    lv_obj_t *panel = lv_obj_create(screen);
    lv_obj_set_size(panel, 440, 320);
    lv_obj_align(panel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(panel, 24, 0);
    lv_obj_set_style_bg_color(panel, lv_color_white(), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0xE3ECEF), 0);
    lv_obj_set_style_shadow_width(panel, 16, 0);
    lv_obj_set_style_shadow_opa(panel, LV_OPA_20, 0);
    lv_obj_set_style_shadow_color(panel, lv_color_hex(0x8EA8B0), 0);
    lv_obj_set_style_pad_all(panel, 20, 0);
    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    state->panel = panel;

    lv_obj_t *title = lv_label_create(panel);
    lv_label_set_text(title, "Wi-Fi 配置");
    lv_obj_set_width(title, LV_PCT(100));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    set_text_style(title, lv_color_hex(0x173454));

    lv_obj_t *fields_grid = lv_obj_create(panel);
    lv_obj_set_size(fields_grid, LV_PCT(100), 120);
    lv_obj_set_style_pad_all(fields_grid, 0, 0);
    lv_obj_set_style_border_width(fields_grid, 0, 0);
    lv_obj_set_style_bg_opa(fields_grid, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(fields_grid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(fields_grid, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_grid_dsc_array(fields_grid, grid_columns, grid_rows);
    lv_obj_set_grid_align(fields_grid, LV_GRID_ALIGN_START, LV_GRID_ALIGN_CENTER);

    lv_obj_t *ssid_label = lv_label_create(fields_grid);
    lv_label_set_text(ssid_label, "SSID");
    set_text_style(ssid_label, lv_color_hex(0x2586B0));
    lv_obj_set_grid_cell(ssid_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    state->ssid = create_ssid_dropdown(fields_grid, state->context);
    lv_obj_set_grid_cell(state->ssid, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *scan = lv_btn_create(fields_grid);
    lv_obj_set_size(scan, 58, 40);
    lv_obj_set_style_radius(scan, 12, 0);
    lv_obj_set_style_bg_color(scan, lv_color_hex(0x459BDA), 0);
    lv_obj_set_style_bg_grad_color(scan, lv_color_hex(0x58C2A2), 0);
    lv_obj_set_style_bg_grad_dir(scan, LV_GRAD_DIR_HOR, 0);
    lv_obj_add_event_cb(scan, scan_event, LV_EVENT_CLICKED, state);
    lv_obj_t *scan_text = lv_label_create(scan);
    lv_label_set_text(scan_text, "扫描");
    lv_obj_center(scan_text);
    set_text_style(scan_text, lv_color_white());
    lv_obj_set_grid_cell(scan, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *password_label = lv_label_create(fields_grid);
    lv_label_set_text(password_label, "密码");
    set_text_style(password_label, lv_color_hex(0x2586B0));
    lv_obj_set_grid_cell(password_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    state->password = create_input(fields_grid, "请输入 Wi-Fi 密码");
    lv_textarea_set_password_mode(state->password, true);
    lv_obj_add_event_cb(state->password, input_event, LV_EVENT_ALL, state);
    lv_obj_set_grid_cell(state->password, LV_GRID_ALIGN_STRETCH, 1, 2, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_t *connect_row = lv_obj_create(panel);
    lv_obj_set_size(connect_row, LV_PCT(100), 48);
    lv_obj_set_style_pad_all(connect_row, 0, 0);
    lv_obj_set_style_border_width(connect_row, 0, 0);
    lv_obj_set_style_bg_opa(connect_row, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_align(connect_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *connect = lv_btn_create(connect_row);
    lv_obj_set_size(connect, 110, 40);
    lv_obj_set_style_radius(connect, 12, 0);
    lv_obj_set_style_bg_color(connect, lv_color_hex(0x459BDA), 0);
    lv_obj_set_style_bg_grad_color(connect, lv_color_hex(0x58C2A2), 0);
    lv_obj_set_style_bg_grad_dir(connect, LV_GRAD_DIR_HOR, 0);
    lv_obj_add_event_cb(connect, connect_event, LV_EVENT_CLICKED, state);
    lv_obj_t *connect_text = lv_label_create(connect);
    lv_label_set_text(connect_text, "连接");
    lv_obj_center(connect_text);
    set_text_style(connect_text, lv_color_white());

    lv_obj_t *keyboard = lv_keyboard_create(screen);
    lv_obj_set_size(keyboard, 440, 170);
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(keyboard, keyboard_event, LV_EVENT_ALL, state);
    state->keyboard = keyboard;
    return screen;
}

network_settings_state_t *network_settings_state_create(network_settings_context_t *context) {
    if (context == NULL)
        return NULL;
    network_settings_state_t *state = lv_malloc(sizeof(*state));
    if (state == NULL)
        return NULL;
    memset(state, 0, sizeof(*state));
    state->context = context;
    state->screen  = create_network_screen(state);
    if (state->screen == NULL) {
        lv_free(state);
        return NULL;
    }
    return state;
}

void network_settings_state_load(network_settings_state_t *state) {
    if (state == NULL || state->screen == NULL)
        return;
    char     options[(NETWORK_SETTINGS_MAX_SSIDS * NETWORK_SETTINGS_SSID_MAX_LEN) + 32] = {0};
    size_t   offset                                                                     = 0;
    uint32_t selected                                                                   = 0;
    for (size_t index = 0; index < state->context->ssid_count && offset < sizeof(options); index++) {
        offset += (size_t)snprintf(options + offset, sizeof(options) - offset,
                                   "%s%s", index == 0 ? "" : "\n", state->context->ssid_list[index]);
        if (strcmp(state->context->ssid, state->context->ssid_list[index]) == 0)
            selected = (uint32_t)index;
    }
    lv_dropdown_set_options(state->ssid, options);
    lv_dropdown_set_selected(state->ssid, selected);
    lv_textarea_set_text(state->password, state->context->password);
    lv_keyboard_set_textarea(state->keyboard, NULL);
    lv_obj_add_flag(state->keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_center(state->panel);
    lv_scr_load(state->screen);
}

void network_settings_state_destroy(network_settings_state_t *state) {
    if (state == NULL)
        return;
    if (state->screen != NULL)
        lv_obj_del(state->screen);
    lv_free(state);
}
