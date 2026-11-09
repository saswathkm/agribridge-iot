package com.agribridge.app.data.repository

import com.agribridge.app.data.model.EnvironmentTelemetry
import com.agribridge.app.data.model.MotorTelemetry
import com.agribridge.app.data.model.SensorNodeData
import com.agribridge.app.data.model.StatsTelemetry
import com.google.firebase.FirebaseApp
import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.DatabaseError
import com.google.firebase.database.DatabaseReference
import com.google.firebase.database.FirebaseDatabase
import com.google.firebase.database.ValueEventListener
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.callbackFlow
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.launch
import kotlinx.coroutines.tasks.await
import java.util.Locale
import kotlin.math.cos
import kotlin.math.sin

sealed class ConnectionMode {
    object FirebaseLive : ConnectionMode()
    object Simulated : ConnectionMode()
}

class AgriBridgeRepository {

    private var database: FirebaseDatabase? = null
    private var nodeRef: DatabaseReference? = null

    companion object {
        const val FIREBASE_DATABASE_URL = "https://agribridge-iot-default-rtdb.firebaseio.com"
    }

    init {
        try {
            database = try {
                FirebaseDatabase.getInstance(FIREBASE_DATABASE_URL)
            } catch (e: Exception) {
                FirebaseDatabase.getInstance()
            }
            nodeRef = database?.getReference("sensor_node_1")
            // Keep synced offline with Firebase Realtime Database disk persistence
            nodeRef?.keepSynced(true)
        } catch (e: Exception) {
            e.printStackTrace()
            database = null
            nodeRef = null
        }
    }

    /**
     * Observe continuous live stream from Firebase Realtime Database under "sensor_node_1"
     */
    fun observeSensorNode(): Flow<Pair<SensorNodeData, ConnectionMode>> = callbackFlow {
        val targetRef = nodeRef

        if (targetRef == null) {
            // Fallback to internal simulation engine
            val simScope = CoroutineScope(Dispatchers.Default)
            val job = simScope.launch {
                var simState = SensorNodeData()
                var timeTick = 0L

                while (true) {
                    timeTick += 1500L
                    val v1 = 230.0 + sin(timeTick / 3000.0) * 2.5
                    val v2 = 228.0 + cos(timeTick / 4000.0) * 2.0
                    val m1On = simState.motor1.status
                    val m2On = simState.motor2.status

                    val i1 = if (m1On) (4.0 + sin(timeTick / 2000.0) * 0.3) else 0.0
                    val p1 = if (m1On) (v1 * i1) else 0.0

                    val i2 = if (m2On) (3.4 + cos(timeTick / 2500.0) * 0.2) else 0.0
                    val p2 = if (m2On) (v2 * i2) else 0.0

                    val temp = String.format(Locale.US, "%.1f", 29.0 + sin(timeTick / 15000.0) * 0.8)
                    val hum = String.format(Locale.US, "%.1f", 68.0 + cos(timeTick / 18000.0) * 1.5)

                    simState = SensorNodeData(
                        motor1 = MotorTelemetry(
                            status = m1On,
                            voltage = String.format(Locale.US, "%.1f", v1),
                            current = String.format(Locale.US, "%.1f", i1),
                            power = String.format(Locale.US, "%.1f", p1)
                        ),
                        motor2 = MotorTelemetry(
                            status = m2On,
                            voltage = String.format(Locale.US, "%.1f", v2),
                            current = String.format(Locale.US, "%.1f", i2),
                            power = String.format(Locale.US, "%.1f", p2)
                        ),
                        environment = EnvironmentTelemetry(
                            temperature = temp,
                            humidity = hum
                        ),
                        stats = StatsTelemetry(
                            today = "5.2 kWh",
                            week = "34.1 kWh",
                            month = "142.0 kWh"
                        )
                    )

                    trySend(Pair(simState, ConnectionMode.Simulated))
                    delay(1500L)
                }
            }

            awaitClose {
                job.cancel()
            }
            return@callbackFlow
        }

        // Live Firebase Realtime Database ValueEventListener
        val listener = object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                if (snapshot.exists()) {
                    val data = snapshot.getValue(SensorNodeData::class.java)
                    if (data != null) {
                        trySend(Pair(data, ConnectionMode.FirebaseLive))
                    }
                } else {
                    // Seed initial node schema if empty
                    val initial = SensorNodeData()
                    targetRef.setValue(initial)
                    trySend(Pair(initial, ConnectionMode.FirebaseLive))
                }
            }

            override fun onCancelled(error: DatabaseError) {
                // If permission denied or network fault, gracefully fall back
                close(error.toException())
            }
        }

        targetRef.addValueEventListener(listener)

        awaitClose {
            targetRef.removeEventListener(listener)
        }
    }

    /**
     * Actuate motor status: Writes explicit boolean value (true / false) to Firebase RTDB:
     * path: `sensor_node_1/motorX/status`
     */
    suspend fun setMotorStatus(motorKey: String, newStatus: Boolean, currentVoltage: Double = 230.0) {
        val targetRef = nodeRef
        if (targetRef != null) {
            try {
                val motorRef = targetRef.child(motorKey)
                val updates = mutableMapOf<String, Any>()
                updates["status"] = newStatus

                // Compute instantaneous power when turned ON
                if (newStatus) {
                    val nominalCurrent = if (motorKey == "motor1") 4.2 else 3.5
                    val power = currentVoltage * nominalCurrent
                    updates["current"] = String.format(Locale.US, "%.1f", nominalCurrent)
                    updates["power"] = String.format(Locale.US, "%.1f", power)
                } else {
                    updates["current"] = "0.0"
                    updates["power"] = "0.0"
                }

                motorRef.updateChildren(updates).await()
            } catch (e: Exception) {
                e.printStackTrace()
                throw e
            }
        }
    }
}
