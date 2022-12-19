/*****************************************************************
/* Init BMP280                                                   *
/*****************************************************************/
bool initBMP280(char addr) {
  debug_out(F("Trying BMP280 sensor on "), DEBUG_MIN_INFO, 0);
  debug_out(String(addr, HEX), DEBUG_MIN_INFO, 0);

  if (bmp280.begin(addr)) {
    debug_out(F(" ... found"), DEBUG_MIN_INFO, 1);
    return true;
  } else {
    debug_out(F(" ... not found"), DEBUG_MIN_INFO, 1);
    return false;
  }
}

/*****************************************************************
/* read BMP280 sensor values                                     *
/*****************************************************************/
String sensorBMP280() {
  String s = "";
  int p;
  float t;

  debug_out(F("Start reading BMP280"), DEBUG_MED_INFO, 1);

  p = bmp280.readPressure();
  t = bmp280.readTemperature();
  last_value_BMP280_T = "";
  last_value_BMP280_P = "";
  if (isnan(p) || isnan(t)) {
    debug_out(F("BMP280 couldn't be read"), DEBUG_ERROR, 1);
  } else {
    debug_out(F("Pressure    : "), DEBUG_MIN_INFO, 0);
    debug_out(Float2String(float(p) / 100) + " hPa", DEBUG_MIN_INFO, 1);
    debug_out(F("Temperature : "), DEBUG_MIN_INFO, 0);
    debug_out(String(t) + " C", DEBUG_MIN_INFO, 1);
    last_value_BMP280_T = Float2String(t);
    last_value_BMP280_P = String(p);
    s += Value2Json(F("BMP_pressure"), last_value_BMP280_P);
    s += Value2Json(F("BMP_temperature"), last_value_BMP280_T);
    last_value_BMP280_T.remove(last_value_BMP280_T.length() - 1);
  }
  debug_out(F("------"), DEBUG_MIN_INFO, 1);

  debug_out(F("End reading BMP180"), DEBUG_MED_INFO, 1);

  return s;
}

