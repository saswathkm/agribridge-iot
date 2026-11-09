package com.agribridge.app.ui.components

import androidx.compose.animation.animateColorAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Bolt
import androidx.compose.material.icons.filled.ElectricMeter
import androidx.compose.material.icons.filled.Power
import androidx.compose.material.icons.filled.Speed
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Switch
import androidx.compose.material3.SwitchDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.agribridge.app.data.model.ProcessedMotorUiModel
import com.agribridge.app.ui.theme.AgriGreenLight
import com.agribridge.app.ui.theme.AgriGreenPrimary
import com.agribridge.app.ui.theme.CurrentCyan
import com.agribridge.app.ui.theme.PowerEmerald
import com.agribridge.app.ui.theme.Slate400
import com.agribridge.app.ui.theme.Slate500
import com.agribridge.app.ui.theme.Slate700
import com.agribridge.app.ui.theme.Slate800
import com.agribridge.app.ui.theme.Slate900
import com.agribridge.app.ui.theme.Slate950
import com.agribridge.app.ui.theme.VoltageAmber

@Composable
fun MotorCard(
    modifier: Modifier = Modifier,
    title: String,
    motorData: ProcessedMotorUiModel,
    isActuating: Boolean,
    onToggle: () -> Unit
) {
    val isON = motorData.status

    val borderColor by animateColorAsState(
        targetValue = if (isON) AgriGreenLight.copy(alpha = 0.5f) else Slate700.copy(alpha = 0.5f),
        animationSpec = tween(durationMillis = 300),
        label = "MotorBorderColor"
    )

    val cardBgColor by animateColorAsState(
        targetValue = if (isON) Slate850Color else Slate900,
        animationSpec = tween(durationMillis = 300),
        label = "MotorBgColor"
    )

    Box(
        modifier = modifier
            .clip(RoundedCornerShape(16.dp))
            .background(cardBgColor)
            .border(1.dp, borderColor, RoundedCornerShape(16.dp))
            .padding(14.dp)
    ) {
        Column(
            modifier = Modifier.fillMaxWidth(),
            verticalArrangement = Arrangement.spacedBy(10.dp)
        ) {
            // Top Row: Motor Title + Status Badge & Interactive Switch (Top Right)
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Box(
                        modifier = Modifier
                            .size(32.dp)
                            .clip(RoundedCornerShape(8.dp))
                            .background(if (isON) AgriGreenPrimary.copy(alpha = 0.2f) else Slate800),
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(
                            imageVector = Icons.Default.Power,
                            contentDescription = null,
                            tint = if (isON) AgriGreenLight else Slate400,
                            modifier = Modifier.size(18.dp)
                        )
                    }

                    Spacer(modifier = Modifier.width(8.dp))

                    Column {
                        Text(
                            text = title,
                            style = MaterialTheme.typography.titleMedium,
                            fontWeight = FontWeight.Bold,
                            color = Color.White
                        )
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Box(
                                modifier = Modifier
                                    .size(5.dp)
                                    .clip(CircleShape)
                                    .background(if (isON) AgriGreenLight else Slate500)
                            )
                            Spacer(modifier = Modifier.width(4.dp))
                            Text(
                                text = if (isON) "RUNNING" else "OFF",
                                fontSize = 9.sp,
                                fontWeight = FontWeight.Bold,
                                color = if (isON) AgriGreenLight else Slate500
                            )
                        }
                    }
                }

                // Interactive Toggle Switch at top right (ON/OFF)
                if (isActuating) {
                    CircularProgressIndicator(
                        modifier = Modifier.size(24.dp),
                        strokeWidth = 2.dp,
                        color = AgriGreenLight
                    )
                } else {
                    Switch(
                        checked = isON,
                        onCheckedChange = { onToggle() },
                        colors = SwitchDefaults.colors(
                            checkedThumbColor = Color.White,
                            checkedTrackColor = AgriGreenPrimary,
                            uncheckedThumbColor = Slate400,
                            uncheckedTrackColor = Slate800,
                            uncheckedBorderColor = Slate700
                        )
                    )
                }
            }

            // Metric 1: Line Voltage (Continuous 24/7)
            TelemetryRow(
                icon = Icons.Default.Bolt,
                iconTint = VoltageAmber,
                label = "Voltage",
                sublabel = "Continuous",
                valueDisplay = motorData.voltageDisplay,
                valueColor = VoltageAmber,
                isActive = true
            )

            // Metric 2: Current (Displayed conditionally only when switch is ON)
            TelemetryRow(
                icon = Icons.Default.ElectricMeter,
                iconTint = CurrentCyan,
                label = "Current",
                sublabel = if (isON) "Active" else "Motor OFF",
                valueDisplay = motorData.currentDisplay,
                valueColor = if (isON) CurrentCyan else Slate500,
                isActive = isON
            )

            // Metric 3: Power (Dynamically computed V * I only when switch is ON)
            TelemetryRow(
                icon = Icons.Default.Speed,
                iconTint = PowerEmerald,
                label = "Power",
                sublabel = if (isON) "V × I" else "Motor OFF",
                valueDisplay = motorData.powerDisplay,
                valueColor = if (isON) PowerEmerald else Slate500,
                isActive = isON
            )
        }
    }
}

@Composable
private fun TelemetryRow(
    icon: ImageVector,
    iconTint: Color,
    label: String,
    sublabel: String,
    valueDisplay: String,
    valueColor: Color,
    isActive: Boolean
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(10.dp))
            .background(if (isActive) Slate950.copy(alpha = 0.6f) else Slate950.copy(alpha = 0.3f))
            .border(
                0.5.dp,
                if (isActive) Slate800 else Slate800.copy(alpha = 0.4f),
                RoundedCornerShape(10.dp)
            )
            .padding(horizontal = 10.dp, vertical = 7.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Row(verticalAlignment = Alignment.CenterVertically) {
            Box(
                modifier = Modifier
                    .size(24.dp)
                    .clip(RoundedCornerShape(6.dp))
                    .background(if (isActive) iconTint.copy(alpha = 0.15f) else Slate800),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    imageVector = icon,
                    contentDescription = null,
                    tint = if (isActive) iconTint else Slate500,
                    modifier = Modifier.size(14.dp)
                )
            }
            Spacer(modifier = Modifier.width(8.dp))
            Column {
                Text(
                    text = label,
                    fontSize = 11.sp,
                    fontWeight = FontWeight.SemiBold,
                    color = if (isActive) Color.White else Slate400
                )
                Text(
                    text = sublabel,
                    fontSize = 8.sp,
                    color = Slate500
                )
            }
        }

        Text(
            text = valueDisplay,
            fontSize = 14.sp,
            fontFamily = FontFamily.Monospace,
            fontWeight = FontWeight.Bold,
            color = valueColor
        )
    }
}

private val Slate850Color = Color(0xFF131D2E)
