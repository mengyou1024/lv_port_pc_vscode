#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "main_screen.h"
#include "network_settings_screen.h"
#include "settings_screen.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct screen_manager screen_manager_t;

screen_manager_t *screen_manager_create(void);
void screen_manager_load_main(screen_manager_t *manager);
void screen_manager_destroy(screen_manager_t *manager);

main_screen_state_t *screen_manager_get_main(screen_manager_t *manager);
settings_screen_context_t *screen_manager_get_settings_context(screen_manager_t *manager);
network_settings_context_t *screen_manager_get_network_context(screen_manager_t *manager);

#ifdef __cplusplus
}
#endif

#endif
