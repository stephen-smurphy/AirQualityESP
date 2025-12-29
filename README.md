# ESP32 Air Quality Monitoring Device

This project is an air quality monitoring device built using the ESP32-C6 chip and written using ESP-IDF. It collects environmental data using the SGP30 (TVOC and eCO2) and SHT3X (temperature and humidity) sensors and transmits the data via MQTT over Wi-Fi.  

## Features

- Measures indoor air quality
- Communicates with sensors via I2C bus
- Custom sensor service and MQTT service collect and send data independently using FreeRTOS tasks
- Publishes sensor data to an MQTT broker
- SGP30 sensor is fed absolute humidity which is calculated from the SHT3X measurements for more accurate Air Quality measurements
- SGP30 baseline value stored on NVS on ESP32 and restored to the sensor on startup to prevent long term drift

## Hardware Used

**Microcontroller:** ESP32-C6
**Sensors:**  Sensirion SGP30 and SHT3X

## Setup

1. **Install ESP-IDF**  

2. **Configure Project**  
   "idf.py menuconfig" to configure Wi-Fi SSID and Password, MQTT URI, Username and Password