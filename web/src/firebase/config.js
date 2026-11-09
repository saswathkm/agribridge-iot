import { initializeApp, getApps, getApp } from 'firebase/app';
import { getDatabase, ref, onValue, set, update } from 'firebase/database';

// Default initial state matching the exact user specification
export const DEFAULT_SENSOR_DATA = {
  motor1: {
    status: false,
    voltage: "230.5",
    current: "0.0",
    power: "0.0"
  },
  motor2: {
    status: true,
    voltage: "228.0",
    current: "3.5",
    power: "798.0"
  },
  environment: {
    temperature: "29.2",
    humidity: "68.0",
    soil: "52.0"
  },
  alerts: {
    active: "NONE",
    last_updated: 0
  },
  connection: {
    esp32_online: false,
    lora_link_active: false,
    last_field_packet_ts: 0,
    last_firebase_push_ts: 0,
    lora_rssi: 0
  },
  stats: {
    today: "5.2 kWh",
    week: "34.1 kWh",
    month: "142.0 kWh"
  }
};

// Retrieve configuration from environment or localStorage
export function getFirebaseConfig() {
  const saved = localStorage.getItem('agribridge_firebase_config');
  if (saved) {
    try {
      return JSON.parse(saved);
    } catch (e) {
      console.warn('Failed to parse saved Firebase config', e);
    }
  }

  return {
    apiKey: import.meta.env.VITE_FIREBASE_API_KEY || "AIzaSyBMe0BjWk36VQypQM6thOG-CvnvP3rG5ZQ",
    authDomain: import.meta.env.VITE_FIREBASE_AUTH_DOMAIN || "agribridge-iot.firebaseapp.com",
    databaseURL: import.meta.env.VITE_FIREBASE_DATABASE_URL || "https://agribridge-iot-default-rtdb.firebaseio.com",
    projectId: import.meta.env.VITE_FIREBASE_PROJECT_ID || "agribridge-iot",
    storageBucket: import.meta.env.VITE_FIREBASE_STORAGE_BUCKET || "agribridge-iot.firebasestorage.app",
    messagingSenderId: import.meta.env.VITE_FIREBASE_MESSAGING_SENDER_ID || "87914715815",
    appId: import.meta.env.VITE_FIREBASE_APP_ID || "1:87914715815:web:bfcb49ab2b20399f6d1982"
  };
}

let firebaseApp = null;
let databaseInstance = null;

export function initFirebase() {
  const config = getFirebaseConfig();
  const hasValidConfig = Boolean(config.apiKey && config.databaseURL);

  if (!hasValidConfig) {
    return { app: null, db: null, isConfigured: false };
  }

  try {
    firebaseApp = getApps().length > 0 ? getApp() : initializeApp(config);
    databaseInstance = getDatabase(firebaseApp);
    return { app: firebaseApp, db: databaseInstance, isConfigured: true };
  } catch (error) {
    console.error('Firebase initialization error:', error);
    return { app: null, db: null, isConfigured: false, error };
  }
}

export { ref, onValue, set, update };
