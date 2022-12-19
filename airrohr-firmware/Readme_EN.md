# Version for sensors PPD42NS, SDS011, DHT22, BMP180 and NEO-6M.

features:
* simultaneous operation of several sensors
* Configuration via WLAN (sensor as access point) possible
* Supports OLED displays with SSD1306
* Select the API (s) to which the data will be sent, including the ability to output the data as a CSV via USB
* usable for ESP8266 and Arduino Feather M0 (LoRa)

Todos:
* Optimizations (actually always)

The basic configuration of the parameters is done via the file `ext_dev.h`.

## WLAN configuration
see also wiki page on Github [Configuration of the sensors] (https://github.com/opendata-stuttgart/meta/wiki/Konfiguration-der-Sensoren)

If the default WLAN can not be reached after 20 seconds, an access point is set up, which can be reached via "Particulate matter sensor - \ [Sensor-ID \]". After connecting to this access point, all requests should be redirected to the configuration page. Direct address of the page is http://192.168.4.1/.

Configurable are:
* Wi-Fi name and password
* Sensors to be read
* Targets for sending the data

After 5 minutes the access point should be deactivated again (does not work at the moment).


## Save as CSV

The data can be output as CSV via USB. For this purpose debug should be set to 0 both in ext_def.h and in the WLAN configuration, so that the output data is only the sensor data. When restarting the ESP8266, only a few characters will appear that represent the start state.

## Wiring

* SDS and DHT wiring: [https://raw.githubusercontent.com/opendata-stuttgart/meta/master/files/nodemcu-v3-schaltplan-sds011.jpg](https://raw.githubusercontent.com/opendata- Stuttgart / meta / master / files / nodemcu v3 Schematic sds011.jpg)

## Required software (version tested in brackets and the type of license):

* [Arduino IDE] (https://www.arduino.cc/en/Main/Software) (Version 1.8.3) (GNU Lesser General Public License v2.1)
* [ESP8266 for Arduino] (http://arduino.esp8266.com/stable/package_esp8266com_index.json) (Version 2.3.0)


### Settings Arduino IDE

* Board: NodeMCU 1.0 (ESP-12E modules)
* CPU Frequency: 80MHz
* Flash Size: 4M (3M SPIFFS)

### Libraries Used (for ESP8266):

Included in Arduino:
* Wire (GNU Lesser General Public License v2.1)

Included in ESP8266 for Arduino IDE:
* FS (GNU Lesser Public License> = 2.1)
* ESP8266WiFi (GNU Lesser Public License> = 2.1)
* ESP8266 Web Server (GNU Lesser Public License> = 2.1)
* ESP8266httpUpdate (1.1.0) (GNU Lesser Public License> = 2.1)
* DNS server (GNU Lesser Public License> = 2.1)

Installable via Arduino IDE (menu Sketch -> Include Library -> Manage Libraries, in brackets the tested version and the type of license):
* [ArduinoJson] (https://github.com/bblanchon/ArduinoJson) (5.11.1) (MIT)
* [Adafruit Unified Sensor] (https://github.com/adafruit/Adafruit_Sensor) (1.0.2) (Apache)
* [Adafruit BMP085 library] (https://github.com/adafruit/Adafruit-BMP085-Library) (1.0.0) (BSD)
* [Adafruit BMP280 library] (https://github.com/adafruit/Adafruit_BMP280_Library) (1.0.2) (BSD)
* [Adafruit BME280 library] (https://github.com/adafruit/Adafruit_BME280_Library) (1.0.5) (BSD)
* [DallasTemperature] (https://github.com/milesburton/Arduino-Temperature-Control-Library) (3.7.6)
* [DHT sensor library] (https://github.com/adafruit/DHT-sensor-library) (1.3.0) (MIT)
* [ESP8266 and ESP32 Oled driver for SSD1306 display] (https://github.com/squix78/esp8266-oled-ssd1306) (3.2.7) (MIT)
* [OneWire] (www.pjrc.com/teensy/td_libs_OneWire.html) (2.3.3)
* [LiquidCrystal I2C] (https://github.com/marcoschwartz/LiquidCrystal_I2C) (1.1.2)
* [SparkFun HTU21D Humidity and Temperature Sensor Breakout] (https://github.com/sparkfun/SparkFun_HTU21D_Breakout_Arduino_Library) (1.1.3)
* [PubSubClient] (http://pubsubclient.knolleary.net/) (2.6.0) (MIT)
* [SoftwareSerial] (https://github.com/plerup/espsoftwareserial) (1.0.0) (GNU Lesser Public License> = 2.1)

Manually install:
* [TinyGPS ++] (http://arduiniana.org/libraries/tinygpsplus/) (0.94b) (GNU Lesser Public License> = 2.1)


Until version NRZ-2016-15:
* [DHT] (https://github.com/adafruit/DHT-sensor-library)
  Download (`DHT.cpp` and` DHT.h` and copy to the software directory)


## Connection of the sensors

When connecting sensors with 5V, please note the board version. NodeMCU v3 delivers 5V to `VU`, version 2 lacks this connector and` VIN` can be used for it.

### SDS011
* Pin 1 (TX) -> Pin D1 (GPIO5)
* Pin 2 (RX) -> Pin D2 (GPIO4)
* Pin 3 (GND) -> GND
* Pin 4 (2.5m) -> unused
* Pin 5 (5V) -> VU
* Pin 6 (1m) -> unused

### PPD42NS
* Pin 1 => GND
* Pin 2 => Pin D5 (GPIO14)
* Pin 3 => VU
* Pin 4 => Pin D6 (GPIO12)
* Pin 5 => unused

### DHT22
* Pin 1 => 3V3
* Pin 2 => pin D7 (GPIO13)
* Pin 3 => unused
* Pin 4 => GND

### DS18B20 (OneWire interface)
Please check your version (pinout) at [https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf]
Uses the same PIN D7 as DHT22, so DHT22 OR DS18B20 can be used. 
* GND -> Pin GND
* DQ -> Pin D7 (GPIO 13)
* VCC -> Pin 3V3 or Pin VU

### PMS1003 to PMS6003
pinout:
   8 7 6 5 4 3 2 1

* Pin 1 (VCC) -> VU
* Pin 2 (GND) -> GND
* Pin 3 (SET) -> unused
* Pin 4 (RX) -> Pin D2 (GPIO4)
* Pin 5 (TX) -> Pin D1 (GPIO5)
* Pin 6 (RESET) -> unused
* Pin 7 (NC) _> unused
* Pin 8 (NC) -> unused

### PMS7003
Pinout PMS7003:
   9 7 5 3 1
  10 8 6 4 2

* Pin 1/2 (VCC) -> VU
* Pin 3/4 (GND) -> GND
* Pin 5 (RESET) -> GND
* Pin 6 (NC) -> unused
* Pin 7 (RX) -> Pin D2 (GPIO4)
* Pin 8 (NC) -> unused
* Pin 9 (TX) -> Pin D1 (GPIO5)
* Pin 10 (SET) -> unused

### BMP180 / BMP280 / BME280 (I2C)
* VCC -> Pin 3V3
* GND -> Pin GND
* SCL -> pin D4 (GPIO2)
* SDA -> Pin D3 (GPIO0)

### HTU21D (I2C)
* VCC -> Pin 3V3
* GND -> Pin GND
* SCL -> pin D4 (GPIO2)
* SDA -> Pin D3 (GPIO0)

### LCD1602 (I2C, 5V - check your version)
* VCC -> Pin VU
* GND -> Pin GND
* SCL -> pin D4 (GPIO2)
* SDA -> Pin D3 (GPIO0)

### OLED displays with SSD1306 (I2C, 128x64 pixels)
* VCC -> Pin VU
* GND -> Pin GND
* SCL -> pin D4 (GPIO2)
* SDA -> Pin D3 (GPIO0)

### GPS NEO 6M (serial)
Current and mass from the board. (GND and usually 3.3v, check before / instructions, description GPS!)
TX (transmit) and RX (receive) are cross-wired! 
* TX from Neo -> Pin D5 (RX) 
* RX from Neo -> Pin D6 (TX) 


### Luftdaten.info API "Pins"
When activating several sensors, eg "simultaneously" DHT22 and PPD42NS, the API for assigning the sensor values ​​requires the specification of a pin to which the sensor is (virtually) connected.
This firmware defines the pins for the various sensors as follows:
* PPD42NS => pin 5
* DHT22 => pin 7
* SDS011 => pin 1
* BMP180 => pin 3
* BMP280 => pin 3
* BME280 => pin 11
* GPS (Neo-6M) => Pin 9


Used libraries for Adafruit Feather M0 LoRa:

To use the Adafruit Feather Boards, you must also enter the Adafruit Board Index in the Arduino IDE. In the settings the URL
https://adafruit.github.io/arduino-board-index/package_adafruit_index.json as an additional board administrator URL. After restarting the IDE, install in the board administrator "Adafruit SAMD boards".

Included in Arduino:
* Wire
* SPI

Included in "Adafruit SAMD Boards":


Installable via Arduino IDE (for versions see also ESP8266):
* [ArduinoJson] (https://github.com/bblanchon/ArduinoJson) (5.10.1) (MIT)
* [Adafruit Unified Sensor] (https://github.com/adafruit/Adafruit_Sensor) (1.0.2) (Apache)
* [Adafruit BMP085 library] (https://github.com/adafruit/Adafruit-BMP085-Library) (1.0.0) (BSD)
* [Adafruit BMP280 library] (https://github.com/adafruit/Adafruit_BMP280_Library) (1.0.2) (BSD)
* [Adafruit BME280 library] (https://github.com/adafruit/Adafruit_BME280_Library) (1.0.5) (BSD)
* [DHT sensor library] (https://github.com/adafruit/DHT-sensor-library) (1.3.0) (MIT)
* [LiquidCrystal I2C] (https://github.com/marcoschwartz/LiquidCrystal_I2C) (1.1.2)
* [PubSubClient] (http://pubsubclient.knolleary.net/) (2.6.0) (MIT)
* [SoftwareSerial] (https://github.com/plerup/espsoftwareserial) (1.0.0) (GNU Lesser Public License> = 2.1)

Manually install:
* [RadioHead Packet Radio library] (http://www.airspayce.com/mikem/arduino/RadioHead/) (1.6.1), link to download the zip archive in section 2
* [TinyGPS ++] (http://arduiniana.org/libraries/tinygpsplus/) (0.94b) (GNU Lesser Public License> = 2.1)

I hope to have caught all the libraries. If a library is missing when compiling, please report as [Issue] (https://github.com/opendata-stuttgart/sensors-software/issues/). I then carry the information.