/*****************************************************************
/* read SDS011 sensor values                                     *
/*****************************************************************/
String SDS_version_date() {
  const uint8_t version_SDS_cmd[] = {0xAA, 0xB4, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x05, 0xAB};
  String s = "";
  String value_hex;
  char buffer;
  int value;
  int len = 0;
  String version_date = "";
  String device_id = "";
  int checksum_is;
  int checksum_ok = 0;
  int position = 0;

  debug_out(F("Start reading SDS011 version date"), DEBUG_MED_INFO, 1);

  start_SDS();

  delay(100);

  serialSDS.write(version_SDS_cmd, sizeof(version_SDS_cmd));

  delay(500);

  while (serialSDS.available() > 0) {
    buffer = serialSDS.read();
    debug_out(String(len) + " - " + String(buffer, DEC) + " - " + String(buffer, HEX) + " - " + int(buffer) + " .", DEBUG_MED_INFO, 1);
//    "aa" = 170, "ab" = 171, "c0" = 192
    value = int(buffer);
    switch (len) {
    case (0): if (value != 170) { len = -1; }; break;
    case (1): if (value != 197) { len = -1; }; break;
    case (2): if (value != 7) { len = -1; }; break;
    case (3): version_date  = String(value); checksum_is = 7 + value; break;
    case (4): version_date += "-" + String(value); checksum_is += value; break;
    case (5): version_date += "-" + String(value); checksum_is += value; break;
    case (6): if (value < 0x10) {device_id  = "0" + String(value, HEX);} else {device_id  = String(value, HEX);}; checksum_is += value; break;
    case (7): if (value < 0x10) {device_id += "0";}; device_id += String(value, HEX); checksum_is += value; break;
    case (8):
      debug_out(F("Checksum is: "), DEBUG_MED_INFO, 0);
      debug_out(String(checksum_is % 256), DEBUG_MED_INFO, 0);
      debug_out(F(" - should: "), DEBUG_MED_INFO, 0);
      debug_out(String(value), DEBUG_MED_INFO, 1);
      if (value == (checksum_is % 256)) { checksum_ok = 1; } else { len = -1; }; break;
    case (9): if (value != 171) { len = -1; }; break;
    }
    len++;
    if (len == 10 && checksum_ok == 1) {
      s = version_date + "(" + device_id + ")";
      debug_out(F("SDS version date : "), DEBUG_MIN_INFO, 0);
      debug_out(version_date, DEBUG_MIN_INFO, 1);
      debug_out(F("SDS device ID:     "), DEBUG_MIN_INFO, 0);
      debug_out(device_id, DEBUG_MIN_INFO, 1);
      len = 0; checksum_ok = 0; version_date = ""; device_id = ""; checksum_is = 0;
    }
    yield();
  }

  debug_out(F("End reading SDS011 version date"), DEBUG_MED_INFO, 1);

  return s;
}

/*****************************************************************
/* start SDS011 sensor                                           *
/*****************************************************************/
void start_SDS() {
  const uint8_t start_SDS_cmd[] = {0xAA, 0xB4, 0x06, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x06, 0xAB};
  serialSDS.write(start_SDS_cmd, sizeof(start_SDS_cmd)); is_SDS_running = true;
}

/*****************************************************************
/* stop SDS011 sensor                                            *
/*****************************************************************/
void stop_SDS() {
  const uint8_t stop_SDS_cmd[] = {0xAA, 0xB4, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x05, 0xAB};
  serialSDS.write(stop_SDS_cmd, sizeof(stop_SDS_cmd)); is_SDS_running = false;
}

/*****************************************************************
/* read SDS011 sensor values                                     *
/*****************************************************************/
String sensorSDS() {
    String s = "";

    last_value_SDS_P1 = Float2String(10.0);
    last_value_SDS_P2 = Float2String(2.5);
    s += Value2Json("SDS_P1", last_value_SDS_P1);
    s += Value2Json("SDS_P2", last_value_SDS_P2);
    return s;
}

