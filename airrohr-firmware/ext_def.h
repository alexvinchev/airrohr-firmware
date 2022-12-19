// Wifi config
#define WLANSSID_TPL1   "TP-LINK_9D20"
#define WLANPWD_TPL1    "06102777"

#define WLANSSID_TPL2   "Martin Router King"
#define WLANPWD_TPL2    "Fatality1"

#define WLANSSID_GUEST  "ML4FL7AP26_G918293"
#define WLANPWD_GUEST   "26918293"

#define WLANSSID_APPLE  "Apple Network 1234"
#define WLANPWD_APPLE   "fatal1ty"

#define WLANSSID_CISCO  "Ciscosb"
#define WLANPWD_CISCO   "Fatality1"

#define WLANSSID_HOME   "ML4FL7AP26"
#define WLANPWD_HOME    "ULLEDAN2AMAOLL1"

#define WLANSSID        WLANSSID_HOME
#define WLANPWD         WLANPWD_HOME

// BasicAuth config
#define WWW_USERNAME    ""
#define WWW_PASSWORD    ""
#define WWW_BASICAUTH_ENABLED 0

// Sensor Wifi config (config mode)
#define FS_SSID         ""
#define FS_PWD          ""

// Definition for external API
#define DATA_HOST       "sen.vinchevi.info"
#define DATA_URL        "/data.php"
#define DATA_PORT       80
#define DATA_SEC_USR    ""
#define DATA_SEC_PWD    ""

// DHT11, temperature, humidity
#define DHT11_READ      0
#if defined(ESP8266)
#define DHT11_PIN       D2
#endif

// DHT22, temperature, humidity
#define DHT22_READ      0
#if defined(ESP8266)
#define DHT22_PIN       D2
#endif

// BMP180, temperature, pressure
#define BMP_READ        0
#if defined(ESP8266)
#define BMP_PIN_SCL     D4
#define BMP_PIN_SDA     D3
#endif

// BMP280, temperature, pressure
#define BMP280_READ     0
#if defined(ESP8266)
#define BMP280_PIN_SCL  D4
#define BMP280_PIN_SDA  D3
#endif

// BME280, temperature, humidity, pressure
#define BME280_READ     0
#if defined(ESP8266)
#define BME280_PIN_SCL  D4
#define BME280_PIN_SDA  D3
#endif

// DS18B20, temperature
#define DS18B20_READ    0
#if defined(ESP8266)
#define DS18B20_PIN     D2
#endif

// Definition for Debuglevel
#define DEBUG_NULL      0
#define DEBUG_ERROR     1
#define DEBUG_WARNING   2
#define DEBUG_MIN_INFO  3
#define DEBUG_MED_INFO  4
#define DEBUG_MAX_INFO  5

// Debug level
#define DEBUG_LEVEL     DEBUG_MAX_INFO

