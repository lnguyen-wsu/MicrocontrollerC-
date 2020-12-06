/*********************** Integrated Code for ASK Hub + App ***********************/


#define USE_WROVER_BOARD           // LOLIN D32 Pro board, upload speed to 115200, partition scheme to Large APP
#define APP_DEBUG
#define BLYNK_SSL_USE_LETSENCRYPT
#define BLYNK_PRINT Serial

#define SERVICE_UUID_KNOB          "903eb116-84da-11ea-bc55-0242ac130003"   // service for Knob
#define SERVICE_UUID_SENSOR        "97920c4e-4b19-45c8-a526-330e5e543886"   // service for Sensor
#define CHARACTERISTIC_UUID_KNOB   "b9f37d40-c452-11ea-87d0-0242ac130003"   // characteristic Knob
#define CHARACTERISTIC_UUID_SENSOR "d6c6b454-c00f-4b1e-b180-1c7d7a983ebe"   // characteristic for Sensor

#include "BlynkProvisioning.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include "RTClib.h"
RTC_DS3231 rtc;

BLEServer* pServer = NULL;                              // BLE for both sensor Unit and the knob
BLECharacteristic* pCharacteristicKnob = NULL;          // Chracteristic for Knob
BLECharacteristic* pCharacteristicSensor = NULL;        // Characteristic for Sensor
//const int LED = 0;
bool SensorConnected = false;
bool KnobConnected = false;
const int TIME_WO_MOTION = 90;                          // TEMP 1 minute in seconds
const int TIME_WO_ACK = 90;                             // TEMP 30 seconds

uint32_t value = 0;
static const int LOW_BATTERY = 25;                      // low battery threshold

BlynkTimer timer;                                       // timer for main loop
int numLoop = 0;                                        // for debugging, counting each loop of delayRead

/*************** Sensor ***************/

void SensorDataSend(const int );                        // may not be used
void SensorDataReceive(const std::string);              // sensor data received via JSON
int batterySensor = 100;
int receivedBatterySensor = 100;
bool gasValue = false;
bool motionValue = false;
RTC_DATA_ATTR DateTime lastMotionTime = DateTime();
RTC_DATA_ATTR std::string sensor_text = "Inactive";
bool warning = true;
bool shutoff = false;

/*************** Knob ***************/

void KnobDataSend(const int);                           // to send requests from the app through the hub
void KnobDataReceive(const std::string);                // knob data received via JSON
int  batteryKnob = 100;
int  receivedBatteryKnob = 100;
int  knobRotation = 0;                                  // current rotation value of the knob (25 by default for testing)
int  requestKnobRotation = 0;                           // for sending rotation change requests to knob
bool turnOff = false;                                   // flag for turning off the knob

/*********************** BLE Communication ***********************/

class KnobServerCallbacks: public BLEServerCallbacks {         // callback for knob unit
	void onConnect(BLEServer* pServer) {
		KnobConnected = true;
		BLEDevice::startAdvertising();
	}
	void onDisconnect(BLEServer* pServer) {
		KnobConnected = false;
	}
};

class KnobCallbacks: public BLECharacteristicCallbacks {       // callback for knob characteristic
	void onWrite(BLECharacteristic* pChacracteristic1) {
		std::string KnobSent = pCharacteristicKnob->getValue();
		Serial.println("\nFrom the Knob - Current Position is: ");
		Serial.println(KnobSent.c_str());
		KnobDataReceive(KnobSent);
    if ((sensor_text =="Inactive") && (knobRotation != 0)){lastMotionTime = rtc.now();}
    if (knobRotation == 0)  sensor_text = "Inactive";
     
    // if ((knobRotation != 0)&& (sensor_text != "Warn") && (sensor_text != "Stop"))  {sensor_text ="Active";}
    pCharacteristicSensor -> setValue(sensor_text.c_str());
    if (sensor_text == "Active") {
      warning = true;
      shutoff = false;
    }
    pCharacteristicKnob->setValue(String(knobRotation).c_str());
	}
};

