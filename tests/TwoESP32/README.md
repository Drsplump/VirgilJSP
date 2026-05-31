# TwoESP32

PlatformIO project for testing two ESP32 boards communicating over WiFi using VirgilJSP:

- **esp32sensor** — reads a GPIO pin and sends alarms via `VirgilJSP`
- **esp32receiver** — receives JSP messages over HTTP via `VirgilJSPReceiver`

## Quick Setup

1. Open [include/app_config.h](include/app_config.h).
2. Set `kWifiSsid` and `kWifiPassword`.
3. Upload **esp32receiver** first, then **esp32sensor**.

## Commands

```bash
# Build
pio run -e esp32receiver
pio run -e esp32sensor

# Upload
pio run -e esp32receiver -t upload
pio run -e esp32sensor -t upload

# Monitor
pio device monitor -b 115200
```

## Notes

- If mDNS (`esp32receiver.local`) doesn't resolve on your network, set `kReceiverHost` to the receiver's static IP in `app_config.h`.
- When both boards are connected simultaneously, set `upload_port` and `monitor_port` in `platformio.ini` for each environment to avoid conflicts.
