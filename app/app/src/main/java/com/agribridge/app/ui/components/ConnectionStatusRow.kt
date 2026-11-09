package com.agribridge.app.ui.components

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Sensors
import androidx.compose.material.icons.filled.SignalCellularAlt
import androidx.compose.material.icons.filled.Wifi
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.agribridge.app.data.model.ConnectionUiState
import com.agribridge.app.ui.theme.AgriGreenLight
import com.agribridge.app.ui.theme.Slate800

/**
 * Three-pill connection health row displayed below the TopBar.
 *
 * Indicators:
 *   1. ESP32 Cloud Bridge — is the gateway online?
 *   2. LoRa Link — is the field node reachable via radio?
 *   3. Data Stream — did a fresh packet arrive in the last 90 s?
 */
@Composable
fun ConnectionStatusRow(
    connection: ConnectionUiState,
    modifier: Modifier = Modifier
) {
    Row(
        modifier = modifier
            .fillMaxWidth()
            .background(Color(0xFF0F172A))
            .padding(horizontal = 16.dp, vertical = 6.dp),
        horizontalArrangement = Arrangement.spacedBy(8.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = "SYS",
            fontSize = 9.sp,
            fontWeight = FontWeight.Bold,
            color = Color(0xFF475569),
            letterSpacing = 1.sp,
            modifier = Modifier.padding(end = 2.dp)
        )

        StatusPill(
            icon = Icons.Default.Wifi,
            label = "ESP32",
            subLabel = if (connection.esp32Online) "Online" else "Offline",
            active = connection.esp32Online
        )

        StatusPill(
            icon = Icons.Default.SignalCellularAlt,
            label = "LoRa",
            subLabel = connection.loraRssiDisplay,
            active = connection.loraLinkActive
        )

        StatusPill(
            icon = Icons.Default.Sensors,
            label = "Stream",
            subLabel = if (connection.dataStreamLive) "Live" else "Stale",
            active = connection.dataStreamLive
        )
    }
}

@Composable
private fun StatusPill(
    icon: ImageVector,
    label: String,
    subLabel: String,
    active: Boolean
) {
    val bg     = if (active) Color(0xFF052E16).copy(alpha = 0.8f) else Color(0xFF1E293B)
    val border = if (active) AgriGreenLight.copy(alpha = 0.25f)  else Color(0xFF334155)
    val text   = if (active) AgriGreenLight                       else Color(0xFF64748B)
    val dot    = if (active) AgriGreenLight                       else Color(0xFF334155)

    Row(
        modifier = Modifier
            .clip(RoundedCornerShape(20.dp))
            .background(bg)
            .border(1.dp, border, RoundedCornerShape(20.dp))
            .padding(horizontal = 8.dp, vertical = 4.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Box(
            modifier = Modifier
                .size(5.dp)
                .clip(CircleShape)
                .background(dot)
        )
        Spacer(Modifier.width(4.dp))
        Icon(
            imageVector = icon,
            contentDescription = null,
            tint = text,
            modifier = Modifier.size(11.dp)
        )
        Spacer(Modifier.width(4.dp))
        Text(text = label, fontSize = 10.sp, fontWeight = FontWeight.SemiBold, color = text)
        Spacer(Modifier.width(3.dp))
        Text(text = "· $subLabel", fontSize = 9.sp, color = text.copy(alpha = 0.7f))
    }
}
