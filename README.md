# ESP Low Power

This project is designed to be run on low-power esp edge devices that spend the
majority of their time in sleep mode, only to publish information at an
interval or when triggered by some external factor.

## Architecture
The system consists of three main parts. A minimal example would look like this.

1. Edge Device - ESP8266
2. Receiver - ESP8266
3. Router - ESP32

The receiver and router are connected together via a serial connection in a "hub" somewhere in the building.

Edge devices are battery-powered devices that spend the majority of their time in deep-sleep, only coming
alive on a timer to take readings and send information, or in reaction to a physical event that wakes them up.
When edge devices gather information or are triggered by an event, they send it to the receiver via ESP-NOW.
This protocol is used to ensure the least amount of uptime is necessary for the edge device, maximizing battery life.
Connecting to a peer with ESP-NOW is much faster than Bluetooth or WiFi.

When the receiver receives information from an edge device, it sends it to the router via its serial connection.
The router then processes and forwards that information to Home Assistant via MQTT.

The router and receiver are necessary, because a single esp device cannot efficiently connect to ESP-NOW and connect
to a WiFi network to send MQTT messages. Having both a receiver and router ensure the receiver is always connected to the
ESP-NOW peers, and the router is always connected to the WiFi network to send MQTT messages.

Only one router-receiver hub is necessary in a building. After that, new edge devices flashed with the software will automatically
broadcast their information and be picked up by the receiver to publish to Home Assistant as a MQTT "Device" through the router.

After the initial pairing, the edge device will be able to send information to Home Assistant through the receiver via ESP-NOW which
will forward it to the router, which will process and forward the information to Home Assistant's MQTT integration to update device
readings.

## Development Setup
To setup your environment, you will need to run a few commands.

First install platformIO by following [this guide](https://docs.platformio.org/en/latest/core/installation/index.html)

### LSP Setup
To set up the required configurations to use clangd as an LSP in Neovim, follow these steps.

First, install [esp-idf](https://github.com/espressif/esp-idf)
1. Clone the repo
2. Run the ./install.sh script at the root
3. Set your environment by running `. ./export.sh` at the root of the repository after the install.
4. Install esp-clang by running `idif_tools.py install esp-clang`
5. With the environment setup (. ./export.sh was ran in the current terminal window)
6. Make sure the clangd is pointing to the version in the esp-idf install folder with `which clangd`

Next, run the following commands to initialize either the ESP32 or ESP8266 part of the project. This will set up the required
configurations for the clangd LSP to work with this.

> Note: You can't have autocomplete for both esp32 and esp8266 at the same time.

```bash
# To work on the ESP32 part of the project
pio project init --ide vim --board upesy_wroom
python conv.py

# To work on the ESP8266 part of the project
pio project init --ide vim --board d1_mini
python conv.py
```

That's it, you can open the project in Neovim and the LSP should work.
If you change the libraries that are included, you will need to re-run the `pio project init ...` and `python conv.py` commands.

### Helper scripts

Helper scripts to
- build
- upload
- check the serial monitor

are available in the `scripts` directory.

### Router and Receiver development
To work on both the router and receiver part at the same time, plug the router device FIRST, then plug the receiver device second.
This will make sure that /dev/ttyUSB0 is the router, and /dev/ttyUSB1 is the receiver, which is what platform.io specifies.
If you want to change that order, or change the ports used, simply update `platformio.ini`

## Configuration
To configure this project, copy the `lib/esplp/include/config_template.h` to `lib/esplp/include/config.h` and fill in the values with your own.

