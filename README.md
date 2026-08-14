# ESP32-based CAN Datalogger

TODO add [animation](./ppt/esp32-can_datalogger_fb.mp4)
TODO add [Youtube video for reference](TODO)

## Idea

- Overall datalogger using CAN
- One CAN for/from the motorcycle, one for/from sensors
- Request signals over OBD from the motorcycle
- Onboard GPS for location and time
- Onboard IMU
- External antenna
- Onboard button cell for hot-starts

## Hardware

| Utility           | Component          |
| ----------------- | ------------------ |
| Microcontroller   | ESP32-C6 Supermini |
| CAN Transceiver   | TJA1051 (x2)       |
| SD card connector | MSD-4-A            |
| IMU               | ICM-20948          |
| GPS               | PA-1616D           |

## References

- [cantools](https://github.com/cantools/cantools)
- [MT07_CAN_Project](https://github.com/JackHat1/MT07_CAN_Project)

## Future Work (Suggest)

- Super-capacitor for saving logs on sudden power-cut?
- Revise mounting for reducing engine vibration
- Improved GPS chip

  | Chip                                                                                             | Features                        | Cost  |
  | ------------------------------------------------------------------------------------------------ | ------------------------------- | ----- |
  | [Quectel LC29HCA](https://forums.quectel.com/uploads/short-url/nu6XgBFKtXOYPPegStMeuTfr7L9.pdf)  | dual band, 10hz, DR             | 24.69 |
  | [Quectel LC29HBA](https://forums.quectel.com/uploads/short-url/nu6XgBFKtXOYPPegStMeuTfr7L9.pdf)  | dual band, 10hz, DR, RTK        | 42.73 |
  | [NEO-M9V](https://content.u-blox.com/sites/default/files/NEO-M9V-20B_DataSheet_UBX-21029781.pdf) | single-band, 50hz (with DR), DR | 18.67 |

- Improved IMU

  | Chip                                                                                                                             | Features                 | Cost  |
  | -------------------------------------------------------------------------------------------------------------------------------- | ------------------------ | ----- |
  | [BNO085](https://www.digikey.com/en/products/detail/ceva-technologies-inc/BNO085/9445940)                                        | Active, TODO             | 13.57 |
  | [BNO086](https://www.digikey.com/en/products/detail/ceva-technologies-inc/BNO086/14114190)                                       | Active, Updated from 085 | 13.93 |
  | [ICM-42688-P](https://www.digikey.com/en/products/detail/tdk-invensense/ICM-42688-P/10824934?s=N4IgTCBcDaIJYGMC2AWMA2AHJkBdAvkA) | Active, TODO             | 4.91  |
  | [ICM-42670-P](https://www.digikey.com/en/products/detail/tdk-invensense/ICM-42670-P/14319524)                                    | Active, TODO             | 3.54  |
