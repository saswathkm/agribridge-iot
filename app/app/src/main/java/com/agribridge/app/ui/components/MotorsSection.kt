package com.agribridge.app.ui.components

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.agribridge.app.data.model.ProcessedMotorUiModel
import com.agribridge.app.ui.theme.Slate400

@Composable
fun MotorsSection(
    motor1: ProcessedMotorUiModel,
    motor2: ProcessedMotorUiModel,
    isMotor1Actuating: Boolean,
    isMotor2Actuating: Boolean,
    onToggleMotor1: () -> Unit,
    onToggleMotor2: () -> Unit
) {
    Column(modifier = Modifier.fillMaxWidth()) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 4.dp, vertical = 6.dp),
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text(
                text = "PUMP & MOTOR CONTROL",
                fontSize = 11.sp,
                fontWeight = FontWeight.Bold,
                color = Slate400,
                letterSpacing = 1.sp
            )
            Text(
                text = "2 Relays",
                fontSize = 11.sp,
                color = Slate400
            )
        }

        // Side-by-Side Cards for Motor 1 and Motor 2
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(10.dp)
        ) {
            MotorCard(
                modifier = Modifier.weight(1f),
                title = "Motor 1",
                motorData = motor1,
                isActuating = isMotor1Actuating,
                onToggle = onToggleMotor1
            )

            MotorCard(
                modifier = Modifier.weight(1f),
                title = "Motor 2",
                motorData = motor2,
                isActuating = isMotor2Actuating,
                onToggle = onToggleMotor2
            )
        }
    }
}
