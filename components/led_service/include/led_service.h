/**
* @file led_service.h
* @brief LED controller for a red yellow and green LED on the board. Supports setting different states for each LED.
* 
*/

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

/**
* @brief Initialises the LED GPIO pins and starts the LED freeRTOS task
*
* @return esp_err_t The esp error code
*/
esp_err_t led_service_init(void);

/**
* @brief Used to set a particular led addressed by its ID to a state, if the state is blink, the period_ms parameter dictates how fast the blink period is.
*
* @param id led_it_t type the LED to perform the action on
* @param state led_state_t type the state to set the LED to
* @param period_ms uint32_t value in milliseconds of the blink period, N/A if state != blink
* @return esp_err_t The esp error code
*/
esp_err_t led_service_set_led(led_id_t id, led_state_t state, uint32_t period_ms);
