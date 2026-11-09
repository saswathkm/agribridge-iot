package com.agribridge.app.ui.components

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Thermostat
import androidx.compose.material.icons.filled.WaterDrop
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.agribridge.app.data.model.EnvironmentTelemetry
import com.agribridge.app.ui.theme.HumSkyBlue
import com.agribridge.app.ui.theme.Slate400
import com.agribridge.app.ui.theme.Slate500
import com.agribridge.app.ui.theme.Slate700
import com.agribridge.app.ui.theme.Slate900
import com.agribridge.app.ui.theme.TempOrange

@Composable
fun EnvironmentalSection(
    environment: EnvironmentTelemetry
) {
    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 4.dp, vertical = 4.dp),
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text(
                text = "FIELD MICROCLIMATE",
                fontSize = 11.sp,
                fontWeight = FontWeight.Bold,
                color = Slate400,
                letterSpacing = 1.sp
            )
            Text(
                text = "DHT11 Sensor",
                fontSize = 11.sp,
                color = Slate400
            )
        }

        // Card 1: Temperature — shows "--" when DHT11 fails ("NA")
        EnvCardItem(
            icon = Icons.Default.Thermostat,
            iconTint = TempOrange,
            title = "Ambient Temperature",
            subtitle = "Field Canopy Sensor Probe",
            rawValue = environment.temperature,
            unit = "°C",
            valueColor = TempOrange,
            rangeHint = "Optimal: 24°C - 32°C"
        )

        // Card 2: Humidity — shows "--" when DHT11 fails ("NA")
        EnvCardItem(
            icon = Icons.Default.WaterDrop,
            iconTint = HumSkyBlue,
            title = "Relative Humidity",
            subtitle = "Air Moisture Level",
            rawValue = environment.humidity,
            unit = "%",
            valueColor = HumSkyBlue,
            rangeHint = "Target: 60% - 75%"
        )
    }
}

@Composable
private fun EnvCardItem(
    icon: ImageVector,
    iconTint: Color,
    title: String,
    subtitle: String,
    rawValue: String,
    unit: String,
    valueColor: Color,
    rangeHint: String
) {
    // Handle "NA" (DHT11 read failure) gracefully
    val isNA = rawValue == "NA" || rawValue.isBlank()
    val displayValue = if (isNA) "--" else rawValue
    val displayUnit  = if (isNA) "" else unit
    val displayColor = if (isNA) Slate500 else valueColor
    val displayHint  = if (isNA) "Sensor unavailable" else rangeHint

    Box(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(16.dp))
            .background(Slate900)
            .border(1.dp, Slate700.copy(alpha = 0.5f), RoundedCornerShape(16.dp))
            .padding(14.dp)
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Box(
                    modifier = Modifier
                        .size(38.dp)
                        .clip(RoundedCornerShape(10.dp))
                        .background(iconTint.copy(alpha = if (isNA) 0.05f else 0.15f)),
                    contentAlignment = Alignment.Center
                ) {
                    Icon(
                        imageVector = icon,
                        contentDescription = null,
                        tint = if (isNA) Slate500 else iconTint,
                        modifier = Modifier.size(20.dp)
                    )
                }

                Spacer(modifier = Modifier.width(12.dp))

                Column {
                    Text(
                        text = title,
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.Bold,
                        color = if (isNA) Slate400 else Color.White
                    )
                    Text(text = subtitle, fontSize = 11.sp, color = Slate400)
                }
            }

            Column(horizontalAlignment = Alignment.End) {
                Row(verticalAlignment = Alignment.Bottom) {
                    Text(
                        text = displayValue,
                        fontSize = 22.sp,
                        fontFamily = FontFamily.Monospace,
                        fontWeight = FontWeight.ExtraBold,
                        color = displayColor
                    )
                    if (displayUnit.isNotEmpty()) {
                        Spacer(modifier = Modifier.width(3.dp))
                        Text(
                            text = displayUnit,
                            fontSize = 13.sp,
                            fontWeight = FontWeight.SemiBold,
                            color = displayColor.copy(alpha = 0.8f),
                            modifier = Modifier.padding(bottom = 2.dp)
                        )
                    }
                }
                Text(text = displayHint, fontSize = 9.sp, color = Slate500)
            }
        }
    }
}
