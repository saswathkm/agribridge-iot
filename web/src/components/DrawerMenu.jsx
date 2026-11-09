import React from 'react';
import { X, Cpu, Radio, Shield, Settings, Activity, Database, RefreshCw, ExternalLink } from 'lucide-react';

export default function DrawerMenu({
  isOpen,
  onClose,
  connectionStatus,
  mode,
  setMode,
  onOpenSettings
}) {
  if (!isOpen) return null;

  return (
    <div className="fixed inset-0 z-40 overflow-hidden">
      {/* Backdrop */}
      <div
        onClick={onClose}
        className="fixed inset-0 bg-black/60 backdrop-blur-sm transition-opacity"
      />

      {/* Drawer Panel */}
      <div className="fixed inset-y-0 left-0 max-w-xs w-full bg-slate-900 border-r border-slate-800 shadow-2xl flex flex-col justify-between z-50 animate-slide-right">
        <div>
          {/* Header */}
          <div className="p-5 border-b border-slate-800 flex items-center justify-between">
            <div className="flex items-center space-x-3">
              <div className="w-9 h-9 rounded-xl bg-gradient-to-tr from-emerald-600 to-green-400 flex items-center justify-center text-white">
                <Radio className="w-5 h-5" />
              </div>
              <div>
                <h3 className="font-bold text-white text-base">AgriBridge</h3>
                <p className="text-xs text-slate-400">IoT Field Gateway v1.0</p>
              </div>
            </div>
            <button
              onClick={onClose}
              className="p-1.5 rounded-lg text-slate-400 hover:text-white hover:bg-slate-800"
            >
              <X className="w-5 h-5" />
            </button>
          </div>

          {/* Navigation & Status Content */}
          <div className="p-5 space-y-6">
            
            {/* Active Sensor Node Selection */}
            <div>
              <label className="text-xs font-semibold uppercase tracking-wider text-slate-400 block mb-2">
                Active Node Path
              </label>
              <div className="bg-slate-800/80 rounded-xl p-3 border border-slate-700/60 flex items-center justify-between">
                <div className="flex items-center space-x-2.5">
                  <Cpu className="w-4 h-4 text-emerald-400" />
                  <span className="text-sm font-mono font-medium text-white">sensor_node_1</span>
                </div>
                <span className="text-[10px] font-semibold text-emerald-400 uppercase bg-emerald-500/10 px-2 py-0.5 rounded-full border border-emerald-500/20">
                  Online
                </span>
              </div>
            </div>

            {/* Operating Mode Selector */}
            <div>
              <label className="text-xs font-semibold uppercase tracking-wider text-slate-400 block mb-2">
                Telemetry Source Mode
              </label>
              <div className="grid grid-cols-2 gap-2">
                <button
                  type="button"
                  onClick={() => setMode('auto')}
                  className={`px-3 py-2 text-xs font-medium rounded-lg border transition-all ${
                    mode === 'auto'
                      ? 'bg-emerald-500/20 text-emerald-300 border-emerald-500/50'
                      : 'bg-slate-800 text-slate-400 border-slate-700 hover:text-white'
                  }`}
                >
                  Firebase RTDB
                </button>
                <button
                  type="button"
                  onClick={() => setMode('simulator')}
                  className={`px-3 py-2 text-xs font-medium rounded-lg border transition-all ${
                    mode === 'simulator'
                      ? 'bg-blue-500/20 text-blue-300 border-blue-500/50'
                      : 'bg-slate-800 text-slate-400 border-slate-700 hover:text-white'
                  }`}
                >
                  Simulated Field
                </button>
              </div>
            </div>

            {/* Field Hardware Diagnostics */}
            <div>
              <label className="text-xs font-semibold uppercase tracking-wider text-slate-400 block mb-2">
                Hardware Health
              </label>
              <div className="space-y-2 text-xs">
                <div className="flex justify-between items-center text-slate-400 bg-slate-800/40 p-2.5 rounded-lg border border-slate-800">
                  <span>ESP32 Field Node</span>
                  <span className="text-emerald-400 font-mono font-semibold">230V Mains OK</span>
                </div>
                <div className="flex justify-between items-center text-slate-400 bg-slate-800/40 p-2.5 rounded-lg border border-slate-800">
                  <span>LoRa SX1278 (868MHz)</span>
                  <span className="text-emerald-400 font-mono font-semibold">-88 dBm (Good)</span>
                </div>
                <div className="flex justify-between items-center text-slate-400 bg-slate-800/40 p-2.5 rounded-lg border border-slate-800">
                  <span>Cloud Gateway Relay</span>
                  <span className="text-emerald-400 font-mono font-semibold">Ready</span>
                </div>
              </div>
            </div>

            {/* Firebase Config Action */}
            <div>
              <button
                onClick={() => {
                  onClose();
                  onOpenSettings();
                }}
                className="w-full flex items-center justify-between p-3 rounded-xl bg-slate-800 hover:bg-slate-750 text-slate-200 border border-slate-700 text-xs font-medium transition-colors"
              >
                <div className="flex items-center space-x-2.5">
                  <Database className="w-4 h-4 text-emerald-400" />
                  <span>Configure Firebase Keys</span>
                </div>
                <Settings className="w-4 h-4 text-slate-400" />
              </button>
            </div>

          </div>
        </div>

        {/* Drawer Footer */}
        <div className="p-5 border-t border-slate-800 bg-slate-900/50 text-[11px] text-slate-500">
          <p>AgriBridge Unified Client</p>
          <p className="font-mono text-slate-400">Android & React Web Synchronized</p>
        </div>
      </div>
    </div>
  );
}
