/*****************************************************************
/* read PPD42NS sensor values                                    *
/*****************************************************************/
String sensorPPD42NS() {
  boolean valP1 = HIGH;
  boolean valP2 = HIGH;
  float ratio = 0;
  float concentration = 0;
  String s = "";

  debug_out(F("Start reading PPD42NS"), DEBUG_MED_INFO, 1);

  if ((act_milli - starttime) <= sampletime_ms) {

    // Read pins connected to ppd42ns
    valP1 = digitalRead(PPD_PIN_PM1);
    valP2 = digitalRead(PPD_PIN_PM2);

    if(valP1 == LOW && trigP1 == false) {
      trigP1 = true;
      trigOnP1 = act_micro;
    }

    if (valP1 == HIGH && trigP1 == true) {
      durationP1 = act_micro - trigOnP1;
      lowpulseoccupancyP1 = lowpulseoccupancyP1 + durationP1;
      trigP1 = false;
    }

    if(valP2 == LOW && trigP2 == false) {
      trigP2 = true;
      trigOnP2 = act_micro;
    }

    if (valP2 == HIGH && trigP2 == true) {
      durationP2 = act_micro - trigOnP2;
      lowpulseoccupancyP2 = lowpulseoccupancyP2 + durationP2;
      trigP2 = false;
    }

  }
  // Checking if it is time to sample
  if (send_now) {
    last_value_PPD_P1 = "";
    last_value_PPD_P2 = "";
    ratio = lowpulseoccupancyP1 / (sampletime_ms * 10.0);         // int percentage 0 to 100
    concentration = (1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62); // spec sheet curve
    // Begin printing
    debug_out(F("LPO P10    : "), DEBUG_MIN_INFO, 0);
    debug_out(String(lowpulseoccupancyP1), DEBUG_MIN_INFO, 1);
    debug_out(F("Ratio PM10 : "), DEBUG_MIN_INFO, 0);
    debug_out(Float2String(ratio) + " %", DEBUG_MIN_INFO, 1);
    debug_out(F("PM10 Count : "), DEBUG_MIN_INFO, 0);
    debug_out(Float2String(concentration), DEBUG_MIN_INFO, 1);

    // json for push to api / P1
    last_value_PPD_P1 = Float2String(concentration);
    s += Value2Json("durP1", String(long(lowpulseoccupancyP1)));
    s += Value2Json("ratioP1", Float2String(ratio));
    s += Value2Json("P1", last_value_PPD_P1);

    ratio = lowpulseoccupancyP2 / (sampletime_ms * 10.0);
    concentration = (1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62);
    // Begin printing
    debug_out(F("LPO PM25   : "), DEBUG_MIN_INFO, 0);
    debug_out(String(lowpulseoccupancyP2), DEBUG_MIN_INFO, 1);
    debug_out(F("Ratio PM25 : "), DEBUG_MIN_INFO, 0);
    debug_out(Float2String(ratio) + " %", DEBUG_MIN_INFO, 1);
    debug_out(F("PM25 Count : "), DEBUG_MIN_INFO, 0);
    debug_out(Float2String(concentration), DEBUG_MIN_INFO, 1);

    // json for push to api / P2
    last_value_PPD_P2 = Float2String(concentration);
    s += Value2Json("durP2", String(long(lowpulseoccupancyP2)));
    s += Value2Json("ratioP2", Float2String(ratio));
    s += Value2Json("P2", last_value_PPD_P2);

    debug_out(F("------"), DEBUG_MIN_INFO, 1);
  }

  debug_out(F("End reading PPD42NS"), DEBUG_MED_INFO, 1);

  return s;
}

