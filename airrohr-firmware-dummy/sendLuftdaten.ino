/*****************************************************************
/* send single sensor data to luftdaten.info api                 *
/*****************************************************************/
void sendLuftdaten(const String& data, const int pin, const char* host, const int httpPort, const char* url, const char* replace_str) {
  String data_4_dusti = "";
  data_4_dusti  = data_first_part + data;
  data_4_dusti.remove(data_4_dusti.length() - 1);
  data_4_dusti.replace(replace_str, "");
  data_4_dusti += "]}";
  if (data != "") {
    sendRestData(data_4_dusti, pin, host, httpPort, url, "", FPSTR(TXT_CONTENT_TYPE_JSON));
  } else {
    debug_out(F("No data sent..."), DEBUG_MIN_INFO, 1);
  }
}

