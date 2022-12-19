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

  debug_out(F("Starte Webserver... "), DEBUG_MIN_INFO, 0);
  debug_out(IPAddress2String(WiFi.localIP()), DEBUG_MIN_INFO, 1);
  server.begin();
}

/*****************************************************************
/* Webserver root: show all options                              *
/*****************************************************************/
void webserver_root() {
  if (WiFi.status() != WL_CONNECTED) {
    server.sendHeader(F("Location"), F("http://192.168.4.1/config"));
    server.send(302, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), "");
  } else {
    webserver_request_auth();

    String page_content = "";
    last_page_load = millis();
    debug_out(F("output root page..."), DEBUG_MIN_INFO, 1);
    page_content = make_header(" ");
    page_content += FPSTR(WEB_ROOT_PAGE_CONTENT);
    page_content.replace("{t}", FPSTR(INTL_AKTUELLE_WERTE));
    page_content.replace(F("{karte}"), FPSTR(INTL_KARTE_DER_AKTIVEN_SENSOREN));
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

  debug_out(F("output config page ..."), DEBUG_MIN_INFO, 1);
  page_content += make_header(FPSTR(INTL_KONFIGURATION));
  if (WiFi.status() != WL_CONNECTED) {  // scan for wlan ssids
    page_content += FPSTR(WEB_CONFIG_SCRIPT);
  }
  if (server.method() == HTTP_GET) {
    page_content += F("<form method='POST' action='/config' style='width: 100%;'><b>");
    page_content += FPSTR(INTL_WLAN_DATEN);
    page_content += F("</b><br/>");
    debug_out(F("output config page 1"), DEBUG_MIN_INFO, 1);
    if (WiFi.status() != WL_CONNECTED) {  // scan for wlan ssids
      page_content += F("<div id='wifilist'>");
      page_content += FPSTR(INTL_WLAN_LISTE);
      page_content += F("</div><br/>");
    }
    page_content += F("<table>");
    page_content += form_input(F("wlanssid"), F("WLAN"), wlanssid, 64);
    page_content += form_password(F("wlanpwd"), FPSTR(INTL_PASSWORT), wlanpwd, 64);
    page_content += form_submit(FPSTR(INTL_SPEICHERN));
    page_content += F("</table><br/><hr/><b>");

    page_content += FPSTR(INTL_AB_HIER_NUR_ANDERN);
    page_content += F("</b><br/><br/><b>");
    page_content += FPSTR(INTL_BASICAUTH);
    page_content += F("</b><br/>");
    page_content += F("<table>");
    page_content += form_input("www_username", FPSTR(INTL_BENUTZER), www_username, 64);
    page_content += form_password("www_password", FPSTR(INTL_PASSWORT), www_password, 64);
    page_content += form_checkbox("www_basicauth_enabled", FPSTR(INTL_BASICAUTH), www_basicauth_enabled);
    page_content += form_submit(FPSTR(INTL_SPEICHERN));

    page_content += F("</table><br/><b>APIs</b><br/>");
    page_content += form_checkbox("send2dusti", F("API Luftdaten.info"), send2dusti);
    page_content += form_checkbox("send2madavi", F("API Madavi.de"), send2madavi);
    page_content += F("<br/><b>");
    page_content += FPSTR(INTL_SENSOREN);
    page_content += F("</b><br/>");
    page_content += form_checkbox("sds_read", FPSTR(INTL_SDS011), sds_read);
    page_content += form_checkbox("pms32_read", FPSTR(INTL_PMS32), pms32_read);
    page_content += form_checkbox("pms24_read", FPSTR(INTL_PMS24), pms24_read);
    page_content += form_checkbox("dht_read", FPSTR(INTL_DHT22), dht_read);
    page_content += form_checkbox("htu21d_read", FPSTR(INTL_HTU21D), htu21d_read);
    page_content += form_checkbox("ppd_read", FPSTR(INTL_PPD42NS), ppd_read);
    page_content += form_checkbox("bmp_read", FPSTR(INTL_BMP180), bmp_read);
    page_content += form_checkbox("bmp280_read", FPSTR(INTL_BMP280), bmp280_read);
    page_content += form_checkbox("bme280_read", FPSTR(INTL_BME280), bme280_read);
    page_content += form_checkbox("ds18b20_read", FPSTR(INTL_DS18B20), ds18b20_read);
    page_content += form_checkbox("gps_read", FPSTR(INTL_NEO6M), gps_read);
    page_content += F("<br/><b>"); page_content += FPSTR(INTL_WEITERE_EINSTELLUNGEN); page_content += F("</b><br/>");
    page_content += form_checkbox("auto_update", FPSTR(INTL_AUTO_UPDATE), auto_update);
    page_content += form_checkbox("has_display", FPSTR(INTL_DISPLAY), has_display);
    page_content += form_checkbox("has_lcd1602_27", FPSTR(INTL_LCD1602_27), has_lcd1602_27);
    page_content += form_checkbox("has_lcd1602", FPSTR(INTL_LCD1602_3F), has_lcd1602);
    page_content += F("<table>");
    page_content += form_select_lang();
    page_content += form_input("debug", FPSTR(INTL_DEBUG_LEVEL), String(debug), 5);
    page_content += form_input("sending_intervall_ms", FPSTR(INTL_MESSINTERVALL), String(sending_intervall_ms / 1000), 5);
    page_content += form_input("time_for_wifi_config", FPSTR(INTL_DAUER_ROUTERMODUS), String(time_for_wifi_config / 1000), 5);
    page_content += F("</table><br/><b>"); page_content += FPSTR(INTL_WEITERE_APIS); page_content += F("</b><br/><br/>");
    page_content += form_checkbox("send2sensemap", tmpl(FPSTR(INTL_SENDEN_AN), F("OpenSenseMap")), send2sensemap);
    page_content += F("<table>");
    page_content += form_input("senseboxid", "senseBox-ID: ", senseboxid, 50);
    page_content += F("</table><br/>");
    page_content += form_checkbox("send2custom", FPSTR(INTL_AN_EIGENE_API_SENDEN), send2custom);
    page_content += F("<table>");
    page_content += form_input("host_custom", FPSTR(INTL_SERVER), host_custom, 50);
    page_content += form_input("url_custom", FPSTR(INTL_PFAD), url_custom, 50);
    page_content += form_input("port_custom", FPSTR(INTL_PORT), String(port_custom), 30);
    page_content += form_input("user_custom", FPSTR(INTL_BENUTZER), user_custom, 50);
    page_content += form_password("pwd_custom", FPSTR(INTL_PASSWORT), pwd_custom, 50);
    page_content += F("</table><br/>");
    page_content += form_checkbox(F("send2influx"), tmpl(FPSTR(INTL_SENDEN_AN), F("InfluxDB")), send2influx);
    page_content += F("<table>");
    page_content += form_input("host_influx", FPSTR(INTL_SERVER), host_influx, 50);
    page_content += form_input("url_influx", FPSTR(INTL_PFAD), url_influx, 50);
    page_content += form_input("port_influx", FPSTR(INTL_PORT), String(port_influx), 30);
    page_content += form_input("user_influx", FPSTR(INTL_BENUTZER), user_influx, 50);
    page_content += form_password("pwd_influx", FPSTR(INTL_PASSWORT), pwd_influx, 50);
    page_content += form_submit(FPSTR(INTL_SPEICHERN));
    page_content += F("</table><br/>");
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
    readCharParam(current_lang);
    readCharParam(www_username);
    readPasswdParam(www_password);
    readBoolParam(www_basicauth_enabled);
    readBoolParam(send2dusti);
    readBoolParam(send2madavi);
    readBoolParam(send2sensemap);
    readBoolParam(dht_read);
    readBoolParam(htu21d_read);
    readBoolParam(sds_read);
    readBoolParam(pms24_read);
    readBoolParam(pms32_read);
    readBoolParam(ppd_read);
    readBoolParam(bmp_read);
    readBoolParam(bmp280_read);
    readBoolParam(bme280_read);
    readBoolParam(ds18b20_read);
    readBoolParam(gps_read);
    readBoolParam(auto_update);
    readBoolParam(has_display);
    readBoolParam(has_lcd1602);
    readBoolParam(has_lcd1602_27);
    readIntParam(debug);
    readTimeParam(sending_intervall_ms);
    readTimeParam(time_for_wifi_config);

    readCharParam(senseboxid);

    readBoolParam(send2custom);
    readCharParam(host_custom);
    readCharParam(url_custom);
    readIntParam(port_custom);
    readCharParam(user_custom);
    readPasswdParam(pwd_custom);

    readBoolParam(send2influx);
    readCharParam(host_influx);
    readCharParam(url_influx);
    readIntParam(port_influx);
    readCharParam(user_influx);
    readPasswdParam(pwd_influx);

#undef readCharParam
#undef readBoolParam
#undef readIntParam

    config_needs_write = true;

    page_content += line_from_value(tmpl(FPSTR(INTL_SENDEN_AN), F("Luftdaten.info")), String(send2dusti));
    page_content += line_from_value(tmpl(FPSTR(INTL_SENDEN_AN), F("Madavi")), String(send2madavi));
    page_content += line_from_value(tmpl(FPSTR(INTL_LESE), "DHT"), String(dht_read));
    page_content += line_from_value(tmpl(FPSTR(INTL_LESE), "HTU21D"), String(htu21d_read));
    page_content += line_from_value(tmpl(FPSTR(INTL_LESE), "SDS"), String(sds_read));
    page_content += line_from_value(tmpl(FPSTR(INTL_LESE), "PMS1003, PMS5003, PMS6003, PMS7003"), String(pms32_read));
    page_content += line_from_value(tmpl(FPSTR(INTL_LESE), "PMS3003"), String(pms24_read));
    page_content += line_from_value(tmpl(FPSTR(INTL_LESE), "PPD"), String(ppd_read));
    page_content += line_from_value(tmpl(FPSTR(INTL_LESE), "BMP180"), String(bmp_read));
    page_content += line_from_value(tmpl(FPSTR(INTL_LESE), "BMP280"), String(bmp280_read));
    page_content += line_from_value(tmpl(FPSTR(INTL_LESE), "BME280"), String(bme280_read));
    page_content += line_from_value(tmpl(FPSTR(INTL_LESE), "DS18B20"), String(ds18b20_read));
    page_content += line_from_value(tmpl(FPSTR(INTL_LESE), "GPS"), String(gps_read));
    page_content += line_from_value(FPSTR(INTL_AUTO_UPDATE), String(auto_update));
    page_content += line_from_value(FPSTR(INTL_DISPLAY), String(has_display));
    page_content += line_from_value(FPSTR(INTL_LCD1602_27), String(has_lcd1602_27));
    page_content += line_from_value(FPSTR(INTL_LCD1602_3F), String(has_lcd1602));
    page_content += line_from_value(FPSTR(INTL_DEBUG_LEVEL), String(debug));
    page_content += line_from_value(FPSTR(INTL_MESSINTERVALL), String(sending_intervall_ms));
    page_content += line_from_value(tmpl(FPSTR(INTL_SENDEN_AN), "opensensemap"), String(send2sensemap));
    page_content += F("<br/>senseBox-ID "); page_content += senseboxid;
    page_content += F("<br/><br/>Eigene API: "); page_content += String(send2custom);
    page_content += line_from_value(FPSTR(INTL_SERVER), host_custom);
    page_content += line_from_value(FPSTR(INTL_PFAD), url_custom);
    page_content += line_from_value(FPSTR(INTL_PORT), String(port_custom));
    page_content += line_from_value(FPSTR(INTL_BENUTZER), user_custom);
    page_content += line_from_value(FPSTR(INTL_PASSWORT), pwd_custom);
    page_content += F("<br/><br/>InfluxDB: "); page_content += String(send2influx);
    page_content += line_from_value(FPSTR(INTL_SERVER), host_influx);
    page_content += line_from_value(FPSTR(INTL_PFAD), url_influx);
    page_content += line_from_value(FPSTR(INTL_PORT), String(port_influx));
    page_content += line_from_value(FPSTR(INTL_BENUTZER), user_influx);
    page_content += line_from_value(FPSTR(INTL_PASSWORT), pwd_influx);
    if (wificonfig_loop) {
      page_content += F("<br/><br/>"); page_content += FPSTR(INTL_GERAT_WIRD_NEU_GESTARTET);
    } else {
      page_content += F("<br/><br/><a href='/reset?confirm=yes'>"); page_content += FPSTR(INTL_GERAT_NEU_STARTEN); page_content += F("?</a>");
    }
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
    for (int i = 0; i < n; i++) { indices[i] = i; }
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
      if (indices[i] == -1) { continue; }
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
      if (indices[i] == -1) { continue; }
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
#if defined(ESP8266)
  if (WiFi.status() != WL_CONNECTED) {
    server.sendHeader(F("Location"), F("http://192.168.4.1/config"));
    server.send(302, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), "");
  } else {
    String page_content = "";
    String empty_row = F("<tr><td colspan='3'>&nbsp;</td></tr>");
    last_page_load = millis();
    long signal_strength = WiFi.RSSI();
    if (signal_strength > -50) {signal_strength = -50; }
    if (signal_strength < -100) {signal_strength = -100; }
    int signal_quality = (signal_strength + 100) * 2;
    debug_out(F("output values to web page..."), DEBUG_MIN_INFO, 1);
    page_content += make_header(FPSTR(INTL_AKTUELLE_WERTE));
    if (first_cycle) {
      page_content += F("<b style='color: red'>");
      page_content += warning_first_cycle();
      page_content += F("</b><br/><br/>");
    } else {
      page_content += age_last_values();
    }
    page_content += F("<table cellspacing='0' border='1' cellpadding='5'>");
    page_content += tmpl(F("<tr><th>{v1}</th><th>{v2}</th><th>{v3}</th>"), FPSTR(INTL_SENSOR), FPSTR(INTL_PARAMETER), FPSTR(INTL_WERT));
    if (ppd_read) {
      page_content += empty_row;
      page_content += table_row_from_value("PPD42NS", "PM1",  last_value_PPD_P1, FPSTR(INTL_PARTIKEL_LITER));
      page_content += table_row_from_value("PPD42NS", "PM2.5", last_value_PPD_P2, FPSTR(INTL_PARTIKEL_LITER));
    }
    if (sds_read) {
      page_content += empty_row;
      page_content += table_row_from_value("SDS011", "PM2.5", last_value_SDS_P2, "µg/m³");
      page_content += table_row_from_value("SDS011", "PM10", last_value_SDS_P1, "µg/m³");
    }
    if (pms24_read || pms32_read) {
      page_content += empty_row;
      page_content += table_row_from_value("PMS", "PM1", last_value_PMS_P0, "µg/m³");
      page_content += table_row_from_value("PMS", "PM2.5", last_value_PMS_P2, "µg/m³");
      page_content += table_row_from_value("PMS", "PM10", last_value_PMS_P1, "µg/m³");
    }
    if (dht_read) {
      page_content += empty_row;
      page_content += table_row_from_value("DHT22", FPSTR(INTL_TEMPERATUR), last_value_DHT_T, "°C");
      page_content += table_row_from_value("DHT22", FPSTR(INTL_LUFTFEUCHTE), last_value_DHT_H, "%");
    }
    if (htu21d_read) {
      page_content += empty_row;
      page_content += table_row_from_value("HTU21D", FPSTR(INTL_TEMPERATUR), last_value_HTU21D_T, "°C");
      page_content += table_row_from_value("HTU21D", FPSTR(INTL_LUFTFEUCHTE), last_value_HTU21D_H, "%");
    }
    if (bmp_read) {
      page_content += empty_row;
      page_content += table_row_from_value("BMP180", FPSTR(INTL_TEMPERATUR), last_value_BMP_T, "°C");
      page_content += table_row_from_value("BMP180", FPSTR(INTL_LUFTDRUCK), Float2String(last_value_BMP_P.toFloat() / 100.0), "hPa");
    }
    if (bmp280_read) {
      page_content += empty_row;
      page_content += table_row_from_value("BMP280", FPSTR(INTL_TEMPERATUR), last_value_BMP280_T, "°C");
      page_content += table_row_from_value("BMP280", FPSTR(INTL_LUFTDRUCK), Float2String(last_value_BMP280_P.toFloat() / 100.0), "hPa");
    }
    if (bme280_read) {
      page_content += empty_row;
      page_content += table_row_from_value("BME280", FPSTR(INTL_TEMPERATUR), last_value_BME280_T, "°C");
      page_content += table_row_from_value("BME280", FPSTR(INTL_LUFTFEUCHTE), last_value_BME280_H, "%");
      page_content += table_row_from_value("BME280", FPSTR(INTL_LUFTDRUCK),  Float2String(last_value_BME280_P.toFloat() / 100.0), "hPa");
    }
    if (ds18b20_read) {
      page_content += empty_row;
      page_content += table_row_from_value("DS18B20", FPSTR(INTL_TEMPERATUR), last_value_DS18B20_T, "°C");
    }

    page_content += empty_row;
    page_content += table_row_from_value("WiFi", FPSTR(INTL_SIGNAL),  String(signal_strength), "dBm");
    page_content += table_row_from_value("WiFi", FPSTR(INTL_QUALITAT), String(signal_quality), "%");
    page_content += F("</table>");
    page_content += make_footer();
    server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
  }
#endif
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

  if (server.hasArg("level")) {
    switch (server.arg("level").toInt()) {
    case (0): debug = 0; page_content += tmpl(message_string, FPSTR(INTL_SETZE_DEBUG_AUF), FPSTR(INTL_NONE)); break;
    case (1): debug = 1; page_content += tmpl(message_string, FPSTR(INTL_SETZE_DEBUG_AUF), FPSTR(INTL_ERROR)); break;
    case (2): debug = 2; page_content += tmpl(message_string, FPSTR(INTL_SETZE_DEBUG_AUF), FPSTR(INTL_WARNING)); break;
    case (3): debug = 3; page_content += tmpl(message_string, FPSTR(INTL_SETZE_DEBUG_AUF), FPSTR(INTL_MIN_INFO)); break;
    case (4): debug = 4; page_content += tmpl(message_string, FPSTR(INTL_SETZE_DEBUG_AUF), FPSTR(INTL_MED_INFO)); break;
    case (5): debug = 5; page_content += tmpl(message_string, FPSTR(INTL_SETZE_DEBUG_AUF), FPSTR(INTL_MAX_INFO)); break;
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
#if defined(ESP8266)
    if (SPIFFS.exists("/config.json")) {  //file exists
      debug_out(F("removing config.json..."), DEBUG_MIN_INFO, 1);
      if (SPIFFS.remove("/config.json")) {
        page_content += tmpl(message_string, FPSTR(INTL_CONFIG_GELOSCHT));
      } else {
        page_content += tmpl(message_string, FPSTR(INTL_CONFIG_KONNTE_NICHT_GELOSCHT_WERDEN));
      }
    } else {
      page_content += tmpl(message_string, FPSTR(INTL_CONFIG_NICHT_GEFUNDEN));
    }
#endif
  }
  page_content += make_footer();
  server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
}

/*****************************************************************
/* Webserver reset NodeMCU                                       *
/*****************************************************************/
void webserver_reset() {
  webserver_request_auth();

  String page_content = "";
  last_page_load = millis();
  debug_out(F("output reset NodeMCU page..."), DEBUG_MIN_INFO, 1);
  page_content += make_header(FPSTR(INTL_SENSOR_NEU_STARTEN));

  if (server.method() == HTTP_GET) {
    page_content += FPSTR(WEB_RESET_CONTENT);
    page_content.replace("{t}", FPSTR(INTL_SENSOR_WIRKLICH_NEU_STARTEN));
    page_content.replace("{b}", FPSTR(INTL_NEU_STARTEN));
    page_content.replace("{c}", FPSTR(INTL_ABBRECHEN));
  } else {
#if defined(ESP8266)
    ESP.restart();
#endif
  }
  page_content += make_footer();
  server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
}

/*****************************************************************
/* Webserver data.json                                           *
/*****************************************************************/
void webserver_data_json() {
  debug_out(F("output data json..."), DEBUG_MIN_INFO, 1);
  String s1 = last_data_string;
  debug_out(F("last data: "), DEBUG_MIN_INFO, 0);
  debug_out(s1, DEBUG_MIN_INFO, 1);
  String s2 = ", \"age\":\"" + String((long)((act_milli - starttime + 500) / 1000)) + "\", \"sensordatavalues\"";
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
  if (server.hasArg("name")) {
    if (server.arg("name") == "luftdaten_logo") {
      debug_out(F("output luftdaten.info logo..."), DEBUG_MIN_INFO, 1);
      server.send(200, FPSTR(TXT_CONTENT_TYPE_IMAGE_SVG), FPSTR(LUFTDATEN_INFO_LOGO_SVG));
    } else if (server.arg("name") == "cfg_logo") {
      debug_out(F("output codefor.de logo..."), DEBUG_MIN_INFO, 1);
      server.send(200, FPSTR(TXT_CONTENT_TYPE_IMAGE_SVG), FPSTR(CFG_LOGO_SVG));
    } else {
      webserver_not_found();
    }
  } else {
    webserver_not_found();
  }
//    server.client().setNoDelay(1);
//    server.sendHeader(F("Content-Encoding"),"gzip");
//    server.send_P(200, TXT_CONTENT_TYPE_IMAGE_SVG, CFG_LOGO_SVG_GZIP, CFG_LOGO_SVG_GZIP_LEN);
//    server.client().setNoDelay(0);
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
/* Webserver request auth: prompt for BasicAuth
 *
 * -Provide BasicAuth for all page contexts except /values and images
/*****************************************************************/
void webserver_request_auth() {
  debug_out(F("validate request auth..."), DEBUG_MIN_INFO, 1);
  if (www_basicauth_enabled) {
    if (!server.authenticate(www_username, www_password))
    { return server.requestAuthentication(); }
  }
}

