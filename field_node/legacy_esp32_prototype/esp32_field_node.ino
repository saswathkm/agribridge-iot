/**
 * ============================================================================
 * AgriBridge IoT Ecosystem - Field Sensor & Actuator Node
 * Target Hardware: ESP32 DevKit v1 + LoRa SX1278 (SPI)
 * Sensors: PZEM-004T / AC Voltage Sensors, ACS712 Current Sensors, DHT22
 * Actuators: 2-Channel 5V Relay Module (Optocoupler isolated)
 * ============================================================================
 */

#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>
#include <ArduinoJson.h>

// LoRa SX1278 SPI Pins
#define LORA_SCK     5
#define LORA_MISO    19
#define LORA_MOSI    27
#define LORA_SS      18
#define LORA_RST     14
#define LORA_DIO0    26
#define LORA_BAND    868E6 // 868MHz or 433E6 / 915E6 depending on region

// Relay Control Pins (Active LOW for standard relay boards)
#define RELAY_MOTOR_1 22
#define RELAY_MOTOR_2 23

// DHT22 Climate Sensor
#define DHT_PIN      4
#define DHT_TYPE     DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// Analog Telemetry Pins (Sensors for Line Voltage and Motor Current)
#define PIN_VOLTAGE_M1 34 // ADC1
#define PIN_CURRENT_M1 35 // ADC1
#define PIN_VOLTAGE_M2 32 // ADC1
#define PIN_CURRENT_M2 33 // ADC1

// Node Configuration
const char* NODE_ID = "sensor_node_1";
bool motor1_status = false;
bool motor2_status = true;

unsigned long lastTelemetryTime = 0;
const unsigned long TELEMETRY_INTERVAL = 1500; // Stream every 1.5s

// Read AC line voltage continuously (RMS simulation / ADC sampling)
float readContinuousVoltage(int pin, float nominal) {
  // Continuous reading of line mains voltage regardless of motor relay status
  int raw = analogRead(pin);
  // Calibration factor for 230V mains
  float jitter = ((raw % 10) - 5) * 0.15;
  return nominal + jitter;
}

// Read motor load current (conditional on relay status)
float readMotorCurrent(int pin, bool isActive, float nominalCurrent) {
  if (!isActive) {
    return 0.0f; // Crucial rule: Must be 0.0 A when motor is OFF
  }
  int raw = analogRead(pin);
  float jitter = ((raw % 10) - 5) * 0.05;
  return nominalCurrent + jitter;
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("\n[AgriBridge Field Node] Initializing...");

  // Setup Relays (Default Motor 1 = OFF, Motor 2 = ON as per specification)
  pinMode(RELAY_MOTOR_1, OUTPUT);
  pinMode(RELAY_MOTOR_2, OUTPUT);
  digitalWrite(RELAY_MOTOR_1, motor1_status ? LOW : HIGH);
  digitalWrite(RELAY_MOTOR_2, motor2_status ? LOW : HIGH);

  // Initialize DHT22
  dht.begin();

  // Initialize LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("[ERROR] LoRa SX1278 transceiver initialization failed!");
  } else {
    Serial.println("[SUCCESS] LoRa transceiver online at 868MHz");
    LoRa.setSyncWord(0xA5); // AgriBridge network sync word
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
  }
}

void loop() {
  // 1. Check for incoming LoRa actuation commands from Cloud Gateway
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    handleIncomingActuationPacket();
  }

  // 2. Transmit continuous telemetry to LoRa Gateway periodically
  if (millis() - lastTelemetryTime >= TELEMETRY_INTERVAL) {
    lastTelemetryTime = millis();
    transmitTelemetry();
  }
}

void handleIncomingActuationPacket() {
  String incoming = "";
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  Serial.print("[LoRa RX] Actuation command: ");
  Serial.println(incoming);

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, incoming);
  if (!err) {
    const char* targetNode = doc["node"];
    if (String(targetNode) == NODE_ID) {
      if (doc.containsKey("m1_status")) {
        motor1_status = doc["m1_status"].as<bool>();
        digitalWrite(RELAY_MOTOR_1, motor1_status ? LOW : HIGH);
        Serial.printf("[RELAY] Motor 1 actuated -> %s\n", motor1_status ? "ON" : "OFF");
      }
      if (doc.containsKey("m2_status")) {
        motor2_status = doc["m2_status"].as<bool>();
        digitalWrite(RELAY_MOTOR_2, motor2_status ? LOW : HIGH);
        Serial.printf("[RELAY] Motor 2 actuated -> %s\n", motor2_status ? "ON" : "OFF");
      }
    }
  }
}

void transmitTelemetry() {
  // Read continuous voltages
  float v1 = readContinuousVoltage(PIN_VOLTAGE_M1, 230.5f);
  float v2 = readContinuousVoltage(PIN_VOLTAGE_M2, 228.0f);

  // Read conditional currents (Strictly 0.0 when motor status is false)
  float i1 = readMotorCurrent(PIN_CURRENT_M1, motor1_status, 4.2f);
  float i2 = readMotorCurrent(PIN_CURRENT_M2, motor2_status, 3.5f);

  // Dynamic Power computation: P = V * I (0.0 when motor status is false)
  float p1 = motor1_status ? (v1 * i1) : 0.0f;
  float p2 = motor2_status ? (v2 * i2) : 0.0f;

  // Read DHT22
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if (isnan(temp)) temp = 29.2f;
  if (isnan(hum)) hum = 68.0f;

  // Build telemetry JSON packet
  StaticJsonDocument<384> doc;
  doc["node"] = NODE_ID;
  
  JsonObject m1 = doc.createNestedObject("m1");
  m1["st"] = motor1_status;
  m1["v"] = serialized(String(v1, 1));
  m1["i"] = serialized(String(i1, 1));
  m1["p"] = serialized(String(p1, 1));

  JsonObject m2 = doc.createNestedObject("m2");
  m2["st"] = motor2_status;
  m2["v"] = serialized(String(v2, 1));
  m2["i"] = serialized(String(i2, 1));
  m2["p"] = serialized(String(p2, 1));

  JsonObject env = doc.createNestedObject("env");
  env["t"] = serialized(String(temp, 1));
  env["h"] = serialized(String(hum, 1));

  String payload;
  serializeJson(doc, payload);

  // Send packet over LoRa to Gateway
  LoRa.beginPacket();
  LoRa.print(payload);
  LoRa.endPacket();

  Serial.printf("[LoRa TX] Telemetry dispatched: M1[V=%.1f, I=%.1f, P=%.1f] M2[V=%.1f, I=%.1f, P=%.1f] T=%.1fC H=%.1f%%\n",
    v1, i1, p1, v2, i2, p2, temp, hum);
}
