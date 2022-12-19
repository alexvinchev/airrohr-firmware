#include <Arduino.h>
#define INTL_EN

/*****************************************************************
/*                                                               *
/*  This source code needs to be compiled for the board          *
/*  NodeMCU 1.0 (ESP-12E Module)                                 *
/*                                                               *
/*****************************************************************
/* OK LAB Particulate Matter Sensor                              *
/*      - nodemcu-LoLin board                                    *
/*      - Nova SDS0111                                           *
/*  ﻿http://inovafitness.com/en/Laser-PM2-5-Sensor-SDS011-35.html *
/*                                                               *
/* Wiring Instruction:                                           *
/*      - SDS011 Pin 1  (TX)   -> Pin D1 / GPIO5                 *
/*      - SDS011 Pin 2  (RX)   -> Pin D2 / GPIO4                 *
/*      - SDS011 Pin 3  (GND)  -> GND                            *
/*      - SDS011 Pin 4  (2.5m) -> unused                         *
/*      - SDS011 Pin 5  (5V)   -> VU                             *
/*      - SDS011 Pin 6  (1m)   -> unused                         *
/*                                                               *
/*****************************************************************
/*                                                               *
/* Alternative                                                   *
/*      - nodemcu-LoLin board                                    *
/*      - Shinyei PPD42NS                                        *
/*      http://www.sca-shinyei.com/pdf/PPD42NS.pdf               *
/*                                                               *
/* Wiring Instruction:                                           *
/*      Pin 2 of dust sensor PM2.5 -> Digital 6 (PWM)            *
/*      Pin 3 of dust sensor       -> +5V                        *
/*      Pin 4 of dust sensor PM1   -> Digital 3 (PMW)            *
/*                                                               *
/*      - PPD42NS Pin 1 (grey or green)  => GND                  *
/*      - PPD42NS Pin 2 (green or white)) => Pin D5 /GPIO14      *
/*        counts particles PM25                                  *
/*      - PPD42NS Pin 3 (black or yellow) => Vin                 *
/*      - PPD42NS Pin 4 (white or black) => Pin D6 / GPIO12      *
/*        counts particles PM10                                  *
/*      - PPD42NS Pin 5 (red)   => unused                        *
/*                                                               *
/*****************************************************************
/* Extension: DHT22 (AM2303)                                     *
/*  ﻿http://www.aosong.com/en/products/details.asp?id=117         *
/*                                                               *
/* DHT22 Wiring Instruction                                      *
/* (left to right, front is perforated side):                    *
/*      - DHT22 Pin 1 (VDD)     -> Pin 3V3 (3.3V)                *
/*      - DHT22 Pin 2 (DATA)    -> Pin D7 (GPIO13)               *
/*      - DHT22 Pin 3 (NULL)    -> unused                        *
/*      - DHT22 Pin 4 (GND)     -> Pin GND                       *
/*                                                               *
/*****************************************************************
/* Extensions connected via I2C:                                 *
/* HTU21D (https://www.sparkfun.com/products/13763),             *
/* BMP180, BMP280, BME280, OLED Display with SSD1306 (128x64 px) *
/*                                                               *
/* Wiring Instruction                                            *
/* (see labels on display or sensor board)                       *
/*      VCC       ->     Pin 3V3                                 *
/*      GND       ->     Pin GND                                 *
/*      SCL       ->     Pin D4 (GPIO2)                          *
/*      SDA       ->     Pin D3 (GPIO0)                          *
/*                                                               *
/*****************************************************************
/*                                                               *
/* Please check Readme.md for other sensors and hardware         *
/*                                                               *
/*****************************************************************/
// increment on change
#define SOFTWARE_VERSION "NRZ-2017-dummy"

/*****************************************************************
/* Includes                                                      *
/*****************************************************************/
#if defined(ESP8266)
#include <FS.h>                     // must be first
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <SoftwareSerial.h>
#include <SSD1306.h>
#include <LiquidCrystal_I2C.h>
#include <base64.h>
#endif
#include <ArduinoJson.h>
#include <DHT.h>
#include <SparkFunHTU21D.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BME280.h>
#include <DallasTemperature.h>
#include <TinyGPS++.h>
#include <Ticker.h>

