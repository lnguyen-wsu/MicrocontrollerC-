/* Programmer: Luan Nguyen 
 * Project: Find the api of the stock and display in the led
 * 
 * 
 */ 
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>


// Part 1: API
WiFiMulti WiFiMulti;
float stockPrice = 0;
// member function 
void badAnnounce();
void goodAnnounce();

// Part 2: Display

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

void setup(){
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("lnguyen", "12345678");
}

void loop(){
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
    http.begin("http://api.thingspeak.com/apps/thinghttp/send_request?api_key=HRW09ZTBIBY4WS9A"); //HTTP

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();
    
    // setting the condition for the stock 
    
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        stockPrice = payload.toFloat(); 
        Serial.println(payload);
        if (stockPrice >= 40){goodAnnounce();} else{badAnnounce();}
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }

  delay(10000);
}
// member function - implementation 
void badAnnounce(){
  Serial.print("Chet cha :: ");
  Serial.println(stockPrice);
  lcd.setCursor(0,0);
  lcd.println("Chet cha, anh Ty oi!!");
  lcd.print(stockPrice);
  lcd.println(" is Nio");
  delay(1000);
  lcd.clear();
}
void goodAnnounce(){
  Serial.print("hehe:: ");
  Serial.println(stockPrice);
  lcd.setCursor(0,0);
  lcd.println("hehe Len May, anh Ty oi");
  lcd.print(stockPrice);
  lcd.println(" is Nio");
  delay(1000);
  lcd.clear();
}
