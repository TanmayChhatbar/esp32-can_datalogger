# ESP32-based CAN Datalogger

TODO add [video](./ppt/esp32-can_datalogger_fb.mp4)
TODO add [Youtube video for reference](TODO)

## References

- [MT07_CAN_Project](https://github.com/JackHat1/MT07_CAN_Project)
- [cantools](https://github.com/cantools/cantools)

## Hardware

| Utility           | Component          |
| ----------------- | ------------------ |
| Microcontroller   | ESP32-C6 Supermini |
| CAN Transceiver   | TJA1051 (x2)       |
| SD card connector | MSD-4-A            |
| IMU               | ICM-20948          |
| GPS               | PA-1616D           |

## Idea

- Overall datalogger using CAN
- One CAN for/from the motorcycle, one for/from sensors
- Onboard GPS for location and time
- Onboard IMU
- External antenna
- Onboard button cell for hot-starts

## Future Work (Suggest)

- Super-capacitor for saving logs on sudden power-cut?
