/*This is test between retrieve RTC over wifi and build RTCwifi object in the separated libraries.
  This test also apply run second core in the ESp32 
  Programmer: Luan Nguyen
  Project: Smart Knob 
*/
# include "RtcWifi.h"
const char *ssid     = "ATT2NP4529";
const char *password = "36k3bs73a62w";

RtcWifi Rtc_Wifi;

void setup() {
  Serial.begin(115200); 
  WiFi.begin(ssid, password);
  //pinMode(led1, OUTPUT);
  Rtc_Wifi.RtcSetUp();
  
}

void loop() {
  Rtc_Wifi.RtcLoop();
  Serial.println(Rtc_Wifi.Rtc_Date());
  delay(10000);
  
  }
