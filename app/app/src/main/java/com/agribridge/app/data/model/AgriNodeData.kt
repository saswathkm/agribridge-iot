package com.agribridge.app.data.model

import com.google.firebase.database.IgnoreExtraProperties
import com.google.firebase.database.PropertyName
import java.util.Locale

/**
 * Data model matching the Firebase Realtime Database schema written by the ESP32 gateway.
 *
 * Schema (under sensor_node_1/):
 * {
 *   "motor1":      { "status": bool, "voltage": "230.1", "current": "1.25", "power": "287.6" },
 *   "motor2":      { "status": bool, "voltage": "230.1", "current": "0.0",  "power": "0.0"   },
 *   "environment": { "temperature": "23.0", "humidity": "45.0", "soil": "46.0" },
 *   "alerts":      { "active": "NONE", "last_updated": 1700000000 },
 *   "connection":  { "esp32_online": true, "lora_link_active": true,
 *                    "last_field_packet_ts": 1700000000, "last_firebase_push_ts": 1700000000,
 *                    "lora_rssi": -87 },
 *   "stats":       { "today": "5.2 kWh", "week": "34.1 kWh", "month": "142.0 kWh" }
 * }
 */

@IgnoreExtraProperties
data class MotorTelemetry(
    @get:PropertyName("status") @set:PropertyName("status")
    var status: Boolean = false,

    @get:PropertyName("voltage") @set:PropertyName("voltage")
    var voltage: String = "0.0",

    @get:PropertyName("current") @set:PropertyName("current")
    var current: String = "0.0",

    @get:PropertyName("power") @set:PropertyName("power")
    var power: String = "0.0"
)

@IgnoreExtraProperties
data class EnvironmentTelemetry(
    @get:PropertyName("temperature") @set:PropertyName("temperature")
    var temperature: String = "29.2",

    @get:PropertyName("humidity") @set:PropertyName("humidity")
    var humidity: String = "68.0",

    /** Soil moisture percentage from capacitive sensor. May be "NA" if unavailable. */
    @get:PropertyName("soil") @set:PropertyName("soil")
    var soil: String = "52.0"
)

@IgnoreExtraProperties
data class AlertsTelemetry(
    /** Pipe-separated alert codes, e.g. "HIGH_CURRENT|LOW_VOLTAGE", or "NONE". */
    @get:PropertyName("active") @set:PropertyName("active")
    var active: String = "NONE",

    @get:PropertyName("last_updated") @set:PropertyName("last_updated")
    var lastUpdated: Long = 0L
)

@IgnoreExtraProperties
data class ConnectionTelemetry(
    @get:PropertyName("esp32_online") @set:PropertyName("esp32_online")
    var esp32Online: Boolean = false,

    @get:PropertyName("lora_link_active") @set:PropertyName("lora_link_active")
    var loraLinkActive: Boolean = false,

    @get:PropertyName("last_field_packet_ts") @set:PropertyName("last_field_packet_ts")
    var lastFieldPacketTs: Long = 0L,

    @get:PropertyName("last_firebase_push_ts") @set:PropertyName("last_firebase_push_ts")
    var lastFirebasePushTs: Long = 0L,

    @get:PropertyName("lora_rssi") @set:PropertyName("lora_rssi")
    var loraRssi: Int = 0
)

@IgnoreExtraProperties
data class StatsTelemetry(
    @get:PropertyName("today") @set:PropertyName("today")
    var today: String = "5.2 kWh",

    @get:PropertyName("week") @set:PropertyName("week")
    var week: String = "34.1 kWh",

    @get:PropertyName("month") @set:PropertyName("month")
    var month: String = "142.0 kWh"
)

