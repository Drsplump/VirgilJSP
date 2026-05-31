/*
  VirgilJSP - Battery Sensor Example

  Sensore con deep sleep ottimizzato
  per lunga durata della batteria.
  Wake on flood + heartbeat ogni 6 ore.

  Hardware:
  - ESP32 qualsiasi
  - Sensore umidità su GPIO 4
  - Partitore resistivo batteria su GPIO 34
    (100kΩ + 100kΩ)
*/

#include <WiFi.h>
#include <VirgilJSP.h>

const char* SSID     = "tuarete";
const char* PASSWORD = "tuapassword";
const char* VIRGIL   = "vergus-virgil.local";

#define SENSOR_PIN   4
#define BATTERY_PIN  34
#define SLEEP_US     (6ULL * 3600ULL * 1000000ULL)

// RTC memory per WiFi rapido
RTC_DATA_ATTR struct {
    uint8_t  bssid[6];
    uint8_t  channel;
    uint32_t ip;
    uint32_t gateway;
    uint32_t subnet;
    bool     valid = false;
} rtcWifi;

VirgilJSP sensor("sensor_bagno",
                  "Bagno",
                  VIRGIL);

void setup() {
    Serial.begin(115200);

    bool waterDetected =
        digitalRead(SENSOR_PIN);
    int  battery = readBatteryPercent();

    connectWiFi();

    sensor.setTimeout(3000);
    sensor.setWakeReason(getWakeReason());
    sensor.sendStatus(waterDetected, battery);

    saveWiFiToRTC();

    esp_sleep_enable_ext0_wakeup(
        (gpio_num_t)SENSOR_PIN, HIGH);
    esp_sleep_enable_timer_wakeup(SLEEP_US);
    esp_deep_sleep_start();
}

void loop() {}

int readBatteryPercent() {
    uint32_t sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += analogRead(BATTERY_PIN);
        delay(5);
    }
    float v = (sum / 10.0f / 4095.0f)
              * 3.3f * 2.0f;
    float pct = (v - 3.0f) /
                (4.2f - 3.0f) * 100.0f;
    return (int)constrain(pct, 0.0f, 100.0f);
}

const char* getWakeReason() {
    switch(esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_EXT0:
            return JSP_WAKE_FLOOD;
        case ESP_SLEEP_WAKEUP_TIMER:
            return JSP_WAKE_HEARTBEAT;
        default:
            return JSP_WAKE_BOOT;
    }
}

void connectWiFi() {
    if (rtcWifi.valid) {
        WiFi.config(
            IPAddress(rtcWifi.ip),
            IPAddress(rtcWifi.gateway),
            IPAddress(rtcWifi.subnet));
        WiFi.begin(SSID, PASSWORD,
                   rtcWifi.channel,
                   rtcWifi.bssid, true);
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED) {
            if (millis() - start > 2000) {
                rtcWifi.valid = false;
                break;
            }
            delay(50);
        }
    }
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(SSID, PASSWORD);
        while (WiFi.status() != WL_CONNECTED)
            delay(100);
    }
}

void saveWiFiToRTC() {
    memcpy(rtcWifi.bssid, WiFi.BSSID(), 6);
    rtcWifi.channel = WiFi.channel();
    rtcWifi.ip      = (uint32_t)WiFi.localIP();
    rtcWifi.gateway = (uint32_t)WiFi.gatewayIP();
    rtcWifi.subnet  = (uint32_t)WiFi.subnetMask();
    rtcWifi.valid   = true;
}
