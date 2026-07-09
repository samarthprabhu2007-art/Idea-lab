# IDEALAB – How to Run

## Prerequisites

| Tool | Version | Notes |
|------|---------|-------|
| **Arduino IDE** | 2.x+ | With ESP32 board package installed |
| **Node.js** | 18+ | [Download](https://nodejs.org) |
| **ESP32 Dev Board** | Any | Must support WiFi |
| **WiFi Network** | — | SSID: `hello`, Password: `12345678` |

## Hardware Wiring

| Sensor | ESP32 Pin |
|--------|-----------|
| SOS Button | GPIO 27 |
| Buzzer | GPIO 23 |
| DHT22 (Data) | GPIO 4 |
| Ultrasonic TRIG | GPIO 5 |
| Ultrasonic ECHO | GPIO 18 |
| MQ2 (Analog) | GPIO 34 |
| MQ2 (Digital) | GPIO 35 |
| PIR Sensor | GPIO 19 |
| OLED SH1106 | I2C (SDA/SCL) |

## Step 1 — Find Your PC's IP Address

Both the ESP32 and your PC must be on the **same WiFi network** (`hello`).

Open a terminal and run:

```bash
# Windows
ipconfig

# Linux / macOS
ifconfig
```

Look for your **IPv4 address** under the WiFi adapter (e.g. `192.168.1.100`).

## Step 2 — Update the Server IP in Arduino Code

Open `arduino/arduino.ino` and update line ~14 with your PC's IP:

```cpp
String serverURL = "http://<YOUR_PC_IP>:3000/api/sensor";
```

For example:

```cpp
String serverURL = "http://192.168.1.100:3000/api/sensor";
```

> **Important:** The IP `192.168.4.1` in the code is a placeholder. You **must** change it to your actual PC IP.

## Step 3 — Flash the ESP32

1. Open `arduino/arduino.ino` in Arduino IDE
2. Install required libraries (if not already installed):
   - **U8g2** (by oliver) — for OLED display
   - **DHT sensor library** (by Adafruit) — for DHT22
3. Select your ESP32 board under **Tools → Board**
4. Select the correct COM port under **Tools → Port**
5. Click **Upload**

After flashing, the ESP32 will:
- Connect to WiFi (`hello` / `12345678`)
- Show its IP on the OLED screen
- Start sending sensor data to your server via HTTP

## Step 4 — Start the Dashboard Server

```bash
cd esp32-dashboard
npm install
npm start
```

The server will start and display:
- Dashboard URL: `http://localhost:3000`
- Your PC's IP addresses for the Arduino config

## Step 5 — Open the Dashboard

Open your browser and go to:

```
http://localhost:3000
```

The dashboard will show **ONLINE** once the ESP32 connects and starts sending data.

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Dashboard shows **OFFLINE** | Check that ESP32 is connected to WiFi and the server IP in `arduino.ino` is correct |
| ESP32 OLED says **WiFi FAILED** | Verify WiFi SSID (`hello`) and password (`12345678`) are correct, and the hotspot is on |
| ESP32 connects but no data on dashboard | Make sure your PC firewall allows incoming connections on port 3000 |
| `npm install` fails | Make sure Node.js is installed: `node --version` |

## Communication Flow

```
ESP32 (sensors)
      │
      │  WiFi HTTP POST (JSON)
      │  every 500ms
      ▼
Node.js Server (port 3000)
      │
      │  GET /data (polling)
      │  every 800ms
      ▼
Browser Dashboard (index.html)
```

## WiFi Configuration

- **SSID:** `hello`
- **Password:** `12345678`
- **Protocol:** HTTP POST with JSON body
- **Endpoint:** `POST /api/sensor`
- **Data format:**

```json
{
  "temperature": 25.3,
  "humidity": 60.1,
  "distance": 120.5,
  "gas": 450,
  "gasAlert": false,
  "motion": true,
  "sos": false
}
```
