#include <Arduino.h>
#define INTL_EN

// increment on change
#define SOFTWARE_VERSION "AV-SEN-00.00.0001"

/*****************************************************************
/* Includes                                                      *
/*****************************************************************/
#include <FS.h>                     // must be first
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <base64.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Adafruit_BME280.h>
#include <DallasTemperature.h>
#include <Ticker.h>

#include "intl_en.h"
#include "ext_def.h"
#include "html-content.h"

/*****************************************************************
/* Variables with defaults                                       *
/*****************************************************************/
char                    wlanssid[33] = WLANSSID;
char                    wlanpwd[65] = WLANPWD;

char                    www_username[65] = WWW_USERNAME;
char                    www_password[65] = WWW_PASSWORD;
bool                    www_basicauth_enabled = WWW_BASICAUTH_ENABLED;

char                    fs_ssid[33] = FS_SSID;
char                    fs_pwd[65] = FS_PWD;

char                    version_from_local_config[20] = "";

bool                    dht11_read = 0;
bool                    dht22_read = 0;

bool                    bme280_read = 0;
bool                    bme280_init_failed = 0;

bool                    ds18b20_read = 0;

int                     debug_level = DEBUG_LEVEL;

long int                sample_count = 0;

char                    data_host[100] = DATA_HOST;
char                    data_url[100] = DATA_URL;
int                     data_port = DATA_PORT;
char                    data_sec_usr[65] = DATA_SEC_USR;
char                    data_sec_pwd[65] = DATA_SEC_PWD;
String                  sec_basic_auth = "";

const unsigned long     sampletime_ms = 30000;

bool                    send_now = false;
unsigned long           starttime;
unsigned long           act_micro;
unsigned long           act_milli;
unsigned long           last_micro = 0;
unsigned long           min_micro = 1000000000;
unsigned long           max_micro = 0;
unsigned long           diff_micro = 0;
unsigned long           sending_intervall_ms = 145000;
unsigned long           sending_time = 0;
bool                    send_failed = false;

unsigned long           time_for_wifi_config = 600000;

double                  last_value_DHT11_T = -128.0;
double                  last_value_DHT11_H = -1.0;
double                  last_value_DHT22_T = -128.0;
double                  last_value_DHT22_H = -1.0;
double                  last_value_BMP_T = -128.0;
double                  last_value_BMP_P = -1.0;
double                  last_value_BMP280_T = -128.0;
double                  last_value_BMP280_P = -1.0;
double                  last_value_BME280_T = -128.0;
double                  last_value_BME280_H = -1.0;
double                  last_value_BME280_P = -1.0;
double                  last_value_DS18B20_T = -1.0;
String                  last_data_string = "";

String                  esp_chipid;

String                  server_name;

long                    last_page_load = millis();

bool                    wificonfig_loop = false;
bool                    restart_needed = false;

bool                    config_needs_write = false;

bool                    first_cycle = 1;

String                  data_first_part = "{\"software_version\": \"" +
                                          String(SOFTWARE_VERSION) +
                                          "\", \"sensordatavalues\":[";

long                    count_sends = 0;

/*****************************************************************
/* WEB server                                                    *
/*****************************************************************/
ESP8266WebServer        server(80);

/*****************************************************************
/* DHT11/22 declaration                                          *
/*****************************************************************/
DHT                     dht11(DHT11_PIN, DHT11);
DHT                     dht22(DHT22_PIN, DHT22);

/*****************************************************************
/* BME280 declaration                                            *
/*****************************************************************/
Adafruit_BME280         bme280;

/*****************************************************************
/* DS18B20 declaration                                           *
/*****************************************************************/
OneWire                 oneWire(DS18B20_PIN);
DallasTemperature       ds18b20(&oneWire);

/*****************************************************************
/* Debug output                                                  *
/*****************************************************************/
void debug_out(const String& text, const int level, const bool linebreak) {
    if (level <= debug_level) {
        if (linebreak) {
            Serial.println(text);
        } else {
            Serial.print(text);
        }
    }
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
/* File size to String                                           *
/*****************************************************************/
String FileSize2String(const size_t& size) {
    char myFSizeStr[24];

    sprintf(myFSizeStr, "File size %ll bytes", size);
    return String(myFSizeStr);
}

/*****************************************************************
/* check display values, return '-' if undefined                 *
/*****************************************************************/
String check_display_value(double value, double undef, int len, int str_len) {
    String s = (value != undef ? Float2String(value,len) : "-");
    while (s.length() < str_len) {
        s = " " + s;
    }
    return s;
}

/*****************************************************************
/* convert float to string with a                                *
/* precision of two decimal places                               *
/*****************************************************************/
String Float2String(const double value) {
    return Float2String(value,2);
}

String Float2String(const double value, unsigned int digits) {
    // Convert a float to String with two decimals.
    char temp[15];
    String s;

    dtostrf(value, 13, digits, temp);
    s = String(temp);
    s.trim();
    return s;
}

/*****************************************************************
/* convert value to json string                                  *
/*****************************************************************/
String Value2Json(const String& type, const String& value) {
    String s = F("{\"value_type\":\"{t}\",\"value\":\"{v}\"},");
    s.replace("{t}", type);
    s.replace("{v}", value);
    return s;
}

/*****************************************************************
/* copy config from ext_def                                      *
/*****************************************************************/
void copyExtDef() {

#define strcpyDef(var, def) if (def != NULL) { strcpy(var, def); }
#define setDef(var, def)    if (def != var) { var = def; }

    strcpyDef(wlanssid, WLANSSID);
    strcpyDef(wlanpwd, WLANPWD);
    strcpyDef(www_username, WWW_USERNAME);
    strcpyDef(www_password, WWW_PASSWORD);
    strcpyDef(fs_ssid, FS_SSID);
    strcpyDef(fs_pwd, FS_PWD);
    if (strcmp(fs_ssid,"") == 0) {
        strcpy(fs_ssid, "Sensor-");
        strcat(fs_ssid, esp_chipid.c_str());
    }
    setDef(www_basicauth_enabled, WWW_BASICAUTH_ENABLED);
    setDef(dht11_read, DHT11_READ);
    setDef(dht22_read, DHT22_READ);
    setDef(bme280_read, BME280_READ);
    setDef(ds18b20_read, DS18B20_READ);

    setDef(debug_level, DEBUG_LEVEL);

    strcpyDef(data_host, DATA_HOST);
    strcpyDef(data_url, DATA_URL);
    setDef(data_port, DATA_PORT);
    strcpyDef(data_sec_usr, DATA_SEC_USR);
    strcpyDef(data_sec_pwd, DATA_SEC_PWD);

#undef strcpyDef
#undef setDef
}

/*****************************************************************
/* read config from spiffs                                       *
/*****************************************************************/
void readConfig() {
    String              json_string = "";

    debug_out(F("Mounting file system..."), DEBUG_MIN_INFO, 0);
    if (SPIFFS.begin()) {
        FSInfo          fsInfo;

        debug_out(F("OK..."), DEBUG_MIN_INFO, 1);
        SPIFFS.info(fsInfo);
        debug_out("totalBytes     : ", DEBUG_MIN_INFO, 0); debug_out(String(fsInfo.totalBytes   ), DEBUG_MIN_INFO, 1);
        debug_out("usedBytes      : ", DEBUG_MIN_INFO, 0); debug_out(String(fsInfo.usedBytes    ), DEBUG_MIN_INFO, 1);
        debug_out("blockSize      : ", DEBUG_MIN_INFO, 0); debug_out(String(fsInfo.blockSize    ), DEBUG_MIN_INFO, 1);
        debug_out("pageSize       : ", DEBUG_MIN_INFO, 0); debug_out(String(fsInfo.pageSize     ), DEBUG_MIN_INFO, 1);
        debug_out("maxOpenFiles   : ", DEBUG_MIN_INFO, 0); debug_out(String(fsInfo.maxOpenFiles ), DEBUG_MIN_INFO, 1);
        debug_out("maxPathLength  : ", DEBUG_MIN_INFO, 0); debug_out(String(fsInfo.maxPathLength), DEBUG_MIN_INFO, 1);
        if (SPIFFS.exists("/config.json")) {
            File        configFile;

            debug_out(F("Reading configuration..."), DEBUG_MIN_INFO, 1);
            if (NULL != (configFile = SPIFFS.open("/config.json", "r"))) {
                debug_out(F("Opened config file..."), DEBUG_MIN_INFO, 1);
                size_t size = configFile.size();
                debug_out(FileSize2String(size), DEBUG_MIN_INFO, 1);
                // Allocate a buffer to store contents of the file.
                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);
                StaticJsonBuffer<2000> jsonBuffer;
                JsonObject& json = jsonBuffer.parseObject(buf.get());
                json.printTo(json_string);
                debug_out(F("File content: "), DEBUG_MAX_INFO, 0);
                debug_out(String(buf.get()), DEBUG_MAX_INFO, 1);
                debug_out(F("JSON Buffer content: "), DEBUG_MAX_INFO, 0);
                debug_out(json_string, DEBUG_MAX_INFO, 1);
                if (json.success()) {
                    debug_out(F("Parsed json..."), DEBUG_MIN_INFO, 1);
                    if (json.containsKey("SOFTWARE_VERSION")) {
                        strcpy(version_from_local_config, json["SOFTWARE_VERSION"]);
                    }

#define setFromJSON(key)    if (json.containsKey(#key)) key = json[#key];
#define strcpyFromJSON(key) if (json.containsKey(#key)) strcpy(key, json[#key]);
                    strcpyFromJSON(wlanssid);
                    strcpyFromJSON(wlanpwd);
                    strcpyFromJSON(www_username);
                    strcpyFromJSON(www_password);
                    strcpyFromJSON(fs_ssid);
                    strcpyFromJSON(fs_pwd);
                    setFromJSON(www_basicauth_enabled);
                    setFromJSON(dht11_read);
                    setFromJSON(dht22_read);
                    setFromJSON(bme280_read);
                    setFromJSON(ds18b20_read);
                    setFromJSON(debug_level);
                    setFromJSON(sending_intervall_ms);
                    setFromJSON(time_for_wifi_config);
                    strcpyFromJSON(data_host);
                    strcpyFromJSON(data_url);
                    setFromJSON(data_port);
                    strcpyFromJSON(data_sec_usr);
                    strcpyFromJSON(data_sec_pwd);
#undef setFromJSON
#undef strcpyFromJSON
                } else {
                    debug_out(F("Failed to load json config"), DEBUG_ERROR, 1);
                }
            }
        } else {
            debug_out(F("Config file not opened/found..."), DEBUG_ERROR, 1);
        }
    } else {
        debug_out(F("Failed..."), DEBUG_ERROR, 1);
    }
}

