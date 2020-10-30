// this is local library for google output 
// Programmer : Luan Nguyen
// Prject : SmartKnob

#include<Arduino.h>
#include<WiFi.h>
#include <HTTPClient.h>

class GoogleHub{
   private : 
	   String GoogleHub_Convert (int, int , int);
   public:
	  void GoogleHub_SetUp();
	  void GoogleHub_SendData (int , int , int);
};
