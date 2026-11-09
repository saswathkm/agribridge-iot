import React from 'react';
import MotorCard from './MotorCard';

export default function MotorsSection({ motor1, motor2, onToggleMotor, isActuating }) {
  return (
    <section className="mb-6">
      <div className="flex items-center justify-between mb-3 px-1">
        <h3 className="text-sm font-semibold uppercase tracking-wider text-slate-400">
          Pump & Motor Control
        </h3>
        <span className="text-xs text-slate-500 font-mono">
          2 Field Actuators Synchronized
        </span>
      </div>

      {/* Side-by-Side Grid for Motor 1 and Motor 2 */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-4 sm:gap-5">
        <MotorCard
          title="Motor 1"
          motorId="motor1"
          motorData={motor1}
          onToggle={onToggleMotor}
          isActuating={isActuating.motor1}
        />
        <MotorCard
          title="Motor 2"
          motorId="motor2"
          motorData={motor2}
          onToggle={onToggleMotor}
          isActuating={isActuating.motor2}
        />
      </div>
    </section>
  );
}
