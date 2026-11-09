import React from 'react';
import { Zap, Activity, Power, Gauge, ArrowUpRight } from 'lucide-react';

export default function MotorCard({
  title,
  motorId,
  motorData,
  onToggle,
  isActuating
}) {
  const isON = Boolean(motorData.status);

  return (
    <div
      className={`relative flex flex-col justify-between rounded-2xl p-5 sm:p-6 transition-all duration-300 border ${
        isON
          ? 'bg-gradient-to-b from-slate-800/90 to-slate-900/95 border-emerald-500/40 shadow-lg shadow-emerald-500/10 ring-1 ring-emerald-500/30'
          : 'bg-gradient-to-b from-slate-800/50 to-slate-900/70 border-slate-700/60 shadow-sm'
      }`}
    >
      {/* Top Row: Motor Title & Interactive Switch */}
      <div className="flex items-center justify-between gap-2 mb-4">
        <div className="flex items-center space-x-2.5">
          <div
            className={`w-10 h-10 rounded-xl flex items-center justify-center transition-colors ${
              isON ? 'bg-emerald-500/20 text-emerald-400' : 'bg-slate-700/50 text-slate-400'
            }`}
          >
            <Power className={`w-5 h-5 ${isON ? 'animate-pulse' : ''}`} />
          </div>
          <div>
            <h2 className="text-base sm:text-lg font-bold text-white tracking-tight flex items-center gap-1.5">
              {title}
            </h2>
            <span
              className={`inline-flex items-center gap-1 text-[11px] font-semibold tracking-wider uppercase ${
                isON ? 'text-emerald-400' : 'text-slate-400'
              }`}
            >
              <span className={`w-1.5 h-1.5 rounded-full ${isON ? 'bg-emerald-400' : 'bg-slate-500'}`} />
              {isON ? 'Running' : 'Stopped'}
            </span>
          </div>
        </div>

        {/* Interactive Toggle Switch */}
        <div className="flex items-center space-x-2">
          <button
            type="button"
            role="switch"
            aria-checked={isON}
            disabled={isActuating}
            onClick={() => onToggle(motorId)}
            className={`relative inline-flex h-7 w-13 flex-shrink-0 cursor-pointer rounded-full border-2 border-transparent transition-colors duration-200 ease-in-out focus:outline-none focus:ring-2 focus:ring-emerald-500 focus:ring-offset-2 focus:ring-offset-slate-900 ${
              isON ? 'bg-emerald-500' : 'bg-slate-700'
            } ${isActuating ? 'opacity-60 cursor-not-allowed' : ''}`}
          >
            <span className="sr-only">Toggle {title}</span>
            <span
              aria-hidden="true"
              className={`pointer-events-none inline-block h-6 w-6 transform rounded-full bg-white shadow-lg ring-0 transition duration-200 ease-in-out ${
                isON ? 'translate-x-6' : 'translate-x-0'
              }`}
            />
          </button>
        </div>
      </div>

      {/* Telemetry Metrics Grid */}
      <div className="space-y-3.5">
        
        {/* 1. Continuous Voltage Telemetry (Always Live) */}
        <div className="bg-slate-900/80 rounded-xl p-3.5 border border-slate-800 flex items-center justify-between">
          <div className="flex items-center space-x-2.5">
            <div className="w-8 h-8 rounded-lg bg-amber-500/10 text-amber-400 flex items-center justify-center">
              <Zap className="w-4 h-4" />
            </div>
            <div>
              <p className="text-[11px] font-medium text-slate-400 uppercase tracking-wider">Line Voltage</p>
              <p className="text-[10px] text-slate-500">Continuous 24/7</p>
            </div>
          </div>
          <div className="text-right">
            <span className="text-xl sm:text-2xl font-bold font-mono text-amber-300">
              {motorData.voltageDisplay}
            </span>
          </div>
        </div>

        {/* 2. Conditional Current (Only displayed when status is ON) */}
        <div
          className={`rounded-xl p-3.5 border transition-all duration-300 flex items-center justify-between ${
            isON
              ? 'bg-slate-900/80 border-cyan-500/30 text-cyan-300'
              : 'bg-slate-900/40 border-slate-800/60 opacity-60'
          }`}
        >
          <div className="flex items-center space-x-2.5">
            <div
              className={`w-8 h-8 rounded-lg flex items-center justify-center ${
                isON ? 'bg-cyan-500/15 text-cyan-400' : 'bg-slate-800 text-slate-500'
              }`}
            >
              <Activity className="w-4 h-4" />
            </div>
            <div>
              <p className="text-[11px] font-medium text-slate-400 uppercase tracking-wider">Current</p>
              <p className="text-[10px] text-slate-500">{isON ? 'Active Load' : 'Motor OFF (0.0 A)'}</p>
            </div>
          </div>
          <div className="text-right">
            <span className={`text-xl sm:text-2xl font-bold font-mono ${isON ? 'text-cyan-300' : 'text-slate-500'}`}>
              {motorData.currentDisplay}
            </span>
          </div>
        </div>

        {/* 3. Conditional Power (Dynamically computed P = V * I only when status is ON) */}
        <div
          className={`rounded-xl p-3.5 border transition-all duration-300 flex items-center justify-between ${
            isON
              ? 'bg-slate-900/80 border-emerald-500/30 text-emerald-300'
              : 'bg-slate-900/40 border-slate-800/60 opacity-60'
          }`}
        >
          <div className="flex items-center space-x-2.5">
            <div
              className={`w-8 h-8 rounded-lg flex items-center justify-center ${
                isON ? 'bg-emerald-500/15 text-emerald-400' : 'bg-slate-800 text-slate-500'
              }`}
            >
              <Gauge className="w-4 h-4" />
            </div>
            <div>
              <p className="text-[11px] font-medium text-slate-400 uppercase tracking-wider">Instant Power</p>
              <p className="text-[10px] text-slate-500">{isON ? 'V × I Computed' : 'Motor OFF (0.0 W)'}</p>
            </div>
          </div>
          <div className="text-right">
            <span className={`text-xl sm:text-2xl font-bold font-mono ${isON ? 'text-emerald-300' : 'text-slate-500'}`}>
              {motorData.powerDisplay}
            </span>
          </div>
        </div>

      </div>

      {/* Actuation Pending Indicator */}
      {isActuating && (
        <div className="absolute inset-0 bg-slate-950/70 backdrop-blur-xs rounded-2xl flex items-center justify-center z-10">
          <div className="flex items-center space-x-2 text-emerald-400 text-xs font-semibold">
            <div className="w-4 h-4 border-2 border-emerald-400 border-t-transparent rounded-full animate-spin" />
            <span>Transmitting LoRa packet...</span>
          </div>
        </div>
      )}
    </div>
  );
}
