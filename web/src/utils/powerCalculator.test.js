import { computeMotorMetrics, parseNumericValue } from './powerCalculator.js';

function runTests() {
  console.log('Running AgriBridge Web Power & Logic Unit Tests...');

  // Test 1: Motor OFF enforces 0.0 A and 0.0 W
  const offResult = computeMotorMetrics({
    status: false,
    voltage: '230.5',
    current: '4.2',
    power: '968.1'
  });

  if (offResult.status !== false) throw new Error('Test 1 failed: status must be false');
  if (offResult.voltageDisplay !== '230.5 V') throw new Error('Test 1 failed: voltageDisplay must be 230.5 V');
  if (offResult.currentDisplay !== '0.0 A') throw new Error(`Test 1 failed: currentDisplay is ${offResult.currentDisplay}, expected 0.0 A`);
  if (offResult.powerDisplay !== '0.0 W') throw new Error(`Test 1 failed: powerDisplay is ${offResult.powerDisplay}, expected 0.0 W`);
  console.log('✓ Test 1 Passed: Motor OFF enforces 0.0 A and 0.0 W with continuous voltage');

  // Test 2: Motor ON computes dynamic power P = V * I
  const onResult = computeMotorMetrics({
    status: true,
    voltage: '228.0',
    current: '3.5',
    power: '0.0'
  });

  if (onResult.status !== true) throw new Error('Test 2 failed: status must be true');
  if (onResult.voltageDisplay !== '228.0 V') throw new Error('Test 2 failed: voltageDisplay must be 228.0 V');
  if (onResult.currentDisplay !== '3.5 A') throw new Error('Test 2 failed: currentDisplay must be 3.5 A');
  if (onResult.powerDisplay !== '798.0 W') throw new Error(`Test 2 failed: powerDisplay is ${onResult.powerDisplay}, expected 798.0 W`);
  console.log('✓ Test 2 Passed: Motor ON dynamically computes P = V * I (228.0V * 3.5A = 798.0W)');

  // Test 3: Continuous Voltage
  const zeroMotor = computeMotorMetrics({ status: false, voltage: '231.8', current: '0.0', power: '0.0' });
  if (zeroMotor.voltageDisplay !== '231.8 V') throw new Error('Test 3 failed');
  console.log('✓ Test 3 Passed: Continuous voltage is preserved at 231.8 V');

  console.log('All Web Dashboard Logic Tests Passed Successfully!');
}

runTests();
