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
import androidx.compose.material.icons.filled.BarChart
import androidx.compose.material.icons.filled.CalendarMonth
import androidx.compose.material.icons.filled.DateRange
import androidx.compose.material.icons.filled.Schedule
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
import com.agribridge.app.data.model.StatsTelemetry
import com.agribridge.app.ui.theme.AgriGreenLight
import com.agribridge.app.ui.theme.HumSkyBlue
import com.agribridge.app.ui.theme.Slate400
import com.agribridge.app.ui.theme.Slate500
import com.agribridge.app.ui.theme.Slate700
import com.agribridge.app.ui.theme.Slate800
import com.agribridge.app.ui.theme.Slate900
import com.agribridge.app.ui.theme.Slate950
import com.agribridge.app.ui.theme.StatPurple

@Composable
fun PowerStatsCard(
    stats: StatsTelemetry
) {
    Column(modifier = Modifier.fillMaxWidth()) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 4.dp, vertical = 4.dp),
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text(
                text = "POWER STATISTICS",
                fontSize = 11.sp,
                fontWeight = FontWeight.Bold,
                color = Slate400,
                letterSpacing = 1.sp
            )
            Text(
                text = "Meter Telemetry",
                fontSize = 11.sp,
                color = Slate400
            )
        }

        Box(
            modifier = Modifier
                .fillMaxWidth()
                .clip(RoundedCornerShape(16.dp))
                .background(Slate900)
                .border(1.dp, Slate700.copy(alpha = 0.5f), RoundedCornerShape(16.dp))
                .padding(16.dp)
        ) {
            Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
                // Header inside card
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Box(
                        modifier = Modifier
                            .size(36.dp)
                            .clip(RoundedCornerShape(8.dp))
                            .background(StatPurple.copy(alpha = 0.15f)),
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(
                            imageVector = Icons.Default.BarChart,
                            contentDescription = null,
                            tint = StatPurple,
                            modifier = Modifier.size(20.dp)
                        )
                    }

                    Spacer(modifier = Modifier.width(10.dp))

                    Column {
                        Text(
                            text = "Aggregated Energy Consumption",
                            style = MaterialTheme.typography.titleMedium,
                            fontWeight = FontWeight.Bold,
                            color = Color.White
                        )
                        Text(
                            text = "Cumulative reading from sensor node",
                            fontSize = 11.sp,
                            color = Slate400
                        )
                    }
                }

                // 3 Fields: Used Today, Week, Month
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    StatMetricBox(
                        modifier = Modifier.weight(1f),
                        icon = Icons.Default.Schedule,
                        title = "Used Today",
                        value = stats.today,
                        valueColor = StatPurple,
                        subtext = "24h Total"
                    )

                    StatMetricBox(
                        modifier = Modifier.weight(1f),
                        icon = Icons.Default.DateRange,
                        title = "Week",
                        value = stats.week,
                        valueColor = AgriGreenLight,
                        subtext = "7 Days"
                    )

                    StatMetricBox(
                        modifier = Modifier.weight(1f),
                        icon = Icons.Default.CalendarMonth,
                        title = "Month",
                        value = stats.month,
                        valueColor = HumSkyBlue,
                        subtext = "Billing"
                    )
                }
            }
        }
    }
}

@Composable
private fun StatMetricBox(
    modifier: Modifier = Modifier,
    icon: ImageVector,
    title: String,
    value: String,
    valueColor: Color,
    subtext: String
) {
    Box(
        modifier = modifier
            .clip(RoundedCornerShape(12.dp))
            .background(Slate950.copy(alpha = 0.7f))
            .border(0.5.dp, Slate800, RoundedCornerShape(12.dp))
            .padding(10.dp)
    ) {
        Column(
            modifier = Modifier.fillMaxWidth(),
            verticalArrangement = Arrangement.spacedBy(4.dp)
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = title,
                    fontSize = 10.sp,
                    fontWeight = FontWeight.SemiBold,
                    color = Slate400
                )
                Icon(
                    imageVector = icon,
                    contentDescription = null,
                    tint = valueColor.copy(alpha = 0.7f),
                    modifier = Modifier.size(12.dp)
                )
            }

            Text(
                text = value,
                fontSize = 15.sp,
                fontFamily = FontFamily.Monospace,
                fontWeight = FontWeight.ExtraBold,
                color = valueColor
            )

            Text(
                text = subtext,
                fontSize = 8.sp,
                color = Slate500
            )
        }
    }
}
