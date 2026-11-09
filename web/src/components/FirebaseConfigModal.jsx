import React, { useState } from 'react';
import { X, Key, Database, Globe, Check, AlertCircle } from 'lucide-react';
import { getFirebaseConfig } from '../firebase/config';

export default function FirebaseConfigModal({ isOpen, onClose, onSave }) {
  if (!isOpen) return null;

  const currentConfig = getFirebaseConfig();
  const [apiKey, setApiKey] = useState(currentConfig.apiKey || '');
  const [databaseURL, setDatabaseURL] = useState(currentConfig.databaseURL || '');
  const [projectId, setProjectId] = useState(currentConfig.projectId || '');
  const [savedSuccess, setSavedSuccess] = useState(false);

  const handleSubmit = (e) => {
    e.preventDefault();
    const newConfig = {
      ...currentConfig,
      apiKey: apiKey.trim(),
      databaseURL: databaseURL.trim(),
      projectId: projectId.trim(),
    };
    localStorage.setItem('agribridge_firebase_config', JSON.stringify(newConfig));
    setSavedSuccess(true);
    setTimeout(() => {
      setSavedSuccess(false);
      onSave(newConfig);
      onClose();
      window.location.reload();
    }, 800);
  };

  const handleReset = () => {
    localStorage.removeItem('agribridge_firebase_config');
    setApiKey('');
    setDatabaseURL('');
    setProjectId('');
    onSave({});
    onClose();
    window.location.reload();
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/75 backdrop-blur-sm animate-fade-in">
      <div className="bg-slate-900 border border-slate-700 rounded-2xl w-full max-w-md p-6 shadow-2xl relative">
        <button
          onClick={onClose}
          className="absolute top-4 right-4 text-slate-400 hover:text-white p-1 rounded-lg"
        >
          <X className="w-5 h-5" />
        </button>

        <div className="flex items-center space-x-3 mb-5">
          <div className="w-10 h-10 rounded-xl bg-emerald-500/15 text-emerald-400 flex items-center justify-center">
            <Database className="w-5 h-5" />
          </div>
          <div>
            <h3 className="text-lg font-bold text-white">Firebase RTDB Settings</h3>
            <p className="text-xs text-slate-400">Target path: <code className="text-emerald-400">sensor_node_1</code></p>
          </div>
        </div>

        <form onSubmit={handleSubmit} className="space-y-4">
          <div>
            <label className="block text-xs font-semibold uppercase tracking-wider text-slate-300 mb-1">
              Database URL
            </label>
            <input
              type="text"
              placeholder="https://your-rtdb-default-rtdb.firebaseio.com"
              value={databaseURL}
              onChange={(e) => setDatabaseURL(e.target.value)}
              className="w-full bg-slate-800 border border-slate-700 rounded-lg px-3 py-2 text-sm text-white placeholder-slate-500 focus:outline-none focus:border-emerald-500 font-mono text-xs"
            />
          </div>

          <div>
            <label className="block text-xs font-semibold uppercase tracking-wider text-slate-300 mb-1">
              API Key
            </label>
            <input
              type="text"
              placeholder="AIzaSy..."
              value={apiKey}
              onChange={(e) => setApiKey(e.target.value)}
              className="w-full bg-slate-800 border border-slate-700 rounded-lg px-3 py-2 text-sm text-white placeholder-slate-500 focus:outline-none focus:border-emerald-500 font-mono text-xs"
            />
          </div>

          <div>
            <label className="block text-xs font-semibold uppercase tracking-wider text-slate-300 mb-1">
              Project ID
            </label>
            <input
              type="text"
              placeholder="agribridge-iot"
              value={projectId}
              onChange={(e) => setProjectId(e.target.value)}
              className="w-full bg-slate-800 border border-slate-700 rounded-lg px-3 py-2 text-sm text-white placeholder-slate-500 focus:outline-none focus:border-emerald-500 font-mono text-xs"
            />
          </div>

          <div className="pt-2 flex items-center justify-between">
            <button
              type="button"
              onClick={handleReset}
              className="text-xs text-rose-400 hover:text-rose-300 underline font-medium"
            >
              Reset to Simulated Mode
            </button>

            <button
              type="submit"
              className="px-4 py-2 rounded-lg bg-emerald-600 hover:bg-emerald-500 text-white font-medium text-sm flex items-center space-x-1.5 transition-colors"
            >
              {savedSuccess ? (
                <>
                  <Check className="w-4 h-4 text-white" />
                  <span>Saved!</span>
                </>
              ) : (
                <span>Save & Connect</span>
              )}
            </button>
          </div>
        </form>
      </div>
    </div>
  );
}
