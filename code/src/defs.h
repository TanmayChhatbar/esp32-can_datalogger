#pragma once

// CAN
#define CAN0_TX 0
#define CAN0_RX 1
#define CAN0_SX 15
#define CAN1_TX 2
#define CAN1_RX 3
#define CAN1_SX CAN0_SX

// GPS
#define GPS_TX 16
#define GPS_RX 17
#define GPS_FIX 14

// SPI
#define SPI_MOSI 5
#define SPI_MISO 6
#define SPI_SCK 7

// IMU
#define SPI_CS_IMU 4

// SD card
#define SPI_CS_SD 18
#define SD_CD 21

// I2C
#define I2C_SDA 20
#define I2C_SCL 19
#define I2C_DISP 0x3C // TODO

// IO
#define ARGB_LED 8
#define NUM_LED 1
// #define BUTS 22 // 22 does not support analogRead(), TODO needs PCB change. GPIO0-6 support analogRead() https://documentation.espressif.com/esp32-c6_datasheet_en.html
#define BUT1 22

// voltages if BUTS is analogRead() capable
// #define BUTS_V1 
// #define BUTS_V2
// #define BUTS_V3
// #define BUTS_V4

// settings
#define SERIAL_GPS 1
#define SERIAL_IMU 1
#define SERIAL_CAN0 1
#define SERIAL_CAN1 1
#define SERIAL_OBD_REQUESTS 0
#define SERIAL_PRINT_CAN 1
#define SERIAL_COMMANDS 1
