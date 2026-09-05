#ifndef NETWORK_SETTINGS_SCREEN_H
#define NETWORK_SETTINGS_SCREEN_H

#include "lvgl/lvgl.h"

#define NETWORK_SETTINGS_SSID_MAX_LEN     64
#define NETWORK_SETTINGS_PASSWORD_MAX_LEN 64
#define NETWORK_SETTINGS_MAX_SSIDS        16

typedef struct network_settings_context network_settings_context_t;
typedef struct network_settings_state   network_settings_state_t;

typedef void (*network_settings_scan_callback_t)(network_settings_context_t *context, void *user_data);
typedef void (*network_settings_connect_callback_t)(const network_settings_context_t *context, void *user_data);

struct network_settings_context {
    char                                ssid[NETWORK_SETTINGS_SSID_MAX_LEN];
    char                                password[NETWORK_SETTINGS_PASSWORD_MAX_LEN];
    char                                ssid_list[NETWORK_SETTINGS_MAX_SSIDS][NETWORK_SETTINGS_SSID_MAX_LEN];
    size_t                              ssid_count;
    network_settings_scan_callback_t    scan_callback;
    network_settings_connect_callback_t connect_callback;
    void                               *user_data;
};

void network_settings_context_init(network_settings_context_t *context);

void network_settings_set_ssid_list(network_settings_context_t *context,
                                     const char *const ssids[],
                                     size_t count);

void network_settings_set_callbacks(network_settings_context_t         *context,
                                    network_settings_scan_callback_t    scan_callback,
                                    network_settings_connect_callback_t connect_callback,
                                    void                               *user_data);

network_settings_state_t *network_settings_state_create(network_settings_context_t *context);
/* Synchronizes the context and makes the network settings screen active. */
void network_settings_state_load(network_settings_state_t *state);
void network_settings_state_destroy(network_settings_state_t *state);

#endif
