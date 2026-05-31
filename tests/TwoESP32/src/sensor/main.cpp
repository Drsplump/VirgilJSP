#include <Arduino.h>
#include <WiFi.h>
#include <VirgilJSP.h>

#include "app_config.h"

namespace {
constexpr uint32_t kWifiConnectTimeoutMs = 15000;
constexpr uint32_t kLoopDelayMs = 1000;

VirgilJSP sensor(kSensorId, kSensorName, kReceiverHost, kReceiverPort);

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(kWifiSsid, kWifiPassword);

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < kWifiConnectTimeoutMs) {
    delay(300);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[esp32sensor] WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[esp32sensor] WiFi connect timeout");
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  // Typical dry-contact wiring: idle HIGH via pull-up, alarm when pulled LOW.
  pinMode(kSensorPin, INPUT_PULLUP);
  delay(300);

  Serial.println("[esp32sensor] boot");
  connectWiFi();

  sensor.setType(JSP_TYPE_WIFI);
  sensor.setWakeReason(JSP_WAKE_BOOT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  const int rawPin = digitalRead(kSensorPin);
  const bool waterDetected = kSensorActiveLow ?
    (rawPin == LOW) :
    (rawPin == HIGH);
  const bool ok = sensor.sendAlarm(waterDetected);

  Serial.print("[esp32sensor] pin=");
  Serial.print(rawPin);
  Serial.print(" activeLow=");
  Serial.print(kSensorActiveLow ? 1 : 0);
  Serial.print(" ");
  Serial.print("[esp32sensor] alarm=");
  Serial.print(waterDetected ? 1 : 0);
  Serial.print(" send=");
  Serial.println(ok ? "ok" : "fail");

  sensor.loop();
  delay(kLoopDelayMs);
}
