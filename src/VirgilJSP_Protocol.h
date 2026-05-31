#pragma once

#include <Arduino.h>

// ═══════════════════════════════════════
// VIRGIL JSON SENSOR PROTOCOL — v1.0.0
// Open sensor protocol for Vergus Virgil
// ═══════════════════════════════════════

// ── Versione protocollo ─────────────────
#define JSP_PROTOCOL_VERSION        "1.0.0"
#define JSP_PROTOCOL_NAME           "VirgilJSP"

// ── Endpoint ────────────────────────────
#define JSP_ENDPOINT                "/json"
#define JSP_STATUS_ENDPOINT         "/status"
#define JSP_SENSORS_ENDPOINT        "/sensors"

// ── Campi JSON obbligatori ───────────────
#define JSP_FIELD_ID                "id"
#define JSP_FIELD_NAME              "name"
#define JSP_FIELD_ALARM             "alarm"
#define JSP_FIELD_CONTROL           "control"

// ── Campi JSON opzionali ─────────────────
#define JSP_FIELD_BATTERY           "battery"
#define JSP_FIELD_RSSI              "rssi"
#define JSP_FIELD_TYPE              "type"
#define JSP_FIELD_WAKE_REASON       "wake_reason"
#define JSP_FIELD_VOLTAGE           "voltage"
#define JSP_FIELD_READINGS          "readings"

// ── Valori alarm/control ─────────────────
#define JSP_ALARM_OFF               0
#define JSP_ALARM_ON                1
#define JSP_CONTROL_OFF             0
#define JSP_CONTROL_ON              1

// ── Valori type ──────────────────────────
#define JSP_TYPE_WIFI               "wifi"
#define JSP_TYPE_ZIGBEE             "zigbee"
#define JSP_TYPE_ETHERNET           "ethernet"
#define JSP_TYPE_CUSTOM             "custom"

// ── Valori wake_reason ───────────────────
#define JSP_WAKE_FLOOD              "flood"
#define JSP_WAKE_HEARTBEAT          "heartbeat"
#define JSP_WAKE_BOOT               "boot"
#define JSP_WAKE_TIMER              "timer"
#define JSP_WAKE_BUTTON             "button"

// ── Readings ────────────────────────────
#define JSP_READING_TEMPERATURE     "temperature"
#define JSP_READING_HUMIDITY        "humidity"
#define JSP_READING_PRESSURE        "pressure"
#define JSP_READING_CO2             "co2"
#define JSP_READING_LUX             "lux"
#define JSP_READING_VOLTAGE         "voltage"
#define JSP_READING_FLOW            "flow"

// ── Unità di misura ──────────────────────
#define JSP_UNIT_CELSIUS            "C"
#define JSP_UNIT_PERCENT            "%"
#define JSP_UNIT_HPA                "hPa"
#define JSP_UNIT_PPM                "ppm"
#define JSP_UNIT_LUX                "lux"
#define JSP_UNIT_VOLT               "V"
#define JSP_UNIT_LITER_MIN          "L/min"

// ── Limiti protocollo ────────────────────
#define JSP_ID_MAX_LENGTH           20
#define JSP_NAME_MAX_LENGTH         50
#define JSP_BATTERY_MIN             0
#define JSP_BATTERY_MAX             100
#define JSP_MAX_SENSORS             10

// ── HTTP ─────────────────────────────────
#define JSP_HTTP_OK                 200
#define JSP_CONTENT_TYPE            "application/json"
#define JSP_DEFAULT_PORT            80
#define JSP_DEFAULT_TIMEOUT_MS      5000

// ── Heartbeat ────────────────────────────
#define JSP_HEARTBEAT_INTERVAL_MS   30000
#define JSP_SENSOR_TIMEOUT_MS       45000

// ── Messaggio ricevuto (lato receiver) ───

struct JSPReading {
    char  name[32];
    float value;
};

struct JSPMessage {
    String   id;
    String   name;
    bool     alarm;
    bool     control;
    int      battery;       // -1 se non presente
    int      rssi;
    String   type;
    String   wakeReason;
    bool     hasReadings;
    uint32_t timestamp;
    IPAddress senderIP;

    float getReading(const char* readingName) const {
        for (uint8_t i = 0; i < _readingCount; i++)
            if (strcmp(_readings[i].name, readingName) == 0)
                return _readings[i].value;
        return 0.0f;
    }

    bool hasReading(const char* readingName) const {
        for (uint8_t i = 0; i < _readingCount; i++)
            if (strcmp(_readings[i].name, readingName) == 0)
                return true;
        return false;
    }

    // Uso interno del parser — non modificare direttamente
    JSPReading _readings[10];
    uint8_t    _readingCount = 0;
};
