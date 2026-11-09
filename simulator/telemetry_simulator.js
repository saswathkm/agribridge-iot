#!/usr/bin/env node

/**
 * ============================================================================
 * AgriBridge IoT Ecosystem - Field & Gateway Simulator CLI
 * Simulates physical ESP32 field node, LoRa link, and Cloud Gateway
 * Path: sensor_node_1
 * ============================================================================
 */

import https from 'https';

// Optional database URL via CLI argument or environment variable
const DB_URL = process.env.FIREBASE_DATABASE_URL || process.argv[2] || "https://agribridge-iot-default-rtdb.firebaseio.com";

console.log('====================================================');
console.log('       AgriBridge IoT Field & Cloud Simulator       ');
console.log('====================================================');

let state = {
  motor1: {
    status: false,
    voltage: "230.5",
    current: "0.0",
    power: "0.0"
  },
  motor2: {
    status: true,
    voltage: "228.0",
    current: "3.5",
    power: "798.0"
  },
  environment: {
    temperature: "29.2",
    humidity: "68.0"
  },
  stats: {
    today: "5.2 kWh",
    week: "34.1 kWh",
    month: "142.0 kWh"
  }
};

let tick = 0;

function computeTick() {
  tick++;

  // Continuous Line Voltage variations
  const v1 = 230.0 + Math.sin(tick * 0.4) * 2.2 + (Math.random() - 0.5) * 0.6;
  const v2 = 228.0 + Math.cos(tick * 0.3) * 1.8 + (Math.random() - 0.5) * 0.5;

  // Environmental variations
  const temp = (29.0 + Math.sin(tick * 0.05) * 0.8).toFixed(1);
  const hum = (68.0 + Math.cos(tick * 0.04) * 1.5).toFixed(1);

  // Motor 1 conditional logic
  const m1On = state.motor1.status;
  const i1 = m1On ? (4.0 + Math.sin(tick * 0.5) * 0.25).toFixed(1) : "0.0";
  const p1 = m1On ? (v1 * parseFloat(i1)).toFixed(1) : "0.0";

  // Motor 2 conditional logic
  const m2On = state.motor2.status;
  const i2 = m2On ? (3.4 + Math.cos(tick * 0.6) * 0.20).toFixed(1) : "0.0";
  const p2 = m2On ? (v2 * parseFloat(i2)).toFixed(1) : "0.0";

  state.motor1.voltage = v1.toFixed(1);
  state.motor1.current = i1;
  state.motor1.power = p1;

  state.motor2.voltage = v2.toFixed(1);
  state.motor2.current = i2;
  state.motor2.power = p2;

  state.environment.temperature = temp;
  state.environment.humidity = hum;

  // Terminal telemetry printout
  process.stdout.write(
    `\r[TELEMETRY] M1: [${m1On ? 'ON ' : 'OFF'}] ${state.motor1.voltage}V | ${state.motor1.current}A | ${state.motor1.power}W  ||  M2: [${m2On ? 'ON ' : 'OFF'}] ${state.motor2.voltage}V | ${state.motor2.current}A | ${state.motor2.power}W  ||  Env: ${temp}°C, ${hum}%`
  );

  // If Firebase Database URL is configured, push updates via REST
  if (DB_URL) {
    pushToFirebase(DB_URL, state);
  }
}

function pushToFirebase(baseUrl, payload) {
  try {
    const cleanUrl = baseUrl.replace(/\/$/, '');
    const url = new URL(`${cleanUrl}/sensor_node_1.json`);

    const dataString = JSON.stringify(payload);
    const req = https.request(
      url,
      {
        method: 'PATCH',
        headers: {
          'Content-Type': 'application/json',
          'Content-Length': Buffer.byteLength(dataString)
        }
      },
      (res) => {
        // Handle response
      }
    );

    req.on('error', (e) => {
      // Ignore network noise in terminal
    });

    req.write(dataString);
    req.end();
  } catch (e) {
    // URL format issue
  }
}

console.log('Running continuous telemetry engine (1.5s interval)...');
console.log('Simulating Field Relays, LoRa SX1278 link, and Cloud Gateway.');
console.log('Press Ctrl+C to terminate.\n');

setInterval(computeTick, 1500);
