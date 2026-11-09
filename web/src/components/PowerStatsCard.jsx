import React from 'react';
import { BarChart3, Calendar, Clock, TrendingUp } from 'lucide-react';

export default function PowerStatsCard({ stats }) {
  const todayVal = stats?.today ?? "5.2 kWh";
  const weekVal = stats?.week ?? "34.1 kWh";
  const monthVal = stats?.month ?? "142.0 kWh";

  return (
    <section className="mb-8">
      <div className="flex items-center justify-between mb-3 px-1">
        <h3 className="text-sm font-semibold uppercase tracking-wider text-slate-400">
          Power Statistics
        </h3>
        <span className="text-xs text-slate-500 font-mono">
          Cumulative Energy Telemetry
        </span>
      </div>

      <div className="bg-gradient-to-b from-slate-800/90 to-slate-900/90 rounded-2xl p-5 sm:p-6 border border-slate-700/60 shadow-lg">
        {/* Card Header */}
        <div className="flex items-center justify-between pb-4 mb-4 border-b border-slate-800">
          <div className="flex items-center space-x-3">
            <div className="w-10 h-10 rounded-xl bg-purple-500/15 text-purple-400 flex items-center justify-center border border-purple-500/20">
              <BarChart3 className="w-5 h-5" />
            </div>
            <div>
              <h4 className="text-base font-bold text-white">Aggregated Energy Consumption</h4>
              <p className="text-xs text-slate-400">Real-time meter accumulation from sensor node</p>
            </div>
          </div>
          <div className="hidden sm:flex items-center space-x-1 text-xs text-emerald-400 bg-emerald-500/10 px-2.5 py-1 rounded-full border border-emerald-500/20 font-medium">
            <TrendingUp className="w-3.5 h-3.5" />
            <span>Optimal Grid Efficiency</span>
          </div>
        </div>

        {/* 3 Summary Fields: Used Today, Week, Month */}
        <div className="grid grid-cols-1 sm:grid-cols-3 gap-3.5 sm:gap-4">
          
          {/* 1. Used Today */}
          <div className="bg-slate-900/70 rounded-xl p-4 border border-slate-800/80 flex flex-col justify-between hover:border-slate-700 transition-colors">
            <div className="flex items-center justify-between text-slate-400 mb-2">
              <span className="text-xs font-semibold uppercase tracking-wider text-slate-400">
                Used Today
              </span>
              <Clock className="w-4 h-4 text-purple-400/80" />
            </div>
            <div className="flex items-baseline space-x-1.5">
              <span className="text-2xl sm:text-3xl font-extrabold font-mono text-purple-300">
                {todayVal}
              </span>
            </div>
            <span className="text-[10px] text-slate-500 mt-1">24h running total</span>
          </div>

          {/* 2. Used Week */}
          <div className="bg-slate-900/70 rounded-xl p-4 border border-slate-800/80 flex flex-col justify-between hover:border-slate-700 transition-colors">
            <div className="flex items-center justify-between text-slate-400 mb-2">
              <span className="text-xs font-semibold uppercase tracking-wider text-slate-400">
                Week
              </span>
              <Calendar className="w-4 h-4 text-emerald-400/80" />
            </div>
            <div className="flex items-baseline space-x-1.5">
              <span className="text-2xl sm:text-3xl font-extrabold font-mono text-emerald-300">
                {weekVal}
              </span>
            </div>
            <span className="text-[10px] text-slate-500 mt-1">Last 7 rolling days</span>
          </div>

          {/* 3. Used Month */}
          <div className="bg-slate-900/70 rounded-xl p-4 border border-slate-800/80 flex flex-col justify-between hover:border-slate-700 transition-colors">
            <div className="flex items-center justify-between text-slate-400 mb-2">
              <span className="text-xs font-semibold uppercase tracking-wider text-slate-400">
                Month
              </span>
              <BarChart3 className="w-4 h-4 text-blue-400/80" />
            </div>
            <div className="flex items-baseline space-x-1.5">
              <span className="text-2xl sm:text-3xl font-extrabold font-mono text-blue-300">
                {monthVal}
              </span>
            </div>
            <span className="text-[10px] text-slate-500 mt-1">Billing cycle to date</span>
          </div>

        </div>
      </div>
    </section>
  );
}
