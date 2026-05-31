#pragma once

#include <Arduino.h>
#include "VirgilJSP_Protocol.h"

#ifdef ESP32
  #include <WiFi.h>
  #include <HTTPClient.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClient.h>
#endif

#include <ArduinoJson.h>

class VirgilJSP {
public:
    VirgilJSP(const char* sensorId,
              const char* sensorName,
              const char* virgilHost,
              uint16_t    port = JSP_DEFAULT_PORT);

    // ── Metodi principali ────────────────
    bool sendAlarm(bool alarm,
                   int  battery = -1,
                   bool control = true);

    bool sendHeartbeat(int battery = -1);

    bool sendStatus(bool alarm,
                    int  battery,
                    int  rssi = 0);

    // ── Readings ─────────────────────────
    void addReading(const char* name,
                    float value);
    void addReading(const char* name,
                    float value,
                    const char* unit);
    void clearReadings();
    bool sendReadings(int battery = -1);
    bool sendFull(bool alarm,
                  int  battery = -1,
                  bool control = false);

    // ── Configurazione ───────────────────
    void setTimeout(uint32_t ms);
    void setAutoHeartbeat(bool enabled);
    void setWakeReason(const char* reason);
    void setType(const char* type);

    // ── Loop ─────────────────────────────
    void loop();

    // ── Diagnostica ──────────────────────
    bool    isVirgilReachable();
    uint8_t getLastHTTPCode();
    String  getLastError();

private:
    const char* _id;
    const char* _name;
    const char* _host;
    uint16_t    _port;
    uint32_t    _timeout;
    uint32_t    _lastHeartbeat;
    bool        _autoHeartbeat;
    uint8_t     _lastHTTPCode;
    String      _lastError;
    String      _wakeReason;
    String      _type;

    struct Reading {
        char  name[32];
        float value;
        char  unit[16];
    };
    Reading _readings[10];
    uint8_t _readingCount = 0;

    bool   _sendJSON(JsonDocument& doc);
    String _buildURL();
};

// Includi il receiver come parte dell'unico header pubblico
#include "VirgilJSPReceiver.h"
