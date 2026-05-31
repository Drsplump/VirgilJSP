/*
  VirgilJSP - Basic Receiver Example

  Riceve messaggi JSP da qualsiasi sensore
  e li stampa sul seriale.

  Perfetto come punto di partenza per
  costruire il tuo hub JSP personalizzato.
*/

#include <WiFi.h>
#include <VirgilJSP.h>

const char* SSID     = "tuarete";
const char* PASSWORD = "tuapassword";

VirgilJSPReceiver receiver(80);

void setup() {
    Serial.begin(115200);

    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
        delay(500);

    Serial.println("IP: " +
        WiFi.localIP().toString());

    receiver.begin();

    receiver.onAlarm([](JSPMessage msg) {
        Serial.println("ALLARME!");
        Serial.println("  Sensore: " + msg.name);
        Serial.println("  ID: " + msg.id);
        if (msg.battery >= 0)
            Serial.println("  Batteria: " +
                String(msg.battery) + "%");
    });

    receiver.onAlarmClear([](JSPMessage msg) {
        Serial.println("Allarme risolto: "
                       + msg.name);
    });

    receiver.onHeartbeat([](JSPMessage msg) {
        Serial.println(msg.name + " online");
    });

    receiver.onReading([](JSPMessage msg) {
        Serial.println(msg.name);
        if (msg.hasReading(
                JSP_READING_TEMPERATURE)) {
            Serial.println("  Temp: " +
                String(msg.getReading(
                    JSP_READING_TEMPERATURE))
                + "C");
        }
    });

    Serial.println("JSP Receiver pronto!");
}

void loop() {
    receiver.loop();
}
