/*****************************************************************
/* read DS18B20 sensor values                                    *
/*****************************************************************/
String sensorDS18B20() {
  String s = "";
  int i = 0;
  float t;
  debug_out(F("Start reading DS18B20"), DEBUG_MED_INFO, 1);

  //it's very unlikely (-127: impossible) to get these temperatures in reality. Most times this means that the sensor is currently faulty
  //try 5 times to read the sensor, otherwise fail
  do {
    ds18b20.requestTemperatures();
    //for now, we want to read only the first sensor
    t = ds18b20.getTempCByIndex(0);
    last_value_DS18B20_T = "";
    i++;
    debug_out(F("DS18B20 trying...."), DEBUG_MIN_INFO, 0);
    debug_out(String(i), DEBUG_MIN_INFO, 1);
  } while(i < 5 && (isnan(t) || t == 85.0 || t == (-127.0)));

  if(i == 5) {
    debug_out(F("DS18B20 couldn't be read."), DEBUG_ERROR, 1);
  } else {
    debug_out(F("Temperature : "), DEBUG_MIN_INFO, 0);
    debug_out(Float2String(t) + " C", DEBUG_MIN_INFO, 1);
    last_value_DS18B20_T = Float2String(t);
    s += Value2Json(F("DS18B20_temperature"), last_value_DS18B20_T);
    last_value_DS18B20_T.remove(last_value_DS18B20_T.length() - 1);
  }
  debug_out(F("------"), DEBUG_MIN_INFO, 1);
  debug_out(F("End reading DS18B20"), DEBUG_MED_INFO, 1);

  return s;
}