/*****************************************************************
/* Base64 encode user:password                                   *
/*****************************************************************/
void create_basic_auth_strings() {
    sec_basic_auth = "";
    if (strcmp(data_sec_usr, "") != 0 || strcmp(data_sec_pwd, "") != 0) {
        sec_basic_auth = base64::encode(String(data_sec_usr) + ":" + String(data_sec_pwd));
    }
}

/*****************************************************************
/* write config to spiffs                                        *
/*****************************************************************/
void writeConfig() {
    String json_string = "";

    if (!config_needs_write) {
        return;
    }
    debug_out(F("Saving config..."), DEBUG_MIN_INFO, 1);

    json_string = "{";
#define copyToJSON_Bool(varname) json_string +="\""+String(#varname)+"\":"+(varname ? "true":"false")+",";
#define copyToJSON_Int(varname) json_string +="\""+String(#varname)+"\":"+String(varname)+",";
#define copyToJSON_String(varname) json_string +="\""+String(#varname)+"\":\""+String(varname)+"\",";
    copyToJSON_String(SOFTWARE_VERSION);
    copyToJSON_String(wlanssid);
    copyToJSON_String(wlanpwd);
    copyToJSON_String(www_username);
    copyToJSON_String(www_password);
    copyToJSON_String(fs_ssid);
    copyToJSON_String(fs_pwd);
    copyToJSON_Bool(www_basicauth_enabled);
    copyToJSON_Bool(dht11_read);
    copyToJSON_Bool(dht22_read);
    copyToJSON_Bool(bme280_read);
    copyToJSON_Bool(ds18b20_read);
    copyToJSON_String(debug_level);
    copyToJSON_String(sending_intervall_ms);
    copyToJSON_String(time_for_wifi_config);
    copyToJSON_String(data_host);
    copyToJSON_String(data_url);
    copyToJSON_Int(data_port);
    copyToJSON_String(data_sec_usr);
    copyToJSON_String(data_sec_pwd);
#undef copyToJSON_Bool
#undef copyToJSON_Int
#undef copyToJSON_String

    json_string.remove(json_string.length() - 1);
    json_string += "}";

    debug_out(F("Opening configuration file for update..."), DEBUG_ERROR, 0);
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
        debug_out(F("Failed..."), DEBUG_ERROR, 1);
    } else {
        debug_out(F("OK..."), DEBUG_MIN_INFO, 1);
    }
    debug_out(F("Configuration prepared: "), DEBUG_MIN_INFO, 0);
    debug_out(json_string, DEBUG_MIN_INFO, 1);
    configFile.print(json_string);
    debug_out(FileSize2String(configFile.size()), DEBUG_MIN_INFO, 1);
    debug_out(String(json_string.length()), DEBUG_MIN_INFO, 1);
    configFile.close();
    configFile.flush();
    debug_out(F("Config written..."), DEBUG_MIN_INFO, 0);
    // End save
    create_basic_auth_strings();
    config_needs_write = false;
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
    s.replace("{i}", info);
    s.replace("{n}", name);
    s.replace("{v}", value);
    s.replace("{l}", String(length));
    return s;
}

String form_password(const String& name, const String& info, const String& value, const int length) {
    String password = "";
    for (int i = 0; i < value.length(); i++) {
        password += "*";
    }
    String s = F("<tr><td>{i} </td><td style='width:90%;'><input type='password' name='{n}' id='{n}' placeholder='{i}' value='{v}' maxlength='{l}'/></td></tr>");
    s.replace("{i}", info);
    s.replace("{n}", name);
    s.replace("{v}", password);
    s.replace("{l}", String(length));
    return s;
}

String form_checkbox(const String& name, const String& info, const bool checked) {
    String s = F("<label for='{n}'><input type='checkbox' name='{n}' value='1' id='{n}' {c}/><input type='hidden' name='{n}' value='0' /> {i}</label><br/>");
    if (checked) {
        s.replace("{c}", F(" checked='checked'"));
    }
    else {
        s.replace("{c}", "");
    };
    s.replace("{i}", info);
    s.replace("{n}", name);
    return s;
}

String form_checkbox_sensor(const String& name, const String& info, const bool checked) {
    return form_checkbox(name, add_sensor_type(info), checked);
}

String form_submit(const String& value) {
    String s = F("<tr><td>&nbsp;</td><td><input type='submit' name='submit' value='{v}' /></td></tr>");
    s.replace("{v}", value);
    return s;
}

String tmpl(const String& patt, const String& value) {
    String s = F("{patt}");
    s.replace("{patt}", patt);
    s.replace("{v}", value);
    return s;
}

String tmpl(const String& patt, const String& value1, const String& value2) {
    String s = F("{patt}");
    s.replace("{patt}", patt);
    s.replace("{v1}", value1);
    s.replace("{v2}", value2);
    return s;
}

String tmpl(const String& patt, const String& value1, const String& value2, const String& value3) {
    String s = F("{patt}");
    s.replace("{patt}", patt);
    s.replace("{v1}", value1);
    s.replace("{v2}", value2);
    s.replace("{v3}", value3);
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
    s.replace("{s}", sensor);
    s.replace("{p}", param);
    s.replace("{v}", value);
    s.replace("{u}", unit);
    return s;
}

