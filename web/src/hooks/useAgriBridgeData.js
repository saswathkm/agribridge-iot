import { useState, useEffect, useCallback, useRef } from 'react';
import { initFirebase, ref, onValue, update, DEFAULT_SENSOR_DATA } from '../firebase/config';
import { computeMotorMetrics, parseNumericValue } from '../utils/powerCalculator';

export function useAgriBridgeData() {
  const [data, setData] = useState(DEFAULT_SENSOR_DATA);
  const [connectionStatus, setConnectionStatus] = useState('connecting');
  const [errorMsg, setErrorMsg] = useState(null);
  const [lastSyncTime, setLastSyncTime] = useState(new Date());
  const [isActuating, setIsActuating] = useState({ motor1: false, motor2: false });
  const [mode, setMode] = useState('auto');

  // Live 1-second ticker to re-evaluate heartbeat freshness in real time
  const [, setTick] = useState(0);
  useEffect(() => {
    const timer = setInterval(() => setTick((t) => t + 1), 1000);
    return () => clearInterval(timer);
  }, []);

  const dbRefHolder = useRef(null);
  const simIntervalHolder = useRef(null);

  useEffect(() => {
    const { db, isConfigured } = initFirebase();

    if (isConfigured && db && mode !== 'simulator') {
      try {
        setConnectionStatus('connecting');
        const nodeRef = ref(db, 'sensor_node_1');
        dbRefHolder.current = db;

        const unsubscribe = onValue(
          nodeRef,
          (snapshot) => {
            if (snapshot.exists()) {
              const val = snapshot.val();
              setData(val);
              setConnectionStatus('connected');
              setLastSyncTime(new Date());
              setErrorMsg(null);
            } else {
              update(nodeRef, DEFAULT_SENSOR_DATA);
              setData(DEFAULT_SENSOR_DATA);
              setConnectionStatus('connected');
            }
          },
          (err) => {
            console.warn('Firebase RTDB listener error, switching to simulation mode:', err);
            setErrorMsg(err.message);
            setConnectionStatus('simulated');
          }
        );

        return () => { unsubscribe(); };
      } catch (err) {
        console.error('Firebase setup failed:', err);
        setConnectionStatus('simulated');
      }
    } else {
      setConnectionStatus('simulated');
    }
  }, [mode]);

  // Continuous IoT Telemetry Simulation Engine (Only when explicitly offline/simulated)
  useEffect(() => {
    if (connectionStatus !== 'simulated') {
      if (simIntervalHolder.current) clearInterval(simIntervalHolder.current);
      return;
    }

    simIntervalHolder.current = setInterval(() => {
      setData((prev) => {
        const v1Base = 230.0 + (Math.sin(Date.now() / 3000) * 2.5);
        const v2Base = 228.0 + (Math.cos(Date.now() / 4000) * 2.0);

        const nextTemp = (29.0 + Math.sin(Date.now() / 15000) * 0.8).toFixed(1);
        const nextHum  = (68.0 + Math.cos(Date.now() / 18000) * 1.5).toFixed(1);
        const nextSoil = (52.0 + Math.sin(Date.now() / 20000) * 12.0).toFixed(1);

        const m1Status = Boolean(prev?.motor1?.status);
        const m1CurrentVal = m1Status ? (4.0 + Math.sin(Date.now() / 2000) * 0.3).toFixed(1) : "0.0";
        const m1PowerVal   = m1Status ? (v1Base * parseFloat(m1CurrentVal)).toFixed(1) : "0.0";

        const m2Status = Boolean(prev?.motor2?.status);
        const m2CurrentVal = m2Status ? (3.4 + Math.cos(Date.now() / 2500) * 0.2).toFixed(1) : "0.0";
        const m2PowerVal   = m2Status ? (v2Base * parseFloat(m2CurrentVal)).toFixed(1) : "0.0";

        setLastSyncTime(new Date());

        return {
          motor1: { status: m1Status, voltage: v1Base.toFixed(1), current: m1CurrentVal, power: m1PowerVal },
          motor2: { status: m2Status, voltage: v2Base.toFixed(1), current: m2CurrentVal, power: m2PowerVal },
          environment: { temperature: nextTemp, humidity: nextHum, soil: nextSoil },
          alerts: { active: "NONE", last_updated: Math.floor(Date.now() / 1000) },
          connection: {
            esp32_online: false,
            lora_link_active: false,
            last_field_packet_ts: 0,
            last_firebase_push_ts: 0,
            lora_rssi: 0
          },
          stats: prev?.stats || { today: "5.2 kWh", week: "34.1 kWh", month: "142.0 kWh" }
        };
      });
    }, 1500);

    return () => { if (simIntervalHolder.current) clearInterval(simIntervalHolder.current); };
  }, [connectionStatus]);

  const toggleMotor = useCallback(async (motorKey) => {
    const currentMotor = data?.[motorKey];
    const newStatus = !Boolean(currentMotor?.status);

    setIsActuating((prev) => ({ ...prev, [motorKey]: true }));

    const rawVoltage = parseNumericValue(currentMotor?.voltage || (motorKey === 'motor1' ? 230.5 : 228.0));
    const nominalCurrent = motorKey === 'motor1' ? 4.2 : 3.5;
    const computedCurrent = newStatus ? nominalCurrent.toFixed(1) : "0.0";
    const computedPower   = newStatus ? (rawVoltage * nominalCurrent).toFixed(1) : "0.0";

    const updatedMotorPayload = {
      status: newStatus,
      voltage: rawVoltage.toFixed(1),
      current: computedCurrent,
      power: computedPower
    };

    setData((prev) => ({ ...prev, [motorKey]: updatedMotorPayload }));

    if (dbRefHolder.current && connectionStatus === 'connected') {
      try {
        const motorRef = ref(dbRefHolder.current, `sensor_node_1/${motorKey}`);
        await update(motorRef, { status: newStatus, current: computedCurrent, power: computedPower });
      } catch (err) {
        console.error(`Failed to update ${motorKey} status in Firebase:`, err);
        setErrorMsg(`Actuation failed: ${err.message}`);
      }
    }

    setTimeout(() => { setIsActuating((prev) => ({ ...prev, [motorKey]: false })); }, 400);
  }, [data, connectionStatus]);

  const motor1Metrics = computeMotorMetrics(data?.motor1);
  const motor2Metrics = computeMotorMetrics(data?.motor2);

  // Derive connection health from the connection object using real-time freshness
  const conn = data?.connection || {};
  const nowMs = Date.now();

  let lastPushMs = 0;
  if (conn.last_firebase_push_ts) {
    const ts = Number(conn.last_firebase_push_ts);
    // Support either 13-digit millisecond or 10-digit second timestamps
    lastPushMs = ts < 10000000000 ? ts * 1000 : ts;
  }

  // ESP32 sends a heartbeat push every 30s. If no push arrives within 50s, ESP32 IS OFFLINE!
  const isHeartbeatFresh = lastPushMs > 0 && (nowMs - lastPushMs) < 50000;
  const esp32Online = connectionStatus === 'connected' && Boolean(conn.esp32_online && isHeartbeatFresh);

  let lastFieldMs = 0;
  if (conn.last_field_packet_ts) {
    const ts = Number(conn.last_field_packet_ts);
    lastFieldMs = ts < 10000000000 ? ts * 1000 : ts;
  }
  const isFieldPacketFresh = lastFieldMs > 0 && (nowMs - lastFieldMs) < 60000;
  const dataStreamLive = esp32Online && isFieldPacketFresh;
  const loraLinkActive = esp32Online && Boolean(conn.lora_link_active && isFieldPacketFresh);

  return {
    raw: data,
    motor1: motor1Metrics,
    motor2: motor2Metrics,
    environment: {
      temperature: data?.environment?.temperature ?? "29.2",
      humidity:    data?.environment?.humidity    ?? "68.0",
      soil:        data?.environment?.soil        ?? "52.0"
    },
    alerts: {
      active:       data?.alerts?.active       ?? "NONE",
      last_updated: data?.alerts?.last_updated ?? 0
    },
    connection: {
      esp32Online,
      loraLinkActive,
      dataStreamLive,
      loraRssi:       conn.lora_rssi ?? 0
    },
    stats: {
      today: data?.stats?.today ?? "5.2 kWh",
      week:  data?.stats?.week  ?? "34.1 kWh",
      month: data?.stats?.month ?? "142.0 kWh"
    },
    connectionStatus,
    lastSyncTime,
    isActuating,
    errorMsg,
    toggleMotor,
    setMode,
    mode
  };
}
