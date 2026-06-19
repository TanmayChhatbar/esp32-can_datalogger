#pragma once

// CAN
#if TWO_CAN
#define CAN0_TX 0
#define CAN0_RX 1
#define CAN0_SX 15
#define CAN1_TX 2
#define CAN1_RX 3
#define CAN1_SX CAN0_SX
#else
#define CAN0_TX 10
#define CAN0_RX 11
#define CAN0_SX 12
#endif
// GPS
// #define GPS_TX TODO
// #define GPS_RX TODO
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
#define ARGB_LED 9
#define NUM_LED 3
#define BUTS 22
