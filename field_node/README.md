# RP2040 LoRa Field Sensor & Relay Node

This firmware targets a **Raspberry Pi Pico (RP2040)** board and an **SX1276/SX1278 (Ra-02)** SPI radio. It continuously measures field sensors (mains AC voltage, motor current, soil moisture, and microclimate), transmits telemetry over 433 MHz LoRa to the ESP32 home gateway, and receives remote actuation commands to control relays.

## Wiring Table

| Function | RP2040 GPIO | Notes |
| --- | --- | --- |
| Radio SPI0 MISO / CS / SCK / MOSI | GP16 / GP17 / GP18 / GP19 | SX1278 Ra-02 SPI bus |
| Radio Reset / DIO0 | GP20 / GP21 | Reset & interrupt line |
| Relay 1 / Relay 2 Control | GP2 / GP3 | Active-HIGH relay drive |
| ZMPT101B AC Voltage Sensor | GP26 / ADC0 | AC RMS waveform sampling |
| ACS712 Current Sensor | GP27 / ADC1 | Motor current measurement |
| Capacitive Soil Moisture Sensor | GP28 / ADC2 | Analog soil moisture |
| DHT11 Temperature & Humidity | GP4 | Pull-up to 3.3V (4.7kΩ–10kΩ) |
| Onboard Status LED | GP25 (PICO_DEFAULT_LED_PIN) | Transmission indicator |

> **Safety Warning:** Never connect AC mains directly to the RP2040. The ZMPT101B and ACS712 sensors must be isolated and keep ADC voltages between 0V and 3.3V.

## LoRa RF Configuration

- **Frequency:** 433.0 MHz (`433000000 Hz`)
- **Spreading Factor (SF):** 7
- **Bandwidth (BW):** 125 kHz
- **Coding Rate (CR):** 4/5
- **Sync Word:** `0x12`
- **Preamble:** 12 symbols

## Telemetry Format

The field node transmits a CSV string every 5 seconds by default:
```text
node=RP2040,v=230.1V,i=1.25A,soil=46.0%,temp=23.0C,humidity=45.0%,r1=ON,r2=OFF,alert=NONE
```

## Actuation Commands

Between transmissions, the node listens for plain-text commands from the ESP32 gateway:
- `R1 ON` / `R1 OFF`
- `R2 ON` / `R2 OFF`
- Combined: `R1 ON,R2 OFF`

## Building and Flashing

1. Set up the Pico SDK or open this directory in VS Code with the Raspberry Pi Pico extension.
2. Build with CMake:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```
3. Hold `BOOTSEL` on the Pico, plug into USB, and copy `field_node.uf2` to the `RPI-RP2` drive.