String wlan_ssid_to_table_row(const String& ssid, const String& encryption, const long rssi) {
    long rssi_temp = rssi;
/*    if (rssi_temp > -50) {
        rssi_temp = -50;
    }*/
    if (rssi_temp < -100) {
        rssi_temp = -100;
    }
    int quality = (rssi_temp + 100) * 2;
    String s = F("<tr><td><a href='#wlanpwd' onclick='setSSID(this)' style='background:none;color:blue;padding:5px;display:inline;'>{n}</a>&nbsp;{e}</a></td><td style='width:80%;vertical-align:middle;'>{v}%</td></tr>");
    s.replace("{n}", ssid);
    s.replace("{e}", encryption);
    s.replace("{v}", String(quality));
    return s;
}

String warning_first_cycle() {
    String s = FPSTR(INTL_ERSTER_MESSZYKLUS);
    unsigned long time_to_first = sending_intervall_ms - (act_milli - starttime);
    if ((time_to_first > sending_intervall_ms) || (time_to_first < 0)) {
        time_to_first = 0;
    }
    s.replace("{v}", String((long)((time_to_first + 500) / 1000)));
    return s;
}

String age_last_values() {
    String s = "<b>";
    unsigned long time_since_last = act_milli - starttime;
    if ((time_since_last > sending_intervall_ms) || (time_since_last < 0)) {
        time_since_last = 0;
    }
    s += String((long)((time_since_last + 500) / 1000));
    s += FPSTR(INTL_ZEIT_SEIT_LETZTER_MESSUNG);
    s += F("</b><br/><br/>");
    return s;
}

String add_sensor_type(const String& sensor_text) {
    String s = "";
    s += sensor_text;
    s.replace("{t}",FPSTR(INTL_TEMPERATUR));
    s.replace("{h}",FPSTR(INTL_LUFTFEUCHTE));
    s.replace("{p}",FPSTR(INTL_LUFTDRUCK));
    return s;
}

/*****************************************************************
/* Webserver request auth: prompt for BasicAuth
 *
 * -Provide BasicAuth for all page contexts except /values and images
/*****************************************************************/
void webserver_request_auth() {
    debug_out(F("validate request auth..."), DEBUG_MIN_INFO, 1);
    if (www_basicauth_enabled) {
        if (!server.authenticate(www_username, www_password))
        {
            return server.requestAuthentication();
        }
    }
}

/*****************************************************************
/* Webserver root: show all options                              *
/*****************************************************************/
void webserver_root() {
    if (WiFi.status() != WL_CONNECTED) {
        server.sendHeader(F("Location"), F("http://192.168.4.1/config"));
        server.send(302, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), "");
    } else {
        String page_content = "";

        webserver_request_auth();
        last_page_load = millis();
        debug_out(F("Output root page..."), DEBUG_MIN_INFO, 1);
        page_content = make_header(" ");
        page_content += FPSTR(WEB_ROOT_PAGE_CONTENT);
        page_content.replace("{t}", FPSTR(INTL_AKTUELLE_WERTE));
        page_content.replace(F("{conf}"), FPSTR(INTL_KONFIGURATION));
        page_content.replace(F("{conf_delete}"), FPSTR(INTL_KONFIGURATION_LOSCHEN));
        page_content.replace(F("{restart}"), FPSTR(INTL_SENSOR_NEU_STARTEN));
        page_content += make_footer();
        server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
    }
}

/*****************************************************************
/* Webserver config: show config page                            *
/*****************************************************************/
void webserver_config() {
    webserver_request_auth();

    String page_content = "";
    String masked_pwd = "";
    int i = 0;
    last_page_load = millis();

    debug_out(F("Output config page ..."), DEBUG_MIN_INFO, 1);
    page_content += make_header(FPSTR(INTL_KONFIGURATION));
    if (WiFi.status() != WL_CONNECTED) {  // scan for wlan ssids
        page_content += FPSTR(WEB_CONFIG_SCRIPT);
    }
    if (server.method() == HTTP_GET) {
        page_content += F("<form method='POST' action='/config' style='width:100%;'><b>");
        page_content += FPSTR(INTL_WLAN_DATEN);
        page_content += F("</b><br/>");
        debug_out(F("output config page 1"), DEBUG_MIN_INFO, 1);
        if (WiFi.status() != WL_CONNECTED) {  // scan for wlan ssids
            page_content += F("<div id='wifilist'>");
            page_content += FPSTR(INTL_WLAN_LISTE);
            page_content += F("</div><br/>");
        }
        page_content += F("<table>");
        page_content += form_input("wlanssid", F("WLAN"), wlanssid, 64);
        page_content += form_password("wlanpwd", FPSTR(INTL_PASSWORT), wlanpwd, 64);
//        page_content += form_submit(FPSTR(INTL_SPEICHERN));
        page_content += F("</table><br/><hr/><b>");

        page_content += FPSTR(INTL_AB_HIER_NUR_ANDERN);
        page_content += F("</b><br/><br/><b>");

        page_content += FPSTR(INTL_BASICAUTH);
        page_content += F("</b><br/>");
        page_content += F("<table>");
        page_content += form_input("www_username", FPSTR(INTL_BENUTZER), www_username, 64);
        page_content += form_password("www_password", FPSTR(INTL_PASSWORT), www_password, 64);
        page_content += form_checkbox("www_basicauth_enabled", FPSTR(INTL_BASICAUTH), www_basicauth_enabled);
//        page_content += form_submit(FPSTR(INTL_SPEICHERN));

        page_content += F("</table><br/><b>");
        page_content += FPSTR(INTL_FS_WIFI);
        page_content += F("</b><br/>");
        page_content += FPSTR(INTL_FS_WIFI_BESCHREIBUNG);
        page_content += F("<br/>");
        page_content += F("<table>");
        page_content += form_input("fs_ssid", FPSTR(INTL_FS_WIFI_NAME), fs_ssid, 64);
        page_content += form_password("fs_pwd", FPSTR(INTL_PASSWORT), fs_pwd, 64);
//        page_content += form_submit(FPSTR(INTL_SPEICHERN));

        page_content += F("</table><br/><b>APIs</b><br/>");
        page_content += F("<br/><b>");
        page_content += FPSTR(INTL_SENSOREN);
        page_content += F("</b><br/>");
        page_content += form_checkbox_sensor("dht11_read", FPSTR(INTL_DHT11), dht11_read);
        page_content += form_checkbox_sensor("dht22_read", FPSTR(INTL_DHT22), dht22_read);
        page_content += form_checkbox_sensor("bme280_read", FPSTR(INTL_BME280), bme280_read);
        page_content += form_checkbox_sensor("ds18b20_read", FPSTR(INTL_DS18B20), ds18b20_read);
        page_content += F("<br/><b>");
        page_content += FPSTR(INTL_WEITERE_EINSTELLUNGEN);
        page_content += F("</b><br/>");
        page_content += F("<table>");
        page_content += form_input("debug_level", FPSTR(INTL_DEBUG_LEVEL), String(debug_level), 5);
        page_content += form_input("sending_intervall_ms", FPSTR(INTL_MESSINTERVALL), String(sending_intervall_ms / 1000), 5);
        page_content += form_input("time_for_wifi_config", FPSTR(INTL_DAUER_ROUTERMODUS), String(time_for_wifi_config / 1000), 5);
        page_content += F("</table><br/><b>");
        page_content += FPSTR(INTL_WEITERE_APIS);
        page_content += F("</b><br/><br/>");
        page_content += F("<table>");
        page_content += form_input("data_host", FPSTR(INTL_SERVER), data_host, 50);
        page_content += form_input("data_url", FPSTR(INTL_PFAD), data_url, 50);
        page_content += form_input("data_port", FPSTR(INTL_PORT), String(data_port), 30);
        page_content += form_input("data_sec_usr", FPSTR(INTL_BENUTZER), data_sec_usr, 50);
        page_content += form_password("data_sec_pwd", FPSTR(INTL_PASSWORT), data_sec_pwd, 50);
        page_content += F("</table><br/>");
        page_content += form_submit(FPSTR(INTL_SPEICHERN));
        page_content += F("<br/></form>");
        if (WiFi.status() != WL_CONNECTED) {  // scan for wlan ssids
            page_content += F("<script>window.setTimeout(load_wifi_list,3000);</script>");
        }
    } else {

#define readCharParam(param) if (server.hasArg(#param)){ server.arg(#param).toCharArray(param, sizeof(param)); }
#define readBoolParam(param) if (server.hasArg(#param)){ param = server.arg(#param) == "1"; }
#define readIntParam(param)  if (server.hasArg(#param)){ int val = server.arg(#param).toInt(); if (val != 0){ param = val; }}
#define readTimeParam(param)  if (server.hasArg(#param)){ int val = server.arg(#param).toInt(); if (val != 0){ param = val*1000; }}
#define readPasswdParam(param) if (server.hasArg(#param)){ i = 0; masked_pwd = ""; for (i=0;i<server.arg(#param).length();i++) masked_pwd += "*"; if (masked_pwd != server.arg(#param) || server.arg(#param) == "") { server.arg(#param).toCharArray(param, sizeof(param)); } }

        if (server.hasArg("wlanssid") && server.arg("wlanssid") != "") {
            readCharParam(wlanssid);
            readPasswdParam(wlanpwd);
        }
        readCharParam(www_username);
        readPasswdParam(www_password);
        readBoolParam(www_basicauth_enabled);
        readCharParam(fs_ssid);
        if (server.hasArg("fs_pwd") && (server.arg("fs_pwd").length() > 7)) {
            readPasswdParam(fs_pwd);
        }
        readBoolParam(dht11_read);
        readBoolParam(dht22_read);
        readBoolParam(bme280_read);
        readBoolParam(ds18b20_read);
        readIntParam(debug_level);
        readTimeParam(sending_intervall_ms);
        readTimeParam(time_for_wifi_config);

        readCharParam(data_host);
        readCharParam(data_url);
        readIntParam(data_port);
        readCharParam(data_sec_usr);
        readPasswdParam(data_sec_pwd);

#undef readCharParam
#undef readBoolParam
#undef readIntParam

        page_content += line_from_value(tmpl(FPSTR(INTL_LESE), "DHT11"), String(dht11_read));
        page_content += line_from_value(tmpl(FPSTR(INTL_LESE), "DHT22"), String(dht22_read));
        page_content += line_from_value(tmpl(FPSTR(INTL_LESE), "BME280"), String(bme280_read));
        page_content += line_from_value(tmpl(FPSTR(INTL_LESE), "DS18B20"), String(ds18b20_read));
        page_content += line_from_value(FPSTR(INTL_DEBUG_LEVEL), String(debug_level));
        page_content += line_from_value(FPSTR(INTL_MESSINTERVALL), String(sending_intervall_ms));
        page_content += line_from_value(FPSTR(INTL_SERVER), data_host);
        page_content += line_from_value(FPSTR(INTL_PFAD), data_url);
        page_content += line_from_value(FPSTR(INTL_PORT), String(data_port));
        page_content += line_from_value(FPSTR(INTL_BENUTZER), data_sec_usr);
        page_content += line_from_value(FPSTR(INTL_PASSWORT), data_sec_pwd);
        if (wificonfig_loop) {
            page_content += F("<br/><br/>");
            page_content += FPSTR(INTL_GERAT_WIRD_NEU_GESTARTET);
        } else {
            page_content += F("<br/><br/><a href='/reset?confirm=yes'>");
            page_content += FPSTR(INTL_GERAT_NEU_STARTEN);
            page_content += F("?</a>");
        }
        config_needs_write = true;
    }
    page_content += make_footer();
    server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
}

