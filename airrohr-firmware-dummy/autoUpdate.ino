/*****************************************************************
/* AutoUpdate                                                    *
/*****************************************************************/
void autoUpdate() {
#if defined(ESP8266)
  String SDS_version = "";
  if (auto_update) {
    debug_out(F("Starting OTA update ..."), DEBUG_MIN_INFO, 1);
    debug_out(F("NodeMCU firmware : "), DEBUG_MIN_INFO, 0);
    debug_out(String(SOFTWARE_VERSION), DEBUG_MIN_INFO, 1);
    debug_out(String(update_host), DEBUG_MED_INFO, 1);
    debug_out(String(update_url), DEBUG_MED_INFO, 1);

    if (sds_read) { SDS_version = SDS_version_date();}
    //SDS_version = "999";
    display_debug(F("Looking for OTA update"));
    last_update_attempt = millis();
    t_httpUpdate_return ret = ESPhttpUpdate.update(update_host, update_port, update_url, String(SOFTWARE_VERSION) + String(" ") + esp_chipid + String(" ") + SDS_version + String(" ") + String(current_lang) + String(" ") + String(INTL_LANG));
    switch(ret) {
    case HTTP_UPDATE_FAILED:
      debug_out(F("[update] Update failed."), DEBUG_ERROR, 0);
      debug_out(ESPhttpUpdate.getLastErrorString().c_str(), DEBUG_ERROR, 1);
      display_debug(F("Update failed."));
      break;
    case HTTP_UPDATE_NO_UPDATES:
      debug_out(F("[update] No Update."), DEBUG_MIN_INFO, 1);
      display_debug(F("No update found."));
      break;
    case HTTP_UPDATE_OK:
      debug_out(F("[update] Update ok."), DEBUG_MIN_INFO, 1); // may not called we reboot the ESP
      break;
    }
  }
  will_check_for_update = false;
#endif
}

