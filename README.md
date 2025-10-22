Recommended before flashing:
```c
idf.py -p /dev/ttyACM0 erase-flash
```

1. Flash secure cert partition. Please check partition table for esp_secure_cert partition address.

```c
esptool.py -p /dev/ttyACM0 write_flash 0xd000 /media/ntson/Linux-SSD/DATN_KS/ESP-Matter-Smart-Plug/certification/DACProvider/esp_secure_cert.bin

```
2. Flash factory partition, Please check the CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION_LABEL for factory partition label. Then check the partition table for address and flash at that address.

```c
esptool.py -p /dev/ttyACM0 write_flash 0x10000 /media/ntson/Linux-SSD/DATN_KS/ESP-Matter-Smart-Plug/certification/DACProvider/partition.bin
```

3. Flash application

```c
idf.py -p /dev/ttyACM0 flash
```