/*****************************************************************
/* Webserver wifi: show available wifi networks                  *
/*****************************************************************/
void webserver_wifi() {
    WiFi.disconnect();
    debug_out(F("scan for wifi networks..."), DEBUG_MIN_INFO, 1);
    int n = WiFi.scanNetworks();
    debug_out(F("wifi networks found: "), DEBUG_MIN_INFO, 0);
    debug_out(String(n), DEBUG_MIN_INFO, 1);
    String page_content = "";
    if (n == 0) {
        page_content += F("<br/>");
        page_content += FPSTR(INTL_KEINE_NETZWERKE);
        page_content += F("<br/>");
    } else {
        page_content += FPSTR(INTL_NETZWERKE_GEFUNDEN);
        page_content += String(n);
        page_content += F("<br/>");
        int indices[n];
        debug_out(F("output config page 2"), DEBUG_MIN_INFO, 1);
        for (int i = 0; i < n; i++) {
            indices[i] = i;
        }
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
                    std::swap(indices[i], indices[j]);
                }
            }
        }
        String cssid;
        debug_out(F("output config page 3"), DEBUG_MIN_INFO, 1);
        for (int i = 0; i < n; i++) {
            if (indices[i] == -1) {
                continue;
            }
            cssid = WiFi.SSID(indices[i]);
            for (int j = i + 1; j < n; j++) {
                if (cssid == WiFi.SSID(indices[j])) {
                    indices[j] = -1; // set dup aps to index -1
                }
            }
        }
        page_content += F("<br/><table>");
        //if(n > 30) n=30;
        for (int i = 0; i < n; ++i) {
            if (indices[i] == -1) {
                continue;
            }
            // Print SSID and RSSI for each network found
            page_content += wlan_ssid_to_table_row(WiFi.SSID(indices[i]), ((WiFi.encryptionType(indices[i]) == ENC_TYPE_NONE) ? " " : "*"), WiFi.RSSI(indices[i]));
        }
        page_content += F("</table><br/><br/>");
    }
    server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
}

/*****************************************************************
/* Webserver root: show latest values                            *
/*****************************************************************/
void webserver_values() {
    if (WiFi.status() != WL_CONNECTED) {
        server.sendHeader(F("Location"), F("http://192.168.4.1/config"));
        server.send(302, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), "");
    } else {
        String page_content = "";
        const String unit_PM = " µg/m³";
        const String unit_T = " °C";
        const String unit_H = " %";
        const String unit_P = " hPa";
        String empty_row = F("<tr><td colspan='3'>&nbsp;</td></tr>");
        last_page_load = millis();
        long signal_strength = WiFi.RSSI();
        if (signal_strength > -50) {
            signal_strength = -50;
        }
        if (signal_strength < -100) {
            signal_strength = -100;
        }
        int signal_quality = (signal_strength + 100) * 2;
        debug_out(F("output values to web page..."), DEBUG_MIN_INFO, 1);
        page_content += make_header(FPSTR(INTL_AKTUELLE_WERTE));
        if (first_cycle) {
            page_content += F("<b style='color:red'>");
            page_content += warning_first_cycle();
            page_content += F("</b><br/><br/>");
        } else {
            page_content += age_last_values();
        }
        page_content += F("<table cellspacing='0' border='1' cellpadding='5'>");
        page_content += tmpl(F("<tr><th>{v1}</th><th>{v2}</th><th>{v3}</th>"), FPSTR(INTL_SENSOR), FPSTR(INTL_PARAMETER), FPSTR(INTL_WERT));
        if (dht11_read) {
            page_content += empty_row;
            page_content += table_row_from_value("DHT11", FPSTR(INTL_TEMPERATUR), check_display_value(last_value_DHT11_T, -128, 1, 0), unit_T);
            page_content += table_row_from_value("DHT11", FPSTR(INTL_LUFTFEUCHTE), check_display_value(last_value_DHT11_H, -1, 1, 0), unit_H);
        }
        if (dht22_read) {
            page_content += empty_row;
            page_content += table_row_from_value("DHT22", FPSTR(INTL_TEMPERATUR), check_display_value(last_value_DHT22_T, -128, 1, 0), unit_T);
            page_content += table_row_from_value("DHT22", FPSTR(INTL_LUFTFEUCHTE), check_display_value(last_value_DHT22_H, -1, 1, 0), unit_H);
        }
        if (bme280_read) {
            page_content += empty_row;
            page_content += table_row_from_value("BME280", FPSTR(INTL_TEMPERATUR), check_display_value(last_value_BME280_T, -128, 1, 0), unit_T);
            page_content += table_row_from_value("BME280", FPSTR(INTL_LUFTFEUCHTE), check_display_value(last_value_BME280_H, -1, 1, 0), unit_H);
            page_content += table_row_from_value("BME280", FPSTR(INTL_LUFTDRUCK),  check_display_value(last_value_BME280_P / 100.0, (-1 / 100.0), 2, 0), unit_P);
        }
        if (ds18b20_read) {
            page_content += empty_row;
            page_content += table_row_from_value("DS18B20", FPSTR(INTL_TEMPERATUR), check_display_value(last_value_DS18B20_T, -128, 1, 0), unit_T);
        }
        page_content += empty_row;
        page_content += table_row_from_value("WiFi", FPSTR(INTL_SIGNAL),  String(signal_strength), "dBm");
        page_content += table_row_from_value("WiFi", FPSTR(INTL_QUALITAT), String(signal_quality), "%");

        page_content += empty_row;
        page_content += F("<tr><td colspan='2'>");
        page_content += FPSTR(INTL_ANZAHL_MESSUNGEN);
        page_content += F("</td><td class='r'>");
        page_content += String(count_sends);
        page_content += F("</td></tr>");
        page_content += F("</table>");
        page_content += make_footer();
        server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
    }
}

