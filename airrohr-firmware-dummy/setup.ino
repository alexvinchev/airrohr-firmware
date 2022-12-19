/*****************************************************************
/* The Setup                                                     *
/*****************************************************************/
void setup() {
  Serial.begin(115200);         // Output to Serial at 115200 baud
#if defined(ESP8266)
  Wire.begin(D3, D4);
  esp_chipid = String(ESP.getChipId());
  WiFi.persistent(false);
#endif
  init_display();
  init_lcd1602();
  copyExtDef();
  display_debug(F("Reading config from SPIFFS"));
  readConfig();
  setup_webserver();
  display_debug("Connecting to " + String(wlanssid));
  connectWifi();            // Start ConnectWifi
  if (restart_needed) {
    display_debug(F("Writing config to SPIFFS and restarting sensor"));
    writeConfig();
    delay(500);
    ESP.restart();
  }
  autoUpdate();
  create_basic_auth_strings();
  serialSDS.begin(9600);
  serialGPS.begin(9600);
  ds18b20.begin();
  pinMode(PPD_PIN_PM1, INPUT_PULLUP); // Listen at the designated PIN
  pinMode(PPD_PIN_PM2, INPUT_PULLUP); // Listen at the designated PIN
  dht.begin();  // Start DHT
  htu21d.begin(); // Start HTU21D
  delay(10);
#if defined(ESP8266)
  debug_out(F("\nChipId: "), DEBUG_MIN_INFO, 0);
  debug_out(esp_chipid, DEBUG_MIN_INFO, 1);
#endif

  if (ppd_read) { debug_out(F("Lese PPD..."), DEBUG_MIN_INFO, 1); }
  if (sds_read) { debug_out(F("Lese SDS..."), DEBUG_MIN_INFO, 1); }
  if (pms24_read) { debug_out(F("Lese PMS3003..."), DEBUG_MIN_INFO, 1); }
  if (pms32_read) { debug_out(F("Lese 1003 or 7003..."), DEBUG_MIN_INFO, 1); }
  if (dht_read) { debug_out(F("Lese DHT..."), DEBUG_MIN_INFO, 1); }
  if (htu21d_read) { debug_out(F("Lese HTU21D..."), DEBUG_MIN_INFO, 1); }
  if (bmp_read) { debug_out(F("Lese BMP..."), DEBUG_MIN_INFO, 1); }
  if (bmp280_read) { debug_out(F("Lese BMP280..."), DEBUG_MIN_INFO, 1); }
  if (bme280_read) { debug_out(F("Lese BME280..."), DEBUG_MIN_INFO, 1); }
  if (ds18b20_read) { debug_out(F("Lese DS18B20..."), DEBUG_MIN_INFO, 1); }
  if (gps_read) { debug_out(F("Lese GPS..."), DEBUG_MIN_INFO, 1); }
  if (send2dusti) { debug_out(F("Sende an luftdaten.info..."), DEBUG_MIN_INFO, 1); }
  if (send2madavi) { debug_out(F("Sende an madavi.de..."), DEBUG_MIN_INFO, 1); }
  if (send2csv) { debug_out(F("Sende als CSV an Serial..."), DEBUG_MIN_INFO, 1); }
  if (send2custom) { debug_out(F("Sende an custom API..."), DEBUG_MIN_INFO, 1); }
  if (send2influx) { debug_out(F("Sende an custom influx DB..."), DEBUG_MIN_INFO, 1); }
  if (auto_update) { debug_out(F("Auto-Update wird ausgeführt..."), DEBUG_MIN_INFO, 1); }
  if (has_display) { debug_out(F("Zeige auf Display..."), DEBUG_MIN_INFO, 1); }
  if (has_lcd1602) { debug_out(F("Zeige auf LCD 1602..."), DEBUG_MIN_INFO, 1); }
  if (bmp_read) {
    if (!bmp.begin()) {
      debug_out(F("No valid BMP085 sensor, check wiring!"), DEBUG_MIN_INFO, 1);
      bmp_init_failed = 1;
    }
  }
  if (bmp280_read && !initBMP280(0x76) && !initBMP280(0x77)) {
    debug_out(F("Check BMP280 wiring"), DEBUG_MIN_INFO, 1);
    bmp280_init_failed = 1;
  }
  if (bme280_read && !initBME280(0x76) && !initBME280(0x77)) {
    debug_out(F("Check BME280 wiring"), DEBUG_MIN_INFO, 1);
    bme280_init_failed = 1;
  }
  if (sds_read) {
    debug_out(F("Stoppe SDS011..."), DEBUG_MIN_INFO, 1);
    stop_SDS();
  }
  if (pms24_read || pms32_read) {
    debug_out(F("Stoppe PMS..."), DEBUG_MIN_INFO, 1);
    stop_PMS();
  }
  data_first_part.replace("FEATHERCHIPID", "");

  if (MDNS.begin(server_name.c_str())) {
    MDNS.addService("http", "tcp", 80);
  }

  // sometimes parallel sending data and web page will stop nodemcu, watchdogtimer set to 30 seconds
  wdt_disable();
  wdt_enable(30000);// 30 sec

  starttime = millis();         // store the start time
  starttime_SDS = millis();
}