#if defined(INTL_BG)
#include "intl_bg.h"
#elif defined(INTL_EN)
#include "intl_en.h"
#elif defined(INTL_ES)
#include "intl_es.h"
#elif defined(INTL_FR)
#include "intl_fr.h"
#elif defined(INTL_IT)
#include "intl_it.h"
#elif defined(INTL_NL)
#include "intl_nl.h"
#elif defined(INTL_PT)
#include "intl_pt.h"
#else
#include "intl_de.h"
#endif
#include "ext_def.h"
#include "html-content.h"

/*****************************************************************
/* Variables with defaults                                       *
/*****************************************************************/
char wlanssid[65] = "Freifunk-disabled";
char wlanpwd[65] = "";
char current_lang[3] = "de";
char www_username[65] = "admin";
char www_password[65] = "feinstaub";
bool www_basicauth_enabled = 0;

char version_from_local_config[30] = "";

bool dht_read = 0;
bool htu21d_read = 0;
bool ppd_read = 0;
bool sds_read = 0;
bool pms24_read = 0;
bool pms32_read = 0;
bool bmp_read = 0;
bool bmp_init_failed = 0;
bool bmp280_read = 0;
bool bmp280_init_failed = 0;
bool bme280_read = 0;
bool bme280_init_failed = 0;
bool ds18b20_read = 0;
bool gps_read = 0;
bool send2dusti = 1;
bool send2madavi = 1;
bool send2sensemap = 0;
bool send2custom = 0;
bool send2influx = 0;
bool send2csv = 0;
bool auto_update = 0;
bool has_display = 0;
bool has_lcd1602 = 0;
bool has_lcd1602_27 = 0;
int  debug = 5;

long int sample_count = 0;

const char* host_madavi = "api-rrd.madavi.de";
const char* url_madavi = "/data.php";
int httpPort_madavi = 443;

const char* host_dusti = "api.luftdaten.info";
const char* url_dusti = "/v1/push-sensor-data/";
int httpPort_dusti = 443;

// IMPORTANT: NO MORE CHANGES TO VARIABLE NAMES NEEDED FOR EXTERNAL APIS

const char* host_sensemap = "ingress.opensensemap.org";
String url_sensemap = "/boxes/BOXID/data?luftdaten=1";
const int httpPort_sensemap = 443;
char senseboxid[30] = "";

char host_influx[100] = "api.luftdaten.info";
char url_influx[100] = "/write?db=luftdaten";
int port_influx = 8086;
char user_influx[100] = "luftdaten";
char pwd_influx[100] = "info";
String basic_auth_influx = "";

char host_custom[100] = "192.168.234.1";
char url_custom[100] = "/data.php";
int port_custom = 80;
char user_custom[100] = "";
char pwd_custom[100] = "";
String basic_auth_custom = "";

const char* update_host = "www.madavi.de";
const char* update_url = "/sensor/update/firmware.php";
const int update_port = 80;

#if defined(ESP8266)
ESP8266WebServer server(80);
int TimeZone = 1;
#endif

/*****************************************************************
/* Display definitions                                           *
/*****************************************************************/
#if defined(ESP8266)
SSD1306   display(0x3c, D3, D4);
LiquidCrystal_I2C lcd_27(0x27, 16, 2);
LiquidCrystal_I2C lcd_3f(0x3F, 16, 2);
#endif

/*****************************************************************
/* SDS011 declarations                                           *
/*****************************************************************/
#if defined(ESP8266)
SoftwareSerial serialSDS(SDS_PIN_RX, SDS_PIN_TX, false, 128);
SoftwareSerial serialGPS(GPS_PIN_RX, GPS_PIN_TX, false, 128);
#endif
/*****************************************************************
/* DHT declaration                                               *
/*****************************************************************/
DHT dht(DHT_PIN, DHT_TYPE);

/*****************************************************************
/* HTU21D declaration                                            *
/*****************************************************************/
HTU21D htu21d;

/*****************************************************************
/* BMP085 declaration                                               *
/*****************************************************************/
Adafruit_BMP085 bmp;

/*****************************************************************
/* BMP280 declaration                                               *
/*****************************************************************/
Adafruit_BMP280 bmp280;

/*****************************************************************
/* BME280 declaration                                            *
/*****************************************************************/
Adafruit_BME280 bme280;