/*****************************************************************
/* Webserver set debug level                                     *
/*****************************************************************/
void webserver_debug_level() {
    webserver_request_auth();

    String page_content = "";
    String message_string = F("<h3>{v1} {v2}.</h3>");
    last_page_load = millis();
    debug_out(F("output change debug level page..."), DEBUG_MIN_INFO, 1);
    page_content += make_header(FPSTR(INTL_DEBUG_LEVEL));

    if (server.hasArg("lvl")) {
        switch (server.arg("lvl").toInt()) {
        case (0):
            debug_level = 0;
            page_content += tmpl(message_string, FPSTR(INTL_SETZE_DEBUG_AUF), FPSTR(INTL_NONE));
            break;
        case (1):
            debug_level = 1;
            page_content += tmpl(message_string, FPSTR(INTL_SETZE_DEBUG_AUF), FPSTR(INTL_ERROR));
            break;
        case (2):
            debug_level = 2;
            page_content += tmpl(message_string, FPSTR(INTL_SETZE_DEBUG_AUF), FPSTR(INTL_WARNING));
            break;
        case (3):
            debug_level = 3;
            page_content += tmpl(message_string, FPSTR(INTL_SETZE_DEBUG_AUF), FPSTR(INTL_MIN_INFO));
            break;
        case (4):
            debug_level = 4;
            page_content += tmpl(message_string, FPSTR(INTL_SETZE_DEBUG_AUF), FPSTR(INTL_MED_INFO));
            break;
        case (5):
            debug_level = 5;
            page_content += tmpl(message_string, FPSTR(INTL_SETZE_DEBUG_AUF), FPSTR(INTL_MAX_INFO));
            break;
        }
    }
    page_content += make_footer();
    server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
}

/*****************************************************************
/* Webserver remove config                                       *
/*****************************************************************/
void webserver_removeConfig() {
    webserver_request_auth();

    String page_content = "";
    String message_string = F("<h3>{v}.</h3>");
    last_page_load = millis();
    debug_out(F("output remove config page..."), DEBUG_MIN_INFO, 1);
    page_content += make_header(FPSTR(INTL_CONFIG_LOSCHEN));

    if (server.method() == HTTP_GET) {
        page_content += FPSTR(WEB_REMOVE_CONFIG_CONTENT);
        page_content.replace("{t}", FPSTR(INTL_KONFIGURATION_WIRKLICH_LOSCHEN));
        page_content.replace("{b}", FPSTR(INTL_LOSCHEN));
        page_content.replace("{c}", FPSTR(INTL_ABBRECHEN));

    } else {
        if (SPIFFS.exists("/config.json")) {    //file exists
            debug_out(F("Removing config.json..."), DEBUG_MIN_INFO, 1);
            if (SPIFFS.remove("/config.json")) {
                page_content += tmpl(message_string, FPSTR(INTL_CONFIG_GELOSCHT));
            } else {
                page_content += tmpl(message_string, FPSTR(INTL_CONFIG_KONNTE_NICHT_GELOSCHT_WERDEN));
            }
            SPIFFS.format();
        } else {
            page_content += tmpl(message_string, FPSTR(INTL_CONFIG_NICHT_GEFUNDEN));
        }
    }
    page_content += make_footer();
    server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
}

/*****************************************************************
/* Perform MCU reset                                             *
/*****************************************************************/
void mcu_reset(void)
{
    writeConfig();
    SPIFFS.end();
    // Set WDT to shorter time in case restart fails
    wdt_disable();
    wdt_enable(500);// 0.5sec
    debug_out(F("calling ESP.restart()"), DEBUG_MIN_INFO, 1);
    ESP.restart();
}

/*****************************************************************
/* Webserver reset MCU                                           *
/*****************************************************************/
void webserver_reset() {
    webserver_request_auth();

    String page_content = "";
    last_page_load = millis();
    debug_out(F("Output reset page..."), DEBUG_MIN_INFO, 1);
    page_content += make_header(FPSTR(INTL_SENSOR_NEU_STARTEN));

    if (server.method() == HTTP_GET) {
        page_content += FPSTR(WEB_RESET_CONTENT);
        page_content.replace("{t}", FPSTR(INTL_SENSOR_WIRKLICH_NEU_STARTEN));
        page_content.replace("{b}", FPSTR(INTL_NEU_STARTEN));
        page_content.replace("{c}", FPSTR(INTL_ABBRECHEN));
    } else {
        mcu_reset();
    }
    page_content += make_footer();
    server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
}

/*****************************************************************
/* Webserver data.json                                           *
/*****************************************************************/
void webserver_data_json() {
    String s1 = "";
    unsigned long age = 0;
    debug_out(F("output data json..."), DEBUG_MIN_INFO, 1);
    if (first_cycle) {
        s1 = data_first_part + "]}";
        age = sending_intervall_ms - (act_milli - starttime);
        if ((age > sending_intervall_ms) || (age < 0)) {
            age = 0;
        }
        age = 0 - age;
    } else {
        s1 = last_data_string;
        debug_out(F("last data: "), DEBUG_MIN_INFO, 0);
        debug_out(s1, DEBUG_MIN_INFO, 1);
        age = act_milli - starttime;
        if ((age > sending_intervall_ms) || (age < 0)) {
            age = 0;
        }
    }
    String s2 = F(", \"age\":\"");
    s2 += String((long)((age + 500) / 1000));
    s2 += F("\", \"sensordatavalues\"");
    debug_out(F("replace with: "), DEBUG_MIN_INFO, 0);
    debug_out(s2, DEBUG_MIN_INFO, 1);
    s1.replace(F(", \"sensordatavalues\""), s2);
    debug_out(F("replaced: "), DEBUG_MIN_INFO, 0);
    debug_out(s1, DEBUG_MIN_INFO, 1);
    server.send(200, FPSTR(TXT_CONTENT_TYPE_JSON), s1);
}

/*****************************************************************
/* Webserver Images                                              *
/*****************************************************************/
void webserver_images() {
    webserver_not_found();
}