void KnobDataSend(int value) {               // sending rotation requests to knob
  String dataSend = String(value);
  pCharacteristicKnob->setValue (dataSend.c_str());
  pCharacteristicKnob -> notify();
  Serial.print("\nThe Hub has sent to the Knob: ");
  Serial.println(dataSend);
}

void KnobDataReceive(const std::string receivedData) {
  Serial.print("\nData received from Knob");
  String message = receivedData.c_str();
  DynamicJsonDocument doc(200);
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  knobRotation  = doc["knob"];
  receivedBatteryKnob = doc["battery"];
  Serial.println("\nGetting data from the knob");
  Serial.print(knobRotation);
}

class SensorServerCallbacks: public BLEServerCallbacks {       // callback for sensor unit
	void onConnect(BLEServer* pServer) {
		SensorConnected = true;
		BLEDevice::startAdvertising();
	}
	void onDisconnect(BLEServer* pServer) {
		SensorConnected = false;
	}
};

class SensorCallbacks: public BLECharacteristicCallbacks {     // callback for sensor characteristic
	void onWrite(BLECharacteristic* pCharacteristicSensor) {
		std::string SensorSent = pCharacteristicSensor->getValue();
		Serial.println("\nFrom the Sensor: ");
		Serial.println(SensorSent.c_str());
		SensorDataReceive(SensorSent);
    pCharacteristicSensor->setValue(sensor_text);
    Serial.print("Gas: ");
    Serial.println(gasValue);
    Serial.print("Motion: ");
    Serial.println(motionValue);
    Serial.print("Sent: ");
    Serial.println(sensor_text.c_str());
	}
};

void SensorDataSend(const int value) {                                  // may not be used
  int temp = map(value,0,1023,1,50);
  String dataSend = String(temp);
  size_t dataLength = dataSend.length();
  uint8_t dataValue[dataLength+1];                                      // convert it to int char
  dataSend.toCharArray((char *)dataValue, dataLength);                  // send string as char array
  pCharacteristicSensor->setValue (dataSend.c_str());
  pCharacteristicSensor -> notify();
  Serial.print("\nHub has sent to the Sensor");
  Serial.println(dataSend);
}

void SensorDataReceive(const std::string receivedData) {                // receiving data from sensor unit
	Serial.print("\nData received from Sensor");
	String message = receivedData.c_str();
	DynamicJsonDocument doc(200);
	DeserializationError error = deserializeJson(doc, message);
	if (error) {
		Serial.print("deserializeJson() failed: ");
		Serial.println(error.c_str());
		return;
	}
	gasValue = doc["smoke_Digital"];
  
	motionValue = doc["motion_Digital"];
  if (motionValue) lastMotionTime = rtc.now();
	receivedBatterySensor = doc["battery"];
}

void SenRTC(){
 if (knobRotation != 0) {
    DateTime currentTime = rtc.now();
    int ellapsed = (currentTime - lastMotionTime).totalseconds();
    //Serial.print("\nEllapsed time: ");
    //Serial.println(ellapsed);
    if (ellapsed > TIME_WO_MOTION + TIME_WO_ACK){
      if (shutoff == true) Blynk.notify("Alert! 35 Minutes Have Passed Without Motion! Shutting Off");
      shutoff = false;
      sensor_text = "Stop";
      pCharacteristicSensor->setValue(sensor_text);
      KnobDataSend(1);
    } else if (ellapsed > TIME_WO_MOTION) {
      if (warning == true) Blynk.notify("Alert! 30 Minutes Have Passed Without Motion! Knob Will Shut Off In 5 Minutes");
      warning = false;
      shutoff = true;
      sensor_text = "Warn";
      pCharacteristicSensor->setValue(sensor_text);
    } else { sensor_text = "Active"; pCharacteristicSensor->setValue(sensor_text); }
  } else { sensor_text = "Inactive"; pCharacteristicSensor->setValue(sensor_text); }
}

