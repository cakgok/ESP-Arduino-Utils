# ESP-Arduino-Utils

ESP-Ardunio-Utils is a general utilities library for ESP32 projects using the Arduino framework. It provides ready-to-use wrapper classes for managing WiFi.h, PubSubCLient for MQTT communication, 
Time.h for time synchronization, telemetry data collection and a logger with circular buffer implementation.

## Features

### Logger
- Singleton pattern for global access
- Multiple log levels (DEBUG, INFO, WARNING, ERROR)
- Thread-safe logging operations
- Support for custom log callbacks
- Optional serial output
- Circular buffer for storing recent log entries
- Ability to retrieve logs as formatted strings or JSON
- Can either peak at a log, or grab it and flush.

### Wifi
- Wrapper for WiFi.h library
- Connection management, static IP configuration, hostname setting, local network discovery via mDNS
- Automatic reconnection using FreeRTOS tasks
- 
### MQTT 
- Wrapper fir PubSubClient
- Secure MQTT connections with TLS support
- Automatic reconnection to MQTT broker
- Message publishing, topic subscription management
- Offline message buffering and retransmission
- Thread-safe operations using FreeRTOS primitives

### Time
- Time.h Wrapper Class
- NTP time synchronization, configurable with NTP server, GMT offset and DST.
- Periodic time resynchronization

### OTA
- ArduinoOTA library wrapper Class
- Provides a single line OTA update configuration. 

### Telemetry
- Automated collection of system metrics (Free heap, WiFi signal, uptime, CPU temp)
- Customizable data points
- Periodic publishing of telemetry data over MQTT
- JSON formatting of telemetry data
- FreeRTOS task monitoring (stack usage, etc.)
- Configurable metrics (enable/disable specific data points)

## Installation
### PlatformIO

Add the following line to your platformio.ini file:
```cpp 
lib_deps = 
    https://github.com/cakgok/ESP-Arduino-Utils
```
PlatformIO will automatically install the library and its dependencies.

## Quick Start

```cpp
#include <ESPLibrary.h>

//

// Get a reference to the logger instance
Logger& logger = Logger::instance();

WiFiWrapper wifi(WIFI_SSID, WIFI_PASSWORD);
ESPTimeSetup timeSetup("pool.ntp.org", 0, 3600);
ESPMQTTManager mqttManager(mqttConfig);
ESPTelemetry espTelemetry(mqttManager, "plant-friend/telemetry");
OTAManager otaManager;

void setup() {
    logger.setFilterLevel(Logger::Level::DEBUG);
    
    wifi.begin();
    timeSetup.begin();
    otaManager.begin();
    mqttManager.begin();

    telemetry.startPeriodicPublish(60000);

    //call the logger
    logger.log("Main", Logger::Level::INFO, "Starting application...");
}

void loop() {
}
```
More examples can be found in repo: plant-friend

## TO DO
### Loggerr
- [ ] Implement configurable buffer sizes.
- [ ] Add timestamp information to log entries.
- [ ] Implement log rotation or file-based logging for persistence.
- [ ] Use compile-time configuration for system-specific optimizations.
- [ ] Consider using a more type-safe formatting library.
- [ ] Optimize memory usage for callbacks and observers.
- [ ] Add utility methods for logging exceptions and stack traces.

### MQTT
- [ ] Swap to a more modern MQTT client with MQTT5 support.
- [ ] Add support for MQTT will messages.
- [ ] Consider adding message persistence across power cycles or crashes.
- [ ] Evaluate and improve scalability for large number of topics or high message throughput.

### General
- [ ] Consider using a state machines approch for wifi and mqtt conenctions.