/*****************************************************************
/* Webserver page not found                                      *
/*****************************************************************/
void webserver_not_found() {
    last_page_load = millis();
    debug_out(F("output not found page..."), DEBUG_MIN_INFO, 1);
    server.send(404, FPSTR(TXT_CONTENT_TYPE_TEXT_PLAIN), F("Not found."));
}

/*****************************************************************
/* Webserver setup                                               *
/*****************************************************************/
void setup_webserver() {
    server_name  = F("Sensor-");
    server_name += esp_chipid;

    server.on("/", webserver_root);
    server.on("/config", webserver_config);
    server.on("/wifi", webserver_wifi);
    server.on("/values", webserver_values);
    server.on("/generate_204", webserver_config);
    server.on("/fwlink", webserver_config);
    server.on("/debug", webserver_debug_level);
    server.on("/removeConfig", webserver_removeConfig);
    server.on("/reset", webserver_reset);
    server.on("/data.json", webserver_data_json);
    server.on("/images", webserver_images);
    server.onNotFound(webserver_not_found);

    debug_out(F("Starting Webserver... "), DEBUG_MIN_INFO, 1);
    server.begin();
}

/*****************************************************************
/* WifiConfig                                                    *
/*****************************************************************/
void wifiConfig() {
    const char          *softAP_password = "";
    const byte          DNS_PORT = 53;
    int                 retry_count = 0;
    DNSServer           dnsServer;
    IPAddress           apIP(192, 168, 4, 1);
    IPAddress           netMsk(255, 255, 255, 0);

    debug_out(F("Starting WiFiManager"), DEBUG_MIN_INFO, 1);
    debug_out(F("AP ID: "), DEBUG_MIN_INFO, 0);
    debug_out(fs_ssid, DEBUG_MIN_INFO, 1);
    debug_out(F("Password: "), DEBUG_MIN_INFO, 0);
    debug_out(fs_pwd, DEBUG_MIN_INFO, 1);

    wificonfig_loop = true;

    WiFi.softAPConfig(apIP, apIP, netMsk);
    WiFi.softAP(fs_ssid, fs_pwd);

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", apIP);

    debug_out(IPAddress2String(WiFi.localIP()), DEBUG_MIN_INFO, 1);

    // Timeout for wifi config
    last_page_load = millis();
    while (((millis() - last_page_load) < time_for_wifi_config) && (! config_needs_write)) {
        dnsServer.processNextRequest();
        server.handleClient();
        wdt_reset(); // nodemcu is alive
        yield();
    }

    // half second to answer last requests
    last_page_load = millis();
    while (((millis() - last_page_load) < 500)) {
        dnsServer.processNextRequest();
        server.handleClient();
        yield();
    }

    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    dnsServer.stop();

    delay(100);

    debug_out(F("Connecting to "), DEBUG_MIN_INFO, 0);
    debug_out(wlanssid, DEBUG_MIN_INFO, 1);

    WiFi.begin(wlanssid, wlanpwd);

    while ((WiFi.status() != WL_CONNECTED) && (retry_count < 20)) {
        delay(500);
        debug_out(".", DEBUG_MIN_INFO, 0);
        retry_count++;
    }
    debug_out("", DEBUG_MIN_INFO, 1);

    debug_out(F("---- Result from Webconfig ----"), DEBUG_MIN_INFO, 1);
    debug_out(F("WLANSSID: "), DEBUG_MIN_INFO, 0);
    debug_out(wlanssid, DEBUG_MIN_INFO, 1);
    debug_out(F("----\nReading ..."), DEBUG_MIN_INFO, 1);
    debug_out(F("DHT11: "), DEBUG_MIN_INFO, 0);
    debug_out(String(dht11_read), DEBUG_MIN_INFO, 1);
    debug_out(F("DHT22: "), DEBUG_MIN_INFO, 0);
    debug_out(String(dht22_read), DEBUG_MIN_INFO, 1);
    debug_out(F("DS18B20: "), DEBUG_MIN_INFO, 0);
    debug_out(String(ds18b20_read), DEBUG_MIN_INFO, 1);
    debug_out(F("Debug: "), DEBUG_MIN_INFO, 0);
    debug_out(String(debug_level), DEBUG_MIN_INFO, 1);
    debug_out(F("----\nRestart needed ..."), DEBUG_MIN_INFO, 1);
    wificonfig_loop = false;
    restart_needed = true;
}

/*****************************************************************
/* WiFi auto connecting script                                   *
/*****************************************************************/
void connectWifi() {
    int retry_count = 0;
    String s1 = "";
    String s2 = "";

    WiFi.mode(WIFI_STA);
    WiFi.begin(wlanssid, wlanpwd); // Start WiFI

    debug_out(F("Connecting to "), DEBUG_MIN_INFO, 0);
    debug_out(wlanssid, DEBUG_MIN_INFO, 1);

    while ((WiFi.status() != WL_CONNECTED) && (retry_count < 40)) {
        delay(500);
        debug_out(".", DEBUG_MIN_INFO, 0);
        retry_count++;
    }
    debug_out("", DEBUG_MIN_INFO, 1);
    if (WiFi.status() != WL_CONNECTED) {
        s1 = String(fs_ssid);
        s1 = s1.substring(0,16);
        s2 = String(fs_ssid);
        s2 = s2.substring(16);
        wifiConfig();
        if (WiFi.status() != WL_CONNECTED) {
            retry_count = 0;
            while ((WiFi.status() != WL_CONNECTED) && (retry_count < 20) && !restart_needed) {
                delay(500);
                debug_out(".", DEBUG_MIN_INFO, 0);
                retry_count++;
            }
            debug_out("", DEBUG_MIN_INFO, 1);
        }
    }
    WiFi.softAPdisconnect(true);
    debug_out(F("WiFi connected\nIP address: "), DEBUG_MIN_INFO, 0);
    debug_out(IPAddress2String(WiFi.localIP()), DEBUG_MIN_INFO, 1);
}

/*****************************************************************
/* send data to rest api                                         *
/*****************************************************************/
void sendData(const String& data, const int pin, const char* host, const int httpPort, const char* url, const char* basic_auth_string, const String& contentType) {

    debug_out(F("Start connecting to "), DEBUG_MIN_INFO, 0);
    debug_out(host, DEBUG_MIN_INFO, 1);

    String request_head = F("POST ");
    request_head += String(url);
    request_head += F(" HTTP/1.1\r\n");
    request_head += F("Host: ");
    request_head += String(host) + "\r\n";
    request_head += F("Content-Type: ");
    request_head += contentType + "\r\n";
    if (basic_auth_string != "") {
        request_head += F("Authorization: Basic ");
        request_head += String(basic_auth_string) + "\r\n";
    }
    request_head += F("X-PIN: ");
    request_head += String(pin) + "\r\n";
    request_head += F("X-Sensor: esp8266-");
    request_head += esp_chipid + "\r\n";
    request_head += F("Content-Length: ");
    request_head += String(data.length(), DEC) + "\r\n";
    request_head += F("Connection: close\r\n\r\n");

    // Use WiFiClient class to create TCP connections

    if (httpPort == 443) {

        WiFiClientSecure client_s;

        client_s.setNoDelay(true);
        client_s.setTimeout(20000);

        if (!client_s.connect(host, httpPort)) {
            debug_out(F("connection failed"), DEBUG_ERROR, 1);
            return;
        }

        debug_out(F("Requesting URL: "), DEBUG_MIN_INFO, 0);
        debug_out(url, DEBUG_MIN_INFO, 1);
        debug_out(esp_chipid, DEBUG_MIN_INFO, 1);
        debug_out(data, DEBUG_MIN_INFO, 1);

        // send request to the server

        client_s.print(request_head);

        client_s.println(data);

        delay(10);

        // Read reply from server and print them
        while(client_s.available()) {
            char c = client_s.read();
            debug_out(String(c), DEBUG_MAX_INFO, 0);
        }

        debug_out(F("\nclosing connection\n----\n\n"), DEBUG_MIN_INFO, 1);

    } else {

        WiFiClient client;

        client.setNoDelay(true);
        client.setTimeout(20000);

        if (!client.connect(host, httpPort)) {
            debug_out(F("connection failed"), DEBUG_ERROR, 1);
            return;
        }

        debug_out(F("Requesting URL: "), DEBUG_MIN_INFO, 0);
        debug_out(url, DEBUG_MIN_INFO, 1);
        debug_out(esp_chipid, DEBUG_MIN_INFO, 1);
        debug_out(data, DEBUG_MIN_INFO, 1);

        client.print(request_head);

        client.println(data);

        delay(10);

        // Read reply from server and print them
        while(client.available()) {
            char c = client.read();
            debug_out(String(c), DEBUG_MAX_INFO, 0);
        }

        debug_out(F("\nclosing connection\n----\n\n"), DEBUG_MIN_INFO, 1);

    }

    debug_out(F("End connecting to "), DEBUG_MIN_INFO, 0);
    debug_out(host, DEBUG_MIN_INFO, 1);

    wdt_reset(); // nodemcu is alive
    yield();
}

