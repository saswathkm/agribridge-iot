import React from 'react';
import { Thermometer, Droplets } from 'lucide-react';

/**
 * Shows Temperature and Humidity sensor cards.
 * Handles "NA" values from DHT11 read failures gracefully.
 */
export default function EnvironmentalSection({ environment }) {
  const rawTemp = environment?.temperature;
  const rawHum  = environment?.humidity;

  // Show '--' if value is "NA", empty, or missing
  const isNA = (v) => !v || v === 'NA';
  const temperature = isNA(rawTemp) ? '--' : rawTemp;
  const humidity    = isNA(rawHum)  ? '--' : rawHum;

  return (
    <section className="mb-0">
      <div className="flex items-center justify-between mb-3 px-1">
        <h3 className="text-sm font-semibold uppercase tracking-wider text-slate-400">
          Field Microclimate
        </h3>
        <span className="text-xs text-slate-500 font-mono">
          DHT11 LoRa Sensor Node
        </span>
      </div>

      <div className="flex flex-col space-y-3.5">

        {/* Card 1: Temperature */}
        <div className="bg-gradient-to-r from-slate-800/80 via-slate-800/60 to-slate-900/80 rounded-2xl p-4 sm:p-5 border border-slate-700/60 shadow-sm flex items-center justify-between hover:border-slate-600 transition-colors">
          <div className="flex items-center space-x-3.5">
            <div className="w-11 h-11 rounded-xl bg-gradient-to-br from-orange-500/20 to-amber-500/10 text-orange-400 flex items-center justify-center border border-orange-500/20 shadow-inner">
              <Thermometer className="w-6 h-6" />
            </div>
            <div>
              <h4 className="text-sm font-bold text-slate-200">Ambient Temperature</h4>
              <p className="text-xs text-slate-400">Field Canopy Sensor Probe</p>
            </div>
          </div>

          <div className="text-right">
            <div className="flex items-baseline space-x-1">
              <span className={`text-2xl sm:text-3xl font-extrabold font-mono ${temperature === '--' ? 'text-slate-500' : 'text-orange-300'}`}>
                {temperature}
              </span>
              <span className={`text-base font-semibold ${temperature === '--' ? 'text-slate-600' : 'text-orange-400/80'}`}>
                {temperature === '--' ? '' : '°C'}
              </span>
            </div>
            <p className="text-[10px] text-slate-500 font-medium">
              {temperature === '--' ? 'Sensor unavailable' : 'Optimal: 24°C - 32°C'}
            </p>
          </div>
        </div>

        {/* Card 2: Humidity */}
        <div className="bg-gradient-to-r from-slate-800/80 via-slate-800/60 to-slate-900/80 rounded-2xl p-4 sm:p-5 border border-slate-700/60 shadow-sm flex items-center justify-between hover:border-slate-600 transition-colors">
          <div className="flex items-center space-x-3.5">
            <div className="w-11 h-11 rounded-xl bg-gradient-to-br from-blue-500/20 to-sky-500/10 text-sky-400 flex items-center justify-center border border-blue-500/20 shadow-inner">
              <Droplets className="w-6 h-6" />
            </div>
            <div>
              <h4 className="text-sm font-bold text-slate-200">Relative Humidity</h4>
              <p className="text-xs text-slate-400">Air Moisture Level</p>
            </div>
          </div>

          <div className="text-right">
            <div className="flex items-baseline space-x-1">
              <span className={`text-2xl sm:text-3xl font-extrabold font-mono ${humidity === '--' ? 'text-slate-500' : 'text-sky-300'}`}>
                {humidity}
              </span>
              <span className={`text-base font-semibold ${humidity === '--' ? 'text-slate-600' : 'text-sky-400/80'}`}>
                {humidity === '--' ? '' : '%'}
              </span>
            </div>
            <p className="text-[10px] text-slate-500 font-medium">
              {humidity === '--' ? 'Sensor unavailable' : 'Target: 60% - 75%'}
            </p>
          </div>
        </div>

      </div>
    </section>
  );
}
