# VirgilJSP

**Virgil JSON Sensor Protocol** — Open sensor 
protocol library for ESP32 and ESP8266.

Connect any sensor to [Vergus Virgil](https://github.com/Vergus) 
smart water monitor — or build your own 
JSP-compatible hub.

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Platform](https://img.shields.io/badge/platform-ESP32%20%7C%20ESP8266-orange)

---

## 🇬🇧 English

### What is VirgilJSP?

VirgilJSP is an open Arduino library that 
implements the **Virgil JSON Sensor Protocol** — 
a lightweight HTTP/JSON protocol designed to 
connect flood sensors, temperature probes and 
any IoT device to a central hub.

Born as the communication layer of 
[Vergus Virgil](https://github.com/Vergus) 
water monitoring system, JSP is now an 
independent open protocol that anyone 
can implement.

**One include. Everything included.**

```cpp
#include <VirgilJSP.h>
// Both sender and receiver are available
```

---

### ✨ Features

**Sender (VirgilJSP):**
- Send alarm events with battery level
- Automatic heartbeat management
- Deep sleep optimized (RTC memory)
- Optional sensor readings (temperature, 
  humidity, pressure, CO2...)
- Compatible with Vergus Virgil and any 
  JSP receiver

**Receiver (VirgilJSPReceiver):**
- Receive JSP messages from any sensor
- Event callbacks (alarm, clear, heartbeat, 
  readings)
- Automatic sensor online/offline tracking
- Up to 10 simultaneous sensors
- Alarm transition detection
- Compatible with ESP32 and ESP8266

---

### 📦 Installation

**Arduino IDE:**
```
Sketch → Include Library → Manage Libraries
→ Search "VirgilJSP" → Install
```

**PlatformIO:**
```ini
lib_deps = Vergus/VirgilJSP
```

**Manual:**
Download ZIP and extract to your 
Arduino libraries folder.

---

### ⚡ Scaling beyond 3 sensors

By default VirgilJSP uses the synchronous
`WebServer` — zero extra dependencies, works
on Arduino IDE and PlatformIO, perfect for
2-3 sensors.

For 10+ sensors enable the async backend
via PlatformIO (`platformio.ini`):

```ini
lib_deps =
    Vergus/VirgilJSP
    ESP Async WebServer
build_flags = -DJSP_USE_ASYNC
```

`loop()` becomes a no-op — the server handles
requests automatically in the background.
No other code changes needed.

> **Note:** `JSP_USE_ASYNC` is a PlatformIO
> feature. In Arduino IDE the default
> synchronous server handles up to ~5 sensors
> comfortably at 30s heartbeat intervals.

---

### 🚀 Quick Start

#### Sender — detect water and send alarm

```cpp
#include <WiFi.h>
#include <VirgilJSP.h>

VirgilJSP sensor("kitchen_01",
                  "Kitchen",
                  "vergus-virgil.local");

void setup() {
    WiFi.begin("ssid", "password");
    while (WiFi.status() != WL_CONNECTED)
        delay(500);
}

void loop() {
    bool water = digitalRead(SENSOR_PIN);
    sensor.sendAlarm(water);
    sensor.loop(); // automatic heartbeat
    delay(1000);
}
```

#### Receiver — build your own JSP hub

```cpp
#include <WiFi.h>
#include <VirgilJSP.h>

VirgilJSPReceiver hub(80);

void setup() {
    WiFi.begin("ssid", "password");
    while (WiFi.status() != WL_CONNECTED)
        delay(500);

    hub.begin();

    hub.onAlarm([](JSPMessage msg) {
        Serial.println("ALARM: " + msg.name);
    });

    hub.onHeartbeat([](JSPMessage msg) {
        Serial.println(msg.name + " online");
    });
}

void loop() {
    hub.loop();
}
```

---

### 📡 The JSP Protocol

Any device can speak JSP with a simple 
HTTP POST:

```json
POST http://your-hub.local/json
Content-Type: application/json

{
  "id":      "sensor_01",
  "name":    "Kitchen",
  "alarm":   1,
  "control": 1,
  "battery": 87,
  "type":    "wifi"
}
```

**Optional readings:**
```json
{
  "id":    "sensor_02",
  "name":  "Boiler Room",
  "alarm": 0,
  "readings": {
    "temperature": 23.5,
    "humidity":    67.2
  }
}
```

**Response:**
```json
{"ok": 1}
```

---

### 📚 API Reference

#### VirgilJSP (Sender)

```cpp
// Constructor
VirgilJSP sensor(id, name, host, port=80);

// Send events
sensor.sendAlarm(bool alarm, int battery=-1);
sensor.sendHeartbeat(int battery=-1);
sensor.sendStatus(bool alarm, int battery);

// Send readings
sensor.addReading("temperature", 23.5, "C");
sensor.addReading("humidity", 67.2, "%");
sensor.sendReadings(int battery=-1);
sensor.sendFull(bool alarm, int battery=-1);

// Configuration
sensor.setTimeout(ms);
sensor.setAutoHeartbeat(bool);
sensor.setWakeReason(JSP_WAKE_FLOOD);
sensor.setType(JSP_TYPE_WIFI);

// Diagnostics
sensor.isVirgilReachable();
sensor.getLastHTTPCode();
sensor.getLastError();

// Loop (call in loop())
sensor.loop();
```

#### VirgilJSPReceiver (Receiver)

```cpp
// Constructor
VirgilJSPReceiver hub(port=80);

// Setup
hub.begin();
hub.loop(); // call in loop()

// Callbacks
hub.onAlarm([](JSPMessage msg) { ... });
hub.onAlarmClear([](JSPMessage msg) { ... });
hub.onHeartbeat([](JSPMessage msg) { ... });
hub.onReading([](JSPMessage msg) { ... });
hub.onAny([](JSPMessage msg) { ... });

// Sensor status
hub.getSensorCount();
hub.isSensorOnline("sensor_id");
hub.getSensorBattery("sensor_id");
```

#### JSPMessage fields

```cpp
msg.id          // sensor ID
msg.name        // sensor name
msg.alarm       // true = alarm active
msg.control     // true = close valve
msg.battery     // 0-100, -1 if absent
msg.rssi        // WiFi signal strength
msg.type        // "wifi", "zigbee"...
msg.wakeReason  // "flood", "heartbeat"...
msg.hasReadings // true if readings present
msg.timestamp   // millis() at reception
msg.senderIP    // sender IP address

// Readings
msg.hasReading("temperature");
msg.getReading("temperature"); // returns float
```

---

### 🔌 Protocol Constants

```cpp
// Types
JSP_TYPE_WIFI       // "wifi"
JSP_TYPE_ZIGBEE     // "zigbee"
JSP_TYPE_ETHERNET   // "ethernet"
JSP_TYPE_CUSTOM     // "custom"

// Wake reasons
JSP_WAKE_FLOOD      // "flood"
JSP_WAKE_HEARTBEAT  // "heartbeat"
JSP_WAKE_BOOT       // "boot"
JSP_WAKE_TIMER      // "timer"

// Readings
JSP_READING_TEMPERATURE  // "temperature"
JSP_READING_HUMIDITY     // "humidity"
JSP_READING_PRESSURE     // "pressure"
JSP_READING_CO2          // "co2"
JSP_READING_LUX          // "lux"
JSP_READING_VOLTAGE      // "voltage"
JSP_READING_FLOW         // "flow"

// Units
JSP_UNIT_CELSIUS    // "C"
JSP_UNIT_PERCENT    // "%"
JSP_UNIT_HPA        // "hPa"
JSP_UNIT_PPM        // "ppm"
JSP_UNIT_VOLT       // "V"
JSP_UNIT_LITER_MIN  // "L/min"
```

---

### 💡 Examples

| Example | Description |
|---------|-------------|
| BasicSensor | Simple always-on sensor |
| BatterySensor | Deep sleep + wake on flood |
| BasicReceiver | Minimal JSP hub receiver |

---

### 🏠 Compatible with

- **Vergus Virgil** — smart water monitor
  with native JSP support
- Any ESP32/ESP8266 running VirgilJSPReceiver
- Home Assistant (via Vergus Virgil MQTT)
- ESPHome (via HTTP POST webhook)
- Tasmota (via webhook rules)
- Node-RED (via HTTP request node)
- Any platform supporting HTTP POST

---

### 📋 Hardware tested

| Board | Status |
|-------|--------|
| ESP32 DevKit | ✅ |
| ESP32-S3 | ✅ |
| ESP32-C6 | ✅ |
| ESP8266 NodeMCU | ✅ |
| XIAO ESP32C6 | ✅ |

---

### ⚠️ Disclaimer

VirgilJSP is designed for home automation 
and maker projects. It is not a certified 
safety system and does not replace 
professional plumbing maintenance.

---

### 📄 License

MIT — free for personal and commercial use.

---

### 🔗 Links

- [Vergus Virgil](https://github.com/Vergus) 
  — smart water monitor
- [JSP Protocol Specification](#the-jsp-protocol)
- [Arduino Library Manager](#installation)

---

*Built with ❤️ by 
[Vergus](https://github.com/Vergus) —
an Italian plumber who codes.*

---
---

## 🇮🇹 Italiano

### Cos'è VirgilJSP?

VirgilJSP è una libreria Arduino open source 
che implementa il **Virgil JSON Sensor Protocol** —
un protocollo HTTP/JSON leggero progettato per 
connettere sensori di allagamento, sonde di 
temperatura e qualsiasi dispositivo IoT 
a un hub centrale.

Nato come layer di comunicazione di 
[Vergus Virgil](https://github.com/Vergus),
sistema di monitoraggio idrico intelligente,
JSP è ora un protocollo aperto indipendente 
che chiunque può implementare.

**Un solo include. Tutto incluso.**

```cpp
#include <VirgilJSP.h>
// Mittente e ricevente disponibili
```

---

### ✨ Caratteristiche

**Mittente (VirgilJSP):**
- Invia eventi di allarme con livello batteria
- Gestione heartbeat automatica
- Ottimizzato per deep sleep (RTC memory)
- Letture opzionali (temperatura, umidità, 
  pressione, CO2...)
- Compatibile con Vergus Virgil e qualsiasi 
  ricevente JSP

**Ricevente (VirgilJSPReceiver):**
- Riceve messaggi JSP da qualsiasi sensore
- Callback per eventi (allarme, fine allarme, 
  heartbeat, letture)
- Tracciamento automatico sensori online/offline
- Fino a 10 sensori simultanei
- Rilevamento transizione allarme
- Compatibile con ESP32 e ESP8266

---

### 📦 Installazione

**Arduino IDE:**
```
Sketch → Includi libreria → Gestisci librerie
→ Cerca "VirgilJSP" → Installa
```

**PlatformIO:**
```ini
lib_deps = Vergus/VirgilJSP
```

---

### ⚡ Più di 3 sensori

Di default VirgilJSP usa il `WebServer`
sincrono — zero dipendenze extra, funziona
su Arduino IDE e PlatformIO, perfetto per
2-3 sensori.

Per 10+ sensori abilita il backend asincrono
via PlatformIO (`platformio.ini`):

```ini
lib_deps =
    Vergus/VirgilJSP
    ESP Async WebServer
build_flags = -DJSP_USE_ASYNC
```

`loop()` diventa un no-op — il server gestisce
le richieste automaticamente in background.
Nessun'altra modifica al codice necessaria.

> **Nota:** `JSP_USE_ASYNC` è una funzionalità
> PlatformIO. In Arduino IDE il server sincrono
> gestisce comodamente fino a ~5 sensori con
> heartbeat a 30s.

---

### 🚀 Avvio rapido

#### Mittente — rileva acqua e invia allarme

```cpp
#include <WiFi.h>
#include <VirgilJSP.h>

VirgilJSP sensore("cucina_01",
                   "Cucina",
                   "vergus-virgil.local");

void setup() {
    WiFi.begin("ssid", "password");
    while (WiFi.status() != WL_CONNECTED)
        delay(500);
}

void loop() {
    bool acqua = digitalRead(SENSOR_PIN);
    sensore.sendAlarm(acqua);
    sensore.loop(); // heartbeat automatico
    delay(1000);
}
```

#### Ricevente — costruisci il tuo hub JSP

```cpp
#include <WiFi.h>
#include <VirgilJSP.h>

VirgilJSPReceiver hub(80);

void setup() {
    WiFi.begin("ssid", "password");
    while (WiFi.status() != WL_CONNECTED)
        delay(500);

    hub.begin();

    hub.onAlarm([](JSPMessage msg) {
        Serial.println("ALLARME: " + msg.name);
    });

    hub.onHeartbeat([](JSPMessage msg) {
        Serial.println(msg.name + " online");
    });
}

void loop() {
    hub.loop();
}
```

---

### 📡 Il Protocollo JSP

Qualsiasi dispositivo può parlare JSP 
con un semplice HTTP POST:

```json
POST http://tuo-hub.local/json
Content-Type: application/json

{
  "id":      "sensore_01",
  "name":    "Cucina",
  "alarm":   1,
  "control": 1,
  "battery": 87,
  "type":    "wifi"
}
```

**Risposta:**
```json
{"ok": 1}
```

---

### 🏠 Compatibile con

- **Vergus Virgil** — monitor idrico intelligente
  con supporto JSP nativo
- Home Assistant (via MQTT Vergus Virgil)
- ESPHome (via webhook HTTP POST)
- Tasmota (via regole webhook)
- Node-RED (via nodo HTTP request)
- Shelly Gen4 (via webhook configurabile)
- Qualsiasi piattaforma con supporto HTTP POST

---

### ⚠️ Disclaimer

VirgilJSP è progettato per progetti di 
domotica e maker. Non è un sistema di 
sicurezza certificato e non sostituisce 
la manutenzione professionale dell'impianto.

---

### 📄 Licenza

MIT — libero uso personale e commerciale.

---

*Costruito con ❤️ da 
[Vergus](https://github.com/Vergus) —
un idraulico italiano che programma.*