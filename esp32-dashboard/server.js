const express = require('express');
const path = require('path');
const os = require('os');

const app = express();

// Parse JSON bodies from ESP32 WiFi POST requests
app.use(express.json());

// Current telemetry state object
let sensorData = {
  temperature: null,
  humidity: null,
  distance: null,
  zone: 'UNKNOWN',
  gas: null,
  motion: false,
  sos: false,
  lastUpdate: null
};

// ── WiFi API endpoint: ESP32 POSTs sensor data here ──
app.post('/api/sensor', (req, res) => {
  const d = req.body;

  if (d.temperature !== undefined) sensorData.temperature = d.temperature;
  if (d.humidity !== undefined)    sensorData.humidity = d.humidity;
  if (d.gas !== undefined)         sensorData.gas = d.gas;
  if (d.motion !== undefined)      sensorData.motion = d.motion;
  if (d.sos !== undefined)         sensorData.sos = d.sos;

  if (d.distance !== undefined) {
    sensorData.distance = d.distance;
    const dist = d.distance;
    if (dist < 50)        sensorData.zone = 'DANGER';
    else if (dist < 150)  sensorData.zone = 'CAUTION';
    else if (dist < 400)  sensorData.zone = 'SAFE';
    else                  sensorData.zone = 'OUT_OF_RANGE';
  }

  // Gas alert overrides SOS display
  if (d.gasAlert === true) {
    sensorData.sos = true;
  }

  sensorData.lastUpdate = new Date().toISOString();

  console.log(`📡 WiFi data received → Temp: ${sensorData.temperature}°C | Dist: ${sensorData.distance}cm | Gas: ${sensorData.gas} | Motion: ${sensorData.motion} | SOS: ${sensorData.sos}`);

  res.json({ status: 'ok' });
});

// Serve UI assets static directory
app.use(express.static(path.join(__dirname)));

// API Endpoint consumed by dashboard
app.get('/data', (req, res) => {
  // Fallback safety to force UI online if server is receiving streams
  if (!sensorData.lastUpdate && (sensorData.distance !== null || sensorData.temperature !== null)) {
    sensorData.lastUpdate = new Date().toISOString();
  }
  res.json(sensorData);
});

// Helper: Get local IP addresses for display
function getLocalIPs() {
  const interfaces = os.networkInterfaces();
  const ips = [];
  for (const name of Object.keys(interfaces)) {
    for (const iface of interfaces[name]) {
      if (iface.family === 'IPv4' && !iface.internal) {
        ips.push({ name, address: iface.address });
      }
    }
  }
  return ips;
}

const HTTP_PORT = 3000;

// Bind to 0.0.0.0 so ESP32 can reach the server on the local network
app.listen(HTTP_PORT, '0.0.0.0', () => {
  console.log(`\n🚀 System Dashboard active at: http://localhost:${HTTP_PORT}`);
  console.log(`\n📡 ESP32 WiFi endpoint: POST http://<YOUR_PC_IP>:${HTTP_PORT}/api/sensor`);

  const ips = getLocalIPs();
  if (ips.length > 0) {
    console.log('\n🌐 Your PC IP addresses (use one of these in arduino.ino):');
    ips.forEach(ip => {
      console.log(`   ${ip.name}: http://${ip.address}:${HTTP_PORT}/api/sensor`);
    });
  }

  console.log('\n⏳ Waiting for ESP32 WiFi data...\n');
});