/**
* @file crc8.h
* @brief Crc8 algorithm helper component
*/

#pragma once

#include <stdint.h>
#include <stddef.h>

/**
* @brief Crc8 calculation function. Initial value 0xFF polynomial 0x31
*
* @param data Pointer to the data buffer that the crc will be calculated on
* @param length The number of bytes over which to calculate the crc
* @return uint8_t Result of the crc8 calculation
*/
uint8_t crc8(const uint8_t *data, size_t length);
