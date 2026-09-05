#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H

#include "lvgl/lvgl.h"

typedef struct settings_screen_context settings_screen_context_t;

typedef void (*settings_screen_save_callback_t)(const settings_screen_context_t *context, void *user_data);

struct settings_screen_context {
    float                           altitude;
    bool                            automatic_oxygen;
    float                           start_threshold;
    float                           stop_threshold;
    char                            mqtt_server[128];
    settings_screen_save_callback_t save_callback;
    void                           *save_user_data;
};

typedef struct {
    settings_screen_context_t *context;
    lv_obj_t                  *screen;
    lv_obj_t                  *keyboard;
    lv_obj_t                  *altitude;
    lv_obj_t                  *oxygen_switch;
    lv_obj_t                  *start_input;
    lv_obj_t                  *stop_input;
    lv_obj_t                  *mqtt;
} settings_screen_state_t;

void settings_screen_context_init(settings_screen_context_t *context);

void settings_screen_set_save_callback(settings_screen_context_t      *context,
                                       settings_screen_save_callback_t callback,
                                       void                           *user_data);

settings_screen_state_t *settings_screen_state_create(settings_screen_context_t *context);

void settings_screen_state_destroy(settings_screen_state_t *state);

void settings_screen_state_load(settings_screen_state_t *state);

#endif
