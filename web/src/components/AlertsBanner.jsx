import React, { useState, useEffect } from 'react';
import { AlertTriangle, X } from 'lucide-react';

/**
 * AlertsBanner — shows a dismissible banner when the RP2040 field node
 * reports active alerts (pipe-separated, e.g. "HIGH_CURRENT|LOW_VOLTAGE").
 * Auto-dismisses after 8 seconds. Re-appears when a new alert string arrives.
 *
 * Props:
 *   alerts: { active: string, last_updated: number }
 */
export default function AlertsBanner({ alerts }) {
  const activeStr = alerts?.active ?? 'NONE';
  const lastUpdated = alerts?.last_updated ?? 0;
  const [dismissed, setDismissed] = useState(false);
  const [prevKey, setPrevKey] = useState('');

  // Re-show banner each time a genuinely new alert string arrives
  const key = `${activeStr}-${lastUpdated}`;
  useEffect(() => {
    if (key !== prevKey) {
      setDismissed(false);
      setPrevKey(key);
    }
  }, [key, prevKey]);

  // Auto-dismiss after 8 s
  useEffect(() => {
    if (activeStr === 'NONE' || dismissed) return;
    const t = setTimeout(() => setDismissed(true), 8000);
    return () => clearTimeout(t);
  }, [activeStr, dismissed, key]);

  if (activeStr === 'NONE' || dismissed) return null;

  // Format each pipe-separated alert code into a human-readable label
  const LABELS = {
    HIGH_CURRENT:       { text: 'Overcurrent — Motor 1 tripped',      severity: 'red' },
    LOW_VOLTAGE:        { text: 'Low mains voltage detected',          severity: 'yellow' },
    SOIL_LOW:           { text: 'Soil moisture critically low',         severity: 'yellow' },
    SOIL_HIGH:          { text: 'Soil moisture too high',              severity: 'blue' },
    RELAY1_ON_BLOCKED:  { text: 'Motor 1 ON blocked (overcurrent)',    severity: 'red' },
    RELAY1_ON:          { text: 'Motor 1 turned ON',                   severity: 'green' },
    RELAY1_OFF:         { text: 'Motor 1 turned OFF',                  severity: 'green' },
    RELAY2_ON:          { text: 'Motor 2 turned ON',                   severity: 'green' },
    RELAY2_OFF:         { text: 'Motor 2 turned OFF',                  severity: 'green' },
  };

  const codes = activeStr.split('|').filter(Boolean);
  const hasCritical = codes.some(c => LABELS[c]?.severity === 'red');

  return (
    <div className={`rounded-xl p-3 flex items-start justify-between gap-3 border ${
      hasCritical
        ? 'bg-rose-950/50 border-rose-500/40 text-rose-200'
        : 'bg-amber-950/40 border-amber-500/30 text-amber-200'
    }`}>
      <div className="flex items-start gap-2.5 flex-1">
        <AlertTriangle className={`w-4 h-4 flex-shrink-0 mt-0.5 ${hasCritical ? 'text-rose-400' : 'text-amber-400'}`} />
        <div className="flex flex-col gap-1">
          <span className="text-xs font-bold uppercase tracking-wide">
            Field Node Alert{codes.length > 1 ? 's' : ''}
          </span>
          <ul className="text-xs space-y-0.5">
            {codes.map(code => (
              <li key={code}>
                {LABELS[code]?.text ?? code.replace(/_/g, ' ')}
              </li>
            ))}
          </ul>
        </div>
      </div>
      <button
        onClick={() => setDismissed(true)}
        className="flex-shrink-0 mt-0.5 opacity-60 hover:opacity-100 transition-opacity"
        aria-label="Dismiss alert"
      >
        <X className="w-4 h-4" />
      </button>
    </div>
  );
}
