#include <U8g2lib.h>
#include <Wire.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ── WiFi credentials ──
const char* WIFI_SSID = "hello";
const char* WIFI_PASS = "12345678";

// ── Server endpoint (update this IP to your PC's local IP) ──
String serverURL = "http://10.124.31.200:3000/api/sensor";

// ── Pin definitions ──
#define SOS_BUTTON  27
#define BUZZER      23
#define DHT_PIN     4
#define DHT_TYPE    DHT22
#define TRIG_PIN    5
#define ECHO_PIN    18
#define MQ2_AO      34
#define MQ2_DO      35
#define PIR_PIN     19

DHT dht(DHT_PIN, DHT_TYPE);

bool sosActive = false;
unsigned long sosStartTime = 0;
unsigned long lastDHTRead = 0;
unsigned long lastRadarUpdate = 0;
unsigned long lastWiFiSend = 0;

const unsigned long SOS_DURATION   = 3000;
const unsigned long DHT_INTERVAL   = 2000;
const unsigned long RADAR_INTERVAL = 200;
const unsigned long WIFI_SEND_INTERVAL = 500;  // Send data every 500ms

float temperature = 0;
float humidity = 0;
float distance = 0;
int gasValue = 0;
bool gasAlert = false;
bool motionDetected = false;
bool wifiConnected = false;

#define RADAR_CX  100
#define RADAR_CY  42
#define RADAR_R1  10
#define RADAR_R2  18
#define RADAR_R3  26

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return 400;
  return duration * 0.0343 / 2.0;
}

void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  
  // Show connecting status on OLED
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(10, 20, "Connecting to");
  u8g2.drawStr(10, 35, "WiFi...");
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(10, 55, WIFI_SSID);
  u8g2.sendBuffer();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Show connected status on OLED
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(10, 20, "WiFi Connected!");
    u8g2.setFont(u8g2_font_5x7_tr);
    char ipStr[20];
    WiFi.localIP().toString().toCharArray(ipStr, 20);
    u8g2.drawStr(10, 40, ipStr);
    u8g2.drawStr(10, 55, "Starting sensors...");
    u8g2.sendBuffer();
    delay(1500);
  } else {
    wifiConnected = false;
    Serial.println("\nWiFi connection failed!");
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(10, 20, "WiFi FAILED!");
    u8g2.drawStr(10, 40, "Check credentials");
    u8g2.sendBuffer();
    delay(2000);
  }
}

void sendDataOverWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    Serial.println("WiFi disconnected, reconnecting...");
    connectWiFi();
    return;
  }

  wifiConnected = true;
  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/json");

  // Build JSON payload
  String json = "{";
  json += "\"temperature\":" + String(temperature, 1) + ",";
  json += "\"humidity\":" + String(humidity, 1) + ",";
  json += "\"distance\":" + String(distance, 1) + ",";
  json += "\"gas\":" + String(gasValue) + ",";
  json += "\"gasAlert\":" + String(gasAlert ? "true" : "false") + ",";
  json += "\"motion\":" + String(motionDetected ? "true" : "false") + ",";
  json += "\"sos\":" + String(sosActive ? "true" : "false");
  json += "}";

  int httpCode = http.POST(json);

  if (httpCode > 0) {
    Serial.print("WiFi POST → ");
    Serial.println(httpCode);
  } else {
    Serial.print("WiFi POST failed: ");
    Serial.println(http.errorToString(httpCode));
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  pinMode(SOS_BUTTON, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(MQ2_DO, INPUT);
  pinMode(PIR_PIN, INPUT);
  digitalWrite(BUZZER, LOW);
  dht.begin();
  u8g2.begin();

  // Connect to WiFi before starting main loop
  connectWiFi();

  showRadar();
}

void loop() {
  // Read DHT every 2 seconds
  if (millis() - lastDHTRead >= DHT_INTERVAL) {
    lastDHTRead = millis();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      temperature = t;
      humidity = h;
      Serial.print("Temp: "); Serial.println(temperature);
      Serial.print("Humidity: "); Serial.println(humidity);
    }
  }

  // Read ultrasonic + gas + PIR every 200ms
  if (millis() - lastRadarUpdate >= RADAR_INTERVAL) {
    lastRadarUpdate = millis();
    distance = getDistance();
    gasValue = analogRead(MQ2_AO);
    gasAlert = (digitalRead(MQ2_DO) == HIGH);
    motionDetected = (digitalRead(PIR_PIN) == HIGH);

    Serial.print("Distance: "); Serial.print(distance);
    Serial.print(" | Gas: "); Serial.print(gasValue);
    Serial.print(" | Alert: "); Serial.print(gasAlert);
    Serial.print(" | Motion: "); Serial.println(motionDetected ? "1" : "0");

    if (!sosActive) {
      showRadar();
    }
  }

  // Send data over WiFi periodically
  if (millis() - lastWiFiSend >= WIFI_SEND_INTERVAL) {
    lastWiFiSend = millis();
    sendDataOverWiFi();
  }

  // SOS button
  if (digitalRead(SOS_BUTTON) == LOW && !sosActive) {
    delay(50);
    if (digitalRead(SOS_BUTTON) == LOW) {
      sosActive = true;
      sosStartTime = millis();
      digitalWrite(BUZZER, HIGH);
      Serial.println("SOS:1");
      showSOS();

      // Immediately send SOS over WiFi
      sendDataOverWiFi();
    }
  }

  // SOS timeout
  if (sosActive && millis() - sosStartTime >= SOS_DURATION) {
    sosActive = false;
    digitalWrite(BUZZER, LOW);
    Serial.println("SOS:0");
    showRadar();

    // Immediately send SOS clear over WiFi
    sendDataOverWiFi();
  }
}