void BLEsetup () {         // all the necessary setup for BLE
	BLEDevice::init("MAGLT_TEST");

	pServer = BLEDevice::createServer();                                    // create BLE server
	pServer->setCallbacks(new KnobServerCallbacks());
	BLEService *pService1 = pServer->createService(SERVICE_UUID_KNOB);      // BLE service for knob

	pCharacteristicKnob = pService1->createCharacteristic(                  // chacteristic for knob
			CHARACTERISTIC_UUID_KNOB,
			BLECharacteristic::PROPERTY_READ   |
			BLECharacteristic::PROPERTY_WRITE  |
			BLECharacteristic::PROPERTY_NOTIFY |
			BLECharacteristic::PROPERTY_INDICATE
			);
	pCharacteristicKnob->setCallbacks(new KnobCallbacks());                 // set up callbacks for knob
	pCharacteristicKnob->addDescriptor(new BLE2902());                      // descriptor
	pService1->start();                                                     // start service for knob

	pServer->setCallbacks(new SensorServerCallbacks());                     // sensor unit callbacks
	BLEService *pService2 = pServer->createService(SERVICE_UUID_SENSOR);    // BLE service for sensor

	pCharacteristicSensor = pService2->createCharacteristic(                // characteristic for sensor
			CHARACTERISTIC_UUID_SENSOR,
			BLECharacteristic::PROPERTY_READ   |
			BLECharacteristic::PROPERTY_WRITE  |
			BLECharacteristic::PROPERTY_NOTIFY |
			BLECharacteristic::PROPERTY_INDICATE
			);
	pCharacteristicSensor->setCallbacks (new SensorCallbacks());
	pCharacteristicSensor->addDescriptor(new BLE2902());                   // descriptor
	pService2->start();                                                    // start service for sensor

	BLEAdvertising *pAdvertising1 = BLEDevice::getAdvertising();           // Knob unit advertising
	pAdvertising1->addServiceUUID(SERVICE_UUID_KNOB);
	pAdvertising1->setScanResponse(false);
	pAdvertising1->setMinPreferred(0x0);
	BLEDevice::startAdvertising();
	Serial.println("\nWaiting for Knob");

	BLEAdvertising *pAdvertising2 = BLEDevice::getAdvertising();           // Sensor unit advertising
	pAdvertising2->addServiceUUID(SERVICE_UUID_SENSOR);
	pAdvertising2->setScanResponse(false);
	pAdvertising2->setMinPreferred(0x0);
	BLEDevice::startAdvertising();
	Serial.println("\nWaiting for Sensor Unit");
}

void rtcSetup(){
  if (!rtc.begin()) {
     Serial.println("Couldn't find RTC");    
  }
  rtc.adjust(DateTime());
}

/*********************** App Functions ***********************/

BLYNK_WRITE(V0) {                 // Turn Off button
	Blynk.virtualWrite(V0, HIGH);
	turnOff = true;
}

BLYNK_WRITE(V1) {                 // Low, Medium, High switch, sends rotation request to knob
	if (knobRotation != 0) {
    int order = param.asInt();
    Serial.println(order);
		switch (order) {
			case 1: {     // LOW
				Serial.println("\nLow selected");
				requestKnobRotation = 25;
				//knobRotation = 25; // TEMP for testing
				KnobDataSend(requestKnobRotation);
				break;
			}
			case 2: {     // MEDIUM
				Serial.println("\nMedium selected");
				requestKnobRotation = 50;
				//knobRotation = 50; // TEMP for testing
				KnobDataSend(requestKnobRotation);
				break;
			}
			case 3: {     // HIGH
				Serial.println("\nHigh selected");
				requestKnobRotation = 100;
				//knobRotation = 100; // TEMP for testing
				KnobDataSend(requestKnobRotation);
				break;
			}
			default: {
				Serial.println("\nNo setting selected");
			}
		}
	}
	else {
		Blynk.notify("Knob is Not Active. Turn Knob Manually to Activate");
	}
}

void turnOffKnob () {             // request for knob to turn off
	if (turnOff == true) {
		requestKnobRotation = 1;
		KnobDataSend(requestKnobRotation);
		//knobRotation = 0; // TEMP
		Serial.println("\nThe hub has requested the knob to turn off");
		turnOff = false;
	}
	// else do nothing
}

