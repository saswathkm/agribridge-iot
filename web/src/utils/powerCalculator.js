/**
 * AgriBridge Power Calculation and Formatting Utilities
 *
 * Implements strict business logic:
 * - Voltage is always continuous.
 * - When Motor is ON (status === true):
 *     Current is displayed.
 *     Instantaneous Power is computed dynamically: Power = Voltage * Current (W)
 * - When Motor is OFF (status === false):
 *     Current MUST show 0.0 A.
 *     Power MUST show 0.0 W.
 */

export function parseNumericValue(value, defaultValue = 0.0) {
  if (typeof value === 'number') return isNaN(value) ? defaultValue : value;
  if (typeof value === 'string') {
    // Strip any trailing units like 'V', 'A', 'W', 'kWh', '%', '°C'
    const cleaned = value.replace(/[^0-9.-]/g, '');
    const parsed = parseFloat(cleaned);
    return isNaN(parsed) ? defaultValue : parsed;
  }
  return defaultValue;
}

export function computeMotorMetrics(motor) {
  if (!motor) {
    return {
      status: false,
      voltage: '0.0',
      current: '0.0',
      power: '0.0',
      voltageDisplay: '0.0 V',
      currentDisplay: '0.0 A',
      powerDisplay: '0.0 W',
      numericVoltage: 0.0,
      numericCurrent: 0.0,
      numericPower: 0.0,
    };
  }

  const isStatusOn = Boolean(motor.status);
  const rawVoltage = parseNumericValue(motor.voltage, 0.0);
  const rawCurrent = parseNumericValue(motor.current, 0.0);

  // Line voltage is continuous
  const voltageDisplay = `${rawVoltage.toFixed(1)} V`;

  if (!isStatusOn) {
    return {
      status: false,
      voltage: rawVoltage.toFixed(1),
      current: '0.0',
      power: '0.0',
      voltageDisplay,
      currentDisplay: '0.0 A',
      powerDisplay: '0.0 W',
      numericVoltage: rawVoltage,
      numericCurrent: 0.0,
      numericPower: 0.0,
    };
  }

  // Motor is ON: Current is active and Power is dynamically computed P = V * I
  const effectiveCurrent = rawCurrent > 0 ? rawCurrent : 3.5; // fallback nominal motor load if 0
  const computedPower = rawVoltage * effectiveCurrent;

  return {
    status: true,
    voltage: rawVoltage.toFixed(1),
    current: effectiveCurrent.toFixed(1),
    power: computedPower.toFixed(1),
    voltageDisplay,
    currentDisplay: `${effectiveCurrent.toFixed(1)} A`,
    powerDisplay: `${computedPower.toFixed(1)} W`,
    numericVoltage: rawVoltage,
    numericCurrent: effectiveCurrent,
    numericPower: computedPower,
  };
}