/*****************************************************************
/* DS18B20 declaration                                            *
/*****************************************************************/
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);

/*****************************************************************
/* GPS declaration                                               *
/*****************************************************************/
#if defined(ARDUINO_SAMD_ZERO) || defined(ESP8266)
TinyGPSPlus gps;
#endif

/*****************************************************************
/* Variable Definitions for PPD24NS                              *
/* P1 for PM10 & P2 for PM25                                     *
/*****************************************************************/

unsigned long durationP1;
unsigned long durationP2;

boolean trigP1 = false;
boolean trigP2 = false;
unsigned long trigOnP1;
unsigned long trigOnP2;

unsigned long lowpulseoccupancyP1 = 0;
unsigned long lowpulseoccupancyP2 = 0;

bool send_now = false;
unsigned long starttime;
unsigned long starttime_SDS;
unsigned long starttime_GPS;
unsigned long act_micro;
unsigned long act_milli;
unsigned long last_micro = 0;
unsigned long min_micro = 1000000000;
unsigned long max_micro = 0;
unsigned long diff_micro = 0;

const unsigned long sampletime_ms = 30000;

const unsigned long sampletime_SDS_ms = 1000;
const unsigned long warmup_time_SDS_ms = 15000;
const unsigned long reading_time_SDS_ms = 5000;
// const unsigned long reading_time_SDS_ms = 60000;
bool is_SDS_running = true;
bool is_PMS_running = true;

const unsigned long display_update_interval = 5000;
unsigned long display_last_update;

const unsigned long sampletime_GPS_ms = 50;

unsigned long sending_intervall_ms = 145000;
unsigned long sending_time = 0;
bool send_failed = false;

unsigned long time_for_wifi_config = 600000;

unsigned long last_update_attempt;
const unsigned long pause_between_update_attempts = 86400000;
bool will_check_for_update = false;

int sds_pm10_sum = 0;
int sds_pm25_sum = 0;
int sds_val_count = 0;
int sds_pm10_max = 0;
int sds_pm10_min = 20000;
int sds_pm25_max = 0;
int sds_pm25_min = 20000;

int pms_pm1_sum = 0;
int pms_pm10_sum = 0;
int pms_pm25_sum = 0;
int pms_val_count = 0;
int pms_pm1_max = 0;
int pms_pm1_min = 20000;
int pms_pm10_max = 0;
int pms_pm10_min = 20000;
int pms_pm25_max = 0;
int pms_pm25_min = 20000;

String last_value_PPD_P1 = "";
String last_value_PPD_P2 = "";
String last_value_SDS_P1 = "";
String last_value_SDS_P2 = "";
String last_value_PMS_P0 = "";
String last_value_PMS_P1 = "";
String last_value_PMS_P2 = "";
String last_value_DHT_T = "";
String last_value_DHT_H = "";
String last_value_HTU21D_T = "";
String last_value_HTU21D_H = "";
String last_value_BMP_T = "";
String last_value_BMP_P = "";
String last_value_BMP280_T = "";
String last_value_BMP280_P = "";
String last_value_BME280_T = "";
String last_value_BME280_H = "";
String last_value_BME280_P = "";
String last_value_DS18B20_T = "";
String last_data_string = "";

String last_gps_lat;
String last_gps_lng;
String last_gps_alt;
String last_gps_date;
String last_gps_time;

String esp_chipid;

String server_name;

long last_page_load = millis();

bool wificonfig_loop = false;
bool restart_needed = false;

bool config_needs_write = false;

bool first_csv_line = 1;

bool first_cycle = 1;

String data_first_part = "{\"software_version\": \"" + String(SOFTWARE_VERSION) + "\"FEATHERCHIPID, \"sensordatavalues\":[";

static unsigned long last_loop;

/*****************************************************************
/* Debug output                                                  *
/*****************************************************************/
void debug_out(const String& text, const int level, const bool linebreak) {
	if (level <= debug) {
		if (linebreak) {
			Serial.println(text);
		} else {
			Serial.print(text);
		}
	}
}

