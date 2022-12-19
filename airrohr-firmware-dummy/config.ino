/*****************************************************************
/* read config from spiffs                                       *
/*****************************************************************/
void readConfig() {
#if defined(ESP8266)
  String json_string = "";
  debug_out(F("mounting FS..."), DEBUG_MIN_INFO, 1);

  if (SPIFFS.begin()) {
    debug_out(F("mounted file system..."), DEBUG_MIN_INFO, 1);
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      debug_out(F("reading config file..."), DEBUG_MIN_INFO, 1);
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        debug_out(F("opened config file..."), DEBUG_MIN_INFO, 1);
        size_t size = configFile.size();
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
          debug_out(F("parsed json..."), DEBUG_MIN_INFO, 1);
          if (json.containsKey("SOFTWARE_VERSION")) { strcpy(version_from_local_config, json["SOFTWARE_VERSION"]); }

#define setFromJSON(key)    if (json.containsKey(#key)) key = json[#key];
#define strcpyFromJSON(key) if (json.containsKey(#key)) strcpy(key, json[#key]);
          strcpyFromJSON(current_lang);
          strcpyFromJSON(wlanssid);
          strcpyFromJSON(wlanpwd);
          strcpyFromJSON(www_username);
          strcpyFromJSON(www_password);
          setFromJSON(www_basicauth_enabled);
          setFromJSON(dht_read);
          setFromJSON(htu21d_read);
          setFromJSON(ppd_read);
          setFromJSON(sds_read);
          setFromJSON(pms24_read);
          setFromJSON(pms32_read);
          setFromJSON(bmp_read);
          setFromJSON(bmp280_read);
          setFromJSON(bme280_read);
          setFromJSON(ds18b20_read);
          setFromJSON(gps_read);
          setFromJSON(send2dusti);
          setFromJSON(send2madavi);
          setFromJSON(send2sensemap);
          setFromJSON(send2csv);
          setFromJSON(auto_update);
          setFromJSON(has_display);
          setFromJSON(has_lcd1602);
          setFromJSON(has_lcd1602_27);
          setFromJSON(debug);
          setFromJSON(sending_intervall_ms);
          setFromJSON(time_for_wifi_config);
          strcpyFromJSON(senseboxid);
          setFromJSON(send2custom);
          strcpyFromJSON(host_custom);
          strcpyFromJSON(url_custom);
          setFromJSON(port_custom);
          strcpyFromJSON(user_custom);
          strcpyFromJSON(pwd_custom);
          setFromJSON(send2influx);
          strcpyFromJSON(host_influx);
          strcpyFromJSON(url_influx);
          setFromJSON(port_influx);
          strcpyFromJSON(user_influx);
          strcpyFromJSON(pwd_influx);
#undef setFromJSON
#undef strcpyFromJSON
        } else {
          debug_out(F("failed to load json config"), DEBUG_ERROR, 1);
        }
      }
    } else {
      debug_out(F("config file not found ..."), DEBUG_ERROR, 1);
    }
  } else {
    debug_out(F("failed to mount FS"), DEBUG_ERROR, 1);
  }
#endif
}

/*****************************************************************
/* write config to spiffs                                        *
/*****************************************************************/
void writeConfig() {
#if defined(ESP8266)
  String json_string = "";
  debug_out(F("saving config..."), DEBUG_MIN_INFO, 1);

  json_string = "{";
#define copyToJSON_Bool(varname) json_string +="\""+String(#varname)+"\":"+(varname ? "true":"false")+",";
#define copyToJSON_Int(varname) json_string +="\""+String(#varname)+"\":"+String(varname)+",";
#define copyToJSON_String(varname) json_string +="\""+String(#varname)+"\":\""+String(varname)+"\",";
  copyToJSON_String(current_lang);
  copyToJSON_String(SOFTWARE_VERSION);
  copyToJSON_String(wlanssid);
  copyToJSON_String(wlanpwd);
  copyToJSON_String(www_username);
  copyToJSON_String(www_password);
  copyToJSON_Bool(www_basicauth_enabled);
  copyToJSON_Bool(dht_read);
  copyToJSON_Bool(htu21d_read);
  copyToJSON_Bool(ppd_read);
  copyToJSON_Bool(sds_read);
  copyToJSON_Bool(pms24_read);
  copyToJSON_Bool(pms32_read);
  copyToJSON_Bool(bmp_read);
  copyToJSON_Bool(bmp280_read);
  copyToJSON_Bool(bme280_read);
  copyToJSON_Bool(ds18b20_read);
  copyToJSON_Bool(gps_read);
  copyToJSON_Bool(send2dusti);
  copyToJSON_Bool(send2madavi);
  copyToJSON_Bool(send2sensemap);
  copyToJSON_Bool(send2csv);
  copyToJSON_Bool(auto_update);
  copyToJSON_Bool(has_display);
  copyToJSON_Bool(has_lcd1602);
  copyToJSON_Bool(has_lcd1602_27);
  copyToJSON_String(debug);
  copyToJSON_String(sending_intervall_ms);
  copyToJSON_String(time_for_wifi_config);
  copyToJSON_String(senseboxid);
  copyToJSON_Bool(send2custom);
  copyToJSON_String(host_custom);
  copyToJSON_String(url_custom);
  copyToJSON_Int(port_custom);
  copyToJSON_String(user_custom);
  copyToJSON_String(pwd_custom);

  copyToJSON_Bool(send2influx);
  copyToJSON_String(host_influx);
  copyToJSON_String(url_influx);
  copyToJSON_Int(port_influx);
  copyToJSON_String(user_influx);
  copyToJSON_String(pwd_influx);
#undef copyToJSON_Bool
#undef copyToJSON_Int
#undef copyToJSON_String

  json_string.remove(json_string.length() - 1);
  json_string += "}";

  debug_out(json_string, DEBUG_MIN_INFO, 1);
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    debug_out(F("failed to open config file for writing"), DEBUG_ERROR, 1);
  }

  configFile.print(json_string);
  debug_out(F("Config written: "), DEBUG_MIN_INFO, 0);
  debug_out(json_string, DEBUG_MIN_INFO, 1);
  configFile.close();
  config_needs_write = false;
  //end save
#endif
}

