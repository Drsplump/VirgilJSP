#pragma once

// Shared configuration for both boards.
// Fill these values with your WiFi credentials before upload.
constexpr char kWifiSsid[] = "YOUR_SSID";
constexpr char kWifiPassword[] = "YOUR_PASSWORD";

// Receiver host used by VirgilJSP sender.
// If mDNS works on your network, keep the default.
// Otherwise replace with receiver static IP, e.g. "192.168.1.50".
constexpr char kReceiverHost[] = "esp32receiver.local";
constexpr uint16_t kReceiverPort = 80;

// Sensor identity shown on receiver side.
constexpr char kSensorId[] = "esp32sensor_01";
constexpr char kSensorName[] = "ESP32 Sensor";

// GPIO used by the simple water/alarm input.
constexpr uint8_t kSensorPin = 32;

// Sensor logic polarity.
// true  -> alarm is active when pin reads LOW  (typical dry contact to GND)
// false -> alarm is active when pin reads HIGH (typical push-pull module)
constexpr bool kSensorActiveLow = true;
