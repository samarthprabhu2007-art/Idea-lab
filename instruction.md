# IDEA Lab ESP32 Soldier Safety Monitor - Run Instructions

This repository contains an ESP32-based soldier safety and battlefield environment monitoring prototype. The ESP32 reads environmental sensors, shows local status on an OLED display, and sends live data over WiFi to a laptop dashboard.

## 1. What This Project Does

- Reads temperature and humidity using DHT22.
- Reads smoke/gas value using MQ-2.
- Reads proximity distance using HC-SR04 ultrasonic sensor.
- Reads motion using PIR sensor.
- Supports manual emergency alert using SOS button.
- Shows local status on OLED display.
- Activates buzzer during SOS/emergency condition.
- Sends sensor data from ESP32 to laptop using WiFi HTTP POST.
- Shows live data on a browser dashboard.

Important final implementation notes:

- Only one ESP32 is used.
- The ESP32 does not use deep sleep in the final demonstration code.
- There is no second/base-station ESP32.
- The base station is a laptop running the Node.js dashboard server.

## 2. Folder Structure

```text
IDEALAB/
+-- arduino/
|   +-- arduino.ino
+-- esp32-dashboard/
|   +-- index.html
|   +-- package.json
|   +-- package-lock.json
|   +-- server.js
+-- CIRCUIT_DIAGRAM.jpg
+-- Dashboard.png
+-- FINAL_PROTOTYPE.jpg
+-- Final_IDEALAB_Report_Soldier_Safety_ESP32.docx
+-- run.md
+-- instruction.md
```

## 3. Required Hardware

- ESP32 DevKit board
- DHT22 temperature and humidity sensor
- MQ-2 gas/smoke sensor
- HC-SR04 ultrasonic sensor
- PIR motion sensor
- SH1106 128x64 OLED display
- Push button for SOS
- Buzzer
- Breadboard and jumper wires
- Laptop
- USB cable for ESP32
- WiFi hotspot/router

## 4. Required Software

Install these before running:

- Arduino IDE 2.x or newer
- Node.js 18 or newer
- Git
- ESP32 board package in Arduino IDE

Arduino libraries required:

- `U8g2` by oliver
- `DHT sensor library` by Adafruit
- `Adafruit Unified Sensor`

## 5. Clone the Repository

Open terminal or PowerShell and run:

```bash
git clone https://github.com/samarthprabhu2007-art/Idea-lab.git
cd Idea-lab
```

## 6. Hardware Wiring

Connect the modules as follows:

| Module / Signal | ESP32 Pin |
|---|---|
| DHT22 Data | GPIO 4 |
| MQ-2 Analog Output | GPIO 34 |
| MQ-2 Digital Output | GPIO 35 |
| HC-SR04 TRIG | GPIO 5 |
| HC-SR04 ECHO | GPIO 18 |
| PIR Sensor OUT | GPIO 19 |
| SOS Button | GPIO 27 |
| Buzzer | GPIO 23 |
| OLED SDA | GPIO 21 |
| OLED SCL | GPIO 22 |
| OLED VCC | 3.3V or 5V depending on module |
| OLED GND | GND |

Make sure all sensor GND pins are connected to ESP32 GND.

## 7. Start the Dashboard Server

Open terminal in the project folder:

```bash
cd esp32-dashboard
npm install
npm start
```

The dashboard server runs on:

```text
http://localhost:3000
```

Keep this terminal open while testing.

## 8. Find Your Laptop IP Address

The ESP32 must send data to your laptop IP address.

On Windows, open Command Prompt or PowerShell:

```bash
ipconfig
```

Find the IPv4 address under your WiFi adapter.

Example:

```text
IPv4 Address . . . . . . . . . . . : 192.168.1.50
```

Then your ESP32 server endpoint should be:

```text
http://192.168.1.50:3000/api/sensor
```

## 9. Update WiFi and Server IP in Arduino Code

Open:

```text
arduino/arduino.ino
```

Update these lines:

```cpp
const char* WIFI_SSID = "hello";
const char* WIFI_PASS = "12345678";
String serverURL = "http://YOUR_LAPTOP_IP:3000/api/sensor";
```

Example:

```cpp
const char* WIFI_SSID = "MyHotspot";
const char* WIFI_PASS = "mypassword123";
String serverURL = "http://192.168.1.50:3000/api/sensor";
```

The ESP32 and laptop must be connected to the same WiFi network.

## 10. Upload Code to ESP32

1. Open Arduino IDE.
2. Open `arduino/arduino.ino`.
3. Select the correct ESP32 board from `Tools > Board`.
4. Select the correct COM port from `Tools > Port`.
5. Install missing libraries if Arduino IDE asks.
6. Click Upload.
7. Open Serial Monitor at `115200` baud to check WiFi and sensor status.

## 11. Open the Dashboard

After uploading ESP32 code and starting the server, open this on the laptop:

```text
http://localhost:3000
```

The dashboard should show:

- Temperature
- Humidity
- Ultrasonic distance
- Smoke/gas value
- PIR motion status
- Emergency status
- Radar visualization
- Event log
- Online/offline status

## 12. Testing Procedure

Use this checklist during demonstration:

1. Start the Node.js dashboard server.
2. Upload the ESP32 code.
3. Confirm ESP32 connects to WiFi.
4. Open dashboard on laptop.
5. Check temperature and humidity readings.
6. Move an object near HC-SR04 and verify distance/radar changes.
7. Trigger PIR motion and verify dashboard changes.
8. Press SOS button and verify buzzer plus emergency status.
9. Observe MQ-2 gas value changes.

## 13. Troubleshooting

### Dashboard shows offline

- Check that the dashboard server is running.
- Check ESP32 and laptop are on the same WiFi.
- Check `serverURL` in `arduino.ino`.
- Check Windows firewall allows port `3000`.

### ESP32 says WiFi failed

- Recheck WiFi SSID and password.
- Make sure hotspot/router is turned on.
- Keep ESP32 close to the WiFi source.

### ESP32 connects but dashboard does not update

- Make sure the IP in `serverURL` is the laptop IPv4 address.
- Do not use `localhost` in ESP32 code.
- Restart `npm start`.
- Re-upload Arduino code after changing IP.

### Arduino upload fails

- Select correct ESP32 board.
- Select correct COM port.
- Hold BOOT button while upload starts if required by your board.
- Use a data USB cable, not a charging-only cable.

### Node.js command fails

Run:

```bash
node --version
npm --version
```

If these commands fail, install Node.js again.

## 14. Useful Commands

Start dashboard:

```bash
cd esp32-dashboard
npm install
npm start
```

Check Git status:

```bash
git status
```

Pull latest code:

```bash
git pull
```

## 15. Report and Images

The final report is included:

```text
Final_IDEALAB_Report_Soldier_Safety_ESP32.docx
```

Important images included:

- `FINAL_PROTOTYPE.jpg`
- `Dashboard.png`
- `CIRCUIT_DIAGRAM.jpg`

Note: Some older research diagrams may show two ESP32 boards or deep sleep. The final implemented project uses one ESP32 and no deep sleep.

## 16. Communication Flow

```text
ESP32 sensors
    |
    | WiFi HTTP POST JSON
    v
Laptop Node.js server
    |
    | Browser fetch /data
    v
Dashboard at http://localhost:3000
```

JSON data format:

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

## 17. Team Details

- Saamarth S - 1RV25CS151
- Satvik Tripathi - 1RV25CS162
- Samarth S Prabhu - 1RZ25CS153
- Sanchit Mahajan - 1RZ25CS154

Guide:

- Dr. Vivekanand S Gogi