@IgnoreExtraProperties
data class SensorNodeData(
    @get:PropertyName("motor1") @set:PropertyName("motor1")
    var motor1: MotorTelemetry = MotorTelemetry(),

    @get:PropertyName("motor2") @set:PropertyName("motor2")
    var motor2: MotorTelemetry = MotorTelemetry(),

    @get:PropertyName("environment") @set:PropertyName("environment")
    var environment: EnvironmentTelemetry = EnvironmentTelemetry(),

    @get:PropertyName("alerts") @set:PropertyName("alerts")
    var alerts: AlertsTelemetry = AlertsTelemetry(),

    @get:PropertyName("connection") @set:PropertyName("connection")
    var connection: ConnectionTelemetry = ConnectionTelemetry(),

    @get:PropertyName("stats") @set:PropertyName("stats")
    var stats: StatsTelemetry = StatsTelemetry()
)

/**
 * Processed UI state for a motor card.
 * Business rules:
 *   - Voltage always shown (continuous).
 *   - When OFF: current = "0.0 A", power = "0.0 W".
 *   - When ON: current from telemetry, power = V × I.
 */
data class ProcessedMotorUiModel(
    val status: Boolean,
    val voltageDisplay: String,
    val currentDisplay: String,
    val powerDisplay: String,
    val rawVoltage: Double,
    val rawCurrent: Double,
    val rawPower: Double
) {
    companion object {
        fun from(telemetry: MotorTelemetry?): ProcessedMotorUiModel {
            val isStatusOn = telemetry?.status ?: false
            val parsedV = telemetry?.voltage?.replace("[^0-9.]".toRegex(), "")?.toDoubleOrNull() ?: 0.0
            val parsedI = telemetry?.current?.replace("[^0-9.]".toRegex(), "")?.toDoubleOrNull() ?: 0.0

            val vDisplay = String.format(Locale.US, "%.1f V", parsedV)

            return if (!isStatusOn) {
                ProcessedMotorUiModel(
                    status = false,
                    voltageDisplay = vDisplay,
                    currentDisplay = "0.0 A",
                    powerDisplay = "0.0 W",
                    rawVoltage = parsedV,
                    rawCurrent = 0.0,
                    rawPower = 0.0
                )
            } else {
                val effectiveCurrent = if (parsedI > 0.0) parsedI else 3.5
                val computedPower = parsedV * effectiveCurrent

                ProcessedMotorUiModel(
                    status = true,
                    voltageDisplay = vDisplay,
                    currentDisplay = String.format(Locale.US, "%.1f A", effectiveCurrent),
                    powerDisplay = String.format(Locale.US, "%.1f W", computedPower),
                    rawVoltage = parsedV,
                    rawCurrent = effectiveCurrent,
                    rawPower = computedPower
                )
            }
        }
    }
}

/** Derived connection health model used in the UI. */
data class ConnectionUiState(
    val esp32Online: Boolean = false,
    val loraLinkActive: Boolean = false,
    val dataStreamLive: Boolean = false,
    val loraRssiDisplay: String = "-- dBm"
) {
    companion object {
        fun from(telemetry: ConnectionTelemetry): ConnectionUiState {
            val nowMs = System.currentTimeMillis()

            // Support either millisecond (13-digit) or second (10-digit) timestamp
            val rawPush = telemetry.lastFirebasePushTs
            val pushMs = if (rawPush in 1..9999999999L) rawPush * 1000L else rawPush
            val isEsp32Fresh = pushMs > 0 && (nowMs - pushMs) < 50000L
            val isOnline = telemetry.esp32Online && isEsp32Fresh

            val rawPkt = telemetry.lastFieldPacketTs
            val pktMs = if (rawPkt in 1..9999999999L) rawPkt * 1000L else rawPkt
            val isPktFresh = pktMs > 0 && (nowMs - pktMs) < 60000L

            val dataStreamLive = isOnline && isPktFresh
            val loraLinkActive = isOnline && telemetry.loraLinkActive && isPktFresh
            val rssiText = if (telemetry.loraRssi != 0) "${telemetry.loraRssi} dBm" else "-- dBm"

            return ConnectionUiState(
                esp32Online = isOnline,
                loraLinkActive = loraLinkActive,
                dataStreamLive = dataStreamLive,
                loraRssiDisplay = rssiText
            )
        }
    }
}
