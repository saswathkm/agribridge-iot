import React from 'react';
import { Wifi, Radio, Activity } from 'lucide-react';

/**
 * ConnectionStatusBar — three pills shown below the TopBar.
 * Props:
 *   connection: { esp32Online, loraLinkActive, dataStreamLive, loraRssi }
 */
export default function ConnectionStatusBar({ connection }) {
  const { esp32Online, loraLinkActive, dataStreamLive, loraRssi } = connection || {};

  const indicators = [
    {
      key: 'esp32',
      icon: Wifi,
      label: 'ESP32 Gateway',
      sublabel: esp32Online ? 'Online' : 'Offline',
      active: !!esp32Online,
      critical: true
    },
    {
      key: 'lora',
      icon: Radio,
      label: 'LoRa Link',
      sublabel: loraLinkActive ? (loraRssi ? `${loraRssi} dBm` : 'Connected') : 'Disconnected',
      active: !!loraLinkActive,
      critical: false
    },
    {
      key: 'stream',
      icon: Activity,
      label: 'Data Stream',
      sublabel: dataStreamLive ? 'Live' : 'No Data',
      active: !!dataStreamLive,
      critical: false
    }
  ];

  return (
    <div className="bg-slate-900/90 border-b border-slate-800/80 px-4 sm:px-6 py-2">
      <div className="max-w-4xl mx-auto flex items-center gap-2.5 flex-wrap">
        <span className="text-[10px] font-bold uppercase tracking-wider text-slate-500 mr-1">
          Hardware Status:
        </span>
        {indicators.map(({ key, icon: Icon, label, sublabel, active, critical }) => (
          <div
            key={key}
            className={`flex items-center gap-1.5 px-3 py-1 rounded-full text-[11px] font-semibold border transition-all ${
              active
                ? 'bg-emerald-950/60 text-emerald-300 border-emerald-500/40 shadow-sm shadow-emerald-950/50'
                : critical
                ? 'bg-rose-950/40 text-rose-300 border-rose-800/40'
                : 'bg-slate-900 text-slate-500 border-slate-800'
            }`}
          >
            {/* Status dot */}
            <span className={`w-2 h-2 rounded-full flex-shrink-0 ${
              active 
                ? 'bg-emerald-400 animate-pulse' 
                : critical
                ? 'bg-rose-500'
                : 'bg-slate-600'
            }`} />
            <Icon className="w-3.5 h-3.5 flex-shrink-0" />
            <span>{label}</span>
            <span className={`font-mono text-[10px] ${
              active 
                ? 'text-emerald-400' 
                : critical
                ? 'text-rose-400 font-bold'
                : 'text-slate-500'
            }`}>
              [{sublabel}]
            </span>
          </div>
        ))}
      </div>
    </div>
  );
}
