/*
  VirgilJSP - Basic Sensor Example

  Sensore semplice sempre alimentato.
  Rileva acqua su GPIO 4 e invia
  allarme a Vergus Virgil via JSP.

  Hardware:
  - ESP32 qualsiasi
  - Sensore umidità su GPIO 4
*/

#include <WiFi.h>
#include <VirgilJSP.h>

const char* SSID     = "tuarete";
const char* PASSWORD = "tuapassword";
const char* VIRGIL   = "vergus-virgil.local";

#define SENSOR_PIN 4

VirgilJSP sensor("sensor_01",
                  "Cucina",
                  VIRGIL);

void setup() {
    Serial.begin(115200);
    pinMode(SENSOR_PIN, INPUT);

    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnesso!");
}

void loop() {
    bool waterDetected =
        digitalRead(SENSOR_PIN);

    sensor.sendAlarm(waterDetected);
    sensor.loop(); // heartbeat automatico

    delay(1000);
}
