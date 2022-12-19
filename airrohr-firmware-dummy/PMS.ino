/*****************************************************************
/* read Plantronic PM sensor sensor values                       *
/*****************************************************************/
String sensorPMS(int msg_len) {
  String s = "";
  String value_hex;
  char buffer;
  int value;
  int len = 0;
  int pm1_serial = 0;
  int pm10_serial = 0;
  int pm25_serial = 0;
  int checksum_is;
  int checksum_should;
  int checksum_ok = 0;
  int position = 0;

  debug_out(F("Start reading PMS"), DEBUG_MED_INFO, 1);
  if (long(act_milli - starttime) < (long(sending_intervall_ms) - long(warmup_time_SDS_ms + reading_time_SDS_ms))) {
    if (is_PMS_running) {
      stop_PMS();
    }
  } else {
    if (! is_PMS_running) {
      start_PMS();
    }

    while (serialSDS.available() > 0) {
      buffer = serialSDS.read();
      debug_out(String(len) + " - " + String(buffer, DEC) + " - " + String(buffer, HEX) + " - " + int(buffer) + " .", DEBUG_MAX_INFO, 1);
//      "aa" = 170, "ab" = 171, "c0" = 192
      value = int(buffer);
      switch (len) {
      case (0): if (value != 66) { len = -1; }; break;
      case (1): if (value != 77) { len = -1; }; break;
      case (2): checksum_is = value; break;
      case (3): checksum_is += value; break;
      case (4): checksum_is += value; break;
      case (5): checksum_is += value; break;
      case (6): checksum_is += value; break;
      case (7): checksum_is += value; break;
      case (8): checksum_is += value; break;
      case (9): checksum_is += value; break;
      case (10): pm1_serial += ( value << 8); checksum_is += value; break;
      case (11): pm1_serial += value; checksum_is += value; break;
      case (12): pm25_serial = ( value << 8); checksum_is += value; break;
      case (13): pm25_serial += value; checksum_is += value; break;
      case (14): pm10_serial = ( value << 8); checksum_is += value; break;
      case (15): pm10_serial += value; checksum_is += value; break;
      case (16): checksum_is += value; break;
      case (17): checksum_is += value; break;
      case (18): checksum_is += value; break;
      case (19): checksum_is += value; break;
      case (20): checksum_is += value; break;
      case (21): checksum_is += value; break;
      case (22): if (msg_len == 24) { checksum_should = ( value << 8 ); } else { checksum_is += value; }; break;
      case (23): if (msg_len == 24) { checksum_should += value; } else { checksum_is += value; }; break;
      case (24): checksum_is += value; break;
      case (25): checksum_is += value; break;
      case (26): checksum_is += value; break;
      case (27): checksum_is += value; break;
      case (28): checksum_is += value; break;
      case (29): checksum_is += value; break;
      case (30): checksum_should = ( value << 8 ); break;
      case (31): checksum_should += value; break;
      }
      len++;
      if (len == msg_len) {
        debug_out(F("Checksum is: "), DEBUG_MED_INFO, 0); debug_out(String(checksum_is + 143), DEBUG_MED_INFO, 0);
        debug_out(F(" - should: "), DEBUG_MED_INFO, 0); debug_out(String(checksum_should), DEBUG_MED_INFO, 1);
        if (checksum_should == (checksum_is + 143)) { checksum_ok = 1; } else { len = 0; };
      }
      if (len == msg_len && checksum_ok == 1 && (long(act_milli - starttime) > (long(sending_intervall_ms) - long(reading_time_SDS_ms)))) {
        if ((! isnan(pm1_serial)) && (! isnan(pm10_serial)) && (! isnan(pm25_serial))) {
          pms_pm1_sum += pm1_serial;
          pms_pm10_sum += pm10_serial;
          pms_pm25_sum += pm25_serial;
          if (pms_pm1_min > pm10_serial) { pms_pm1_min = pm1_serial; }
          if (pms_pm1_max < pm10_serial) { pms_pm1_max = pm1_serial; }
          if (pms_pm10_min > pm10_serial) { pms_pm10_min = pm10_serial; }
          if (pms_pm10_max < pm10_serial) { pms_pm10_max = pm10_serial; }
          if (pms_pm25_min > pm25_serial) { pms_pm25_min = pm25_serial; }
          if (pms_pm25_max < pm25_serial) { pms_pm25_max = pm25_serial; }
          debug_out(F("PM1 (sec.): "), DEBUG_MED_INFO, 0); debug_out(Float2String(float(pm1_serial)), DEBUG_MED_INFO, 1);
          debug_out(F("PM2.5 (sec.): "), DEBUG_MED_INFO, 0); debug_out(Float2String(float(pm25_serial)), DEBUG_MED_INFO, 1);
          debug_out(F("PM10 (sec.) : "), DEBUG_MED_INFO, 0); debug_out(Float2String(float(pm10_serial)), DEBUG_MED_INFO, 1);
          pms_val_count++;
        }
        len = 0; checksum_ok = 0; pm1_serial = 0.0; pm10_serial = 0.0; pm25_serial = 0.0; checksum_is = 0;
      }
      yield();
    }

  }
  if (send_now) {
    last_value_PMS_P0 = "";
    last_value_PMS_P1 = "";
    last_value_PMS_P2 = "";
    if (pms_val_count > 2) {
      pms_pm1_sum = pms_pm1_sum - pms_pm1_min - pms_pm1_max;
      pms_pm10_sum = pms_pm10_sum - pms_pm10_min - pms_pm10_max;
      pms_pm25_sum = pms_pm25_sum - pms_pm25_min - pms_pm25_max;
      pms_val_count = pms_val_count - 2;
    }
    if (pms_val_count > 0) {
      debug_out("PM1:   " + Float2String(float(pms_pm1_sum) / (pms_val_count * 1.0)), DEBUG_MIN_INFO, 1);
      debug_out("PM2.5: " + Float2String(float(pms_pm25_sum) / (pms_val_count * 1.0)), DEBUG_MIN_INFO, 1);
      debug_out("PM10:  " + Float2String(float(pms_pm10_sum) / (pms_val_count * 1.0)), DEBUG_MIN_INFO, 1);
      debug_out("-------", DEBUG_MIN_INFO, 1);
      last_value_PMS_P0 = Float2String(float(pms_pm1_sum) / (pms_val_count * 1.0));
      last_value_PMS_P1 = Float2String(float(pms_pm10_sum) / (pms_val_count * 1.0));
      last_value_PMS_P2 = Float2String(float(pms_pm25_sum) / (pms_val_count * 1.0));
      s += Value2Json("PMS_P0", last_value_PMS_P0);
      s += Value2Json("PMS_P1", last_value_PMS_P1);
      s += Value2Json("PMS_P2", last_value_PMS_P2);
      last_value_PMS_P0.remove(last_value_PMS_P0.length() - 1);
      last_value_PMS_P1.remove(last_value_PMS_P1.length() - 1);
      last_value_PMS_P2.remove(last_value_PMS_P2.length() - 1);
    }
    pms_pm1_sum = 0; pms_pm10_sum = 0; pms_pm25_sum = 0; pms_val_count = 0;
    pms_pm1_max = 0; pms_pm1_min = 20000; pms_pm10_max = 0; pms_pm10_min = 20000; pms_pm25_max = 0; pms_pm25_min = 20000;
    if ((sending_intervall_ms > (warmup_time_SDS_ms + reading_time_SDS_ms)) && (! will_check_for_update)) {
      stop_PMS();
    }
  }

  debug_out(F("End reading PMS"), DEBUG_MED_INFO, 1);

  return s;
}

/*****************************************************************
/* start Plantower PMS sensor                                    *
/*****************************************************************/
void start_PMS() {
  const uint8_t start_PMS_cmd[] = { 0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74 };
  serialSDS.write(start_PMS_cmd, sizeof(start_PMS_cmd)); is_PMS_running = true;
}

/*****************************************************************
/* stop Plantower PMS sensor                                     *
/*****************************************************************/
void stop_PMS() {
  const uint8_t stop_PMS_cmd[] = { 0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73 };
  serialSDS.write(stop_PMS_cmd, sizeof(stop_PMS_cmd)); is_PMS_running = false;
}


