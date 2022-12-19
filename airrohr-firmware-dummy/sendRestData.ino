/*****************************************************************
/* send data to rest api                                         *
/*****************************************************************/
void sendRestData(const String& data, const int pin, const char* host, const int httpPort, const char* url, const char* basic_auth_string, const String& contentType) {
#if defined(ESP8266)

  debug_out(F("Start connecting to "), DEBUG_MIN_INFO, 0);
  debug_out(host, DEBUG_MIN_INFO, 1);

  String request_head = F("POST "); request_head += String(url); request_head += F(" HTTP/1.1\r\n");
  request_head += F("Host: "); request_head += String(host) + "\r\n";
  request_head += F("Content-Type: "); request_head += contentType + "\r\n";
  if (basic_auth_string != "") { request_head += F("Authorization: Basic "); request_head += String(basic_auth_string) + "\r\n";}
  request_head += F("X-PIN: "); request_head += String(pin) + "\r\n";
  request_head += F("X-Sensor: esp8266-"); request_head += esp_chipid + "\r\n";
  request_head += F("Content-Length: "); request_head += String(data.length(), DEC) + "\r\n";
  request_head += F("Connection: close\r\n\r\n");

  // Use WiFiClient class to create TCP connections

  if (httpPort == 443) {

    WiFiClientSecure client_s;

    client_s.setNoDelay(true);
    client_s.setTimeout(20000);

    if (!client_s.connect(host, httpPort)) {
      debug_out(F("connection failed"), DEBUG_ERROR, 1);
      return;
    }

    debug_out(F("Requesting URL: "), DEBUG_MIN_INFO, 0);
    debug_out(url, DEBUG_MIN_INFO, 1);
    debug_out(esp_chipid, DEBUG_MIN_INFO, 1);
    debug_out(data, DEBUG_MIN_INFO, 1);

    // send request to the server

    client_s.print(request_head);

    client_s.println(data);

    delay(10);

    // Read reply from server and print them
    while(client_s.available()) {
      char c = client_s.read();
      debug_out(String(c), DEBUG_MAX_INFO, 0);
    }

    debug_out(F("\nclosing connection\n------\n\n"), DEBUG_MIN_INFO, 1);

  } else {

    WiFiClient client;

    client.setNoDelay(true);
    client.setTimeout(20000);

    if (!client.connect(host, httpPort)) {
      debug_out(F("connection failed"), DEBUG_ERROR, 1);
      return;
    }

    debug_out(F("Requesting URL: "), DEBUG_MIN_INFO, 0);
    debug_out(url, DEBUG_MIN_INFO, 1);
    debug_out(esp_chipid, DEBUG_MIN_INFO, 1);
    debug_out(data, DEBUG_MIN_INFO, 1);

    client.print(request_head);

    client.println(data);

    delay(10);

    // Read reply from server and print them
    while(client.available()) {
      char c = client.read();
      debug_out(String(c), DEBUG_MAX_INFO, 0);
    }

    debug_out(F("\nclosing connection\n------\n\n"), DEBUG_MIN_INFO, 1);

  }

  debug_out(F("End connecting to "), DEBUG_MIN_INFO, 0);
  debug_out(host, DEBUG_MIN_INFO, 1);

  wdt_reset(); // nodemcu is alive
  yield();
#endif
}

