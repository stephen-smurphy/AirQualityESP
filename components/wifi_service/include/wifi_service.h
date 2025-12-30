/**
* @file wifi_service.h
* @brief Component responsible for initialising and starting the Wi-Fi and connecting to an access point
*
*/

#pragma once

#include "esp_err.h"
#include <stdbool.h>

/**
* @brief Initialises the wifi event handler, configures the device as a station and waits for a connection to be established
*
* @return esp_err_t The esp error code
*/
esp_err_t wifi_service_start(void);


/**
* @brief Helper function to check if the Wi-Fi has a valid connection to a network
*
* @return bool True if connected, False if disconnected
*/
bool wifi_is_connected(void);