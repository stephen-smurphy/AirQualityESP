/**
* @file ota_service.h
* @brief This service checks for firmware updates on a server. 
*        If a new firmware version is found, the update is downloaded to the inactive flash bank on the device while it runs. 
*        Then the device is restarted and booted into the opposite partition.
* 
*/

#pragma once

#include "esp_err.h"

/**
* @brief Starts the OTA service which checks for any available firmware on the OTA server.
*        If a new version is found the firmware is downloaded to the secondary flash bank.
*        When the download is finished the system is restarted and the flash bank is swapped to the new version
*
* @return esp_err_t The esp error code
*/
esp_err_t ota_service_start(void);
