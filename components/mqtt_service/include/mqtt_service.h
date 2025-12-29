#pragma once

#include "sensor_service.h"
#include "esp_err.h"

// Initialize MQTT client and connect
esp_err_t mqtt_service_start(void);

// Check if client is connected
bool mqtt_client_connected(void);
