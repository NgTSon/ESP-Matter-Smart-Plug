<<<<<<< HEAD
# Matter Plug ESP32
## SDK Version:
- ESP-IDF: https://github.com/espressif/esp-idf/tree/v5.4.1
- ESP-Matter-1.4: https://github.com/espressif/esp-matter/tree/de9164fbfd89a83fade503e67f6f0fb1931a1b95
- Connectedhomeip: https://github.com/project-chip/connectedhomeip/tree/7a54749dc9df8706510767513f2f5656a3bd6f68

## Build & Flash Firmware:
=======
# SDK Version
- esp-idf: https://github.com/espressif/esp-idf/tree/v5.4.1
- esp-matter: https://github.com/espressif/esp-matter/tree/release/v1.4
- connectedhomeip: https://github.com/project-chip/connectedhomeip/tree/7a54749dc9df8706510767513f2f5656a3bd6f68

# Build & Flash Firmware

>>>>>>> 432aa67 (commit)
Recommended before flashing:
```c
idf.py -p /dev/ttyACM0 erase-flash
```

1. Flash secure cert partition. Please check partition table for esp_secure_cert partition address.

```c
<<<<<<< HEAD
esptool.py -p /dev/ttyACM0 write_flash 0xd000 /.../ESP-Matter-Smart-Plug/certification/DACProvider/esp_secure_cert.bin
=======
esptool.py -p /dev/ttyACM0 write_flash 0xd000 /media/ntson/Linux-SSD/DATN_KS/ESP-Matter-Smart-Plug/certification/DACProvider/esp_secure_cert.bin

>>>>>>> 432aa67 (commit)
```
2. Flash factory partition, Please check the CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION_LABEL for factory partition label. Then check the partition table for address and flash at that address.

```c
<<<<<<< HEAD
esptool.py -p /dev/ttyACM0 write_flash 0x10000 /.../ESP-Matter-Smart-Plug/certification/DACProvider/partition.bin
```

3. Flash application and serial

```c
idf.py -p /dev/ttyACM0 flash monitor
```

## Software (Matter)
## Hardware
Config in /main/app_priv.h
```c
#define PLUG_GPIO               GPIO_NUM_22
#define LED_GPIO                GPIO_NUM_19
#define BUTTON_GPIO             GPIO_NUM_9
```
1. Schematic Module ESP32-C6

2. Schematic Smart Plug

## QR Code for commisioning
![qrcode.png](./certification/DACProvider/qrcode.png)
** Setup Code: 1559-121-2356 **
## Control and Read data using CHIP-TOOL:
=======
esptool.py -p /dev/ttyACM0 write_flash 0x10000 /media/ntson/Linux-SSD/DATN_KS/ESP-Matter-Smart-Plug/certification/DACProvider/partition.bin
```

3. Flash application

```c
idf.py -p /dev/ttyACM0 flash
```
>>>>>>> 432aa67 (commit)
