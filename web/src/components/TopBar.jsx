import React from 'react';
import { Menu, Wifi, WifiOff, Cpu, Radio } from 'lucide-react';

export default function TopBar({ onOpenDrawer, connectionStatus, lastSyncTime }) {
  return (
    <header className="sticky top-0 z-30 bg-slate-900/90 backdrop-blur-md border-b border-slate-800 shadow-sm">
      <div className="max-w-4xl mx-auto px-4 sm:px-6 h-16 flex items-center justify-between">
        
        {/* Left: Menu Drawer Hamburger + App Title */}
        <div className="flex items-center space-x-3.5">
          <button
            onClick={onOpenDrawer}
            aria-label="Open menu drawer"
            className="p-2 -ml-2 rounded-lg text-slate-300 hover:text-white hover:bg-slate-800 focus:outline-none focus:ring-2 focus:ring-emerald-500/50 transition-colors"
          >
            {/* Standard ☰ Hamburger Icon */}
            <Menu className="w-6 h-6" />
          </button>

          <div className="flex items-center space-x-2.5">
            <div className="w-9 h-9 rounded-xl bg-gradient-to-tr from-emerald-600 to-green-400 flex items-center justify-center shadow-lg shadow-emerald-500/20">
              <Radio className="w-5 h-5 text-white animate-pulse" />
            </div>
            <div>
              <h1 className="text-xl font-bold tracking-tight text-white flex items-center gap-2">
                AgriBridge
                <span className="text-[10px] font-semibold tracking-wider uppercase px-2 py-0.5 rounded-full bg-emerald-500/10 text-emerald-400 border border-emerald-500/20">
                  Node 1
                </span>
              </h1>
              <p className="text-[11px] text-slate-400 font-medium">IoT LoRa-to-Cloud Telemetry</p>
            </div>
          </div>
        </div>

        {/* Right: Live Connection Heartbeat */}
        <div className="flex items-center space-x-3">
          <div className="hidden sm:flex flex-col items-end text-right">
            <span className="text-[11px] font-mono text-slate-400">
              {lastSyncTime ? lastSyncTime.toLocaleTimeString() : '--:--:--'}
            </span>
            <span className="text-[10px] font-semibold text-slate-500 uppercase tracking-wider">
              {connectionStatus === 'connected' ? 'Firebase Cloud' : connectionStatus === 'simulated' ? 'Offline Mode' : 'Connecting...'}
            </span>
          </div>

          <div 
            title={`Firebase Cloud Connection: ${connectionStatus}`}
            className={`flex items-center space-x-1.5 px-2.5 py-1 rounded-full text-xs font-medium border ${
              connectionStatus === 'connected'
                ? 'bg-slate-800 text-slate-300 border-slate-700'
                : connectionStatus === 'simulated'
                ? 'bg-blue-950/60 text-blue-300 border-blue-500/40'
                : 'bg-amber-950/60 text-amber-300 border-amber-500/40'
            }`}
          >
            <span className={`w-2 h-2 rounded-full ${
              connectionStatus === 'connected'
                ? 'bg-emerald-400'
                : connectionStatus === 'simulated'
                ? 'bg-blue-400'
                : 'bg-amber-400 animate-pulse'
            }`} />
            <span className="capitalize">{connectionStatus === 'connected' ? 'Cloud Sync' : connectionStatus}</span>
          </div>
        </div>

      </div>
    </header>
  );
}
