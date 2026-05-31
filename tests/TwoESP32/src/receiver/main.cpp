#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <VirgilJSP.h>

#include "app_config.h"

namespace {
constexpr uint32_t kWifiConnectTimeoutMs = 15000;

VirgilJSPReceiver receiver(kReceiverPort);

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
    Serial.print("[esp32receiver] WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[esp32receiver] WiFi connect timeout");
  }
}

void setupMdns() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (MDNS.begin("esp32receiver")) {
    MDNS.addService("http", "tcp", kReceiverPort);
    Serial.println("[esp32receiver] mDNS: esp32receiver.local");
  } else {
    Serial.println("[esp32receiver] mDNS start failed");
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println("[esp32receiver] boot");
  connectWiFi();
  setupMdns();

  receiver.begin();

  receiver.onAny([](JSPMessage msg) {
    Serial.print("[ANY] id=");
    Serial.print(msg.id);
    Serial.print(" alarm=");
    Serial.print(msg.alarm ? 1 : 0);
    Serial.print(" control=");
    Serial.print(msg.control ? 1 : 0);
    Serial.print(" wake=");
    Serial.println(msg.wakeReason);
  });

  receiver.onAlarm([](JSPMessage msg) {
    Serial.print("[ALARM] ");
    Serial.print(msg.name);
    Serial.print(" id=");
    Serial.print(msg.id);
    Serial.print(" ip=");
    Serial.println(msg.senderIP);
  });

  receiver.onAlarmClear([](JSPMessage msg) {
    Serial.print("[CLEAR] ");
    Serial.println(msg.name);
  });

  receiver.onHeartbeat([](JSPMessage msg) {
    Serial.print("[HEARTBEAT] ");
    Serial.println(msg.name);
  });

  receiver.onReading([](JSPMessage msg) {
    Serial.print("[READING] from ");
    Serial.println(msg.name);
  });

  Serial.println("[esp32receiver] VirgilJSP receiver ready");
}

void loop() {
  receiver.loop();
}
