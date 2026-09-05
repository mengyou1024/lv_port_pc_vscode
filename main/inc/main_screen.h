#ifndef MAIN_SCREEN_H
#define MAIN_SCREEN_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct main_screen_state main_screen_state_t;

typedef enum {
    MAIN_SCREEN_MODE_STOP = 0,
    MAIN_SCREEN_MODE_WORK
} main_screen_mode_t;

typedef struct {
    float equivalent_altitude;
    float humidity;
    float temperature;
    float carbon_dioxide;
    float oxygen;
} main_screen_data_t;

typedef void (*main_screen_mode_callback_t)(main_screen_state_t *state,
                                            main_screen_mode_t mode,
                                            void *user_data);

typedef void (*main_screen_settings_callback_t)(main_screen_state_t *state,
                                                void *user_data);

main_screen_state_t *main_screen_create(void);
void main_screen_load(main_screen_state_t *state);
void main_screen_destroy(main_screen_state_t *state);

void main_screen_set_mode(main_screen_state_t *state, main_screen_mode_t mode);
main_screen_mode_t main_screen_get_mode(const main_screen_state_t *state);
void main_screen_set_mode_callback(main_screen_state_t *state,
                                   main_screen_mode_callback_t callback,
                                   void *user_data);
void main_screen_set_settings_callbacks(main_screen_state_t *state,
                                        main_screen_settings_callback_t click_callback,
                                        main_screen_settings_callback_t long_press_callback,
                                        void *user_data);

void main_screen_set_network_connected(main_screen_state_t *state, bool connected);
bool main_screen_get_network_connected(const main_screen_state_t *state);

void main_screen_set_data(main_screen_state_t *state, const main_screen_data_t *data);

#ifdef __cplusplus
}
#endif

#endif
