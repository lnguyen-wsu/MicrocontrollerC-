/**
   BasicHTTPClient.ino
    Created on: 24.05.2015
    Note: This is test for getting api 
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#define USE_SERIAL Serial

WiFiMulti WiFiMulti;
float stockPrice = 0;
// member function 
void badAnnounce();
void goodAnnounce();
void setup() {
  USE_SERIAL.begin(115200);
  // USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("lnguyen", "12345678");

}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    HTTPClient http;
    USE_SERIAL.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
    http.begin("http://api.thingspeak.com/apps/thinghttp/send_request?api_key=HRW09ZTBIBY4WS9A"); //HTTP

    USE_SERIAL.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();
    
    // setting the condition for the stock 
    
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        stockPrice = payload.toFloat(); 
        USE_SERIAL.println(payload);
        if (stockPrice >= 40){goodAnnounce();} else{badAnnounce();}
      }
    } else {
      USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }

  delay(10000);
}

// member function - implementation 
void badAnnounce(){
  Serial.print("Chet cha :: ");
  Serial.println(stockPrice);
}
void goodAnnounce(){
  Serial.print("hehe:: ");
  Serial.println(stockPrice);
}