/*****************************************************************
/* read DHT11 sensor values                                      *
/*****************************************************************/
String sensorDHT11() {
    String s = "";
    int i = 0;
    double h;
    double t;

    debug_out(String(FPSTR(DBG_TXT_START_READING)) + "DHT11", DEBUG_MED_INFO, 1);

    // Check if valid number if non NaN (not a number) will be send.

    last_value_DHT11_T = -128;
    last_value_DHT11_H = -1;

    while ((i++ < 5) && (s == "")) {
        h = dht11.readHumidity(); //Read Humidity
        t = dht11.readTemperature(); //Read Temperature
        if (isnan(t) || isnan(h)) {
            delay(100);
            h = dht11.readHumidity(true); //Read Humidity
            t = dht11.readTemperature(false, true); //Read Temperature
        }
        if (isnan(t) || isnan(h)) {
            debug_out(F("DHT11 couldn't be read"), DEBUG_ERROR, 1);
        } else {
            debug_out(FPSTR(DBG_TXT_TEMPERATURE), DEBUG_MIN_INFO, 0);
            debug_out(String(t) + char(223) + "C", DEBUG_MIN_INFO, 1);
            debug_out(FPSTR(DBG_TXT_HUMIDITY), DEBUG_MIN_INFO, 0);
            debug_out(String(h) + "%", DEBUG_MIN_INFO, 1);
            last_value_DHT11_T = t;
            last_value_DHT11_H = h;
            s += Value2Json(F("DHT11_temperature"), Float2String(last_value_DHT11_T));
            s += Value2Json(F("DHT11_humidity"), Float2String(last_value_DHT11_H));
        }
    }
    debug_out(F("----"), DEBUG_MIN_INFO, 1);

    debug_out(String(FPSTR(DBG_TXT_END_READING)) + "DHT11", DEBUG_MED_INFO, 1);

    return s;
}

/*****************************************************************
/* read DHT22 sensor values                                      *
/*****************************************************************/
String sensorDHT22() {
    String s = "";
    int i = 0;
    double h;
    double t;

    debug_out(String(FPSTR(DBG_TXT_START_READING)) + "DHT22", DEBUG_MED_INFO, 1);

    // Check if valid number if non NaN (not a number) will be send.

    last_value_DHT22_T = -128;
    last_value_DHT22_H = -1;

    while ((i++ < 5) && (s == "")) {
        h = dht22.readHumidity(); //Read Humidity
        t = dht22.readTemperature(); //Read Temperature
        if (isnan(t) || isnan(h)) {
            delay(100);
            h = dht22.readHumidity(true); //Read Humidity
            t = dht22.readTemperature(false, true); //Read Temperature
        }
        if (isnan(t) || isnan(h)) {
            debug_out(F("DHT22 couldn't be read"), DEBUG_ERROR, 1);
        } else {
            debug_out(FPSTR(DBG_TXT_TEMPERATURE), DEBUG_MIN_INFO, 0);
            debug_out(String(t) + char(223) + "C", DEBUG_MIN_INFO, 1);
            debug_out(FPSTR(DBG_TXT_HUMIDITY), DEBUG_MIN_INFO, 0);
            debug_out(String(h) + "%", DEBUG_MIN_INFO, 1);
            last_value_DHT22_T = t;
            last_value_DHT22_H = h;
            s += Value2Json(F("DHT22_temperature"), Float2String(last_value_DHT22_T));
            s += Value2Json(F("DHT22_humidity"), Float2String(last_value_DHT22_H));
        }
    }
    debug_out(F("----"), DEBUG_MIN_INFO, 1);

    debug_out(String(FPSTR(DBG_TXT_END_READING)) + "DHT22", DEBUG_MED_INFO, 1);

    return s;
}

/*****************************************************************
/* read BME280 sensor values                                     *
/*****************************************************************/
String sensorBME280() {
    String s = "";
    double t;
    double h;
    double p;

    debug_out(String(FPSTR(DBG_TXT_START_READING)) + "BME280", DEBUG_MED_INFO, 1);

    bme280.takeForcedMeasurement();

    t = bme280.readTemperature();
    h = bme280.readHumidity();
    p = bme280.readPressure();
    last_value_BME280_T = -128;
    last_value_BME280_H = -1;
    last_value_BME280_P = -1;
    if (isnan(t) || isnan(h) || isnan(p)) {
        debug_out(F("BME280 couldn't be read"), DEBUG_ERROR, 1);
    } else {
        debug_out(FPSTR(DBG_TXT_TEMPERATURE), DEBUG_MIN_INFO, 0);
        debug_out(Float2String(t) + " C", DEBUG_MIN_INFO, 1);
        debug_out(FPSTR(DBG_TXT_HUMIDITY), DEBUG_MIN_INFO, 0);
        debug_out(Float2String(h) + " %", DEBUG_MIN_INFO, 1);
        debug_out(FPSTR(DBG_TXT_PRESSURE), DEBUG_MIN_INFO, 0);
        debug_out(Float2String(p / 100) + " hPa", DEBUG_MIN_INFO, 1);
        last_value_BME280_T = t;
        last_value_BME280_H = h;
        last_value_BME280_P = p;
        s += Value2Json(F("BME280_temperature"), Float2String(last_value_BME280_T));
        s += Value2Json(F("BME280_humidity"), Float2String(last_value_BME280_H));
        s += Value2Json(F("BME280_pressure"), Float2String(last_value_BME280_P));
    }
    debug_out(F("----"), DEBUG_MIN_INFO, 1);

    debug_out(String(FPSTR(DBG_TXT_END_READING)) + "BME280", DEBUG_MED_INFO, 1);

    return s;
}

/*****************************************************************
/* read DS18B20 sensor values                                    *
/*****************************************************************/
String sensorDS18B20() {
    String s = "";
    int i = 0;
    double t;
    debug_out(String(FPSTR(DBG_TXT_START_READING)) + "DS18B20", DEBUG_MED_INFO, 1);

    last_value_DS18B20_T = -128;

    //it's very unlikely (-127: impossible) to get these temperatures in reality. Most times this means that the sensor is currently faulty
    //try 5 times to read the sensor, otherwise fail
    do {
        ds18b20.requestTemperatures();
        //for now, we want to read only the first sensor
        t = ds18b20.getTempCByIndex(0);
        i++;
        debug_out(F("DS18B20 trying...."), DEBUG_MIN_INFO, 0);
        debug_out(String(i), DEBUG_MIN_INFO, 1);
    } while(i < 5 && (isnan(t) || t == 85.0 || t == (-127.0)));

    if (i == 5) {
        debug_out(F("DS18B20 couldn't be read."), DEBUG_ERROR, 1);
    } else {
        debug_out(FPSTR(DBG_TXT_TEMPERATURE), DEBUG_MIN_INFO, 0);
        debug_out(Float2String(t) + " C", DEBUG_MIN_INFO, 1);
        last_value_DS18B20_T = t;
        s += Value2Json(F("DS18B20_temperature"), Float2String(last_value_DS18B20_T));
    }
    debug_out(F("----"), DEBUG_MIN_INFO, 1);
    debug_out(String(FPSTR(DBG_TXT_END_READING)) + "DS18B20", DEBUG_MED_INFO, 1);

    return s;
}

