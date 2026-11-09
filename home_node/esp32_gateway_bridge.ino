/**
 * =========================================================================================
 * AgriBridge IoT Ecosystem - ESP32 LoRa Cloud Gateway Bridge (v3.0 Production)
 * =========================================================================================
 * Features:
 *   1. WLED-Style First-Time Setup (Open SoftAP "AgriBridge-Gateway", No Password).
 *   2. Interactive Nearby Wi-Fi Scanner: Scans, lists SSIDs with signal strength (RSSI),
 *      and lets you tap any network to auto-fill the SSID and focus the password box.
 *   3. Dedicated "Rescan Networks" button for real-time live Wi-Fi spectrum refresh.
 *   4. Minimalist Dark-Themed (Pure Black/Slate #0B1120) Captive Portal.
 *   5. Hardware Factory Reset Pin (Hold for 3s to erase credentials & re-enter AP mode).
 *   6. LoRa SX1278 Transceiver at 433 MHz, sync word 0x12 — compatible with
 *      RP2040 field node (CSV text packet format, plain-text relay commands).
 *   7. Real-Time Cloud Bridge to Firebase Realtime Database (/sensor_node_1).
 *   8. Two-Way Actuation Listener — sends "R1 ON"/"R1 OFF" plain-text LoRa commands.
 *   9. Connection health heartbeat: ESP32 online, LoRa link, data stream timestamps.
 *  10. Soil moisture, active alerts, and NA-safe temperature/humidity pushed to Firebase.
 *  11. Highly Modular & Adaptive Code Structure for easy custom extension.
 *
 * ---- FIELD NODE PACKET FORMAT (RP2040 sends this, do NOT change) ----
 *   node=RP2040,v=230.1V,i=1.25A,soil=46.0%,temp=23.0C,humidity=45.0%,r1=ON,r2=OFF,alert=NONE
 *
 * ---- ACTUATION COMMAND FORMAT (ESP32 sends this to RP2040) ----------
 *   Single relay:   "R1 ON"  /  "R1 OFF"  /  "R2 ON"  /  "R2 OFF"
 *   Both at once:   "R1 ON,R2 OFF"
 * =========================================================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <SPI.h>
#include <LoRa.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// =========================================================================================
// 1. HARDWARE PIN DEFINITIONS (Easily editable for your specific ESP32 board)
// =========================================================================================

// Factory Reset Button Pin (Default: GPIO 0 is the onboard 'BOOT' button on DevKit boards)
#define RESET_BUTTON_PIN      0     // Press & hold 3 seconds to reset WiFi credentials
#define STATUS_LED_PIN        2     // Onboard Blue LED (GPIO 2 on most ESP32 DevKits)

// LoRa SX1278 SPI Pins
#define LORA_SCK              5
#define LORA_MISO             19
#define LORA_MOSI             27
#define LORA_SS               18
#define LORA_RST              14
#define LORA_DIO0             26

// ---- LoRa RF Settings — MUST match the RP2040 field node exactly ----
// RP2040 defaults: 433 MHz, SF7, BW 125 kHz, CR 4/5, sync word 0x12
#define LORA_BAND             433E6   // 433 MHz — matches RP2040 field node
#define LORA_SYNC_WORD        0x12    // RP2040 default sync word
#define LORA_SPREADING_FACTOR 7
#define LORA_BANDWIDTH        125E3

// LoRa link watchdog — if no packet arrives within this window, mark link as inactive
#define LORA_LINK_TIMEOUT_MS  60000UL  // 60 seconds

// AP Mode Settings (WLED-style Open Network)
const char* AP_SSID = "AgriBridge-Gateway"; // Open WiFi with NO PASSWORD
const byte DNS_PORT = 53;

// =========================================================================================
// 2. GLOBAL SYSTEM STATE & CONFIGURATION STORAGE
// =========================================================================================

Preferences preferences;
WebServer server(80);
DNSServer dnsServer;

// Configuration variables (Saved in Flash NVS)
String wifi_ssid       = "";
String wifi_password   = "";
String firebase_url    = "https://agribridge-iot-default-rtdb.firebaseio.com";
String firebase_key    = "AIzaSyBMe0BjWk36VQypQM6thOG-CvnvP3rG5ZQ";
String sensor_node_id  = "sensor_node_1";

bool isAPMode = false;
unsigned long lastActuationCheck = 0;
const unsigned long ACTUATION_POLL_INTERVAL = 1000; // Poll Firebase for switch changes every 1s

// LoRa link health tracking
unsigned long lastLoRaPacketMs = 0;   // millis() when last valid LoRa packet arrived
bool loraLinkActive = false;           // true when a packet arrived within LORA_LINK_TIMEOUT_MS

// Cached state to detect when Web/App toggles motor switch
bool last_known_m1_status = false;
bool last_known_m2_status = true;

// =========================================================================================
// 3. MINIMALIST BLACK-THEMED CAPTIVE PORTAL WITH LIVE WIFI SCANNER
// =========================================================================================

const char PORTAL_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>AgriBridge Gateway Setup</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      background-color: #0b1120;
      color: #f1f5f9;
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      padding: 16px;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
    }
    .card {
      background: #151e2e;
      border: 1px solid #1e293b;
      border-radius: 20px;
      width: 100%;
      max-width: 440px;
      padding: 24px;
      box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.7);
    }
    .header {
      display: flex;
      align-items: center;
      gap: 14px;
      margin-bottom: 20px;
      border-bottom: 1px solid #1e293b;
      padding-bottom: 16px;
    }
    .logo {
      width: 42px;
      height: 42px;
      background: linear-gradient(135deg, #16a34a, #22c55e);
      border-radius: 12px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-weight: 800;
      font-size: 20px;
      color: #fff;
      box-shadow: 0 4px 12px rgba(22, 163, 74, 0.3);
    }
    h2 { font-size: 19px; font-weight: 700; color: #fff; }
    p.sub { font-size: 12px; color: #94a3b8; }
    
    .section-title {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-top: 18px;
      margin-bottom: 8px;
    }
    .section-label {
      font-size: 11px;
      text-transform: uppercase;
      letter-spacing: 0.6px;
      font-weight: 700;
      color: #94a3b8;
    }
    .scan-btn {
      background: #1e293b;
      border: 1px solid #334155;
      color: #38bdf8;
      font-size: 11px;
      font-weight: 600;
      padding: 5px 10px;
      border-radius: 6px;
      cursor: pointer;
      display: flex;
      align-items: center;
      gap: 4px;
      transition: all 0.2s;
    }
    .scan-btn:hover { background: #334155; color: #fff; }
    
    /* Scanned Networks List */
    .net-container {
      background: #0b1120;
      border: 1px solid #1e293b;
      border-radius: 12px;
      max-height: 190px;
      overflow-y: auto;
      margin-bottom: 16px;
    }
    .net-item {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 10px 14px;
      border-bottom: 1px solid #1e293b;
      cursor: pointer;
      transition: background 0.15s;
    }
    .net-item:last-child { border-bottom: none; }
    .net-item:hover { background: #1e293b; }
    .net-item.active {
      background: rgba(22, 163, 74, 0.15);
      border-left: 3px solid #22c55e;
    }
    .net-info { display: flex; flex-direction: column; gap: 2px; }
    .net-name { font-size: 13px; font-weight: 600; color: #f8fafc; }
    .net-meta { font-size: 11px; color: #64748b; display: flex; align-items: center; gap: 6px; }
    .net-badge {
      font-size: 10px;
      font-weight: 600;
      color: #4ade80;
      background: rgba(22, 163, 74, 0.2);
      padding: 2px 6px;
      border-radius: 4px;
    }
    .select-pill {
      font-size: 11px;
      font-weight: 600;
      color: #94a3b8;
      background: #1e293b;
      padding: 4px 8px;
      border-radius: 6px;
    }
    .net-item:hover .select-pill { background: #16a34a; color: #fff; }
    
    .loading-state {
      padding: 20px;
      text-align: center;
      color: #94a3b8;
      font-size: 12px;
    }
    .spinner {
      display: inline-block;
      width: 16px;
      height: 16px;
      border: 2px solid rgba(56, 189, 248, 0.2);
      border-radius: 50%;
      border-top-color: #38bdf8;
      animation: spin 0.8s linear infinite;
      margin-right: 8px;
      vertical-align: middle;
    }
    @keyframes spin { to { transform: rotate(360deg); } }

    /* Form controls */
    label {
      display: block;
      font-size: 11px;
      text-transform: uppercase;
      letter-spacing: 0.5px;
      font-weight: 600;
      color: #94a3b8;
      margin-top: 14px;
      margin-bottom: 6px;
    }
    input {
      width: 100%;
      background: #0b1120;
      border: 1px solid #334155;
      color: #fff;
      padding: 11px 14px;
      border-radius: 10px;
      font-size: 13px;
      outline: none;
      transition: border-color 0.2s;
    }
    input:focus { border-color: #22c55e; }
    .btn-submit {
      width: 100%;
      background: linear-gradient(135deg, #16a34a, #15803d);
      color: #fff;
      border: none;
      padding: 13px;
      border-radius: 10px;
      font-size: 14px;
      font-weight: 600;
      cursor: pointer;
      margin-top: 22px;
      box-shadow: 0 4px 14px rgba(22, 163, 74, 0.4);
      transition: opacity 0.2s;
    }
    .btn-submit:hover { opacity: 0.95; }
    .hint { font-size: 11px; color: #64748b; margin-top: 5px; }
  </style>
</head>
<body>
  <div class="card">
    <div class="header">
      <div class="logo">&#9889;</div>
      <div>
        <h2>AgriBridge Gateway</h2>
        <p class="sub">LoRa-to-Firebase Hardware Bridge</p>
      </div>
    </div>

    <!-- Scanned Nearby Wi-Fi Section -->
    <div class="section-title">
      <span class="section-label">Nearby Wi-Fi Networks</span>
      <button type="button" class="scan-btn" id="rescan-btn" onclick="scanNetworks()">
        <span>&#128257; Rescan</span>
      </button>
    </div>

    <div class="net-container" id="network-list">
      <div class="loading-state">
        <div class="spinner"></div> Scanning 2.4GHz Wi-Fi spectrum...
      </div>
    </div>

    <!-- Credentials & Firebase Setup Form -->
    <form action="/save" method="POST">
      <label>Selected Wi-Fi SSID</label>
      <input type="text" id="ssid_input" name="ssid" value="{{CURRENT_SSID}}" placeholder="Click a network above or type SSID" required autocomplete="off">

      <label>Wi-Fi Password</label>
      <input type="password" id="pass_input" name="password" placeholder="Enter Wi-Fi Password (leave blank if open)">
      <p class="hint">Will connect automatically after reboot.</p>

      <label>Firebase Database URL</label>
      <input type="text" name="fb_url" value="{{FB_URL}}" required>

      <label>Target Node Root</label>
      <input type="text" name="node_id" value="{{NODE_ID}}" required>

      <button type="submit" class="btn-submit">Connect & Save Settings</button>
    </form>
  </div>

  <script>
    function escapeHtml(str) {
      if (!str) return '';
      return str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;').replace(/'/g, '&#039;');
    }

    async function scanNetworks() {
      const listEl = document.getElementById('network-list');
      const btn = document.getElementById('rescan-btn');
      btn.innerHTML = '<span class="spinner"></span> Scanning...';
      btn.disabled = true;
      listEl.innerHTML = '<div class="loading-state"><div class="spinner"></div> Scanning nearby 2.4GHz networks...</div>';

      try {
        const res = await fetch('/scan');
        const networks = await res.json();

        if (!networks || networks.length === 0) {
          listEl.innerHTML = '<div class="loading-state">No Wi-Fi networks found. Enter SSID manually below.</div>';
          btn.innerHTML = '&#128257; Rescan';
          btn.disabled = false;
          return;
        }

        // Sort descending by signal strength (RSSI)
        networks.sort((a, b) => b.rssi - a.rssi);

        let html = '';
        networks.forEach(net => {
          if (!net.ssid || net.ssid.trim() === '') return;
          const qual = Math.min(Math.max(2 * (net.rssi + 100), 0), 100);
          const lock = net.secure ? '&#128274; Secured' : '&#128275; Open';
          const safeSsid = escapeHtml(net.ssid);

          html += `
            <div class="net-item" onclick="selectNetwork('${safeSsid}', this)">
              <div class="net-info">
                <span class="net-name">${safeSsid}</span>
                <span class="net-meta">
                  <span>${lock}</span>
                  <span class="net-badge">${qual}% (${net.rssi} dBm)</span>
                </span>
              </div>
              <span class="select-pill">Select</span>
            </div>
          `;
        });

        listEl.innerHTML = html;
      } catch (e) {
        listEl.innerHTML = '<div class="loading-state">Scan failed. Please type SSID manually below.</div>';
      }

      btn.innerHTML = '&#128257; Rescan';
      btn.disabled = false;
    }

    function selectNetwork(ssid, el) {
      document.getElementById('ssid_input').value = ssid;
      
      // Update active selection highlighting
      document.querySelectorAll('.net-item').forEach(item => item.classList.remove('active'));
      if (el) el.classList.add('active');

      // Automatically focus password input field
      const passInput = document.getElementById('pass_input');
      passInput.focus();
    }

    // Auto-scan on page load
    window.addEventListener('DOMContentLoaded', () => {
      scanNetworks();
    });
  </script>
</body>
</html>
)rawliteral";

// =========================================================================================
// 4. PREFERENCES (NVS) & FACTORY RESET
// =========================================================================================

void loadSavedConfig() {
  preferences.begin("agribridge", false);
  wifi_ssid      = preferences.getString("ssid", "");
  wifi_password  = preferences.getString("pass", "");
  firebase_url   = preferences.getString("fb_url", "https://agribridge-iot-default-rtdb.firebaseio.com");
  firebase_key   = preferences.getString("fb_key", "AIzaSyBMe0BjWk36VQypQM6thOG-CvnvP3rG5ZQ");
  sensor_node_id = preferences.getString("node_id", "sensor_node_1");
  preferences.end();

  Serial.println("[NVS] Configuration loaded from flash.");
  Serial.printf("[NVS] Target Node: %s, Saved SSID: %s\n", sensor_node_id.c_str(), wifi_ssid.c_str());
}

void saveConfig(String ssid, String pass, String fbUrl, String nodeId) {
  preferences.begin("agribridge", false);
  preferences.putString("ssid", ssid);
  preferences.putString("pass", pass);
  preferences.putString("fb_url", fbUrl);
  preferences.putString("node_id", nodeId);
  preferences.end();
  Serial.println("[NVS] New credentials saved to Flash. Rebooting...");
}

void checkHardwareResetButton() {
  // If button is held down (LOW when pressed with internal PULLUP)
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    unsigned long pressStart = millis();
    while (digitalRead(RESET_BUTTON_PIN) == LOW) {
      if (millis() - pressStart > 3000) {
        Serial.println("[RESET] Erasing WiFi settings & rebooting into AP mode...");
        for (int i = 0; i < 10; i++) {
          digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
          delay(100);
        }
        preferences.begin("agribridge", false);
        preferences.clear();
        preferences.end();
        ESP.restart();
      }
      delay(50);
    }
  }
}

// =========================================================================================
// 5. CAPTIVE PORTAL WEB SERVER HANDLERS & ASYNC SCAN ENDPOINT
// =========================================================================================

void handlePortalRoot() {
  String page = FPSTR(PORTAL_HTML);
  page.replace("{{CURRENT_SSID}}", wifi_ssid);
  page.replace("{{FB_URL}}", firebase_url);
  page.replace("{{NODE_ID}}", sensor_node_id);

  server.send(200, "text/html", page);
}

// Asynchronous / Interactive Wi-Fi Scanner API endpoint returning JSON
void handleScanJson() {
  Serial.println("[WiFi Scan] Scanning nearby 2.4GHz Wi-Fi networks...");
  int n = WiFi.scanNetworks(false, false); // Active scan

  String json = "[";
  for (int i = 0; i < n; ++i) {
    if (i > 0) json += ",";
    json += "{";
    json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
    json += "\"secure\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false");
    json += "}";
  }
  json += "]";

  Serial.printf("[WiFi Scan] Completed: %d networks found.\n", n);
  server.send(200, "application/json", json);
}

void handlePortalSave() {
  String newSsid   = server.arg("ssid");
  String newPass   = server.arg("password");
  String newFbUrl  = server.arg("fb_url");
  String newNodeId = server.arg("node_id");

  saveConfig(newSsid, newPass, newFbUrl, newNodeId);

  String response = "<!DOCTYPE html><html><body style='background:#0b1120;color:#fff;font-family:sans-serif;padding:30px;text-align:center;'>";
  response += "<h2 style='color:#22c55e;'>&#10004; Configuration Saved!</h2>";
  response += "<p style='color:#94a3b8;margin-top:12px;'>Connecting to <strong>" + newSsid + "</strong>... The gateway is now rebooting.</p>";
  response += "</body></html>";
  server.send(200, "text/html", response);
  delay(1500);
  ESP.restart();
}

void startCaptivePortal() {
  isAPMode = true;
  Serial.println("\n=======================================================");
  Serial.println("  STARTING WLED-STYLE OPEN SETUP ACCESS POINT");
  Serial.printf("  SSID: %s (NO PASSWORD)\n", AP_SSID);
  Serial.println("  Open http://192.168.4.1 in your browser");
  Serial.println("=======================================================\n");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID); // Open AP with no password

  // Captive Portal DNS redirection: Redirect all domain lookups to 192.168.4.1
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/", handlePortalRoot);
  server.on("/scan", HTTP_GET, handleScanJson); // Wi-Fi scanner API
  server.on("/save", HTTP_POST, handlePortalSave);
  server.onNotFound(handlePortalRoot);          // Captive portal fallback
  server.begin();
}

// =========================================================================================
// 6. FIREBASE REALTIME DATABASE SYNC ENGINE (HTTPS REST)
// =========================================================================================

void pushTelemetryToFirebase(const String& jsonPayload) {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure(); // Skip certificate verification for speed & simplicity
  HTTPClient https;

  String endpoint = firebase_url;
  if (!endpoint.endsWith("/")) endpoint += "/";
  endpoint += sensor_node_id + ".json";

  if (https.begin(client, endpoint)) {
    https.addHeader("Content-Type", "application/json");
    int httpCode = https.PATCH(jsonPayload);

    if (httpCode == 200) {
      Serial.println("[Firebase] Telemetry synced to /" + sensor_node_id);
      digitalWrite(STATUS_LED_PIN, HIGH);
      delay(40);
      digitalWrite(STATUS_LED_PIN, LOW);
    } else {
      Serial.printf("[Firebase Error] HTTP %d: %s\n", httpCode, https.getString().c_str());
    }
    https.end();
  }
}

void checkFirebaseActuation() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  String endpoint = firebase_url;
  if (!endpoint.endsWith("/")) endpoint += "/";
  endpoint += sensor_node_id + ".json";

  if (https.begin(client, endpoint)) {
    int httpCode = https.GET();
    if (httpCode == 200) {
      String payload = https.getString();
      StaticJsonDocument<512> doc;
      DeserializationError err = deserializeJson(doc, payload);

      if (!err) {
        bool m1 = doc["motor1"]["status"] | false;
        bool m2 = doc["motor2"]["status"] | false;

        // If user changed motor status from Web or Android app, transmit LoRa actuation command
        if (m1 != last_known_m1_status || m2 != last_known_m2_status) {
          Serial.printf("[Actuation Event] Motor1: %d -> %d | Motor2: %d -> %d\n",
            last_known_m1_status, m1, last_known_m2_status, m2);

          last_known_m1_status = m1;
          last_known_m2_status = m2;

          dispatchLoRaActuationPacket(m1, m2);
        }
      }
    }
    https.end();
  }
}

// =========================================================================================
// 7. LORA SX1278 TRANSCEIVER (FIELD NODE COMMUNICATION)
// =========================================================================================

void setupLoRa() {
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("[ERROR] LoRa SX1278 radio initialization failed!");
  } else {
    Serial.printf("[SUCCESS] LoRa transceiver listening at %.0f MHz\n", LORA_BAND / 1E6);
    // IMPORTANT: These three settings MUST match the RP2040 field node exactly.
    // RP2040 uses sync word 0x12 (SX127x default), SF7, BW 125 kHz.
    LoRa.setSyncWord(LORA_SYNC_WORD);           // 0x12 — matches RP2040
    LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR); // SF7
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);    // 125 kHz
  }
}

