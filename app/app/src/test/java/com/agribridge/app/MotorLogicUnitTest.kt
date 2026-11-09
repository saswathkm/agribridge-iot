package com.agribridge.app

import com.agribridge.app.data.model.MotorTelemetry
import com.agribridge.app.data.model.ProcessedMotorUiModel
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class MotorLogicUnitTest {

    @Test
    fun testMotorOff_enforcesZeroCurrentAndZeroPower() {
        val offMotor = MotorTelemetry(
            status = false,
            voltage = "230.5",
            current = "4.2", // Even if stray sensor noise has a value
            power = "968.1"
        )

        val processed = ProcessedMotorUiModel.from(offMotor)

        assertFalse("Motor status must be false", processed.status)
        assertEquals("Continuous line voltage must still be shown when OFF", "230.5 V", processed.voltageDisplay)
        assertEquals("Current must strictly display 0.0 A when motor is OFF", "0.0 A", processed.currentDisplay)
        assertEquals("Power must strictly display 0.0 W when motor is OFF", "0.0 W", processed.powerDisplay)
    }

    @Test
    fun testMotorOn_computesDynamicPowerCorrectly() {
        val onMotor = MotorTelemetry(
            status = true,
            voltage = "228.0",
            current = "3.5",
            power = "0.0" // Input power uncalculated
        )

        val processed = ProcessedMotorUiModel.from(onMotor)

        assertTrue("Motor status must be true", processed.status)
        assertEquals("Voltage display matches", "228.0 V", processed.voltageDisplay)
        assertEquals("Current display matches active load", "3.5 A", processed.currentDisplay)
        // 228.0 * 3.5 = 798.0 W
        assertEquals("Power must be dynamically computed as Voltage * Current", "798.0 W", processed.powerDisplay)
    }

    @Test
    fun testContinuousVoltage_alwaysVisible() {
        val motorA = MotorTelemetry(status = false, voltage = "231.2", current = "0.0", power = "0.0")
        val processedA = ProcessedMotorUiModel.from(motorA)
        assertEquals("231.2 V", processedA.voltageDisplay)

        val motorB = MotorTelemetry(status = true, voltage = "227.4", current = "4.0", power = "909.6")
        val processedB = ProcessedMotorUiModel.from(motorB)
        assertEquals("227.4 V", processedB.voltageDisplay)
    }
}
