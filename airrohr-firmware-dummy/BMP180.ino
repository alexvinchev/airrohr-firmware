/*****************************************************************
/* read BMP180 sensor values                                     *
/*****************************************************************/
String sensorBMP180() {
  String s = "";
  int p;
  float t;

  debug_out(F("Start reading BMP180"), DEBUG_MED_INFO, 1);

  p = bmp.readPressure();
  t = bmp.readTemperature();
  last_value_BMP_T = "";
  last_value_BMP_P = "";
  if (isnan(p) || isnan(t)) {
    debug_out(F("BMP180 couldn't be read"), DEBUG_ERROR, 1);
  } else {
    debug_out(F("Pressure    : "), DEBUG_MIN_INFO, 0);
    debug_out(Float2String(float(p) / 100) + " hPa", DEBUG_MIN_INFO, 1);
    debug_out(F("Temperature : "), DEBUG_MIN_INFO, 0);
    debug_out(String(t) + " C", DEBUG_MIN_INFO, 1);
    last_value_BMP_T = Float2String(t);
    last_value_BMP_P = String(p);
    s += Value2Json(F("BMP_pressure"), last_value_BMP_P);
    s += Value2Json(F("BMP_temperature"), last_value_BMP_T);
    last_value_BMP_T.remove(last_value_BMP_T.length() - 1);
  }
  debug_out(F("------"), DEBUG_MIN_INFO, 1);

  debug_out(F("End reading BMP180"), DEBUG_MED_INFO, 1);

  return s;
}

