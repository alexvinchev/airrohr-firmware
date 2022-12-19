/*****************************************************************
/* read DHT22 sensor values                                      *
/*****************************************************************/
String sensorDHT22() {
  String s = "";
  int i = 0;
  float h;
  float t;

  debug_out(F("Start reading DHT11/22"), DEBUG_MED_INFO, 1);

  // Check if valid number if non NaN (not a number) will be send.

  last_value_DHT_T = "";
  last_value_DHT_H = "";

  while ((i++ < 5) && (s == "")) {
    h = dht.readHumidity(); //Read Humidity
    t = dht.readTemperature(); //Read Temperature
    if (isnan(t) || isnan(h)) {
      delay(100);
      h = dht.readHumidity(true); //Read Humidity
      t = dht.readTemperature(false, true); //Read Temperature
    }
    if (isnan(t) || isnan(h)) {
      debug_out(F("DHT22 couldn't be read"), DEBUG_ERROR, 1);
    } else {
      debug_out(F("Humidity    : "), DEBUG_MIN_INFO, 0);
      debug_out(String(h) + "%", DEBUG_MIN_INFO, 1);
      debug_out(F("Temperature : "), DEBUG_MIN_INFO, 0);
      debug_out(String(t) + char(223) + "C", DEBUG_MIN_INFO, 1);
      last_value_DHT_T = Float2String(t);
      last_value_DHT_H = Float2String(h);
      s += Value2Json(F("temperature"), last_value_DHT_T);
      s += Value2Json(F("humidity"), last_value_DHT_H);
      last_value_DHT_T.remove(last_value_DHT_T.length() - 1);
      last_value_DHT_H.remove(last_value_DHT_H.length() - 1);
    }
  }
  debug_out(F("------"), DEBUG_MIN_INFO, 1);

  debug_out(F("End reading DHT11/22"), DEBUG_MED_INFO, 1);

  return s;
}