// -----------------------------------------------------------------------------------------
// Actuation: build and send plain-text LoRa relay command to RP2040 field node.
//
// The RP2040 parses commands in the form:
//   "R1 ON"  /  "R1 OFF"  /  "R2 ON"  /  "R2 OFF"
//   or combined: "R1 ON,R2 OFF"
//
// Both relays are sent in a single packet to minimise radio air-time.
// -----------------------------------------------------------------------------------------
void dispatchLoRaActuationPacket(bool m1, bool m2) {
  // Build plain-text command that the RP2040 field node expects
  String cmd = "R1 ";
  cmd += m1 ? "ON" : "OFF";
  cmd += ",R2 ";
  cmd += m2 ? "ON" : "OFF";

  LoRa.beginPacket();
  LoRa.print(cmd);
  LoRa.endPacket();

  Serial.println("[LoRa TX] Actuation command sent: " + cmd);
}

// -----------------------------------------------------------------------------------------
// CSV Parser helpers
// Returns the value string for a given key in the RP2040 CSV packet.
// Packet format example:
//   node=RP2040,v=230.1V,i=1.25A,soil=46.0%,temp=23.0C,humidity=45.0%,r1=ON,r2=OFF,alert=NONE
// -----------------------------------------------------------------------------------------

// Extracts the raw value string for a key, e.g. key="v" returns "230.1V"
// Returns empty String if the key is not found.
String csvGetValue(const String& packet, const String& key) {
  String search = key + "=";
  int start = packet.indexOf(search);
  if (start == -1) return "";
  start += search.length();
  int end = packet.indexOf(',', start);
  if (end == -1) end = packet.length();
  return packet.substring(start, end);
}

