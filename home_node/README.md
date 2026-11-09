# ESP32 LoRa Cloud Gateway Bridge

Firmware for the **AgriBridge ESP32 Home Gateway Node**. It receives long-range LoRa packets from the RP2040 field node, parses sensor data, and pushes live updates to Google Firebase Realtime Database. It also polls Firebase for remote motor switch toggles and dispatches actuation commands back over LoRa.

## Key Features

1. **WLED-Style First-Time Setup:** Open Wi-Fi Access Point (`AgriBridge-Gateway`, no password) with captive portal at `http://192.168.4.1`.
2. **Interactive Live Wi-Fi Scanner:** Live 2.4 GHz spectrum scan with signal strength (RSSI), one-tap SSID selection, and dynamic rescan.
3. **Hardware Factory Reset:** Hold the BOOT button (GPIO 0) for 3 seconds to clear saved NVS credentials and reboot into AP mode.
4. **LoRa SX1278 Bridge:** 433 MHz, sync word `0x12`, SF7, BW 125 kHz matching the RP2040 field node.
5. **Real-Time Firebase Sync:** Streams telemetry (`motor1`, `motor2`, `environment`, `alerts`, `connection`) to `/sensor_node_1`.
6. **Active Heartbeat Watchdog:** Periodically updates `last_firebase_push_ts` so clients can accurately detect online/offline state.

## Pinout (ESP32 DevKit v1)

| Function | ESP32 Pin | Notes |
| --- | --- | --- |
| LoRa SCK | GPIO 5 | SPI Clock |
| LoRa MISO | GPIO 19 | SPI Data Out |
| LoRa MOSI | GPIO 27 | SPI Data In |
| LoRa SS (CS) | GPIO 18 | Chip Select |
| LoRa RST | GPIO 14 | Reset |
| LoRa DIO0 | GPIO 26 | Radio Interrupt |
| Status LED | GPIO 2 | Blue onboard LED |
| Factory Reset | GPIO 0 | BOOT button (Active-LOW) |

## Flashing Instructions

### Using Arduino IDE:
1. Install the **ESP32 by Espressif Systems** board package in Board Manager.
2. Install libraries:
   - `LoRa` by Sandeep Mistry
   - `ArduinoJson` (v6 or v7)
3. Select board: `ESP32 Dev Module`.
4. Open `esp32_gateway_bridge.ino` and click **Upload**.

### Using VS Code / PlatformIO:
```bash
pio run --target upload
pio device monitor -b 115200
```
