#include "VirgilJSPReceiver.h"
#include <ArduinoJson.h>

VirgilJSPReceiver::VirgilJSPReceiver(uint16_t port)
    : _server(port), _sensorCount(0) {}

void VirgilJSPReceiver::begin() {
#ifdef JSP_USE_ASYNC
    _server.on(JSP_ENDPOINT, HTTP_POST,
        [](AsyncWebServerRequest* req) {},
        nullptr,
        [this](AsyncWebServerRequest* req,
               uint8_t* data, size_t len,
               size_t /*index*/, size_t /*total*/) {
            _handleJSONAsync(req, data, len);
        }
    );
#else
    _server.on(JSP_ENDPOINT, HTTP_POST, [this]() {
        _handleJSON();
    });
#endif
    _server.begin();
}

void VirgilJSPReceiver::loop() {
#ifndef JSP_USE_ASYNC
    _server.handleClient();
#endif
}

void VirgilJSPReceiver::onAlarm(JSPCallback cb)      { _onAlarm     = cb; }
void VirgilJSPReceiver::onAlarmClear(JSPCallback cb)  { _onAlarmClear = cb; }
void VirgilJSPReceiver::onHeartbeat(JSPCallback cb)   { _onHeartbeat  = cb; }
void VirgilJSPReceiver::onReading(JSPCallback cb)     { _onReading    = cb; }
void VirgilJSPReceiver::onAny(JSPCallback cb)         { _onAny        = cb; }

int VirgilJSPReceiver::getSensorCount() {
    return _sensorCount;
}

bool VirgilJSPReceiver::isSensorOnline(const char* id) {
    for (int i = 0; i < _sensorCount; i++) {
        if (_sensorEntries[i].valid &&
            strcmp(_sensorEntries[i].id, id) == 0) {
            return (millis() - _sensorEntries[i].lastSeenMs)
                   < JSP_SENSOR_TIMEOUT_MS;
        }
    }
    return false;
}

int VirgilJSPReceiver::getSensorBattery(const char* id) {
    for (int i = 0; i < _sensorCount; i++) {
        if (_sensorEntries[i].valid &&
            strcmp(_sensorEntries[i].id, id) == 0)
            return _sensorEntries[i].battery;
    }
    return -1;
}

VirgilJSPReceiver::SensorEntry* VirgilJSPReceiver::_findOrAdd(const char* id) {
    for (int i = 0; i < _sensorCount; i++) {
        if (_sensorEntries[i].valid &&
            strcmp(_sensorEntries[i].id, id) == 0)
            return &_sensorEntries[i];
    }
    if (_sensorCount >= JSP_MAX_SENSORS) return nullptr;
    SensorEntry* s     = &_sensorEntries[_sensorCount++];
    strncpy(s->id, id, JSP_ID_MAX_LENGTH);
    s->id[JSP_ID_MAX_LENGTH] = '\0';
    s->lastAlarm  = false;
    s->battery    = -1;
    s->lastSeenMs = 0;
    s->valid      = true;
    return s;
}

// Logica condivisa tra modalità sync e async
void VirgilJSPReceiver::_processRequest(String& body, IPAddress senderIP) {
    JSPMessage msg = _parseMessage(body);
    msg.senderIP  = senderIP;
    msg.timestamp = millis();

    SensorEntry* state = _findOrAdd(msg.id.c_str());
    bool alarmCleared = false;
    if (state) {
        alarmCleared      = state->lastAlarm && !msg.alarm;
        state->lastAlarm  = msg.alarm;
        state->lastSeenMs = millis();
        if (msg.battery >= 0) state->battery = msg.battery;
    }

    if (_onAny) _onAny(msg);

    if (msg.alarm) {
        if (_onAlarm) _onAlarm(msg);
    } else if (alarmCleared) {
        if (_onAlarmClear) _onAlarmClear(msg);
    } else if (msg.hasReadings) {
        if (_onReading) _onReading(msg);
    } else {
        if (_onHeartbeat) _onHeartbeat(msg);
    }
}

#ifdef JSP_USE_ASYNC

void VirgilJSPReceiver::_handleJSONAsync(AsyncWebServerRequest* req,
                                          uint8_t* data, size_t len) {
    // Frame JSP < 512 byte: arrivano sempre in un singolo chunk
    String body = String((char*)data, len);
    IPAddress ip = req->client()->remoteIP();
    _processRequest(body, ip);
    req->send(JSP_HTTP_OK, JSP_CONTENT_TYPE, "{\"ok\":1}");
}

#else

void VirgilJSPReceiver::_handleJSON() {
    if (!_server.hasArg("plain")) {
        _server.send(400, JSP_CONTENT_TYPE,
                     "{\"error\":\"no body\"}");
        return;
    }
    String body = _server.arg("plain");
    IPAddress ip = _server.client().remoteIP();
    _processRequest(body, ip);
    _server.send(JSP_HTTP_OK, JSP_CONTENT_TYPE, "{\"ok\":1}");
}

#endif

JSPMessage VirgilJSPReceiver::_parseMessage(String& body) {
    JSPMessage msg;
    msg.alarm         = false;
    msg.control       = false;
    msg.battery       = -1;
    msg.rssi          = 0;
    msg.hasReadings   = false;
    msg.timestamp     = 0;
    msg._readingCount = 0;

    JsonDocument doc;
    if (deserializeJson(doc, body) != DeserializationError::Ok)
        return msg;

    msg.id         = doc[JSP_FIELD_ID]          | "";
    msg.name       = doc[JSP_FIELD_NAME]        | msg.id.c_str();
    msg.alarm      = doc[JSP_FIELD_ALARM]       | false;
    msg.control    = doc[JSP_FIELD_CONTROL]     | false;
    msg.battery    = doc[JSP_FIELD_BATTERY]     | -1;
    msg.rssi       = doc[JSP_FIELD_RSSI]        | 0;
    msg.type       = doc[JSP_FIELD_TYPE]        | JSP_TYPE_WIFI;
    msg.wakeReason = doc[JSP_FIELD_WAKE_REASON] | "";

    if (doc.containsKey(JSP_FIELD_READINGS)) {
        JsonObject readings = doc[JSP_FIELD_READINGS];
        msg.hasReadings = true;
        for (JsonPair kv : readings) {
            if (msg._readingCount >= 10) break;
            strncpy(msg._readings[msg._readingCount].name,
                    kv.key().c_str(), 31);
            msg._readings[msg._readingCount].name[31] = '\0';
            if (kv.value().is<JsonObject>())
                msg._readings[msg._readingCount].value =
                    kv.value()["value"] | 0.0f;
            else
                msg._readings[msg._readingCount].value =
                    kv.value().as<float>();
            msg._readingCount++;
        }
    }

    return msg;
}
