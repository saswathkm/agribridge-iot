package com.agribridge.app.ui.components

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.animation.slideInVertically
import androidx.compose.animation.slideOutVertically
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Warning
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.agribridge.app.data.model.AlertsTelemetry
import kotlinx.coroutines.delay

/**
 * Displays active field-node alerts as a dismissible banner.
 * Auto-dismisses after 8 seconds. Re-appears when a new alert arrives.
 */
@Composable
fun AlertsBanner(alerts: AlertsTelemetry) {
    val activeStr = alerts.active
    val lastUpdated = alerts.lastUpdated

    var dismissed by remember { mutableStateOf(false) }
    var prevKey   by remember { mutableStateOf("") }

    // Re-show when a genuinely new alert string arrives
    val key = "$activeStr-$lastUpdated"
    LaunchedEffect(key) {
        if (key != prevKey) {
            dismissed = false
            prevKey = key
        }
    }

    // Auto-dismiss after 8 s
    LaunchedEffect(key, dismissed) {
        if (activeStr != "NONE" && !dismissed) {
            delay(8000)
            dismissed = true
        }
    }

    val shouldShow = activeStr != "NONE" && !dismissed

    AnimatedVisibility(
        visible = shouldShow,
        enter = slideInVertically { -it } + fadeIn(),
        exit  = slideOutVertically { -it } + fadeOut()
    ) {
        val codes = activeStr.split("|").filter { it.isNotBlank() }
        val hasCritical = codes.any { it == "HIGH_CURRENT" || it == "RELAY1_ON_BLOCKED" }

        val bg     = if (hasCritical) Color(0xFF4C0519).copy(alpha = 0.6f) else Color(0xFF422006).copy(alpha = 0.5f)
        val border = if (hasCritical) Color(0xFFF43F5E).copy(alpha = 0.4f) else Color(0xFFF59E0B).copy(alpha = 0.4f)
        val icon   = if (hasCritical) Color(0xFFFB7185)                    else Color(0xFFFBBF24)
        val text   = if (hasCritical) Color(0xFFFFE4E6)                    else Color(0xFFFEF3C7)

        Row(
            modifier = Modifier
                .fillMaxWidth()
                .clip(RoundedCornerShape(12.dp))
                .background(bg)
                .border(1.dp, border, RoundedCornerShape(12.dp))
                .padding(horizontal = 12.dp, vertical = 10.dp),
            verticalAlignment = Alignment.Top
        ) {
            Icon(
                imageVector = Icons.Default.Warning,
                contentDescription = null,
                tint = icon,
                modifier = Modifier.size(18.dp).padding(top = 1.dp)
            )
            Spacer(Modifier.width(10.dp))
            Column(modifier = Modifier.weight(1f), verticalArrangement = Arrangement.spacedBy(2.dp)) {
                Text(
                    text = "Field Node Alert${if (codes.size > 1) "s" else ""}",
                    fontSize = 11.sp,
                    fontWeight = FontWeight.Bold,
                    color = text,
                    letterSpacing = 0.5.sp
                )
                codes.forEach { code ->
                    Text(
                        text = "• ${alertLabel(code)}",
                        fontSize = 11.sp,
                        color = text.copy(alpha = 0.85f),
                        lineHeight = 15.sp
                    )
                }
            }
            IconButton(
                onClick = { dismissed = true },
                modifier = Modifier.size(24.dp)
            ) {
                Icon(Icons.Default.Close, contentDescription = "Dismiss", tint = text.copy(alpha = 0.6f), modifier = Modifier.size(16.dp))
            }
        }
    }
}

private fun alertLabel(code: String) = when (code) {
    "HIGH_CURRENT"      -> "Overcurrent — Motor 1 tripped"
    "LOW_VOLTAGE"       -> "Low mains voltage detected"
    "SOIL_LOW"          -> "Soil moisture critically low"
    "SOIL_HIGH"         -> "Soil moisture too high"
    "RELAY1_ON_BLOCKED" -> "Motor 1 ON blocked (overcurrent)"
    "RELAY1_ON"         -> "Motor 1 turned ON"
    "RELAY1_OFF"        -> "Motor 1 turned OFF"
    "RELAY2_ON"         -> "Motor 2 turned ON"
    "RELAY2_OFF"        -> "Motor 2 turned OFF"
    else                -> code.replace("_", " ")
}
