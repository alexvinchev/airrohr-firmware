/*****************************************************************
/* read GPS sensor values                                        *
/*****************************************************************/
String sensorGPS() {
  String s = "";
#if defined(ARDUINO_SAMD_ZERO) || defined(ESP8266)
  String gps_lat = "";
  String gps_lng = "";
  String gps_alt = "";
  String gps_date = "";
  String gps_time = "";

  debug_out(F("Start reading GPS"), DEBUG_MED_INFO, 1);

  while (serialGPS.available() > 0) {
    if (gps.encode(serialGPS.read())) {
      if (gps.location.isValid()) {
        last_gps_lat = String(gps.location.lat(), 6);
        last_gps_lng = String(gps.location.lng(), 6);
      } else {
        debug_out(F("Lat/Lng INVALID"), DEBUG_MAX_INFO, 1);
      }
      if (gps.altitude.isValid()) {
        last_gps_alt = String(gps.altitude.meters(), 2);
      } else {
        debug_out(F("Altitude INVALID"), DEBUG_MAX_INFO, 1);
      }
      if (gps.date.isValid()) {
        gps_date = "";
        if (gps.date.month() < 10) { gps_date += "0"; }
        gps_date += String(gps.date.month());
        gps_date += "/";
        if (gps.date.day() < 10) { gps_date += "0"; }
        gps_date += String(gps.date.day());
        gps_date += "/";
        gps_date += String(gps.date.year());
        last_gps_date = gps_date;
      } else {
        debug_out(F("Date INVALID"), DEBUG_MAX_INFO, 1);
      }
      if (gps.time.isValid()) {
        gps_time = "";
        if (gps.time.hour() < 10) { gps_time += "0"; }
        gps_time += String(gps.time.hour());
        gps_time += ":";
        if (gps.time.minute() < 10) { gps_time += "0"; }
        gps_time += String(gps.time.minute());
        gps_time += ":";
        if (gps.time.second() < 10) { gps_time += "0"; }
        gps_time += String(gps.time.second());
        gps_time += ".";
        if (gps.time.centisecond() < 10) { gps_time += "0"; }
        gps_time += String(gps.time.centisecond());
        last_gps_time = gps_time;
      } else {
        debug_out(F("Time: INVALID"), DEBUG_MAX_INFO, 1);
      }
    }
  }

  if (send_now) {
    debug_out("Lat/Lng: " + last_gps_lat + "," + last_gps_lng, DEBUG_MIN_INFO, 1);
    debug_out("Altitude: " + last_gps_alt, DEBUG_MIN_INFO, 1);
    debug_out("Date: " + last_gps_date, DEBUG_MIN_INFO, 1);
    debug_out("Time " + last_gps_time, DEBUG_MIN_INFO, 1);
    debug_out("------", DEBUG_MIN_INFO, 1);
    s += Value2Json(F("GPS_lat"), last_gps_lat);
    s += Value2Json(F("GPS_lon"), last_gps_lng);
    s += Value2Json(F("GPS_height"), last_gps_alt);
    s += Value2Json(F("GPS_date"), last_gps_date);
    s += Value2Json(F("GPS_time"), last_gps_time);
    last_gps_lat = "";
    last_gps_lng = "";
    last_gps_alt = "";
    last_gps_date = "";
    last_gps_time = "";
  }

  if ( gps.charsProcessed() < 10) {
    debug_out(F("No GPS data received: check wiring"), DEBUG_ERROR, 1);
  }

  debug_out(F("End reading GPS"), DEBUG_MED_INFO, 1);

#endif
  return s;
}

