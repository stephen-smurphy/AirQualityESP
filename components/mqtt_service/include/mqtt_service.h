/**
* @file mqtt_service.h
* @brief MQTT Service that runs in a FreeRTOS task and publishes data to a broker
* 
*/

#pragma once

#include "sensor_service.h"
#include "esp_err.h"

/**
* @brief Initialises the mqtt client configuration and starts the FreeRTOS MQTT task
*
* @return esp_err_t The esp error code
*/
esp_err_t mqtt_service_start(void);

/**
* @brief Helper function to check if MQTT client is connected to the broker
*
* @return bool True if connected, false if disconnected
*/
bool mqtt_client_connected(void);