/*****************************************************************
/* display values                                                *
/*****************************************************************/
void display_debug(const String& text) {
#if defined(ESP8266)
	if (has_display) {
		debug_out(F("output debug text to display..."), DEBUG_MIN_INFO, 1);
		debug_out(text, DEBUG_MAX_INFO, 1);
		display.resetDisplay();
		display.clear();
		display.displayOn();
		display.setFont(Monospaced_plain_9);
		display.setTextAlignment(TEXT_ALIGN_LEFT);
		display.drawStringMaxWidth(0, 12, 120, text);
		display.display();
	}
#endif
}

/*****************************************************************
/* IPAddress to String                                           *
/*****************************************************************/
String IPAddress2String(const IPAddress& ipaddress) {
	char myIpString[24];
	sprintf(myIpString, "%d.%d.%d.%d", ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3]);
	return String(myIpString);
}

/*****************************************************************
/* convert float to string with a                                *
/* precision of two decimal places                               *
/*****************************************************************/
String Float2String(const float value) {
	// Convert a float to String with two decimals.
	char temp[15];
	String s;

	dtostrf(value, 13, 2, temp);
	s = String(temp);
	s.trim();
	return s;
}

/*****************************************************************
/* convert value to json string                                  *
/*****************************************************************/
String Value2Json(const String& type, const String& value) {
	String s = F("{\"value_type\":\"{t}\",\"value\":\"{v}\"},");
	s.replace("{t}", type); s.replace("{v}", value);
	return s;
}

/*****************************************************************
/* copy config from ext_def                                      *
/*****************************************************************/
void copyExtDef() {

#define strcpyDef(var, def) if (def != NULL) { strcpy(var, def); }
#define setDef(var, def)    if (def != var) { var = def; }

	strcpyDef(current_lang, CURRENT_LANG);
	strcpyDef(wlanssid, WLANSSID);
	strcpyDef(wlanpwd, WLANPWD);
	strcpyDef(www_username, WWW_USERNAME);
	strcpyDef(www_password, WWW_PASSWORD);
	setDef(www_basicauth_enabled, WWW_BASICAUTH_ENABLED);
	setDef(dht_read, DHT_READ);
	setDef(htu21d_read, HTU21D_READ);
	setDef(ppd_read, PPD_READ);
	setDef(sds_read, SDS_READ);
	setDef(pms24_read, PMS24_READ);
	setDef(pms32_read, PMS32_READ);
	setDef(bmp_read, BMP_READ);
	setDef(bmp280_read, BMP280_READ);
	setDef(bme280_read, BME280_READ);
	setDef(ds18b20_read, DS18B20_READ);
	setDef(gps_read, GPS_READ);
	setDef(send2dusti, SEND2DUSTI);
	setDef(send2madavi, SEND2MADAVI);
	setDef(send2sensemap, SEND2SENSEMAP)
	setDef(send2csv, SEND2CSV);
	setDef(auto_update, AUTO_UPDATE);
	setDef(has_display, HAS_DISPLAY);
	setDef(has_lcd1602, HAS_LCD1602);
	setDef(has_lcd1602_27, HAS_LCD1602_27);

	setDef(debug, DEBUG);

	strcpyDef(senseboxid, SENSEBOXID);

	setDef(send2custom, SEND2CUSTOM);
	strcpyDef(host_custom, HOST_CUSTOM);
	strcpyDef(url_custom, URL_CUSTOM);
	setDef(port_custom, PORT_CUSTOM);
	strcpyDef(user_custom, USER_CUSTOM);
	strcpyDef(pwd_custom, PWD_CUSTOM);

	setDef(send2influx, SEND2INFLUX);
	strcpyDef(host_influx, HOST_INFLUX);
	strcpyDef(url_influx, URL_INFLUX);
	setDef(port_influx, PORT_INFLUX);
	strcpyDef(user_influx, USER_INFLUX);
	strcpyDef(pwd_influx, PWD_INFLUX);

#undef strcpyDef
#undef setDef
}

/*****************************************************************
/* Base64 encode user:password                                   *
/*****************************************************************/
void create_basic_auth_strings() {
	basic_auth_custom = "";
	if (strcmp(user_custom, "") != 0 || strcmp(pwd_custom, "") != 0) {
		basic_auth_custom = base64::encode(String(user_custom) + ":" + String(pwd_custom));
	}
	basic_auth_influx = "";
	if (strcmp(user_influx, "") != 0 || strcmp(pwd_influx, "") != 0) {
		basic_auth_influx = base64::encode(String(user_influx) + ":" + String(pwd_influx));
	}
}

