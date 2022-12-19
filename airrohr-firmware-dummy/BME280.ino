/*****************************************************************
/* Init BME280                                                   *
/*****************************************************************/
bool initBME280(char addr) {
    return true;
}

/*****************************************************************
/* read BME280 sensor values                                     *
/*****************************************************************/
String sensorBME280() {
    String s = "";

    last_value_BME280_T = Float2String(25);
    last_value_BME280_H = Float2String(50);
    last_value_BME280_P = Float2String(100000);
    s += Value2Json(F("BME280_temperature"), last_value_BME280_T);
    s += Value2Json(F("BME280_humidity"), last_value_BME280_H);
    s += Value2Json(F("BME280_pressure"), last_value_BME280_P);
    return s;
}

