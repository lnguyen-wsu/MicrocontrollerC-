// This is header for local object about RTC over Wifi
// Programmer: Luan Nguyen 
// Project: Smart Knob

# include <Arduino.h>
#include <NTPClient.h>
#include <WiFi.h> 

class RtcWifi{
  private:
	//bool _testWifi(void);
	int minute;
	int hour;
	int seconds;
  String dateTime; 

  
  public:
	RtcWifi(){minute = 0; hour = 0; seconds = 0;};
	// member functions 
	int get_mins();
	int get_hour();
	int get_secs();
	void RtcSetUp(void);
	void RtcLoop(void);
	String Rtc_formatted();
  String Rtc_Date();
  bool _testWifi(void);

};
