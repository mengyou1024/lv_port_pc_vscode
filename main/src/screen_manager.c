#include "screen_manager.h"
#include <stdio.h>
#include <string.h>

struct screen_manager {
    main_screen_state_t *main_state;
    settings_screen_context_t settings_context;
    settings_screen_state_t *settings_state;
    network_settings_context_t network_context;
    network_settings_state_t *network_state;
};

static const char *const default_ssids[] = {
    "HomeNetwork_2G",
    "HomeNetwork_5G",
    "OfficeNet_Main",
    "OfficeNet_Guest",
    "LabRouter_A",
    "LabRouter_B",
    "IoT_Test_Net",
    "Workshop_WiFi",
    "MeetingRoom_AP",
    "BackupNetwork"
};

static void open_settings(screen_manager_t *manager) {
    settings_screen_state_load(manager->settings_state);
}

static void open_network_settings(screen_manager_t *manager) {
    network_settings_state_load(manager->network_state);
}

static void main_settings_clicked(main_screen_state_t *state, void *user_data) {
    (void)state;
    open_settings(user_data);
}

static void main_settings_long_pressed(main_screen_state_t *state, void *user_data) {
    (void)state;
    open_network_settings(user_data);
}

static void settings_saved(const settings_screen_context_t *context, void *user_data) {
    (void)context;
    screen_manager_load_main(user_data);
}

static void network_connected(const network_settings_context_t *context, void *user_data) {
    (void)context;
    screen_manager_load_main(user_data);
}

static void network_scan_requested(network_settings_context_t *context, void *user_data) {
    (void)context;
    (void)user_data;
    printf("Wi-Fi scan requested\n");
    fflush(stdout);
}

static void mode_changed(main_screen_state_t *state, main_screen_mode_t mode, void *user_data) {
    (void)state;
    (void)user_data;
    printf("Main mode: %s\n", mode == MAIN_SCREEN_MODE_WORK ? "WORK" : "STOP");
    fflush(stdout);
}

screen_manager_t *screen_manager_create(void) {
    screen_manager_t *manager = lv_malloc_zeroed(sizeof(*manager));
    if(manager == NULL) return NULL;

    settings_screen_context_init(&manager->settings_context);
    settings_screen_set_save_callback(&manager->settings_context, settings_saved, manager);

    network_settings_context_init(&manager->network_context);
    network_settings_set_ssid_list(&manager->network_context,
                                   default_ssids,
                                   sizeof(default_ssids) / sizeof(default_ssids[0]));
    network_settings_set_callbacks(&manager->network_context,
                                   network_scan_requested,
                                   network_connected,
                                   manager);

    manager->main_state = main_screen_create();
    manager->settings_state = settings_screen_state_create(&manager->settings_context);
    manager->network_state = network_settings_state_create(&manager->network_context);

    if(manager->main_state == NULL || manager->settings_state == NULL || manager->network_state == NULL) {
        screen_manager_destroy(manager);
        return NULL;
    }

    main_screen_set_mode_callback(manager->main_state, mode_changed, manager);
    main_screen_set_settings_callbacks(manager->main_state,
                                       main_settings_clicked,
                                       main_settings_long_pressed,
                                       manager);
    const main_screen_data_t sample_data = {
        .equivalent_altitude = 1500.0f,
        .humidity = 45.0f,
        .temperature = 22.0f,
        .carbon_dioxide = 600.0f,
        .oxygen = 45.0f
    };
    main_screen_set_data(manager->main_state, &sample_data);
    main_screen_set_network_connected(manager->main_state, false);
    return manager;
}

void screen_manager_load_main(screen_manager_t *manager) {
    if(manager != NULL && manager->main_state != NULL) {
        main_screen_load(manager->main_state);
    }
}

void screen_manager_destroy(screen_manager_t *manager) {
    if(manager == NULL) return;
    network_settings_state_destroy(manager->network_state);
    settings_screen_state_destroy(manager->settings_state);
    main_screen_destroy(manager->main_state);
    lv_free(manager);
}

main_screen_state_t *screen_manager_get_main(screen_manager_t *manager) {
    return manager != NULL ? manager->main_state : NULL;
}

settings_screen_context_t *screen_manager_get_settings_context(screen_manager_t *manager) {
    return manager != NULL ? &manager->settings_context : NULL;
}

network_settings_context_t *screen_manager_get_network_context(screen_manager_t *manager) {
    return manager != NULL ? &manager->network_context : NULL;
}
