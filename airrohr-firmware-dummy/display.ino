/*****************************************************************
/* display values                                                *
/*****************************************************************/
void display_values(const String& value_DHT_T, const String& value_DHT_H, const String& value_BMP_T, const String& value_BMP_P, const String& value_BMP280_T, const String& value_BMP280_P, const String& value_BME280_T, const String& value_BME280_H, const String& value_BME280_P, const String& value_PPD_P1, const String& value_PPD_P2, const String& value_SDS_P1, const String& value_SDS_P2) {
#if defined(ESP8266)
  int value_count = 0;
  String t_value = "";
  String h_value = "";
  String p_value = "";
  String t_sensor = "";
  String h_sensor = "";
  String p_sensor = "";
  String pm10_value = "";
  String pm25_value = "";
  String pm10_sensor = "";
  String pm25_sensor = "";
  debug_out(F("output values to display..."), DEBUG_MIN_INFO, 1);
  if (dht_read) {
    t_value = last_value_DHT_T; t_sensor = "DHT22";
    h_value = last_value_DHT_H; h_sensor = "DHT22";
  }
  if (bmp_read) {
    t_value = last_value_BMP_T; t_sensor = "BMP180";
    p_value = last_value_BMP_P; p_sensor = "BMP180";
  }
  if (bmp280_read) {
    t_value = last_value_BMP280_T; t_sensor = "BMP280";
    p_value = last_value_BMP280_P; p_sensor = "BMP280";
  }
  if (bme280_read) {
    t_value = last_value_BME280_T; t_sensor = "BME280";
    h_value = last_value_BME280_H; h_sensor = "BME280";
    p_value = last_value_BME280_P; p_sensor = "BME280";
  }
  if (ppd_read) {
    pm10_value = last_value_PPD_P1; pm10_sensor = "PPD42NS";
    pm25_value = last_value_PPD_P2; pm25_sensor = "PPD42NS";
  }
  if (pms24_read || pms32_read) {
    pm10_value = last_value_PMS_P1; pm10_sensor = "PMSx003";
    pm25_value = last_value_PMS_P2; pm25_sensor = "PMSx003";
  }
  if (sds_read) {
    pm10_value = last_value_SDS_P1; pm10_sensor = "SDS011";
    pm25_value = last_value_SDS_P2; pm25_sensor = "SDS011";
  }
  if (pm10_value == "") { pm10_value = "-";}
  if (pm25_value == "") { pm25_value = "-";}
  if (t_value == "") { t_value = "-";}
  if (h_value == "") { h_value = "-";}
  if (p_value == "") { p_value = "-";}

  if (has_display) {
    display.resetDisplay();
    display.clear();
    display.displayOn();
    display.setFont(Monospaced_plain_9);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    value_count = 0;
    display.drawString(0, 10 * (value_count++), "Temp:" + t_value + "  Hum.:" + h_value);
    if (ppd_read) {
      display.drawString(0, 10 * (value_count++), "PPD P1: " + value_PPD_P1);
      display.drawString(0, 10 * (value_count++), "PPD P2: " + value_PPD_P2);
    }
    if (sds_read) {
      display.drawString(0, 10 * (value_count++), "SDS P1: " + value_SDS_P1);
      display.drawString(0, 10 * (value_count++), "SDS P2: " + value_SDS_P2);
    }
    if (gps_read) {
      if(gps.location.isValid()) {
        display.drawString(0, 10 * (value_count++), "lat: " + String(gps.location.lat(), 6));
        display.drawString(0, 10 * (value_count++), "long: " + String(gps.location.lng(), 6));
      }
      display.drawString(0, 10 * (value_count++), "satellites: " + String(gps.satellites.value()));
    }
    display.display();
  }

// ----5----0----5----0
// PM10/2.5: 1999/999
// T/H: -10.0°C/100.0%
// T/P: -10.0°C/1000hPa

  if (has_lcd1602_27) {
    lcd_27.clear();
    lcd_27.setCursor(0, 0);
    lcd_27.print("PM: " + (value_SDS_P1 != "" ? value_SDS_P1 : "-") + " " + (value_SDS_P2 != "" ? value_SDS_P2 : "-"));
    lcd_27.setCursor(0, 1);
    lcd_27.print("T/H:" + t_value + char(223) + "C " + h_value + "%");
  }
  if (has_lcd1602) {
    lcd_3f.clear();
    lcd_3f.setCursor(0, 0);
    lcd_3f.print("PM: " + (value_SDS_P1 != "" ? value_SDS_P1 : "-") + " " + (value_SDS_P2 != "" ? value_SDS_P2 : "-"));
    lcd_3f.setCursor(0, 1);
    lcd_3f.print("T/H:" + t_value + char(223) + "C " + h_value + "%");
  }
  yield();
#endif
}

/*****************************************************************
/* Init display                                                  *
/*****************************************************************/
void init_display() {
#if defined(ESP8266)
  display.init();
  display.resetDisplay();
#endif
}

/*****************************************************************
/* Init display                                                  *
/*****************************************************************/
void init_lcd1602() {
#if defined(ESP8266)
  lcd_27.init();
  lcd_27.backlight();
  lcd_3f.init();
  lcd_3f.backlight();
#endif
}