// Strips non-numeric trailing characters (e.g. "230.1V" → "230.1", "46.0%" → "46.0")
// Handles "NA" gracefully — returns "NA" unchanged.
String csvStripUnit(const String& raw) {
  if (raw == "NA" || raw == "") return raw;
  // Keep digits, a leading minus sign, and the decimal point only
  String out = "";
  for (int i = 0; i < (int)raw.length(); i++) {
    char c = raw.charAt(i);
    if (isdigit(c) || c == '.' || (c == '-' && i == 0)) {
      out += c;
    }
  }
  return out;
}

// -----------------------------------------------------------------------------------------
// Incoming LoRa telemetry processor.
//
// Receives CSV packet from the RP2040 field node, parses all fields, then pushes
// a structured JSON payload to Firebase Realtime Database.
//
// Firebase schema pushed (under sensor_node_id/):
//   motor1   : { status, voltage, current, power }
//   motor2   : { status, voltage, current, power }
//   environment : { temperature, humidity, soil }
//   alerts   : { active, last_updated }
//   connection  : { esp32_online, lora_link_active, last_field_packet_ts,
//                   last_firebase_push_ts, lora_rssi }
// -----------------------------------------------------------------------------------------
void processIncomingLoRaTelemetry() {
  int packetSize = LoRa.parsePacket();
  if (!packetSize) return;

  // Read the raw packet string
  String incoming = "";
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
  incoming.trim();

  int rssi = LoRa.packetRssi();
  Serial.printf("[LoRa RX] RSSI: %d dBm | Raw: %s\n", rssi, incoming.c_str());

  // ---- Basic validation: must contain "node=" ----
  if (incoming.indexOf("node=") == -1) {
    Serial.println("[LoRa RX] Ignored: packet does not look like an RP2040 telemetry frame.");
    return;
  }

  // ---- Parse all CSV fields ----
  String nodeId    = csvGetValue(incoming, "node");
  String rawV      = csvStripUnit(csvGetValue(incoming, "v"));       // AC voltage e.g. "230.1"
  String rawI      = csvStripUnit(csvGetValue(incoming, "i"));       // Current e.g. "1.25"
  String rawSoil   = csvStripUnit(csvGetValue(incoming, "soil"));    // Soil % e.g. "46.0"
  String rawTemp   = csvStripUnit(csvGetValue(incoming, "temp"));    // Temp e.g. "23.0" or "NA"
  String rawHum    = csvStripUnit(csvGetValue(incoming, "humidity")); // Humidity "45.0" or "NA"
  String r1State   = csvGetValue(incoming, "r1");                    // "ON" or "OFF"
  String r2State   = csvGetValue(incoming, "r2");                    // "ON" or "OFF"
  String alertStr  = csvGetValue(incoming, "alert");                 // "NONE" or "HIGH_CURRENT|LOW_VOLTAGE"

  // Convert relay state strings to booleans
  bool r1On = r1State.equalsIgnoreCase("ON");
  bool r2On = r2State.equalsIgnoreCase("ON");

  // Compute instantaneous power (P = V × I) only when relay is ON and values are numeric
  float voltage = rawV.toFloat();
  float currentA = rawI.toFloat();

  String m1Voltage = rawV;
  String m1Current = r1On ? rawI : "0.0";
  String m1Power   = r1On ? String(voltage * currentA, 1) : "0.0";

  String m2Voltage = rawV;   // Field node shares one voltage measurement
  String m2Current = r2On ? rawI : "0.0";
  String m2Power   = r2On ? String(voltage * currentA, 1) : "0.0";

  // Timestamp (seconds since boot — Firebase will store this for staleness checks)
  unsigned long nowSec = millis() / 1000;

  // ---- Update LoRa link tracking ----
  lastLoRaPacketMs = millis();
  loraLinkActive = true;

  // ---- Build Firebase JSON payload ----
  // Using ArduinoJson v6 StaticJsonDocument
  StaticJsonDocument<768> fbDoc;

  JsonObject m1 = fbDoc.createNestedObject("motor1");
  m1["status"]  = r1On;
  m1["voltage"] = m1Voltage;
  m1["current"] = m1Current;
  m1["power"]   = m1Power;

  JsonObject m2 = fbDoc.createNestedObject("motor2");
  m2["status"]  = r2On;
  m2["voltage"] = m2Voltage;
  m2["current"] = m2Current;
  m2["power"]   = m2Power;

  JsonObject env = fbDoc.createNestedObject("environment");
  env["temperature"] = rawTemp;   // May be "NA" — web/app must handle gracefully
  env["humidity"]    = rawHum;    // May be "NA"
  env["soil"]        = rawSoil;   // Soil moisture percentage

  JsonObject alerts = fbDoc.createNestedObject("alerts");
  alerts["active"]       = alertStr.isEmpty() ? "NONE" : alertStr;
  JsonObject alTs = alerts.createNestedObject("last_updated");
  alTs[".sv"] = "timestamp";

  JsonObject conn = fbDoc.createNestedObject("connection");
  conn["esp32_online"]     = true;
  conn["lora_link_active"] = true;
  conn["lora_rssi"]        = rssi;

  // Use Firebase Server Value timestamp for accurate real-time clock tracking
  JsonObject pktTs = conn.createNestedObject("last_field_packet_ts");
  pktTs[".sv"] = "timestamp";

  JsonObject pushTs = conn.createNestedObject("last_firebase_push_ts");
  pushTs[".sv"] = "timestamp";

  String fbPayload;
  serializeJson(fbDoc, fbPayload);

  // ---- Push to Firebase ----
  pushTelemetryToFirebase(fbPayload);

  // ---- Keep local motor state cache in sync ----
  // This prevents a spurious re-actuation right after a packet arrives
  last_known_m1_status = r1On;
  last_known_m2_status = r2On;

  Serial.printf("[Parser] node=%s v=%s i=%s soil=%s temp=%s hum=%s r1=%s r2=%s alert=%s\n",
    nodeId.c_str(), rawV.c_str(), rawI.c_str(), rawSoil.c_str(),
    rawTemp.c_str(), rawHum.c_str(), r1State.c_str(), r2State.c_str(), alertStr.c_str());
}