/*****************************************************************
/* html helper functions                                         *
/*****************************************************************/
String make_header(const String& title) {
	String s = "";
	s += FPSTR(WEB_PAGE_HEADER);
	s.replace("{tt}", FPSTR(INTL_FEINSTAUBSENSOR));
	s.replace("{h}", FPSTR(INTL_UBERSICHT));
	if(title != " ") {
		s.replace("{n}", F("&raquo;"));
	} else {
		s.replace("{n}", "");
	}
	s.replace("{t}", title);
	s.replace("{id}", esp_chipid);
	s.replace("{mac}", WiFi.macAddress());
	s.replace("{fwt}", FPSTR(INTL_FIRMWARE));
	s.replace("{fw}", SOFTWARE_VERSION);
	return s;
}

String make_footer() {
	String s = "";
	s += FPSTR(WEB_PAGE_FOOTER);
	s.replace("{t}", FPSTR(INTL_ZURUCK_ZUR_STARTSEITE));
	return s;
}

String form_input(const String& name, const String& info, const String& value, const int length) {
	String s = F("<tr><td>{i} </td><td style='width:90%;'><input type='text' name='{n}' id='{n}' placeholder='{i}' value='{v}' maxlength='{l}'/></td></tr>");
	s.replace("{i}", info); s.replace("{n}", name); s.replace("{v}", value); s.replace("{l}", String(length));
	return s;
}

String form_password(const String& name, const String& info, const String& value, const int length) {
	String password = "";
	for (int i = 0; i < value.length(); i++) { password += "*"; }
	String s = F("<tr><td>{i} </td><td style='width:90%;'><input type='password' name='{n}' id='{n}' placeholder='{i}' value='{v}' maxlength='{l}'/></td></tr>");
	s.replace("{i}", info); s.replace("{n}", name); s.replace("{v}", password); s.replace("{l}", String(length));
	return s;
}

String form_checkbox(const String& name, const String& info, const bool checked) {
	String s = F("<label for='{n}'><input type='checkbox' name='{n}' value='1' id='{n}' {c}/><input type='hidden' name='{n}' value='0' /> {i}</label><br/>");
	if (checked) {s.replace("{c}", F(" checked='checked'"));} else {s.replace("{c}", "");};
	s.replace("{i}", info); s.replace("{n}", name);
	return s;
}

String form_submit(const String& value) {
	String s = F("<tr><td>&nbsp;</td><td><input type='submit' name='submit' value='{v}' /></td></tr>");
	s.replace("{v}", value);
	return s;
}

String form_select_lang() {
	String s_select = F("selected='selected'");
	String s = F("<tr><td>{t}</td><td><select name='current_lang'><option value='DE' {s_DE}>Deutsch (DE)</option><option value='BG' {s_BG}>Bulgarian (BG)</option><option value='EN' {s_EN}>English (EN)</option><option value='ES' {s_ES}>Español (ES)</option><option value='FR' {s_FR}>Français (FR)</option><option value='IT' {s_IT}>Italiano (IT)</option><option value='NL' {s_NL}>Nederlands (NL)</option><option value='PT' {s_PT}>Português (PT)</option></select></td></tr>");

	s.replace("{t}", FPSTR(INTL_SPRACHE));

	if(String(current_lang) == "DE") {
		s.replace(F("{s_DE}"), s_select);
	} else if(String(current_lang) == "BG") {
		s.replace(F("{s_BG}"), s_select);
	} else if(String(current_lang) == "EN") {
		s.replace(F("{s_EN}"), s_select);
	} else if(String(current_lang) == "ES") {
		s.replace(F("{s_ES}"), s_select);
	} else if(String(current_lang) == "FR") {
		s.replace(F("{s_FR}"), s_select);
	} else if(String(current_lang) == "IT") {
		s.replace(F("{s_IT}"), s_select);
	} else if(String(current_lang) == "NL") {
		s.replace(F("{s_NL}"), s_select);
	} else if(String(current_lang) == "PT") {
		s.replace(F("{s_PT}"), s_select);
	}
	s.replace(F("{s_DE}"), "");
	s.replace(F("{s_BG}"), "");
	s.replace(F("{s_EN}"), "");
	s.replace(F("{s_ES}"), "");
	s.replace(F("{s_FR}"), "");
	s.replace(F("{s_NL}"), "");
	s.replace(F("{s_PT}"), "");
	return s;
}

