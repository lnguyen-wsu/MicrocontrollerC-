// This is test for local variables GoogleHub
// Programmer: Luan Nguyen
// Project: Smart Knob
#include "GoogleHub.h"
//const char *ssid     = "ATT2NP4529";
//const char *password = "36k3bs73a62w";

const char *ssid     = "WSU Guest";
const char *password = "";
GoogleHub hub;
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  hub.GoogleHub_SetUp();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED){
     //int temp = random(100);
     hub.GoogleHub_SendData(random(100),random(100), random(100));
     Serial.println ("Update to Google sheet");
  } else {
     Serial.println("Not connected");
  }
  delay(1000);
}