// -----------------------------------------------------------------------------------------
// ESP32 heartbeat — pushes esp32_online=true to Firebase every 30 s even if
// no LoRa data arrives.  Lets the web/app detect an ESP32-offline condition.
// -----------------------------------------------------------------------------------------
void pushHeartbeat() {
  static unsigned long lastHeartbeatMs = 0;
  if (millis() - lastHeartbeatMs < 30000UL) return;
  lastHeartbeatMs = millis();

  // Check LoRa link watchdog
  bool linkActive = (millis() - lastLoRaPacketMs) < LORA_LINK_TIMEOUT_MS;
  if (loraLinkActive && !linkActive) {
    loraLinkActive = false;
    Serial.println("[LoRa] Link timeout — no packet for 60 s. Marking link inactive.");
  }

  if (WiFi.status() != WL_CONNECTED) return;

  unsigned long nowSec = millis() / 1000;

  StaticJsonDocument<256> hbDoc;
  JsonObject conn = hbDoc.createNestedObject("connection");
  conn["esp32_online"]     = true;
  conn["lora_link_active"] = linkActive;
  JsonObject pushTs = conn.createNestedObject("last_firebase_push_ts");
  pushTs[".sv"] = "timestamp";

  String payload;
  serializeJson(hbDoc, payload);
  pushTelemetryToFirebase(payload);

  Serial.printf("[Heartbeat] Pushed — LoRa link: %s\n", linkActive ? "ACTIVE" : "INACTIVE");
}

