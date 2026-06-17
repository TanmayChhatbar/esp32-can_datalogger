#pragma once

// CAN
#define CAN0_TX 0
#define CAN0_RX 1
#define CAN0_SX 15
#define CAN1_TX 2
#define CAN1_RX 3
#define CAN1_SX CAN0_SX

// GPS
// #define GPS_TX TODO
// #define GPS_RX TODO
#define GPS_FIX 14

// SPI
#define MOSI 5
#define MISO 6
#define SCK 7

// IMU
#define CS_IMU 4

// SD card
#define CS_SD 18
#define SD_CD 21

// I2C
#define SDA 20
#define SCL 19
#define I2C_DISP 0x3C // TODO

// IO
#define ARGB_LED 9
#define NUM_LED 3
#define BUTS 22
