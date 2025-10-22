# Matter Plug ESP32
## SDK Version:
- ESP-IDF: [v5.4.1](https://github.com/espressif/esp-idf/tree/v5.4.1)<br>
- ESP-Matter-1.4: [de9164fbfd89a83fade503e67f6f0fb1931a1b95](https://github.com/espressif/esp-matter/tree/de9164fbfd89a83fade503e67f6f0fb1931a1b95)<br>
- Connectedhomeip: [7a54749dc9df8706510767513f2f5656a3bd6f68](https://github.com/project-chip/connectedhomeip/tree/7a54749dc9df8706510767513f2f5656a3bd6f68)<br>

## Build & Flash Firmware:
Recommended before flashing:
```c
idf.py -p /dev/ttyACM0 erase-flash
```

1. Flash secure cert partition. Please check partition table for esp_secure_cert partition address.

```c
esptool.py -p /dev/ttyACM0 write_flash 0xd000 /.../ESP-Matter-Smart-Plug/certification/DACProvider/esp_secure_cert.bin
```
2. Flash factory partition, Please check the CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION_LABEL for factory partition label. Then check the partition table for address and flash at that address.

```c
esptool.py -p /dev/ttyACM0 write_flash 0x10000 /.../ESP-Matter-Smart-Plug/certification/DACProvider/partition.bin
```
3. Config project
```c
idf.py set-target esp32c6
```

4. Flash Firmware

```c
idf.py -p /dev/ttyACM0 flash monitor
```

## Software (Matter)
## Hardware
Config in: [app_priv.h](https://github.com/NgTSon/ESP-Matter-Smart-Plug/blob/main/main/app_priv.h)<br>
```c
#define PLUG_GPIO               GPIO_NUM_22
#define LED_GPIO                GPIO_NUM_19
#define BUTTON_GPIO             GPIO_NUM_9
```
1. Schematic Module ESP32-C6

2. Schematic Smart Plug

## QR Code for commisioning
<p align="center">
  <img src="./certification/DACProvider/qrcode.png" alt="qrcode" />
</p>

<p align="center"><strong>Setup Code: 1559-121-2356</strong></p>

## Control and Read data using CHIP-TOOL:
1. Start CHIP-TOOL
```c
chip-tool interactive start
```
2. Commissioning using chip-tool: 

Paring: pairing code-wifi 1 MyWifi 12345678  15591212356
```c
pairing code-wifi <node-id> <ssid> <passphrase>  <SetupCode>
```
3. Use the cluster commands to control the attributes.

Toggle: onoff toggle 1 0x1
```c
onoff on <node-id> <bit>
```
Set on: onoff on 1 0x1
```c
onoff on <node-id> <bit>
```
4. Use the cluster commands to read the attributes.

Read Product name and Vendor Name: basicinformation read product-name 1 0
```c
basicinformation read product-name <node-id> <endpoint>
```
```c
basicinformation read vendor-name <node-id> <endpoint>
```

5. Open the commissioning window

Open the commissioning window on the paired Matter device by using the following command pattern: open-commissioning-window 0x2002 1 300 10000 2002
```c
pairing open-commissioning-window <node_id> <option> <window_timeout> <iteration> <discriminator>
```
[More details about CHIP-TOOL](https://project-chip.github.io/connectedhomeip-doc/development_controllers/chip-tool/chip_tool_guide.html)<br>

## Matter Certificate
- Detail in: [Matter Certification](https://github.com/NgTSon/ESP-Matter-Smart-Plug/tree/main/certification)<br>
