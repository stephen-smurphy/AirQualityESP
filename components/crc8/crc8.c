#include "crc8.h"

uint8_t crc8(const uint8_t *data, size_t length) {
    uint8_t crc = 0xFF;
    uint8_t polynomial = 0x31;

    for(size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for(int bit = 0; bit < 8; bit++) {
            if(crc & 0x80) {
                crc = (crc << 1) ^ polynomial;
            }
            else {
                crc <<= 1;
            }
        }
    }
    return crc;
}
