// This is RtcWifi.cpp about local customized library
// Programmer: Luan Nguyen
// Project : Smart Knob

#include "RtcWifi.h"   // local library

// Declaration about global WifiUdp variables
const int led1 = 2;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "north-america.pool.ntp.org", -18000 , 60000);
TaskHandle_t Task0;

void Task0code ( void *parameters){
  for (;;){
    Serial.print("At Core 0 - In WSU, we are at "); 
    Serial.println(timeClient.getFormattedTime());
    delay(5000);
  }
}

bool RtcWifi:: _testWifi(void){
  if(WiFi.status()!= WL_CONNECTED ){
    delay(500);
    return false;
  }
  return true;
}

int RtcWifi::get_mins(){
  minute = timeClient.getMinutes();
  return minute;
}

int RtcWifi::get_hour(){
  hour = timeClient.getHours();
  return hour;
}
int RtcWifi::get_secs(){
  seconds = timeClient.getSeconds();
  return seconds;
}

void RtcWifi::RtcSetUp(void){
  pinMode(led1, OUTPUT);
  if (_testWifi()){
      Serial.println("Connected");
      digitalWrite(led1, HIGH);
  }
 timeClient.begin();
 xTaskCreatePinnedToCore(
                    Task0code,   /* Task function. */
                    "Task0",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task0,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */
  delay(500);
}

void RtcWifi::RtcLoop(void){
  timeClient.update();
}

String RtcWifi::Rtc_formatted(){
  return (timeClient.getFormattedTime());
}

String RtcWifi:: Rtc_Date(){
  dateTime = timeClient.getFormattedDate();
  // Extract date
  int splitT = dateTime.indexOf("T");
  String dayStamp = dateTime.substring(0, splitT);
  return dayStamp;
  
}
