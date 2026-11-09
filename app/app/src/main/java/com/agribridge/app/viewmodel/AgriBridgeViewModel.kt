package com.agribridge.app.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.agribridge.app.data.model.AlertsTelemetry
import com.agribridge.app.data.model.ConnectionUiState
import com.agribridge.app.data.model.EnvironmentTelemetry
import com.agribridge.app.data.model.ProcessedMotorUiModel
import com.agribridge.app.data.model.StatsTelemetry
import com.agribridge.app.data.repository.AgriBridgeRepository
import com.agribridge.app.data.repository.ConnectionMode
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.catch
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

data class AgriBridgeUiState(
    val motor1: ProcessedMotorUiModel = ProcessedMotorUiModel.from(null),
    val motor2: ProcessedMotorUiModel = ProcessedMotorUiModel.from(null),
    val environment: EnvironmentTelemetry = EnvironmentTelemetry(),
    val alerts: AlertsTelemetry = AlertsTelemetry(),
    val connection: ConnectionUiState = ConnectionUiState(),
    val stats: StatsTelemetry = StatsTelemetry(),
    val isMotor1Actuating: Boolean = false,
    val isMotor2Actuating: Boolean = false,
    val connectionStatusText: String = "Connecting...",
    val isConnectedLive: Boolean = false,
    val lastSyncFormatted: String = "--:--:--",
    val errorMessage: String? = null
)

class AgriBridgeViewModel(
    private val repository: AgriBridgeRepository = AgriBridgeRepository()
) : ViewModel() {

    private val _uiState = MutableStateFlow(AgriBridgeUiState())
    val uiState: StateFlow<AgriBridgeUiState> = _uiState.asStateFlow()

    private val timeFormat = SimpleDateFormat("HH:mm:ss", Locale.getDefault())

    init {
        startTelemetryStream()
    }

    private fun startTelemetryStream() {
        viewModelScope.launch {
            repository.observeSensorNode()
                .catch { error ->
                    _uiState.update { current ->
                        current.copy(
                            connectionStatusText = "Offline / Error",
                            isConnectedLive = false,
                            errorMessage = error.message
                        )
                    }
                }
                .collect { (nodeData, mode) ->
                    val m1Processed = ProcessedMotorUiModel.from(nodeData.motor1)
                    val m2Processed = ProcessedMotorUiModel.from(nodeData.motor2)
                    val connUiState = ConnectionUiState.from(nodeData.connection)
                    val timeStr = timeFormat.format(Date())

                    _uiState.update { current ->
                        current.copy(
                            motor1 = m1Processed,
                            motor2 = m2Processed,
                            environment = nodeData.environment,
                            alerts = nodeData.alerts,
                            connection = connUiState,
                            stats = nodeData.stats,
                            isConnectedLive = (mode is ConnectionMode.FirebaseLive),
                            connectionStatusText = if (mode is ConnectionMode.FirebaseLive) "Firebase Live" else "Simulated Node",
                            lastSyncFormatted = timeStr,
                            errorMessage = null
                        )
                    }
                }
        }
    }

    fun toggleMotor1() {
        val currentStatus = _uiState.value.motor1.status
        val nextStatus = !currentStatus
        val currentVoltage = _uiState.value.motor1.rawVoltage

        _uiState.update { it.copy(isMotor1Actuating = true) }

        viewModelScope.launch {
            try {
                repository.setMotorStatus("motor1", nextStatus, currentVoltage)
            } catch (e: Exception) {
                _uiState.update { it.copy(errorMessage = "Actuation failed: ${e.message}") }
            } finally {
                _uiState.update { it.copy(isMotor1Actuating = false) }
            }
        }
    }

    fun toggleMotor2() {
        val currentStatus = _uiState.value.motor2.status
        val nextStatus = !currentStatus
        val currentVoltage = _uiState.value.motor2.rawVoltage

        _uiState.update { it.copy(isMotor2Actuating = true) }

        viewModelScope.launch {
            try {
                repository.setMotorStatus("motor2", nextStatus, currentVoltage)
            } catch (e: Exception) {
                _uiState.update { it.copy(errorMessage = "Actuation failed: ${e.message}") }
            } finally {
                _uiState.update { it.copy(isMotor2Actuating = false) }
            }
        }
    }

    fun clearError() {
        _uiState.update { it.copy(errorMessage = null) }
    }
}
