#pragma once

#include "esp_err.h"

typedef enum {
    LED_STATE_LOW,
    LED_STATE_HIGH,
    LED_STATE_BLINK,
    LED_STATE_SIZE
} led_state_t;

typedef enum {
    LED_GREEN,
    LED_YELLOW,
    LED_RED,
    LED_ID_SIZE
} led_id_t;

typedef struct {
    led_id_t id;
    led_state_t state;
    uint32_t period_ms;
} led_command_t;

esp_err_t led_service_init(void);

esp_err_t led_service_set_led(led_id_t id, led_state_t state, uint32_t period_ms);
