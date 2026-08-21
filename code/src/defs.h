#pragma once

// CAN
#define CAN0_RX 6
#define CAN0_TX 7
#define CAN0_SX -1 // not used, SX is grounded
#define CAN1_RX 3
#define CAN1_TX 4
#define CAN1_SX CAN0_SX

// GPS
#define GPS_TX 16
#define GPS_RX 17
#define GPS_FIX 14

// SPI
#define SPI_MOSI 0
#define SPI_MISO 1
#define SPI_SCK 2

// IMU
#define SPI_CS_IMU 18

// SD card
#define SPI_CS_SD 23
#define SD_CD 21

// I2C
#define I2C_SDA 20
#define I2C_SCL 19
#define I2C_DISP 0x3C // TODO

// IO
#define ARGB_LED 8
#define NUM_LED 1
#define BUTS 5
#define NUMBUTS 2
constexpr float BUTS_V[] = {1.31f, 0.0f};

// settings
#define SERIAL_GPS 0
#define SERIAL_IMU 0
#define SERIAL_CAN0 1
#define SERIAL_CAN1 1
#define SERIAL_OBD 1 // useless if SERIAL_CAN1=1
#define SERIAL_OBD_REQUESTS 0
#define SERIAL_PRINT_CAN 1
#define SERIAL_COMMANDS 1

// wifi
#define USE_WIFI 1 // or hotspot
