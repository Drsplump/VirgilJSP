#include "VirgilJSP.h"

VirgilJSP::VirgilJSP(const char* sensorId,
                     const char* sensorName,
                     const char* virgilHost,
                     uint16_t    port)
    : _id(sensorId),
      _name(sensorName),
      _host(virgilHost),
      _port(port),
      _timeout(JSP_DEFAULT_TIMEOUT_MS),
      _lastHeartbeat(0),
      _autoHeartbeat(true),
      _lastHTTPCode(0),
      _type(JSP_TYPE_WIFI),
      _readingCount(0) {}

bool VirgilJSP::sendAlarm(bool alarm,
                           int  battery,
                           bool control) {
    StaticJsonDocument<256> doc;
    doc[JSP_FIELD_ID]      = _id;
    doc[JSP_FIELD_NAME]    = _name;
    doc[JSP_FIELD_ALARM]   = alarm ?
                             JSP_ALARM_ON :
                             JSP_ALARM_OFF;
    doc[JSP_FIELD_CONTROL] = (alarm && control) ?
                             JSP_CONTROL_ON :
                             JSP_CONTROL_OFF;
    doc[JSP_FIELD_TYPE]    = _type;
    doc[JSP_FIELD_RSSI]    = WiFi.RSSI();

    if (battery >= 0)
        doc[JSP_FIELD_BATTERY] = battery;
    if (_wakeReason.length() > 0)
        doc[JSP_FIELD_WAKE_REASON] = _wakeReason;

    return _sendJSON(doc);
}

bool VirgilJSP::sendHeartbeat(int battery) {
    StaticJsonDocument<256> doc;
    doc[JSP_FIELD_ID]          = _id;
    doc[JSP_FIELD_NAME]        = _name;
    doc[JSP_FIELD_ALARM]       = JSP_ALARM_OFF;
    doc[JSP_FIELD_CONTROL]     = JSP_CONTROL_OFF;
    doc[JSP_FIELD_TYPE]        = _type;
    doc[JSP_FIELD_RSSI]        = WiFi.RSSI();
    doc[JSP_FIELD_WAKE_REASON] = JSP_WAKE_HEARTBEAT;

    if (battery >= 0)
        doc[JSP_FIELD_BATTERY] = battery;

    _lastHeartbeat = millis();
    return _sendJSON(doc);
}

bool VirgilJSP::sendStatus(bool alarm,
                            int  battery,
                            int  rssi) {
    StaticJsonDocument<256> doc;
    doc[JSP_FIELD_ID]      = _id;
    doc[JSP_FIELD_NAME]    = _name;
    doc[JSP_FIELD_ALARM]   = alarm ?
                             JSP_ALARM_ON :
                             JSP_ALARM_OFF;
    doc[JSP_FIELD_CONTROL] = alarm ?
                             JSP_CONTROL_ON :
                             JSP_CONTROL_OFF;
    doc[JSP_FIELD_TYPE]    = _type;
    doc[JSP_FIELD_BATTERY] = battery;
    doc[JSP_FIELD_RSSI]    = rssi > 0 ?
                             rssi :
                             WiFi.RSSI();

    return _sendJSON(doc);
}

void VirgilJSP::addReading(const char* name,
                            float value) {
    addReading(name, value, "");
}

void VirgilJSP::addReading(const char* name,
                            float value,
                            const char* unit) {
    if (_readingCount >= 10) return;
    strncpy(_readings[_readingCount].name,
            name, 31);
    _readings[_readingCount].name[31] = '\0';
    _readings[_readingCount].value = value;
    strncpy(_readings[_readingCount].unit,
            unit, 15);
    _readings[_readingCount].unit[15] = '\0';
    _readingCount++;
}

void VirgilJSP::clearReadings() {
    _readingCount = 0;
}

bool VirgilJSP::sendReadings(int battery) {
    return sendFull(false, battery, false);
}

bool VirgilJSP::sendFull(bool alarm,
                          int  battery,
                          bool control) {
    StaticJsonDocument<512> doc;
    doc[JSP_FIELD_ID]      = _id;
    doc[JSP_FIELD_NAME]    = _name;
    doc[JSP_FIELD_ALARM]   = alarm ?
                             JSP_ALARM_ON :
                             JSP_ALARM_OFF;
    doc[JSP_FIELD_CONTROL] = control ?
                             JSP_CONTROL_ON :
                             JSP_CONTROL_OFF;
    doc[JSP_FIELD_TYPE]    = _type;
    doc[JSP_FIELD_RSSI]    = WiFi.RSSI();

    if (battery >= 0)
        doc[JSP_FIELD_BATTERY] = battery;

    if (_readingCount > 0) {
        JsonObject readings =
            doc.createNestedObject(
                JSP_FIELD_READINGS);
        for (int i = 0; i < _readingCount; i++) {
            if (strlen(_readings[i].unit) > 0) {
                JsonObject r =
                    readings.createNestedObject(
                        _readings[i].name);
                r["value"] = _readings[i].value;
                r["unit"]  = _readings[i].unit;
            } else {
                readings[_readings[i].name] =
                    _readings[i].value;
            }
        }
        clearReadings();
    }

    return _sendJSON(doc);
}

void VirgilJSP::loop() {
    if (!_autoHeartbeat) return;
    if (millis() - _lastHeartbeat >
        JSP_HEARTBEAT_INTERVAL_MS) {
        sendHeartbeat();
    }
}

bool VirgilJSP::isVirgilReachable() {
    String url = "http://" + String(_host) +
                 ":" + String(_port) +
                 JSP_STATUS_ENDPOINT;
    HTTPClient http;
#ifdef ESP8266
    WiFiClient client;
    http.begin(client, url);
#else
    http.begin(url);
#endif
    http.setTimeout(_timeout);
    int code = http.GET();
    http.end();
    return code == JSP_HTTP_OK;
}

bool VirgilJSP::_sendJSON(JsonDocument& doc) {
    if (WiFi.status() != WL_CONNECTED) {
        _lastError = "WiFi not connected";
        return false;
    }

    String payload;
    serializeJson(doc, payload);

    HTTPClient http;
#ifdef ESP8266
    WiFiClient client;
    http.begin(client, _buildURL());
#else
    http.begin(_buildURL());
#endif
    http.setTimeout(_timeout);
    http.addHeader("Content-Type",
                   JSP_CONTENT_TYPE);

    _lastHTTPCode = http.POST(payload);
    http.end();

    if (_lastHTTPCode != JSP_HTTP_OK) {
        _lastError = "HTTP " +
                     String(_lastHTTPCode);
        return false;
    }

    _lastError = "";
    return true;
}

String VirgilJSP::_buildURL() {
    return "http://" + String(_host) +
           ":" + String(_port) +
           JSP_ENDPOINT;
}

uint8_t VirgilJSP::getLastHTTPCode() {
    return _lastHTTPCode;
}

String VirgilJSP::getLastError() {
    return _lastError;
}

void VirgilJSP::setTimeout(uint32_t ms) {
    _timeout = ms;
}

void VirgilJSP::setAutoHeartbeat(bool en) {
    _autoHeartbeat = en;
}

void VirgilJSP::setWakeReason(
    const char* reason) {
    _wakeReason = reason;
}

void VirgilJSP::setType(const char* type) {
    _type = type;
}
