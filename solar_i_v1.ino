#define LDR_PIN 26
#define IMP_PER_KWH 3200

volatile long rawCount = 0;
volatile unsigned long lastPulseTime = 0;
float totalKwh = 0.0;
float currentWatts = 0;

void IRAM_ATTR onPulse() {
  unsigned long now = micros();
  if (now - lastPulseTime > 50000) {
    unsigned long interval = now - lastPulseTime;
    currentWatts = (3600000000.0 / IMP_PER_KWH) / interval;
    lastPulseTime = now;
    rawCount++;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LDR_PIN, INPUT_PULLUP);
  attachInterrupt(
    digitalPinToInterrupt(LDR_PIN),
    onPulse,
    CHANGE
  );
  Serial.println("Solar-i v1.0 — Zenthropic");
}

void loop() {
  // divide raw count by 2 — ON and OFF both count as 1
  long pulseCount = rawCount / 2;
  totalKwh = (float)pulseCount / IMP_PER_KWH;
  
  Serial.print("Pulses: ");
  Serial.print(pulseCount);
  Serial.print(" | kWh: ");
  Serial.print(totalKwh, 4);
  Serial.print(" | Watts: ");
  Serial.println(currentWatts, 1);
  delay(1000);
}
