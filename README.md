# TP-Link_TL-WR841N
TP-Link_TL-WR841N Firmware Reverse Engineering

### Equipment
- TP-Link TL-WR841N WiFi router
- ESP32-S3
### Software 
- ESP-IDF 5.x

## Steps
### Locate series port
I used [this github article](https://github.com/adamhlt/TL-WR841N?tab=readme-ov-file) which identified the the series connectors, so I did not need to do any probing there.

Here is a view of the TL-WR841N when you crack it open:
<img width="610" height="455" alt="Screenshot 2026-03-06 at 8 28 16 AM" src="https://github.com/user-attachments/assets/bc147827-3cbf-4439-a804-2976b89ce2a9" />

And these are the labeled series ports that we are going to plug into our ESP32:

<img width="325" height="624" alt="Screenshot 2026-03-06 at 8 36 22 AM" src="https://github.com/user-attachments/assets/9c050dac-506f-4ec5-83f0-f4bd323bc71a" />

As you can see, I just used some hot glue to tack some male-male jumper wires to the series ports. I'd recommend taking the time to solder on some header pins for a more reliable connection. 

<img width="571" height="504" alt="Screenshot 2026-03-06 at 8 48 43 AM" src="https://github.com/user-attachments/assets/2db1dcad-f326-4ec3-ac83-f3010f1c33a3" />

### Flash the ESP32
The next step is to flash the UART-USB Interface firmware onto the ESP32.

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/tty.usbmodemXXXX flash
idf.py -p /dev/tty.usbmodemXXXX monitor
```

```bash
Replace `/dev/tty.usbmodemXXXX` with your actual serial device.
```

Next, connect the router's UART to the ESP32

```bash
ESP32 pin 18 (Rx) <---> Router Tx
ESP32 pin 17 (Tx) <---> Router Rx
          ESP32 G <---> Router G
```
<img width="991" height="720" alt="Screenshot 2026-03-09 at 12 35 57 PM" src="https://github.com/user-attachments/assets/e63ebebe-7ee7-40ac-8fd4-371b15346bad" />

### The UART Console

The ESP32’s firmware reads raw bytes coming from the router’s UART hardware, and those bytes are places in a USB packet buffer, and the TinyUSB driver sends those packets over the ESP32's native USB controller. The operating system will 

The operating system recognizes the device as a USB CDC ACM serial device, so it automatically creates a serial port:
```bash
> ls /dev/cu*
/dev/cu.usbmodem1234561 
```

Then, you can use ```screen``` or ```picocom``` to view the bytes coming through UART. I found that the baud rate was 115200:

```bash
> picocom -b 115200 /dev/cu.usbmodem1234561
```
Now, powering the router on should produce a looping U-boot sequence. Already, here are several things that we can learn about the router's architecture:

<img width="823" height="735" alt="Screenshot 2026-03-09 at 1 07 18 PM" src="https://github.com/user-attachments/assets/53103b99-67e3-450f-bc51-c33f5709cee4" />

```U-Boot 1.1.4 (Build from LSDK-9.5.3)```: 
Bootloader: U-Boot 1.1.4

SDK: LSDK-9.5.3.16

```ap143 - Honey Bee 1.1```
Qualcomm Atheros AP143 platform.

```
DRAM: 32 MB
Flash Manuf Id 0xc2
DeviceId0 0x20
DeviceId1 0x16
Flash: 4 MB
```
We only have 32 MB of RAM and 4 MB of FLASH memory on this board, so already we very limited with what kinds of modern firmware we can run on this platform.
