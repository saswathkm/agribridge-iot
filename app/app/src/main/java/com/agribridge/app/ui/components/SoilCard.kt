package com.agribridge.app.ui.components

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
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Grass
import androidx.compose.material3.Icon
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.agribridge.app.ui.theme.Slate400
import com.agribridge.app.ui.theme.Slate500
import com.agribridge.app.ui.theme.Slate700
import com.agribridge.app.ui.theme.Slate900

/**
 * Soil moisture card with a colour-coded linear progress bar.
 *
 * Colour bands:
 *   < 30%  → red   (too dry)
 *   30–80% → green (optimal)
 *   > 80%  → blue  (too wet)
 *
 * Handles "NA" values gracefully (shows "--").
 */
@Composable
fun SoilCard(soil: String) {
    val isNA   = soil == "NA" || soil.isBlank()
    val pct    = if (isNA) null else soil.replace("[^0-9.]".toRegex(), "").toFloatOrNull()?.coerceIn(0f, 100f)
    val fraction = (pct ?: 0f) / 100f

    // Colour selection
    val (barColor, iconBg, iconTint, rangeHint) = when {
        isNA || pct == null -> Quadruple(Color(0xFF475569), Color(0xFF1E293B), Slate400, "Sensor unavailable")
        pct < 30f           -> Quadruple(Color(0xFFF43F5E), Color(0xFF4C0519).copy(alpha = 0.4f), Color(0xFFFB7185), "Too dry — irrigate")
        pct > 80f           -> Quadruple(Color(0xFF38BDF8), Color(0xFF0C4A6E).copy(alpha = 0.4f), Color(0xFF7DD3FC), "Too wet — check drainage")
        else                -> Quadruple(Color(0xFF22C55E), Color(0xFF052E16).copy(alpha = 0.4f), Color(0xFF4ADE80), "Optimal: 30% – 80%")
    }

    Box(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(16.dp))
            .background(Slate900)
            .border(1.dp, Slate700.copy(alpha = 0.5f), RoundedCornerShape(16.dp))
            .padding(14.dp)
    ) {
        Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
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
                            .background(iconBg),
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(Icons.Default.Grass, contentDescription = null, tint = iconTint, modifier = Modifier.size(20.dp))
                    }
                    Spacer(Modifier.width(12.dp))
                    Column {
                        Text("Soil Moisture", fontWeight = FontWeight.Bold, color = Color.White, fontSize = 14.sp)
                        Text("Capacitive Field Sensor", fontSize = 11.sp, color = Slate400)
                    }
                }

                Column(horizontalAlignment = Alignment.End) {
                    Row(verticalAlignment = Alignment.Bottom) {
                        Text(
                            text = if (isNA) "--" else String.format("%.1f", pct),
                            fontSize = 22.sp,
                            fontFamily = FontFamily.Monospace,
                            fontWeight = FontWeight.ExtraBold,
                            color = barColor
                        )
                        if (!isNA) {
                            Spacer(Modifier.width(2.dp))
                            Text(text = "%", fontSize = 13.sp, fontWeight = FontWeight.SemiBold, color = barColor.copy(alpha = 0.8f), modifier = Modifier.padding(bottom = 2.dp))
                        }
                    }
                    Text(text = rangeHint, fontSize = 9.sp, color = Slate500)
                }
            }

            // Progress bar
            LinearProgressIndicator(
                progress = { fraction },
                modifier = Modifier.fillMaxWidth().height(7.dp).clip(RoundedCornerShape(4.dp)),
                color = barColor,
                trackColor = Slate700.copy(alpha = 0.4f),
                strokeCap = StrokeCap.Round
            )

            // Scale ticks
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                Text("0%",   fontSize = 9.sp, color = Color(0xFF334155))
                Text("30%",  fontSize = 9.sp, color = Color(0xFF7F1D1D).copy(alpha = 0.7f))
                Text("80%",  fontSize = 9.sp, color = Color(0xFF14532D).copy(alpha = 0.7f))
                Text("100%", fontSize = 9.sp, color = Color(0xFF334155))
            }
        }
    }
}

// Helper to destructure 4-item tuples
private data class Quadruple<A, B, C, D>(val first: A, val second: B, val third: C, val fourth: D)
