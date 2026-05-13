#include <WiFi.h>
#include <FirebaseESP32.h>

// ── CONFIG — fill these in ──────────────────
#define WIFI_SSID     "WIFI USERNAME"
#define WIFI_PASS     "WIFI PASSWORD"
#define FIREBASE_URL  "URL FROM REALTIME DASHBOARD"
#define FIREBASE_AUTH "PASTE YOUR SECRET KEY"
#define DEVICE_ID     "device_001"
#define LDR_PIN       26
// ────────────────────────────────────────────

FirebaseData fbData;
FirebaseAuth auth;
FirebaseConfig config;

// pulse counting variables
volatile long rawCount = 0;
volatile unsigned long lastPulseTime = 0;

void IRAM_ATTR onPulse() {
  unsigned long now = micros();
  if (now - lastPulseTime > 50000) {
    lastPulseTime = now;
    rawCount++;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LDR_PIN, INPUT_PULLUP);
  
  // attach interrupt
  attachInterrupt(
    digitalPinToInterrupt(LDR_PIN),
    onPulse,
    CHANGE
  );

  // connect WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());

  // connect Firebase
config.host = FIREBASE_URL;
config.signer.tokens.legacy_token = FIREBASE_AUTH;
Firebase.begin(&config, &auth);
Firebase.reconnectWiFi(true);
Serial.println("Firebase connected!");

  // set device online status
  String basePath = "/devices/" + String(DEVICE_ID);
  Firebase.setString(fbData, 
    basePath + "/status", "online");
  Firebase.setString(fbData,
    basePath + "/location", "Nagpur");

  Serial.println("Solar-i v1.0 — Zenthropic");
  Serial.println("Waiting for pulses...");
}

unsigned long lastSendTime = 0;

void loop() {
  // send to Firebase every 30 seconds
  if (millis() - lastSendTime >= 30000) {
    lastSendTime = millis();
    
    String basePath = "/devices/" + String(DEVICE_ID);
    
    // send raw pulse count only
    // all calculations happen in dashboard
    Firebase.setInt(fbData,
      basePath + "/raw_pulse_count",
      rawCount / 2); // divide by 2 for ON+OFF

    // send uptime in minutes
    Firebase.setInt(fbData,
      basePath + "/uptime_minutes",
      millis() / 60000);

    // send WiFi signal strength
    Firebase.setInt(fbData,
      basePath + "/wifi_strength",
      WiFi.RSSI());

    // send timestamp
    Firebase.setInt(fbData,
      basePath + "/last_seen",
      millis() / 1000);

    // local serial output
    Serial.print("Sent → Raw pulses: ");
    Serial.print(rawCount / 2);
    Serial.print(" | Uptime: ");
    Serial.print(millis() / 60000);
    Serial.println(" mins");
  }

  // still show local readings every second
  Serial.print("Raw count: ");
  Serial.print(rawCount / 2);
  Serial.println();
  delay(1000);
}
