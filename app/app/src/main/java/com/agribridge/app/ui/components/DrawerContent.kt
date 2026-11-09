package com.agribridge.app.ui.components

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.DeveloperBoard
import androidx.compose.material.icons.filled.Info
import androidx.compose.material.icons.filled.Podcasts
import androidx.compose.material.icons.filled.Sensors
import androidx.compose.material3.Divider
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.agribridge.app.ui.theme.AgriGreenLight
import com.agribridge.app.ui.theme.AgriGreenPrimary
import com.agribridge.app.ui.theme.Slate400
import com.agribridge.app.ui.theme.Slate500
import com.agribridge.app.ui.theme.Slate700
import com.agribridge.app.ui.theme.Slate800
import com.agribridge.app.ui.theme.Slate900
import com.agribridge.app.ui.theme.Slate950

@Composable
fun DrawerContent(
    onClose: () -> Unit,
    isConnectedLive: Boolean,
    connectionStatusText: String
) {
    Column(
        modifier = Modifier
            .fillMaxHeight()
            .width(300.dp)
            .background(Slate950)
            .padding(20.dp),
        verticalArrangement = Arrangement.SpaceBetween
    ) {
        Column(verticalArrangement = Arrangement.spacedBy(20.dp)) {
            // Header with close button
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Box(
                        modifier = Modifier
                            .size(36.dp)
                            .clip(RoundedCornerShape(8.dp))
                            .background(AgriGreenPrimary),
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(
                            imageVector = Icons.Default.Sensors,
                            contentDescription = null,
                            tint = Color.White,
                            modifier = Modifier.size(22.dp)
                        )
                    }

                    Spacer(modifier = Modifier.width(10.dp))

                    Column {
                        Text(
                            text = "AgriBridge",
                            style = MaterialTheme.typography.titleMedium,
                            fontWeight = FontWeight.Bold,
                            color = Color.White
                        )
                        Text(
                            text = "IoT Field Gateway v1.0",
                            fontSize = 11.sp,
                            color = Slate400
                        )
                    }
                }

                IconButton(onClick = onClose) {
                    Icon(
                        imageVector = Icons.Default.Close,
                        contentDescription = "Close Menu",
                        tint = Slate400
                    )
                }
            }

            HorizontalDivider(color = Slate800)

            // Section 1: Active Sensor Node Target
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                Text(
                    text = "ACTIVE SENSOR NODE",
                    fontSize = 10.sp,
                    fontWeight = FontWeight.Bold,
                    color = Slate400,
                    letterSpacing = 1.sp
                )

                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(12.dp))
                        .background(Slate900)
                        .border(1.dp, Slate800, RoundedCornerShape(12.dp))
                        .padding(12.dp)
                ) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Icon(
                                imageVector = Icons.Default.DeveloperBoard,
                                contentDescription = null,
                                tint = AgriGreenLight,
                                modifier = Modifier.size(18.dp)
                            )
                            Spacer(modifier = Modifier.width(8.dp))
                            Text(
                                text = "sensor_node_1",
                                fontFamily = FontFamily.Monospace,
                                fontSize = 13.sp,
                                fontWeight = FontWeight.SemiBold,
                                color = Color.White
                            )
                        }

                        Box(
                            modifier = Modifier
                                .clip(RoundedCornerShape(6.dp))
                                .background(AgriGreenPrimary.copy(alpha = 0.2f))
                                .padding(horizontal = 6.dp, vertical = 2.dp)
                        ) {
                            Text(
                                text = "ONLINE",
                                fontSize = 9.sp,
                                fontWeight = FontWeight.Bold,
                                color = AgriGreenLight
                            )
                        }
                    }
                }
            }

            // Section 2: Hardware & Telemetry Diagnostics
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                Text(
                    text = "HARDWARE HEALTH",
                    fontSize = 10.sp,
                    fontWeight = FontWeight.Bold,
                    color = Slate400,
                    letterSpacing = 1.sp
                )

                DiagnosticRow(label = "ESP32 Field Node", status = "230V Mains OK", isHealthy = true)
                DiagnosticRow(label = "LoRa SX1278 (868MHz)", status = "-88 dBm", isHealthy = true)
                DiagnosticRow(label = "Firebase Realtime DB", status = connectionStatusText, isHealthy = isConnectedLive)
            }
        }

        // Footer info
        Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
            HorizontalDivider(color = Slate800)
            Text(
                text = "AgriBridge Mobile Client",
                fontSize = 11.sp,
                fontWeight = FontWeight.Medium,
                color = Slate400
            )
            Text(
                text = "Fully Synchronized with Web Dashboard",
                fontSize = 10.sp,
                color = Slate500
            )
        }
    }
}

@Composable
private fun DiagnosticRow(label: String, status: String, isHealthy: Boolean) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(8.dp))
            .background(Slate900.copy(alpha = 0.5f))
            .padding(horizontal = 10.dp, vertical = 8.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(text = label, fontSize = 11.sp, color = Slate400)
        Text(
            text = status,
            fontSize = 11.sp,
            fontFamily = FontFamily.Monospace,
            fontWeight = FontWeight.SemiBold,
            color = if (isHealthy) AgriGreenLight else Color(0xFF60A5FA)
        )
    }
}
