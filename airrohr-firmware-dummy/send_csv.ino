/*****************************************************************
/* send data as csv to serial out                                *
/*****************************************************************/
void send_csv(const String& data) {
  char* s;
  String tmp_str;
  String headline;
  String valueline;
  int value_count = 0;
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& json2data = jsonBuffer.parseObject(data);
  debug_out(F("CSV Output"), DEBUG_MIN_INFO, 1);
  debug_out(data, DEBUG_MIN_INFO, 1);
  if (json2data.success()) {
    headline = F("Timestamp_ms;");
    valueline = String(act_milli) + ";";
    for (int i = 0; i < json2data["sensordatavalues"].size(); i++) {
      tmp_str = jsonBuffer.strdup(json2data["sensordatavalues"][i]["value_type"].as<char*>());
      headline += tmp_str + ";";
      tmp_str = jsonBuffer.strdup(json2data["sensordatavalues"][i]["value"].as<char*>());
      valueline += tmp_str + ";";
    }
    if (first_csv_line) {
      if (headline.length() > 0) { headline.remove(headline.length() - 1); }
      Serial.println(headline);
      first_csv_line = 0;
    }
    if (valueline.length() > 0) { valueline.remove(valueline.length() - 1); }
    Serial.println(valueline);
  } else {
    debug_out(F("Data read failed"), DEBUG_ERROR, 1);
  }
}

