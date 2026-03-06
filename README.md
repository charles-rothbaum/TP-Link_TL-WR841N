# TP-Link_TL-WR841N
TP-Link_TL-WR841N Firmware Reverse Engineering

## Equipment
- TP-Link TL-WR841N WiFi router
- ESP32-S3

## Steps
### Locate series port
I used [this github article](https://github.com/adamhlt/TL-WR841N?tab=readme-ov-file) which identified the the series connectors, so I did not need to do any probing there.

Here is a view of the TL-WR841N when you crack it open:
<img width="610" height="455" alt="Screenshot 2026-03-06 at 8 28 16 AM" src="https://github.com/user-attachments/assets/bc147827-3cbf-4439-a804-2976b89ce2a9" />

And these are the labeled series ports that we are going to plug into our ESP32:

<img width="325" height="624" alt="Screenshot 2026-03-06 at 8 36 22 AM" src="https://github.com/user-attachments/assets/9c050dac-506f-4ec5-83f0-f4bd323bc71a" />

As you can see, I just used some hot glue to tack some male-male jumper wires to the series ports. I'd recommend taking the time to solder on some header pins for a more reliable connection. 

<img width="571" height="504" alt="Screenshot 2026-03-06 at 8 48 43 AM" src="https://github.com/user-attachments/assets/2db1dcad-f326-4ec3-ac83-f3010f1c33a3" />

### Listening to the console



