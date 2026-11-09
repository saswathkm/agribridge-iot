import React from 'react';
import { Sprout } from 'lucide-react';

/**
 * SoilCard — displays soil moisture percentage from the capacitive sensor.
 * Color coding:
 *   < 30% → red   (too dry)
 *   30–80% → green (optimal)
 *   > 80% → blue  (too wet)
 *
 * Props:
 *   soil: string — numeric string e.g. "46.0" or "NA"
 */
export default function SoilCard({ soil }) {
  const isNA = soil === 'NA' || soil === '' || soil == null;
  const pct = isNA ? null : Math.min(100, Math.max(0, parseFloat(soil)));

  // Determine colour band
  let barColor, textColor, bgColor, labelColor, rangeText;
  if (isNA || pct === null) {
    barColor  = 'bg-slate-600';
    textColor = 'text-slate-400';
    bgColor   = 'from-slate-800/80 via-slate-800/60 to-slate-900/80';
    labelColor = 'text-slate-500';
    rangeText = 'Sensor unavailable';
  } else if (pct < 30) {
    barColor  = 'bg-rose-500';
    textColor = 'text-rose-300';
    bgColor   = 'from-slate-800/80 via-slate-800/60 to-slate-900/80';
    labelColor = 'text-rose-500';
    rangeText = 'Too dry — consider irrigating';
  } else if (pct > 80) {
    barColor  = 'bg-sky-400';
    textColor = 'text-sky-300';
    bgColor   = 'from-slate-800/80 via-slate-800/60 to-slate-900/80';
    labelColor = 'text-sky-500';
    rangeText = 'Too wet — check drainage';
  } else {
    barColor  = 'bg-emerald-500';
    textColor = 'text-emerald-300';
    bgColor   = 'from-slate-800/80 via-slate-800/60 to-slate-900/80';
    labelColor = 'text-emerald-600';
    rangeText = 'Optimal: 30% – 80%';
  }

  return (
    <div className={`bg-gradient-to-r ${bgColor} rounded-2xl p-4 sm:p-5 border border-slate-700/60 shadow-sm hover:border-slate-600 transition-colors`}>
      <div className="flex items-center justify-between mb-3">
        <div className="flex items-center space-x-3">
          <div className={`w-11 h-11 rounded-xl flex items-center justify-center border ${
            pct === null
              ? 'bg-slate-700/30 border-slate-600/20 text-slate-500'
              : pct < 30
              ? 'bg-rose-500/10 border-rose-500/20 text-rose-400'
              : pct > 80
              ? 'bg-sky-500/10 border-sky-500/20 text-sky-400'
              : 'bg-emerald-500/10 border-emerald-500/20 text-emerald-400'
          }`}>
            <Sprout className="w-6 h-6" />
          </div>
          <div>
            <h4 className="text-sm font-bold text-slate-200">Soil Moisture</h4>
            <p className="text-xs text-slate-400">Capacitive Field Sensor</p>
          </div>
        </div>

        <div className="text-right">
          <div className="flex items-baseline space-x-1">
            <span className={`text-2xl sm:text-3xl font-extrabold font-mono ${textColor}`}>
              {isNA ? '--' : pct.toFixed(1)}
            </span>
            <span className={`text-base font-semibold ${textColor} opacity-80`}>%</span>
          </div>
          <p className={`text-[10px] font-medium ${labelColor}`}>{rangeText}</p>
        </div>
      </div>

      {/* Progress bar */}
      <div className="w-full bg-slate-700/50 rounded-full h-2 overflow-hidden">
        <div
          className={`h-2 rounded-full transition-all duration-700 ${barColor}`}
          style={{ width: `${isNA ? 0 : pct}%` }}
        />
      </div>

      {/* Scale labels */}
      <div className="flex justify-between mt-1.5">
        <span className="text-[9px] text-slate-600">0%</span>
        <span className="text-[9px] text-rose-700">30%</span>
        <span className="text-[9px] text-emerald-700">80%</span>
        <span className="text-[9px] text-slate-600">100%</span>
      </div>
    </div>
  );
}
