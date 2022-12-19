/*****************************************************************
/* And action                                                    *
/*****************************************************************/
void loop() {
  String data = "";
  String tmp_str;
  String data_4_dusti = "";
  String data_4_influxdb = "";
  String data_4_custom = "";
  String data_sample_times = "";

  String sensemap_path = "";

  String result_PPD = "";
  String result_SDS = "";
  String result_PMS = "";
  String result_DHT = "";
  String result_HTU21D = "";
  String result_BMP = "";
  String result_BMP280 = "";
  String result_BME280 = "";
  String result_DS18B20 = "";
  String result_GPS = "";
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
    if (max_micro < diff_micro) { max_micro = diff_micro;}
    if (min_micro > diff_micro) { min_micro = diff_micro;}
    last_micro = act_micro;
  } else {
    last_micro = act_micro;
  }

  if (ppd_read) {
    debug_out(F("Call sensorPPD"), DEBUG_MAX_INFO, 1);
    result_PPD = sensorPPD42NS();
  }


  if (((act_milli - starttime_SDS) > sampletime_SDS_ms) || ((act_milli - starttime) > sending_intervall_ms)) {
    if (sds_read) {
      debug_out(F("Call sensorSDS"), DEBUG_MAX_INFO, 1);
      result_SDS = sensorSDS();
      starttime_SDS = act_milli;
    }

    if (pms24_read) {
      debug_out(F("Call sensorPMS(24)"), DEBUG_MAX_INFO, 1);
      result_PMS = sensorPMS(24);
      starttime_SDS = act_milli;
    }

    if (pms32_read) {
      debug_out(F("Call sensorPMS(32)"), DEBUG_MAX_INFO, 1);
      result_PMS = sensorPMS(32);
      starttime_SDS = act_milli;
    }
  }

  server.handleClient();

  if (send_now) {
    if (dht_read) {
      debug_out(F("Call sensorDHT"), DEBUG_MAX_INFO, 1);
      result_DHT = sensorDHT22();     // getting temperature and humidity (optional)
    }

    if (htu21d_read) {
      debug_out(F("Call sensorHTU21D"), DEBUG_MAX_INFO, 1);
      result_HTU21D = sensorHTU21D();     // getting temperature and humidity (optional)
    }

    if (bmp_read && (! bmp_init_failed)) {
      debug_out(F("Call sensorBMP"), DEBUG_MAX_INFO, 1);
      result_BMP = sensorBMP180();     // getting temperature and pressure (optional)
    }

    if (bmp280_read && (! bmp280_init_failed)) {
      debug_out(F("Call sensorBMP280"), DEBUG_MAX_INFO, 1);
      result_BMP280 = sensorBMP280();     // getting temperature, humidity and pressure (optional)
    }

    if (bme280_read && (! bme280_init_failed)) {
      debug_out(F("Call sensorBME280"), DEBUG_MAX_INFO, 1);
      result_BME280 = sensorBME280();     // getting temperature, humidity and pressure (optional)
    }

    if (ds18b20_read) {
      debug_out(F("Call sensorDS18B20"), DEBUG_MAX_INFO, 1);
      result_DS18B20 = sensorDS18B20();     // getting temperature (optional)
    }
  }

  if (gps_read && (((act_milli - starttime_GPS) > sampletime_GPS_ms) || ((act_milli - starttime) > sending_intervall_ms))) {
    debug_out(F("Call sensorGPS"), DEBUG_MAX_INFO, 1);
    result_GPS = sensorGPS();     // getting GPS coordinates
    starttime_GPS = act_milli;
  }

  if (send_now) {
    if (WiFi.psk() != "") {
      httpPort_madavi = 80;
      httpPort_dusti = 80;
    }
    debug_out(F("Creating data string:"), DEBUG_MIN_INFO, 1);
    data = data_first_part;
    data_sample_times  = Value2Json("samples", String(long(sample_count)));
    data_sample_times += Value2Json("min_micro", String(long(min_micro)));
    data_sample_times += Value2Json("max_micro", String(long(max_micro)));

    signal_strength = String(WiFi.RSSI());
    debug_out(F("WLAN signal strength: "), DEBUG_MIN_INFO, 0);
    debug_out(signal_strength, DEBUG_MIN_INFO, 0);
    debug_out(F(" dBm"), DEBUG_MIN_INFO, 1);
    debug_out(F("------"), DEBUG_MIN_INFO, 1);

    server.handleClient();
    yield();
    server.stop();
    if (ppd_read) {
      data += result_PPD;
      if (send2dusti) {
        debug_out(F("## Sending to luftdaten.info (PPD42NS): "), DEBUG_MIN_INFO, 1);
        start_send = micros();
        sendLuftdaten(result_PPD, PPD_API_PIN, host_dusti, httpPort_dusti, url_dusti, "PPD_");
        sum_send_time += micros() - start_send;
      }
    }
    if (sds_read) {
      data += result_SDS;
      if (send2dusti) {
        debug_out(F("## Sending to luftdaten.info (SDS): "), DEBUG_MIN_INFO, 1);
        start_send = micros();
        sendLuftdaten(result_SDS, SDS_API_PIN, host_dusti, httpPort_dusti, url_dusti, "SDS_");
        sum_send_time += micros() - start_send;
      }
    }
    if (pms24_read || pms32_read) {
      data += result_PMS;
      if (send2dusti) {
        debug_out(F("## Sending to luftdaten.info (PMS): "), DEBUG_MIN_INFO, 1);
        start_send = micros();
        sendLuftdaten(result_PMS, PMS_API_PIN, host_dusti, httpPort_dusti, url_dusti, "PMS_");
        sum_send_time += micros() - start_send;
      }
    }
    if (dht_read) {
      data += result_DHT;
      if (send2dusti) {
        debug_out(F("## Sending to luftdaten.info (DHT): "), DEBUG_MIN_INFO, 1);
        start_send = micros();
        sendLuftdaten(result_DHT, DHT_API_PIN, host_dusti, httpPort_dusti, url_dusti, "DHT_");
        sum_send_time += micros() - start_send;
      }
    }
    if (htu21d_read) {
      data += result_HTU21D;
      if (send2dusti) {
        debug_out(F("## Sending to luftdaten.info (HTU21D): "), DEBUG_MIN_INFO, 1);
        start_send = micros();
        sendLuftdaten(result_HTU21D, HTU21D_API_PIN, host_dusti, httpPort_dusti, url_dusti, "HTU_");
        sum_send_time += micros() - start_send;
      }
    }
    if (bmp_read && (! bmp_init_failed)) {
      data += result_BMP;
      if (send2dusti) {
        debug_out(F("## Sending to luftdaten.info (BMP): "), DEBUG_MIN_INFO, 1);
        start_send = micros();
        sendLuftdaten(result_BMP, BMP_API_PIN, host_dusti, httpPort_dusti, url_dusti, "BMP_");
        sum_send_time += micros() - start_send;
      }
    }
    if (bmp280_read && (! bmp280_init_failed)) {
      data += result_BMP280;
      if (send2dusti) {
        debug_out(F("## Sending to luftdaten.info (BMP280): "), DEBUG_MIN_INFO, 1);
        start_send = micros();
        sendLuftdaten(result_BMP280, BMP280_API_PIN, host_dusti, httpPort_dusti, url_dusti, "BMP280_");
        sum_send_time += micros() - start_send;
      }
    }
    if (bme280_read && (! bme280_init_failed)) {
      data += result_BME280;
      if (send2dusti) {
        debug_out(F("## Sending to luftdaten.info (BME280): "), DEBUG_MIN_INFO, 1);
        start_send = micros();
        sendLuftdaten(result_BME280, BME280_API_PIN, host_dusti, httpPort_dusti, url_dusti, "BME280_");
        sum_send_time += micros() - start_send;
      }
    }

    if (ds18b20_read) {
      data += result_DS18B20;
      if (send2dusti) {
        debug_out(F("## Sending to luftdaten.info (DS18B20): "), DEBUG_MIN_INFO, 1);
        start_send = micros();
        sendLuftdaten(result_DS18B20, BME280_API_PIN, host_dusti, httpPort_dusti, url_dusti, "DS18B20_");
        sum_send_time += micros() - start_send;
      }
    }

    if (gps_read) {
      data += result_GPS;
      if (send2dusti) {
        debug_out(F("## Sending to luftdaten.info (GPS): "), DEBUG_MIN_INFO, 1);
        start_send = micros();
        sendLuftdaten(data_4_dusti, GPS_API_PIN, host_dusti, httpPort_dusti, url_dusti, "GPS_");
        sum_send_time += micros() - start_send;
      }
    }

    data_sample_times += Value2Json("signal", signal_strength);
    data += data_sample_times;

    if (data.lastIndexOf(',') == (data.length() - 1)) {
      data.remove(data.length() - 1);
    }
    data += "]}";

    //sending to api(s)

    if ((act_milli - last_update_attempt) > pause_between_update_attempts) {
      will_check_for_update = true;
    }

    if (has_display || has_lcd1602 || has_lcd1602_27) {
      display_values(last_value_DHT_T, last_value_DHT_H, last_value_BMP_T, last_value_BMP_P, last_value_BMP280_T, last_value_BMP280_P, last_value_BME280_T, last_value_BME280_H, last_value_BME280_P, last_value_PPD_P1, last_value_PPD_P2, last_value_SDS_P1, last_value_SDS_P2);
    }

    if (send2madavi) {
      debug_out(F("## Sending to madavi.de: "), DEBUG_MIN_INFO, 1);
      start_send = micros();
      sendRestData(data, 0, host_madavi, httpPort_madavi, url_madavi, "", FPSTR(TXT_CONTENT_TYPE_JSON));
      sum_send_time += micros() - start_send;
    }

    if (send2sensemap && (senseboxid != SENSEBOXID)) {
      debug_out(F("## Sending to opensensemap: "), DEBUG_MIN_INFO, 1);
      start_send = micros();
      sensemap_path = url_sensemap;
      sensemap_path.replace("BOXID", senseboxid);
      sendRestData(data, 0, host_sensemap, httpPort_sensemap, sensemap_path.c_str(), "", FPSTR(TXT_CONTENT_TYPE_JSON));
      sum_send_time += micros() - start_send;
    }

    if (send2influx) {
      debug_out(F("## Sending to custom influx db: "), DEBUG_MIN_INFO, 1);
      start_send = micros();
      data_4_influxdb = create_influxdb_string(data);
      sendRestData(data_4_influxdb, 0, host_influx, port_influx, url_influx, basic_auth_influx.c_str(), FPSTR(TXT_CONTENT_TYPE_INFLUXDB));
      sum_send_time += micros() - start_send;
    }

    if (send2csv) {
      debug_out(F("## Sending as csv: "), DEBUG_MIN_INFO, 1);
      send_csv(data);
    }

    if (send2custom) {
      data_4_custom = data;
      data_4_custom.remove(0, 1);
      data_4_custom = "{\"esp8266id\": \"" + String(esp_chipid) + "\", " + data_4_custom;
      debug_out(F("## Sending to custom api: "), DEBUG_MIN_INFO, 1);
      start_send = micros();
      sendRestData(data_4_custom, 0, host_custom, port_custom, url_custom, basic_auth_custom.c_str(), FPSTR(TXT_CONTENT_TYPE_JSON));
      sum_send_time += micros() - start_send;
    }

    server.begin();

    if ((act_milli - last_update_attempt) > (28 * pause_between_update_attempts)) {
      ESP.restart();
    }

    if ((act_milli - last_update_attempt) > pause_between_update_attempts) {
      autoUpdate();
    }

    if (! send_failed) { sending_time = (4 * sending_time + sum_send_time) / 5; }
    debug_out(F("Time for sending data: "), DEBUG_MIN_INFO, 0);
    debug_out(String(sending_time), DEBUG_MIN_INFO, 1);


    if (WiFi.status() != WL_CONNECTED) {  // reconnect if connection lost
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
    lowpulseoccupancyP1 = 0;
    lowpulseoccupancyP2 = 0;
    sample_count = 0;
    last_micro = 0;
    min_micro = 1000000000;
    max_micro = 0;
    starttime = millis(); // store the start time
    first_cycle = false;
  }
  if (config_needs_write) { writeConfig(); create_basic_auth_strings(); }
  yield();
}

