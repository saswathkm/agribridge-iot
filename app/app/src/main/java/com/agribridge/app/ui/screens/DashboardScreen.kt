package com.agribridge.app.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Info
import androidx.compose.material.icons.filled.Warning
import androidx.compose.material3.DrawerValue
import androidx.compose.material3.Icon
import androidx.compose.material3.ModalDrawerSheet
import androidx.compose.material3.ModalNavigationDrawer
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.rememberDrawerState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewmodel.compose.viewModel
import com.agribridge.app.ui.components.AgriTopBar
import com.agribridge.app.ui.components.AlertsBanner
import com.agribridge.app.ui.components.ConnectionStatusRow
import com.agribridge.app.ui.components.DrawerContent
import com.agribridge.app.ui.components.EnvironmentalSection
import com.agribridge.app.ui.components.MotorsSection
import com.agribridge.app.ui.components.PowerStatsCard
import com.agribridge.app.ui.components.SoilCard
import com.agribridge.app.ui.theme.AgriGreenLight
import com.agribridge.app.ui.theme.ErrorRose
import com.agribridge.app.ui.theme.Slate400
import com.agribridge.app.ui.theme.Slate950
import com.agribridge.app.viewmodel.AgriBridgeViewModel
import kotlinx.coroutines.launch

@Composable
fun DashboardScreen(
    viewModel: AgriBridgeViewModel = viewModel()
) {
    val uiState by viewModel.uiState.collectAsState()
    val drawerState = rememberDrawerState(initialValue = DrawerValue.Closed)
    val scope = rememberCoroutineScope()

    ModalNavigationDrawer(
        drawerState = drawerState,
        drawerContent = {
            ModalDrawerSheet(drawerContainerColor = Slate950) {
                DrawerContent(
                    onClose = { scope.launch { drawerState.close() } },
                    isConnectedLive = uiState.isConnectedLive,
                    connectionStatusText = uiState.connectionStatusText
                )
            }
        }
    ) {
        Scaffold(
            topBar = {
                Column {
                    AgriTopBar(
                        onMenuClick = { scope.launch { drawerState.open() } },
                        connectionStatusText = uiState.connectionStatusText,
                        isConnectedLive = uiState.isConnectedLive,
                        lastSyncTime = uiState.lastSyncFormatted
                    )
                    // 3-pill connection status row below the top bar
                    ConnectionStatusRow(connection = uiState.connection)
                }
            },
            containerColor = Slate950
        ) { paddingValues ->
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(paddingValues)
                    .padding(horizontal = 16.dp, vertical = 12.dp)
                    .verticalScroll(rememberScrollState()),
                verticalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                // Simulation mode info banner
                if (!uiState.isConnectedLive) {
                    Box(
                        modifier = Modifier
                            .fillMaxWidth()
                            .clip(RoundedCornerShape(12.dp))
                            .background(Color(0xFF172554).copy(alpha = 0.5f))
                            .border(1.dp, Color(0xFF3B82F6).copy(alpha = 0.3f), RoundedCornerShape(12.dp))
                            .padding(12.dp)
                    ) {
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Icon(Icons.Default.Info, contentDescription = null, tint = Color(0xFF60A5FA), modifier = Modifier.size(18.dp))
                            Spacer(Modifier.width(10.dp))
                            Text(
                                text = "Field Simulation Active: Live voltage variations & dynamic power computed. Toggle switches to actuate relays.",
                                fontSize = 11.sp,
                                color = Color(0xFFBFDBFE),
                                lineHeight = 16.sp
                            )
                        }
                    }
                }

                // Active alerts banner (auto-dismisses after 8 s)
                AlertsBanner(alerts = uiState.alerts)

                // Error message banner
                uiState.errorMessage?.let { error ->
                    Box(
                        modifier = Modifier
                            .fillMaxWidth()
                            .clip(RoundedCornerShape(12.dp))
                            .background(Color(0xFF4C0519).copy(alpha = 0.5f))
                            .border(1.dp, ErrorRose.copy(alpha = 0.4f), RoundedCornerShape(12.dp))
                            .padding(12.dp)
                    ) {
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Icon(Icons.Default.Warning, contentDescription = null, tint = ErrorRose, modifier = Modifier.size(18.dp))
                            Spacer(Modifier.width(10.dp))
                            Text(text = error, fontSize = 11.sp, color = Color(0xFFFECDD3))
                        }
                    }
                }

                // 1. Motors Section
                MotorsSection(
                    motor1 = uiState.motor1,
                    motor2 = uiState.motor2,
                    isMotor1Actuating = uiState.isMotor1Actuating,
                    isMotor2Actuating = uiState.isMotor2Actuating,
                    onToggleMotor1 = { viewModel.toggleMotor1() },
                    onToggleMotor2 = { viewModel.toggleMotor2() }
                )

                // 2. Environmental Cards (Temp + Humidity)
                EnvironmentalSection(environment = uiState.environment)

                // 3. Soil Moisture Card (separate card)
                SoilCard(soil = uiState.environment.soil)

                // 4. Power Statistics Card
                PowerStatsCard(stats = uiState.stats)

                Spacer(modifier = Modifier.height(16.dp))
            }
        }
    }
}