// =========================================================================================
// 8. ARDUINO SETUP & MAIN LOOP
// =========================================================================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  Serial.println("\n\n=======================================================");
  Serial.println("  AGRIBRIDGE ESP32 LORA-TO-CLOUD GATEWAY BOOTING");
  Serial.println("=======================================================");

  loadSavedConfig();
  setupLoRa();

  // If no SSID configured, launch Captive Portal immediately
  if (wifi_ssid == "") {
    startCaptivePortal();
    return;
  }

  // Attempt connection to Home WiFi
  Serial.printf("[WiFi] Connecting to %s", wifi_ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] CONNECTED! IP Address: " + WiFi.localIP().toString());
    digitalWrite(STATUS_LED_PIN, HIGH);
  } else {
    Serial.println("\n[WiFi] Connection failed! Starting Open Captive Portal for reconfiguration.");
    startCaptivePortal();
  }
}

void loop() {
  // Always monitor the hardware reset button
  checkHardwareResetButton();

  // Mode 1: If in Captive Portal AP Mode, handle DNS and HTTP requests
  if (isAPMode) {
    dnsServer.processNextRequest();
    server.handleClient();
    // Blink LED to indicate AP setup mode
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 400) {
      lastBlink = millis();
      digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
    }
    return;
  }

  // Mode 2: Station Mode - Normal Operation
  // A. Process incoming field telemetry packets from LoRa and push to Firebase
  processIncomingLoRaTelemetry();

  // B. Poll Firebase for motor actuation switch toggles from Web Dashboard / Android App
  if (millis() - lastActuationCheck >= ACTUATION_POLL_INTERVAL) {
    lastActuationCheck = millis();
    checkFirebaseActuation();
  }

  // C. Push ESP32 heartbeat + LoRa link watchdog every 30 s
  pushHeartbeat();
}
