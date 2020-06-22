# WiFi configuration manager

[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fsnakeye%2FWifiConfig.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2Fsnakeye%2FWifiConfig?ref=badge_shield)

[![Known Vulnerabilities](https://snyk.io//test/github/snakeye/WifiConfig/badge.svg?targetFile=package.json)](https://snyk.io//test/github/snakeye/WifiConfig?targetFile=package.json)

Wifi connection and configuration manager for ESP8266 and ESP32.

Based on [ConfigManager](https://github.com/nrwiersma/ConfigManager) library. The major difference
is that the full configuration is provided by the device and configuration form is built
dynamically by JavaScript application.

![Screenshot of the configuration frontend](/docs/images/wifi-config.png)

This library was made to ease the complication of configuring Wifi and other
settings on an ESP8266 or ESP32. It is roughly split into two parts, Wifi configuration
and REST variable configuration.

## Requires

* [ArduinoJson 5.x](https://github.com/bblanchon/ArduinoJson)

## Quick Start

### Installing

#### Arduino

You can install through the Arduino Library Manager. The package name is **WiFiConfig**.

#### PlatformIO

Library name is **WiFiConfig**

### Usage

Include the library in your sketch

```cpp
#include <WiFiConfig.h>
```

Initialize a global instance of the library

```cpp
ConfigManager configManager;
```

Initialize a global instance of the configuration object

```cpp
struct Config
{
    bool enabled = false;
    char server[128] = {0};
    int port = 0;
} config;
```

In your `setup` function define required parameters and start the manager.

```cpp
configManager.setAPName("Config Demo");

configManager.addParameterGroup("mqtt", new Metadata("MQTT Configuration", "Configuration of MQTT connection"))
    .addParameter("enabled", &config.enabled, new Metadata("Enabled"))
    .addParameter("server", config.server, 128, new Metadata("Server"))
    .addParameter("port", &config.port, new Metadata("Port", "Default value 1883"));

configManager.begin(config);
```

In your `loop` function, run the manager's loop

```cpp
configManager.loop();
```

### Upload frontend files

Upload the ```index.html``` file found in the ```data``` directory into the SPIFFS.
Instructions on how to do this vary based on your IDE. Below are links instructions
on the most common IDEs:

#### ESP8266

* [Platform IO](http://docs.platformio.org/en/stable/platforms/espressif.html#uploading-files-to-file-system-spiffs)

#### ESP32

* [Platform IO](http://docs.platformio.org/en/stable/platforms/espressif32.html#uploading-files-to-file-system-spiffs)

### Configure the device

Connect the device from the browser using IP address `http://<ip addres>`. The configuration form will be generated
automatically by the JavaScript application.

## Things TODO / Roadmap

* [ ] Migrate to ArduinoJSON v6
* [ ] Eliminate parallel HTTP requests
* [ ] Proper WiFi connection handling
* [ ] HTTP Authentication

## Documentation

* Class documentation can be generated by `doxygen` tool from the source code.
* [HTTP Endpoints definitions](/docs/openapi.yml)
* Documentation on the [frontend development](/docs/frontend.md)


## License
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fsnakeye%2FWifiConfig.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2Fsnakeye%2FWifiConfig?ref=badge_large)
