#pragma once

#include "VirgilJSP_Protocol.h"
#include <functional>

#ifdef JSP_USE_ASYNC
  #include <ESPAsyncWebServer.h>
  using JSPWebServer = AsyncWebServer;
#else
  #ifdef ESP32
    #include <WebServer.h>
    using JSPWebServer = WebServer;
  #elif defined(ESP8266)
    #include <ESP8266WebServer.h>
    using JSPWebServer = ESP8266WebServer;
  #endif
#endif

typedef std::function<void(JSPMessage)> JSPCallback;

class VirgilJSPReceiver {
public:
    VirgilJSPReceiver(uint16_t port = 80);

    void begin();
    void loop();  // no-op quando JSP_USE_ASYNC è attivo

    // Callbacks
    void onAlarm(JSPCallback cb);
    void onAlarmClear(JSPCallback cb);
    void onHeartbeat(JSPCallback cb);
    void onReading(JSPCallback cb);
    void onAny(JSPCallback cb);

    // Gestione sensori
    int  getSensorCount();
    bool isSensorOnline(const char* id);
    int  getSensorBattery(const char* id);

private:
    JSPWebServer _server;
    JSPCallback  _onAlarm;
    JSPCallback  _onAlarmClear;
    JSPCallback  _onHeartbeat;
    JSPCallback  _onReading;
    JSPCallback  _onAny;

    struct SensorEntry {
        char     id[JSP_ID_MAX_LENGTH + 1];
        bool     lastAlarm;
        int      battery;
        uint32_t lastSeenMs;
        bool     valid = false;
    };
    SensorEntry _sensorEntries[JSP_MAX_SENSORS];
    int         _sensorCount = 0;

    SensorEntry* _findOrAdd(const char* id);
    void         _processRequest(String& body, IPAddress senderIP);
    JSPMessage   _parseMessage(String& body);

#ifdef JSP_USE_ASYNC
    void _handleJSONAsync(AsyncWebServerRequest* req,
                          uint8_t* data, size_t len);
#else
    void _handleJSON();
#endif
};