String tmpl(const String& patt, const String& value) {
	String s = F("{patt}");
	s.replace("{patt}", value); s.replace("{v}", value);
	return s;
}

String tmpl(const String& patt, const String& value1, const String& value2) {
	String s = F("{patt}");
	s.replace("{patt}", patt); s.replace("{v1}", value1); s.replace("{v2}", value2);
	return s;
}

String tmpl(const String& patt, const String& value1, const String& value2, const String& value3) {
	String s = F("{patt}");
	s.replace("{patt}", patt); s.replace("{v1}", value1); s.replace("{v2}", value2); s.replace("{v3}", value3);
	return s;
}

String line_from_value(const String& name, const String& value) {
	String s = F("<br>{n}: {v}");
	s.replace("{n}", name);
	s.replace("{v}", value);
	return s;
}

String table_row_from_value(const String& sensor, const String& param, const String& value, const String& unit) {
	String s = F("<tr><td>{s}</td><td>{p}</td><td class='r'>{v}&nbsp;{u}</td></tr>");
	s.replace("{s}", sensor); s.replace("{p}", param); s.replace("{v}", value); s.replace("{u}", unit);
	return s;
}

String wlan_ssid_to_table_row(const String& ssid, const String& encryption, const long rssi) {
	long rssi_temp = rssi;
	if (rssi_temp > -50) {rssi_temp = -50; }
	if (rssi_temp < -100) {rssi_temp = -100; }
	int quality = (rssi_temp + 100) * 2;
	String s = F("<tr><td><a href='#wlanpwd' onclick='setSSID(this)' style='background:none;color:blue;padding:5px;display:inline;'>{n}</a>&nbsp;{e}</a></td><td style='width:80%;vertical-align:middle;'>{v}%</td></tr>");
	s.replace("{n}", ssid); s.replace("{e}", encryption); s.replace("{v}", String(quality));
	return s;
}

String warning_first_cycle() {
	String s = FPSTR(INTL_ERSTER_MESSZYKLUS);
	unsigned long time_to_first = sending_intervall_ms - (act_milli - starttime);
	if ((time_to_first > sending_intervall_ms) || (time_to_first < 0)) { time_to_first = 0; }
	s.replace("{v}", String((long)((time_to_first + 500) / 1000)));
	return s;
}

String age_last_values() {
	String s = "<b>";
	unsigned long time_since_last = act_milli - starttime;
	if ((time_since_last > sending_intervall_ms) || (time_since_last < 0)) { time_since_last = 0; }
	s += String((long)((time_since_last + 500) / 1000));
	s += FPSTR(INTL_ZEIT_SEIT_LETZTER_MESSUNG);
	s += F("</b><br/><br/>");
	return s;
}

/*****************************************************************
/* send data to influxdb                                         *
/*****************************************************************/
String create_influxdb_string(const String& data) {
	String tmp_str;
	String data_4_influxdb;
	debug_out(F("Parse JSON for influx DB"), DEBUG_MIN_INFO, 1);
	debug_out(data, DEBUG_MIN_INFO, 1);
	StaticJsonBuffer<2000> jsonBuffer;
	JsonObject& json2data = jsonBuffer.parseObject(data);
	if (json2data.success()) {
		data_4_influxdb = "";
		data_4_influxdb += F("feinstaub,node=esp8266-");
		data_4_influxdb += esp_chipid + " ";
		for (int i = 0; i < json2data["sensordatavalues"].size() - 1; i++) {
			tmp_str = jsonBuffer.strdup(json2data["sensordatavalues"][i]["value_type"].as<char*>());
			data_4_influxdb += tmp_str + "=";
			tmp_str = jsonBuffer.strdup(json2data["sensordatavalues"][i]["value"].as<char*>());
			data_4_influxdb += tmp_str;
			if (i < (json2data["sensordatavalues"].size() - 2)) { data_4_influxdb += ","; }
		}
		data_4_influxdb += "\n";
	} else {
		debug_out(F("Data read failed"), DEBUG_ERROR, 1);
	}
	return data_4_influxdb;
}