void showRadar() {
  u8g2.clearBuffer();

  // Top bar: Temp + Humidity + WiFi indicator
  char topBar[30];
  char tStr[8], hStr[8];
  dtostrf(temperature, 4, 1, tStr);
  dtostrf(humidity, 4, 1, hStr);
  sprintf(topBar, "T:%sC H:%s%%", tStr, hStr);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(0, 7, topBar);

  // WiFi status indicator (top-right corner)
  if (wifiConnected) {
    u8g2.drawStr(115, 7, "W");
  } else {
    u8g2.drawStr(115, 7, "!");
  }

  u8g2.drawHLine(0, 10, 128);

  u8g2.setFont(u8g2_font_6x10_tr);

  char distStr[20];
  if (distance >= 400) {
    sprintf(distStr, "D:--");
  } else {
    char dVal[8];
    dtostrf(distance / 100.0, 4, 2, dVal);
    sprintf(distStr, "D:%sm", dVal);
  }
  u8g2.drawStr(0, 22, distStr);

  char gasStr[20];
  sprintf(gasStr, "G:%d", gasValue);
  u8g2.drawStr(0, 34, gasStr);

  // Priority: GAS > MOTION > DISTANCE
  if (gasAlert) {
    u8g2.drawStr(0, 46, "GAS!");
    u8g2.drawStr(0, 58, "TOXIC!");
  } else if (motionDetected) {
    u8g2.drawStr(0, 46, "MOTION");
    u8g2.drawStr(0, 58, "DETECT");
  } else if (distance < 50) {
    u8g2.drawStr(0, 46, "DANGER");
    u8g2.drawStr(0, 58, "CLOSE!");
  } else if (distance < 150) {
    u8g2.drawStr(0, 46, "CAUTION");
    u8g2.drawStr(0, 58, "nearby");
  } else {
    u8g2.drawStr(0, 46, "SAFE");
    u8g2.drawStr(0, 58, "Clear");
  }

  u8g2.drawCircle(RADAR_CX, RADAR_CY, RADAR_R1);
  u8g2.drawCircle(RADAR_CX, RADAR_CY, RADAR_R2);
  u8g2.drawCircle(RADAR_CX, RADAR_CY, RADAR_R3);

  u8g2.drawVLine(RADAR_CX, RADAR_CY - RADAR_R3, RADAR_R3 * 2);
  u8g2.drawHLine(RADAR_CX - RADAR_R3, RADAR_CY, RADAR_R3 * 2);

  u8g2.drawBox(RADAR_CX - 1, RADAR_CY - 1, 3, 3);

  if (distance < 400) {
    int dotY;
    if (distance < 50) {
      dotY = RADAR_CY - RADAR_R1 + 2;
    } else if (distance < 150) {
      dotY = RADAR_CY - RADAR_R2 + 2;
    } else {
      dotY = RADAR_CY - RADAR_R3 + 2;
    }
    u8g2.drawDisc(RADAR_CX, dotY, 3);
  }

  u8g2.sendBuffer();
}

void showSOS() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(20, 30, "!! SOS !!");
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(10, 50, "EMERGENCY ALERT");
  u8g2.drawStr(20, 63, "Sending...");
  u8g2.drawFrame(0, 0, 128, 64);
  u8g2.drawFrame(2, 2, 124, 60);
  u8g2.sendBuffer();
}