void rotationRead () {
	Blynk.virtualWrite(V5, knobRotation);          // writing rotation value received from knob to the gauge in app
	Serial.println("\n\nCurrent rotation: ");
	Serial.print(knobRotation);
	numLoop += 1;                                  // TEMPORARY FOR TESTING, number of loops of delayRead, used for testing
	Serial.println("\nLoop #: ");
	Serial.print(numLoop);
}

void sensorWarning () {     // Smoke warning
	if (gasValue == true) {
		turnOff = true;
		Blynk.notify("Alert! Smoke detected near oven! Shutting Off!");      // app notification for smoke
		gasValue = false;
	}
}

void batteryRead () {
	batteryWarning();
	Blynk.virtualWrite(V2, batteryKnob);         // writing knob battery value to display
	Blynk.virtualWrite(V3, batterySensor);       // writing sensor battery value to display
}

void batteryWarning () {            // sending notifications for low battery, comparing newly received battery values vs current battery
	if (receivedBatteryKnob < batteryKnob && receivedBatteryKnob < LOW_BATTERY) {
		switch (receivedBatteryKnob) {
			case 25: {          // 25% battery warning for knob
				Serial.println("\nKnob Battery at 25%");
				Blynk.notify("Alert! Knob Battery at 25%");
				break;
			}
			case 10: {           // 10% battery warning for knob
				Serial.println("\nKnob Battery at 10%");
				Blynk.notify("Alert! Knob Battery at 10%");
				break;
			}
			case 5: {           // 5% battery warning for knob
				Serial.println("\nKnob Battery at 5%");
				Blynk.notify("Alert! Knob Battery at 5%");
				break;
			}
			case 1: {            // 1% battery warning for knob
				Serial.println("\nKnob Battery at 1%");
				Blynk.notify("Alert! Knob Battery at 1%");
				break;
			}
			default: {
				break;
			}
		}
	}
	batteryKnob = receivedBatteryKnob;      // sets current battery to newly receieved battery after comparisons are done

	if (receivedBatterySensor < batterySensor && receivedBatterySensor < LOW_BATTERY) {
		switch (receivedBatterySensor) {
			case 25: {           // 25% battery warning for sensor
				Serial.println("\nSensor Battery at 25%");
				Blynk.notify("Alert! Sensor Battery at 25%");
				break;
			}
			case 10: {           // 10% battery warning for sensor
				Serial.println("\nSensor Battery at 10%");
				Blynk.notify("Alert! Sensor Battery at 10%");
				break;
			}
			case 5: {         // 5% battery warning for sensor
				Serial.println("\nSensor Battery at 5%");
				Blynk.notify("Alert! Sensor Battery at 5%");
				break;
			}
			case 1: {          // 1% battery warning for sensor
				Serial.println("\nSensor Battery at 1%");
				Blynk.notify("Alert! Sensor Battery at 1%");
				break;
			}
			default: {
				break;
			}
		}
	}
	batterySensor= receivedBatterySensor;
}

void delayRead() {    // this loops every 2 seconds; is where rotation requests will be sent to the knob
	rotationRead();     // reading rotation
	batteryRead();      // reading battery levels
	sensorWarning();    // checks each loop if gasValue is true; if true set turnOff to true; also checking motion values
  SenRTC();
	turnOffKnob();      // checks each loop if turnOff is true; if true then turn off knob
}


/*********************** Setup ***********************/

void setup() {
	delay(100);
	Serial.begin(115200);
  rtcSetup();
  //pinMode(LED, OUTPUT);
	BLEsetup();                              // setting up BLE
	BlynkProvisioning.begin();               // starting Blynk.Inject
	timer.setInterval(2000L, delayRead);     // running delayRead every two seconds for updating and sending values
}


/*********************** Loop ***********************/

void loop() {                              // IMPORTANT: keep loop() as minimal as possible to avoid issues with Blynk.Inject
	BlynkProvisioning.run();
	timer.run();
  //digitalWrite(LED,HIGH);
}
