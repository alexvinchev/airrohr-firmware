/*****************************************************************
/* read HTU21D sensor values                                     *
/*****************************************************************/
String sensorHTU21D() {
  String s = "";
  float t;
  float h;

  debug_out(F("Start reading HTU21D"), DEBUG_MED_INFO, 1);

  t = htu21d.readTemperature();
  h = htu21d.readHumidity();
  if (isnan(t) || isnan(h)) {
    debug_out(F("HTU21D couldn't be read"), DEBUG_ERROR, 1);
  } else {
    debug_out(F("Temperature : "), DEBUG_MIN_INFO, 0);
    debug_out(Float2String(t) + " C", DEBUG_MIN_INFO, 1);
    debug_out(F("humidity : "), DEBUG_MIN_INFO, 0);
    debug_out(Float2String(h) + " %", DEBUG_MIN_INFO, 1);
    last_value_HTU21D_T = Float2String(t);
    last_value_HTU21D_H = Float2String(h);
    s += Value2Json(F("HTU21D_temperature"), last_value_HTU21D_T);
    s += Value2Json(F("HTU21D_humidity"), last_value_HTU21D_H);
  }
  debug_out(F("------"), DEBUG_MIN_INFO, 1);

  debug_out(F("End reading HTU21D"), DEBUG_MED_INFO, 1);

  return s;
}

