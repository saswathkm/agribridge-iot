# AgriBridge — Production-Ready IoT Agricultural Monitoring & Control Ecosystem

**AgriBridge** is a modern, high-reliability Internet of Things (IoT) ecosystem designed for real-time agricultural telemetry and remote pump actuation. It bridges field-side sensors and motors across long-range radio (LoRa) to Google Firebase Realtime Database, synchronizing seamlessly with two cross-platform clients:

1. **Android Mobile Application:** Built from scratch in **Kotlin** and **Jetpack Compose** (Material 3).
2. **Web Dashboard Application:** Built from scratch in **React.js** and **Tailwind CSS**.

---

## 🏛 System Architecture & Data Flow

```text
  [Field Sensors & Motors]
     ├── Mains Voltage Sensor (Continuous 230V)
     ├── ACS712 Motor Current Sensors
     ├── DHT22 Ambient Microclimate Sensor
     └── 2-Channel Relay Board (Motor 1 & Motor 2)
                 │
                 ▼
     [ESP32 Field Node + LoRa SX1278]
                 │
                 │ 868 MHz LoRa Radio Link (Up to 10 km)
                 ▼
    [ESP32 Gateway Node + LoRa + WiFi]
                 │
                 │ TCP/IP WebSockets / HTTPS
                 ▼
  [Google Firebase Realtime Database]
           Path: /sensor_node_1
                 │
        ┌────────┴────────┐
        ▼                 ▼
[Android App]      [Web Dashboard]
(Jetpack Compose)  (React + Tailwind)
```

### Key Operational Rules:
- **Continuous Line Voltage:** Voltage is streamed continuously from the field node and displayed live on both apps at all times (e.g. `230.5 V`), regardless of whether the motor is ON or OFF.
- **Two-Way Actuation Control:** Toggling a motor switch on either the Android or Web client sends an explicit boolean (`true`/`false`) write to `sensor_node_1/motorX/status`. The home gateway detects this event in real-time, sends a command via LoRa to the field node, and toggles the physical relay.
- **Strict Conditional Current & Power Logic:**
  - When Motor status is **ON (`true`)**: Current is actively measured and displayed (e.g. `4.2 A`). Instantaneous power is computed dynamically ($Power = Voltage \times Current$).
  - When Motor status is **OFF (`false`)**: Current strictly displays `0.0 A` and Power strictly displays `0.0 W`.
- **Power Statistics:** Aggregated cumulative energy metrics (`today`, `week`, `month`) are pulled from Firebase and displayed in the summary card.

---

## 📂 Project Directory Structure

```text
agribridge/
├── home_node/                         # ESP32 LoRa-to-Cloud Gateway Bridge
│   ├── esp32_gateway_bridge.ino       # WLED captive portal, WiFi scanner, LoRa & Firebase sync
│   ├── platformio.ini                 # PlatformIO project configuration
│   └── README.md                      # Pinouts, flashing instructions & architecture
│
├── field_node/                        # RP2040 Raspberry Pi Pico Field Node
│   ├── main.c                         # C firmware for SX1278 Ra-02, ZMPT101B, ACS712, DHT11, Relays
│   ├── CMakeLists.txt                 # Raspberry Pi Pico SDK build configuration
│   ├── pico_sdk_import.cmake          # SDK discovery and setup
│   ├── README.md                      # Wiring table, calibration & serial commands
│   └── legacy_esp32_prototype/        # Previous ESP32 prototype code
│
├── web/                               # React.js + Tailwind CSS Web Dashboard
│   ├── src/
│   │   ├── components/                # ConnectionStatusBar, SoilCard, AlertsBanner, MotorCard, etc.
│   │   ├── firebase/                  # Firebase SDK config & schema models
│   │   ├── hooks/                     # useAgriBridgeData real-time data hook
│   │   └── utils/                     # Dynamic power calculation & unit tests
│   ├── index.html
│   ├── package.json
│   ├── tailwind.config.js
│   └── vite.config.js
│
├── app/                               # Native Android Jetpack Compose Mobile App
│   ├── app/
│   │   ├── src/main/java/com/agribridge/app/
│   │   │   ├── MainActivity.kt        # Edge-to-edge entry point
│   │   │   ├── data/model/            # Firebase schema, connection & alerts telemetry models
│   │   │   ├── data/repository/       # Firebase ValueEventListener & Flow stream
│   │   │   ├── viewmodel/             # StateFlow & actuation handlers
│   │   │   └── ui/                    # UI components (ConnectionStatusRow, SoilCard, AlertsBanner)
│   │   ├── build.gradle.kts           # Compose & Firebase BOM dependencies
│   │   └── AndroidManifest.xml
│   ├── build.gradle.kts
│   ├── settings.gradle.kts
│   └── gradlew.bat
│
├── simulator/                         # Node.js Field Telemetry Simulator
│   └── telemetry_simulator.js         # Simulates field variations, relay switching, & Firebase REST updates
│
└── database/
    ├── initial_data.json              # Exact specification schema
    └── database.rules.json            # Firebase Realtime Database validation rules
```

---

## 🗄 Firebase Realtime Database Schema

Import `database/initial_data.json` directly into your Firebase Console:

```json
{
  "sensor_node_1": {
    "motor1": {
      "status": false,
      "voltage": "230.1",
      "current": "0.0",
      "power": "0.0"
    },
    "motor2": {
      "status": true,
      "voltage": "228.0",
      "current": "3.5",
      "power": "798.0"
    },
    "environment": {
      "temperature": "29.2",
      "humidity": "68.0",
      "soil": "52.0"
    },
    "alerts": {
      "active": "NONE",
      "last_updated": 0
    },
    "connection": {
      "esp32_online": true,
      "lora_link_active": true,
      "last_field_packet_ts": 0,
      "last_firebase_push_ts": 0,
      "lora_rssi": -72
    },
    "stats": {
      "today": "5.2 kWh",
      "week": "34.1 kWh",
      "month": "142.0 kWh"
    }
  }
}
```

---

## 🚀 Running the Web Dashboard

1. Navigate to the `web` directory:
   ```bash
   cd web
   ```
2. Start the Vite development server:
   ```bash
   npm run dev
   ```
3. Open your browser at `http://localhost:3000`.

---

## 📱 Running the Android Mobile Application

1. Open Android Studio (Ladybug / Koala / Hedgehog or newer).
2. Select **Open Project** and navigate to `agribridge/app`.
3. Gradle will sync automatically using the configured `build.gradle.kts`.
4. Run on an Android Emulator or physical device (Android 7.0 / API 24+).

---

## ⚡ Running the Field Simulator CLI

To simulate real-time telemetry fluctuations and Firebase updates without physical ESP32 hardware:

```bash
node simulator/telemetry_simulator.js "https://your-project-default-rtdb.firebaseio.com"
```
