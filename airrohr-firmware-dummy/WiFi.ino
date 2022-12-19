/*****************************************************************
/* WifiConfig                                                    *
/*****************************************************************/
void wifiConfig() {
#if defined(ESP8266)
  const char *softAP_password = "";
  const byte DNS_PORT = 53;
  int retry_count = 0;
  DNSServer dnsServer;
  IPAddress apIP(192, 168, 4, 1);
  IPAddress netMsk(255, 255, 255, 0);

  debug_out(F("Starting WiFiManager"), DEBUG_MIN_INFO, 1);
  debug_out(F("AP ID: Sensor-"), DEBUG_MIN_INFO, 0);
  debug_out(String(ESP.getChipId()), DEBUG_MIN_INFO, 1);

  wificonfig_loop = true;

  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(server_name.c_str(), "");

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  // 10 minutes timeout for wifi config
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


  debug_out(F("------ Result from Webconfig ------"), DEBUG_MIN_INFO, 1);
  debug_out(F("WLANSSID: "), DEBUG_MIN_INFO, 0); debug_out(wlanssid, DEBUG_MIN_INFO, 1);
  debug_out(F("DHT_read: "), DEBUG_MIN_INFO, 0); debug_out(String(dht_read), DEBUG_MIN_INFO, 1);
  debug_out(F("PPD_read: "), DEBUG_MIN_INFO, 0); debug_out(String(ppd_read), DEBUG_MIN_INFO, 1);
  debug_out(F("SDS_read: "), DEBUG_MIN_INFO, 0); debug_out(String(sds_read), DEBUG_MIN_INFO, 1);
  debug_out(F("BMP_read: "), DEBUG_MIN_INFO, 0); debug_out(String(bmp_read), DEBUG_MIN_INFO, 1);
  debug_out(F("DS18B20_read: "), DEBUG_MIN_INFO, 0); debug_out(String(ds18b20_read), DEBUG_MIN_INFO, 1);
  debug_out(F("Dusti: "), DEBUG_MIN_INFO, 0); debug_out(String(send2dusti), DEBUG_MIN_INFO, 1);
  debug_out(F("Madavi: "), DEBUG_MIN_INFO, 0); debug_out(String(send2madavi), DEBUG_MIN_INFO, 1);
  debug_out(F("CSV: "), DEBUG_MIN_INFO, 0); debug_out(String(send2csv), DEBUG_MIN_INFO, 1);
  debug_out(F("Autoupdate: "), DEBUG_MIN_INFO, 0); debug_out(String(auto_update), DEBUG_MIN_INFO, 1);
  debug_out(F("Display: "), DEBUG_MIN_INFO, 0); debug_out(String(has_display), DEBUG_MIN_INFO, 1);
  debug_out(F("LCD 1602: "), DEBUG_MIN_INFO, 0); debug_out(String(has_lcd1602), DEBUG_MIN_INFO, 1);
  debug_out(F("Debug: "), DEBUG_MIN_INFO, 0); debug_out(String(debug), DEBUG_MIN_INFO, 1);
  debug_out(F("------"), DEBUG_MIN_INFO, 1);
  debug_out(F("Restart needed ..."), DEBUG_MIN_INFO, 1);
  wificonfig_loop = false;
  restart_needed = true;
#endif
}

/*****************************************************************
/* WiFi auto connecting script                                   *
/*****************************************************************/
void connectWifi() {
#if defined(ESP8266)
  int retry_count = 0;
  debug_out(String(WiFi.status()), DEBUG_MIN_INFO, 1);
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
    display_debug("AP ID: Feinstaubsensor-" + esp_chipid + " - IP: 192.168.4.1");
    wifiConfig();
    if (WiFi.status() != WL_CONNECTED) {
      retry_count = 0;
      while ((WiFi.status() != WL_CONNECTED) && (retry_count < 20)) {
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
#endif
}

