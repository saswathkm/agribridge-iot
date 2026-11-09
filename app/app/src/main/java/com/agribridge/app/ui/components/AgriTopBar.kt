package com.agribridge.app.ui.components

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Menu
import androidx.compose.material.icons.filled.Sensors
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
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
import com.agribridge.app.ui.theme.Slate800
import com.agribridge.app.ui.theme.Slate900
import com.agribridge.app.ui.theme.Slate950

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AgriTopBar(
    onMenuClick: () -> Unit,
    connectionStatusText: String,
    isConnectedLive: Boolean,
    lastSyncTime: String
) {
    TopAppBar(
        colors = TopAppBarDefaults.topAppBarColors(
            containerColor = Slate900,
            titleContentColor = Color.White
        ),
        navigationIcon = {
            IconButton(onClick = onMenuClick) {
                // Menu drawer icon (☰)
                Icon(
                    imageVector = Icons.Default.Menu,
                    contentDescription = "Open Drawer Menu",
                    tint = Color.White
                )
            }
        },
        title = {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Box(
                    modifier = Modifier
                        .size(34.dp)
                        .clip(RoundedCornerShape(8.dp))
                        .background(AgriGreenPrimary),
                    contentAlignment = Alignment.Center
                ) {
                    Icon(
                        imageVector = Icons.Default.Sensors,
                        contentDescription = "AgriBridge Logo",
                        tint = Color.White,
                        modifier = Modifier.size(20.dp)
                    )
                }

                Spacer(modifier = Modifier.width(10.dp))

                Column {
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Text(
                            text = "AgriBridge",
                            style = MaterialTheme.typography.titleMedium,
                            fontWeight = FontWeight.Bold,
                            color = Color.White
                        )
                        Spacer(modifier = Modifier.width(6.dp))
                        Box(
                            modifier = Modifier
                                .clip(RoundedCornerShape(4.dp))
                                .background(AgriGreenPrimary.copy(alpha = 0.2f))
                                .padding(horizontal = 5.dp, vertical = 2.dp)
                        ) {
                            Text(
                                text = "Node 1",
                                color = AgriGreenLight,
                                fontSize = 9.sp,
                                fontWeight = FontWeight.Bold
                            )
                        }
                    }
                    Text(
                        text = "IoT LoRa-to-Cloud Telemetry",
                        fontSize = 10.sp,
                        color = Slate400
                    )
                }
            }
        },
        actions = {
            Row(
                verticalAlignment = Alignment.CenterVertically,
                modifier = Modifier
                    .padding(end = 12.dp)
                    .clip(RoundedCornerShape(12.dp))
                    .background(if (isConnectedLive) Color(0xFF064E3B) else Color(0xFF1E3A8A))
                    .border(
                        1.dp,
                        if (isConnectedLive) AgriGreenLight.copy(alpha = 0.3f) else Color(0xFF60A5FA).copy(alpha = 0.3f),
                        RoundedCornerShape(12.dp)
                    )
                    .padding(horizontal = 8.dp, vertical = 4.dp)
            ) {
                Box(
                    modifier = Modifier
                        .size(6.dp)
                        .clip(CircleShape)
                        .background(if (isConnectedLive) AgriGreenLight else Color(0xFF60A5FA))
                )
                Spacer(modifier = Modifier.width(6.dp))
                Text(
                    text = if (isConnectedLive) "Live" else "Sim",
                    fontSize = 11.sp,
                    fontWeight = FontWeight.SemiBold,
                    color = Color.White
                )
            }
        }
    )
}
