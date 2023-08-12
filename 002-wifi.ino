#include <WiFi.h>


// *** THIS NEEDS TO HAVE A TIMEOUT ON IT
boolean wifi_open() {

  WiFi.begin(wifi_ssid, wifi_password);
  unsigned long wifi_took_too_long = millis() + (20 * 1000);
  while (
    ( WiFi.status() != WL_CONNECTED ) &&
    ( millis() < wifi_took_too_long )
  ) {
    delay(1000);
  }

  if ( WiFi.status() == WL_CONNECTED ) {
    return true;
  }

  return false;
}


void wifi_close() {

  // disconnect from the wifi to prevent timeout issues
  delay(500);
  WiFi.disconnect();
}


// modified from author apicquot from https://forum.arduino.cc/index.php?topic=228884.0
String wifi_ip() {

IPAddress ipAddress = WiFi.localIP();

 return String(ipAddress[0]) + String(".") +
   String(ipAddress[1]) + String(".") +
   String(ipAddress[2]) + String(".") +
   String(ipAddress[3]);
}
