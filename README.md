# Matter Plug ESP32
## Demo
### Demo with Chip-tool
[![Watch the video](https://img.youtube.com/vi/6rf9T36GZWg/0.jpg)](https://www.youtube.com/watch?v=6rf9T36GZWg)

### Demo with Apple Homekit, Tuya, Home Assistant
[![Watch the video](https://img.youtube.com/vi/daLqvrYXIjQ/0.jpg)](https://www.youtube.com/watch?v=daLqvrYXIjQ)

- ESP-IDF: [v5.4.1](https://github.com/espressif/esp-idf/tree/v5.4.1)<br>
- ESP-Matter-1.4: [de9164fbfd89a83fade503e67f6f0fb1931a1b95](https://github.com/espressif/esp-matter/tree/de9164fbfd89a83fade503e67f6f0fb1931a1b95)<br>
- Connectedhomeip: [7a54749dc9df8706510767513f2f5656a3bd6f68](https://github.com/project-chip/connectedhomeip/tree/7a54749dc9df8706510767513f2f5656a3bd6f68)<br>

## Project Tree Structure
```c
ESP-Matter-Smart-Plug/
├── certification/
│   ├── README.md
│   ├── Attestation/
│   │   ├── test-DAC-FFAA-FFA1-cert.der
│   │   ├── test-DAC-FFAA-FFA1-cert.pem
│   │   ├── test-DAC-FFAA-FFA1-key.der
│   │   ├── test-DAC-FFAA-FFA1-key.pem
│   │   ├── test-PAI-FFAA-cert.der
│   │   ├── test-PAI-FFAA-cert.pem
│   │   └── test-PAI-FFAA-key.pem
│   ├── Certification-Declaration/
│   │   └── Chip-Test-CD-FFAA-FFA1.der
│   └── DACProvider/
│       ├── onb_codes.csv
│       └── partition.csv
│       ├── esp_secure_cert.bin
|       └── partition.bin
├── main/
|    ├── app_driver.cpp
|    ├── app_main.cpp
|    ├── app_priv.h
|    ├── CMakeLists.txt
|    └── idf_component.yml
├── CmakeLists.txt
└── partitions.csv
├── README.md
├── sdkconfig.defaults
└── sdkconfig.defaults.esp32c6
```

## Build & Flash Firmware:
Recommended before flashing:
```c
idf.py -p /dev/ttyACM0 erase-flash
```

1. Flash secure cert partition. Please check partition table for esp_secure_cert partition address.

```c
esptool.py -p /dev/ttyUSB0 write_flash 0xd000 /.../ESP-Matter-Smart-Plug/certification/DACProvider/esp_secure_cert.bin
```
2. Flash factory partition, Please check the CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION_LABEL for factory partition label. Then check the partition table for address and flash at that address.

```c
esptool.py -p /dev/ttyUSB0 write_flash 0x10000 /.../ESP-Matter-Smart-Plug/certification/DACProvider/partition.bin
```
3. Config project
```c
idf.py set-target esp32c6
```

4. Flash Firmware

```c
idf.py -p /dev/ttyUSB0 flash monitor
```

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
onoff toggle <node-id> <bit>
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

Open the commissioning window on the paired Matter device by using the following command pattern: pairing open-commissioning-window 1 1 300 10000 2002
```c
pairing open-commissioning-window <node_id> <option> <window_timeout> <iteration> <discriminator>
```
[More details about CHIP-TOOL](https://project-chip.github.io/connectedhomeip-doc/development_controllers/chip-tool/chip_tool_guide.html)<br>

## Matter Certificate
- Detail in: [Matter Certification](https://github.com/NgTSon/ESP-Matter-Smart-Plug/tree/main/certification)<br>

## Software (Matter)

### Endpoints, Clusters, Attributes using (Update later) 

### Initial Commissioning (Pairing)

1. **Power on the device** 
   - Blue LED lights up to indicate power; Plug and Red LED are off.
   - LED BLUE ON -> (PLUG và LED RED OFF)
   - LED BLUE OFF -> (PLUG và LED RED ON)
2. **Press and hold the button for 5 seconds** 
   - Blue LED starts blinking (commissioning mode)
3. **Open the Matter Controller app**:
   - Google Home: Add Device → Matter
   - Apple Home: Add Accessory → Scan QR Code
   - Amazon Alexa: Add Device → Matter
4. **Scan the QR code** or manually enter the Setup Code
5. **Commissioning completed** - LED stop blingking

### Button Functions

| Action                                     | Function                     |
| ------------------------------------------ | ---------------------------- |
| **Short press (< 5s)**                     | Toggle the plug ON/OFF       |
| **Press and hold for 5s (not yet paired)** | Enter commissioning mode           |
| **Press and hold for 5s (already paired)** | Factory reset + auto-commissioning |


### LED Status Indicators

| BLUE LED                           | Meaning                  |
| ---------------------------------- | ------------------------ |
| **BLUE LED ON**                    | Plug (RED LED) is OFF    |
| **BLUE LED OFF**                   | Plug (RED LED) is ON     |
| **Slow blinking (500ms interval)** | In commissioning mode    |
| **Fast blinking (200ms interval)** | Performing factory reset |



## Hardware
Define in [app_priv.h](https://github.com/NgTSon/ESP-Matter-Smart-Plug/blob/main/main/app_priv.h)<br>
```c
#define PLUG_GPIO               GPIO_NUM_22
#define LED_GPIO                GPIO_NUM_19
#define BUTTON_GPIO             GPIO_NUM_9
```
### 1. Module ESP32-C6

<p align="center">
  <img src="./hardware/Module-ESP Schematic.png" width="70%" height="70%" alt="Schematic Module ESP32-C6" />
</p>

<p align="center"><strong>Schematic MCU</strong></p>

<p align="center">
  <img src="./hardware/Module-ESP 2D.png" width="70%" height="70%" alt="PCB 2D Module ESP32-C6" />
</p>

<p align="center"><strong>PCB 2D Module ESP32-C6.png</strong></p>

<p align="center">
  <img src="./hardware/Module-ESP 3D.png" width="70%" height="70%" alt="PCB 3D Module ESP32-C6" />
</p>

<p align="center"><strong>PCB 3D Module ESP32-C6</strong></p>

### 2. Module Plug

<p align="center">
  <img src="./hardware/Plug Schematic.png" alt="Schematic Plug" />
</p>

<p align="center"><strong>Schematic Module Plug</strong></p>

<p align="center">
  <img src="./hardware/Plug 2D.png" width="70%" height="70%" alt="PCB 2D Plug" />
</p>

<p align="center"><strong>PCB 2D Module Plug</strong></p>

<p align="center">
  <img src="./hardware/Plug 3D.png" width="70%" height="70%" alt="PCB 3D  PLug" />
</p>

<p align="center"><strong>PCB 3D Module Plug</strong></p>
