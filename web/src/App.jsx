import React, { useState } from 'react';
import TopBar from './components/TopBar';
import ConnectionStatusBar from './components/ConnectionStatusBar';
import MotorsSection from './components/MotorsSection';
import EnvironmentalSection from './components/EnvironmentalSection';
import SoilCard from './components/SoilCard';
import PowerStatsCard from './components/PowerStatsCard';
import AlertsBanner from './components/AlertsBanner';
import DrawerMenu from './components/DrawerMenu';
import FirebaseConfigModal from './components/FirebaseConfigModal';
import { useAgriBridgeData } from './hooks/useAgriBridgeData';
import { Info } from 'lucide-react';

export default function App() {
  const [drawerOpen, setDrawerOpen] = useState(false);
  const [settingsOpen, setSettingsOpen] = useState(false);

  const {
    motor1,
    motor2,
    environment,
    alerts,
    connection,
    stats,
    connectionStatus,
    lastSyncTime,
    isActuating,
    errorMsg,
    toggleMotor,
    setMode,
    mode
  } = useAgriBridgeData();

  return (
    <div className="min-h-screen bg-slate-950 text-slate-100 flex flex-col font-sans">

      {/* Top Bar */}
      <TopBar
        onOpenDrawer={() => setDrawerOpen(true)}
        connectionStatus={connectionStatus}
        lastSyncTime={lastSyncTime}
      />

      {/* Connection Status Pills (3 indicators: ESP32, LoRa, Stream) */}
      <ConnectionStatusBar connection={connection} />

      {/* Main Content */}
      <main className="flex-1 max-w-4xl w-full mx-auto px-4 sm:px-6 py-6 space-y-6">

        {/* Simulation mode notice */}
        {connectionStatus === 'simulated' && (
          <div className="bg-gradient-to-r from-blue-950/40 via-slate-900 to-slate-900 border border-blue-500/30 rounded-xl p-3 sm:p-4 text-xs flex items-center justify-between gap-3 text-blue-200">
            <div className="flex items-center space-x-2.5">
              <Info className="w-4 h-4 text-blue-400 flex-shrink-0" />
              <span>
                <strong>Field Simulation Mode Active:</strong> Generating live IoT telemetry. Connect your Firebase credentials in the menu to sync with live hardware.
              </span>
            </div>
            <button
              onClick={() => setSettingsOpen(true)}
              className="text-xs font-semibold px-2.5 py-1 bg-blue-500/20 hover:bg-blue-500/30 text-blue-300 rounded-lg border border-blue-500/40 whitespace-nowrap transition-colors"
            >
              Config RTDB
            </button>
          </div>
        )}

        {/* Active Alerts Banner (auto-dismisses after 8s) */}
        <AlertsBanner alerts={alerts} />

        {/* 1. Motors Section */}
        <MotorsSection
          motor1={motor1}
          motor2={motor2}
          onToggleMotor={toggleMotor}
          isActuating={isActuating}
        />

        {/* 2. Environmental Cards (Temp + Humidity) */}
        <EnvironmentalSection environment={environment} />

        {/* 3. Soil Moisture Card (separate card) */}
        <SoilCard soil={environment?.soil} />

        {/* 4. Power Statistics Card */}
        <PowerStatsCard stats={stats} />

      </main>

      {/* Footer */}
      <footer className="border-t border-slate-900 bg-slate-950 py-4 text-center text-xs text-slate-500">
        <div className="max-w-4xl mx-auto px-4 flex flex-col sm:flex-row items-center justify-between gap-2">
          <span>AgriBridge IoT Agricultural Monitoring &amp; Control Ecosystem</span>
          <span className="font-mono text-[11px] text-slate-600">RTDB Path: /sensor_node_1</span>
        </div>
      </footer>

      {/* Menu Drawer */}
      <DrawerMenu
        isOpen={drawerOpen}
        onClose={() => setDrawerOpen(false)}
        connectionStatus={connectionStatus}
        mode={mode}
        setMode={setMode}
        onOpenSettings={() => setSettingsOpen(true)}
      />

      {/* Firebase Config Modal */}
      <FirebaseConfigModal
        isOpen={settingsOpen}
        onClose={() => setSettingsOpen(false)}
        onSave={(cfg) => { console.log('Firebase config updated:', cfg); }}
      />

    </div>
  );
}
