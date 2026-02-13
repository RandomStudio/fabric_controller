# Fabric Controller

A capacitive touch controller for conductive fabric interfaces using an ESP32. 
The controller reads touch inputs from conductive fabric sensors and publishes touch events via MQTT.
- **Touch Inputs:**
Three different data are read by the ESP32 and sent via MQTT to Home Assistant:
  - T2 (white wire) - Approach sensor (proximity detection)
  - T6 (grey wire) - Curtain touch sensor
  - T9 (purple wire) - Button touch sensor

## Hardware

- **Board:** Seeed XIAO ESP32-S3 (but the logic works with very ESP32 who has WiFi)
![2](https://github.com/user-attachments/assets/41a4f83e-1865-4e66-b8a2-4062a7acf7dc)

## Features

- WiFi connectivity for wireless communication
- MQTT publishing for real-time touch event streaming
- Three independent touch channels:
  - **Approach sensor:** Returns a value from 0-255 based on proximity
  - **Curtain sensor:** Boolean on/off state
  - **Button sensor:** Boolean on/off state

## MQTT Topics

| Topic | Data Type | Description |
|-------|-----------|-------------|
| `fabric_approach/touch/change` | `unsigned int` (0-255) | Proximity value |
| `fabric_curtain/touch/change` | `bool` | Curtain touch state |
| `fabric_button/touch/change` | `bool` | Button press state |

The data are sent via MQTT to the Home Assistant only when the value changes from the one before.

## Configuration

If you want to connect to another WiFi, edit the following constants in `src/main.cpp`:

```cpp
const char* ssid = "Your_WiFi_SSID";
const char* password = "Your_WiFi_Password";
const char* mqtt_server = "Your_MQTT_Broker_IP";
```

## Building

This project uses PlatformIO. To build and upload you can use directly the PlatformIO IDE extension in VS Code.