/*****************************************************************
/* Init BME280                                                   *
/*****************************************************************/
bool initBME280(char addr) {
    debug_out(F("Trying BME280 sensor on "), DEBUG_MIN_INFO, 0);
    debug_out(String(addr, HEX), DEBUG_MIN_INFO, 0);

    if (bme280.begin(addr)) {
        debug_out(F(" ... found"), DEBUG_MIN_INFO, 1);
        bme280.setSampling(
        Adafruit_BME280::MODE_FORCED,
        Adafruit_BME280::SAMPLING_X1,
        Adafruit_BME280::SAMPLING_X1,
        Adafruit_BME280::SAMPLING_X1,
        Adafruit_BME280::FILTER_OFF);
        return true;
    } else {
        debug_out(F(" ... not found"), DEBUG_MIN_INFO, 1);
        return false;
    }
}

/*****************************************************************
/* The Setup                                                     *
/*****************************************************************/
void setup() {
    delay(50);
    Serial.begin(9600);                    // Output to Serial at 9600 baud
    Serial.println("\nStarting...");
    Wire.begin(D3, D4);
    esp_chipid = String(ESP.getChipId());
    WiFi.persistent(false);
    copyExtDef();
    readConfig();
    setup_webserver();
    connectWifi();                        // Start ConnectWifi
    if (restart_needed) {
        mcu_reset();
    }
    create_basic_auth_strings();
    debug_out(F("\nChipId: "), DEBUG_MIN_INFO, 0);
    debug_out(esp_chipid, DEBUG_MIN_INFO, 1);

    if (dht11_read) {
        dht11.begin();                                      // Start DHT11
        debug_out(F("Starting DHT11..."), DEBUG_MIN_INFO, 1);
    }
    if (dht22_read) {
        dht22.begin();                                      // Start DHT22
        debug_out(F("Starting DHT22..."), DEBUG_MIN_INFO, 1);
    }
    if (bme280_read) {
        debug_out(F("Read BME280..."), DEBUG_MIN_INFO, 1);
    }
    if (ds18b20_read) {
        ds18b20.begin();                                    // Start DS18B20
        debug_out(F("Read DS18B20..."), DEBUG_MIN_INFO, 1);
    }
    if (bme280_read && !initBME280(0x76) && !initBME280(0x77)) {
        debug_out(F("Check BME280 wiring"), DEBUG_MIN_INFO, 1);
        bme280_init_failed = 1;
    }

    delay(50);

    // sometimes parallel sending data and web page will stop nodemcu, watchdogtimer set to 30 seconds
    wdt_disable();
    wdt_enable(30000);// 30 sec

    starttime = millis();                                   // store the start time
    Serial.println("Setup done\n");
}

/*****************************************************************
/* And action                                                    *
/*****************************************************************/
void loop() {
    String data = "";
    String tmp_str;
    String data_sample_times = "";

    String result_DHT11 = "";
    String result_DHT22 = "";
    String result_BMP = "";
    String result_BMP280 = "";
    String result_BME280 = "";
    String result_DS18B20 = "";
    String signal_strength = "";

    unsigned long sum_send_time = 0;
    unsigned long start_send;

    send_failed = false;

    act_micro = micros();
    act_milli = millis();
    send_now = (act_milli - starttime) > sending_intervall_ms;

    sample_count++;

    wdt_reset(); // nodemcu is alive

    if (last_micro != 0) {
        diff_micro = act_micro - last_micro;
        if (max_micro < diff_micro) {
            max_micro = diff_micro;
        }
        if (min_micro > diff_micro) {
            min_micro = diff_micro;
        }
        last_micro = act_micro;
    } else {
        last_micro = act_micro;
    }

    server.handleClient();

    if (send_now) {
        if (dht11_read) {
            debug_out(F("Call sensorDHT11"), DEBUG_MAX_INFO, 1);
            result_DHT11 = sensorDHT11();                     // getting temperature and humidity (optional)
        }

        if (dht22_read) {
            debug_out(F("Call sensorDHT22"), DEBUG_MAX_INFO, 1);
            result_DHT22 = sensorDHT22();                     // getting temperature and humidity (optional)
        }

        if (bme280_read && (! bme280_init_failed)) {
            debug_out(F("Call sensorBME280"), DEBUG_MAX_INFO, 1);
            result_BME280 = sensorBME280();                 // getting temperature, humidity and pressure (optional)
        }

        if (ds18b20_read) {
            debug_out(F("Call sensorDS18B20"), DEBUG_MAX_INFO, 1);
            result_DS18B20 = sensorDS18B20();               // getting temperature (optional)
        }
    }

    if (send_now) {
        debug_out(F("Creating data string:"), DEBUG_MIN_INFO, 1);
        data = data_first_part;
        data_sample_times  = Value2Json(F("samples"), String(long(sample_count)));
        data_sample_times += Value2Json(F("min_micro"), String(long(min_micro)));
        data_sample_times += Value2Json(F("max_micro"), String(long(max_micro)));

        signal_strength = String(WiFi.RSSI());
        debug_out(F("WLAN signal strength: "), DEBUG_MIN_INFO, 0);
        debug_out(signal_strength, DEBUG_MIN_INFO, 0);
        debug_out(F(" dBm"), DEBUG_MIN_INFO, 1);
        debug_out(F("----"), DEBUG_MIN_INFO, 1);

        server.handleClient();
        yield();
        server.stop();
        if (dht11_read) {
            data += result_DHT11;
        }
        if (dht22_read) {
            data += result_DHT22;
        }
        if (bme280_read && (! bme280_init_failed)) {
            data += result_BME280;
        }

        if (ds18b20_read) {
            data += result_DS18B20;
        }
        data_sample_times += Value2Json("signal", signal_strength);
        data += data_sample_times;

        if (data.lastIndexOf(',') == (data.length() - 1)) {
            data.remove(data.length() - 1);
        }
        data += "]}";

        //sending to api(s)

        data.remove(0, 1);
        data = "{\"esp8266id\": \"" + String(esp_chipid) + "\", " + data;
        debug_out(F("## Sending to the API: "), DEBUG_MIN_INFO, 1);
        start_send = micros();
        sendData(data, 0, data_host, data_port, data_url, sec_basic_auth.c_str(), FPSTR(TXT_CONTENT_TYPE_JSON));
        sum_send_time += micros() - start_send;

        server.begin();

        if (! send_failed) {
            sending_time = (4 * sending_time + sum_send_time) / 5;
        }
        debug_out(F("Time for sending data: "), DEBUG_MIN_INFO, 0);
        debug_out(String(sending_time), DEBUG_MIN_INFO, 1);


        if (WiFi.status() != WL_CONNECTED) {                // reconnect if connection lost
            int retry_count = 0;
            debug_out(F("Connection lost, reconnecting "), DEBUG_MIN_INFO, 0);
            WiFi.reconnect();
            while ((WiFi.status() != WL_CONNECTED) && (retry_count < 20)) {
                delay(500);
                debug_out(".", DEBUG_MIN_INFO, 0);
                retry_count++;
            }
            debug_out("", DEBUG_MIN_INFO, 1);
        }

        // Resetting for next sampling
        last_data_string = data;
        sample_count = 0;
        last_micro = 0;
        min_micro = 1000000000;
        max_micro = 0;
        starttime = millis();                               // store the start time
        first_cycle = false;
        count_sends += 1;
    }
    writeConfig();
    yield();
}
