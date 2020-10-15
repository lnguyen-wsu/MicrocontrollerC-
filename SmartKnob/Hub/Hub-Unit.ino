/*
   BLE Hub server for both Knob and Sensor Unit
   There are couple problems have to figure out :
   1. How the app can connect with that assigned characteristic
   2. function conversion on data
   3. Test to see whether it work on multiple devices as clients
Note:
All services relates to Knob : 1
All services relates to SensorUnit : 2
Project: _ this code is used for the hub, which will use Ble and Wifi to make the connection.
 * Hub plays as the main brain of this subject.
 *  _ incoming data:
 *      _ sensor unit will send the Json data  {"gas":"gas_data","motion":"motion_data"}
 *      _ Knob will send data about knob_pos to the hub
 *      function :
 *      _ Extract input Json data.
 *
 *  _ outcoming data:
 *      _ After getting all data from the sensor unit and knob, the hub create Json data as the format
 * {"knob":"knob_position", "gas":"gas_data","motion":"motion_data", } then send it to Blynk App
 *      function"
 *    _  convert Json data into string
 *
 * ** The app :
 * Two way communication between the app and Hub unit
// Sensor Unit includes Gas detector, buzzer
// App will include data tracking of Gas values, and notifications
// Note for the app : V1,V2,V3 are virtual pins in this summer test
// V1 is used for get smoke value
// V2 is used for get the notification in terminal
// V3 is used for writeback as button to turn off the Knob through send data to Sensor Unit and to KNob
// Wait for Greg to decide : next two Pin for battery Life
 */
// Programmer : Maglt
// Purpose : The hub in Smart Knob Project - Internal/external connection
// This is summer personal project
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>
#include <ArduinoJson.h>
// for Blynk
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>

//********* Declarations Variables************************

// Service for The Knob
#define SERVICE_UUID1        "903eb116-84da-11ea-bc55-0242ac130003"
// Service for the Sensor
#define SERVICE_UUID2        "97920c4e-4b19-45c8-a526-330e5e543886"
// characteristic for Knob
#define CHARACTERISTIC_UUID1 "b9f37d40-c452-11ea-87d0-0242ac130003"
// characteristic for The Sensor
#define CHARACTERISTIC_UUID2 "d6c6b454-c00f-4b1e-b180-1c7d7a983ebe"

//  BLE for both sensor Unit and the knob
BLEServer* pServer = NULL;
// Chracteristic for Knob
BLECharacteristic* pCharacteristic1 = NULL;
// Characteristic for Sensor
BLECharacteristic* pCharacteristic2 = NULL;
bool SenConnected = false;
bool KnobConnected = false;
bool oldKnobConnected = false;
bool oldSenConnected = false;

// For the Blynk
BlynkTimer timer;
const char* authenKey = "Y7haI6NX6wssIJMC50S3-mYoac_SdQje";
const char* magltWifi = "WSU Guest";
const char* passWord = "";
// int smokeValue;

// just for testing variables
int tracking = 0;
int appCommand;   // command get from the App

// global variable
uint32_t value = 0;

// for the sensor Unit
int gas_data = 0;
int motion_data= 0;
int battery_sen = 0;

// for the knob
int knob_position = 0;
int battery_knob =0;


//******************* member functions *************************
// for sensor unit
void Sen_DataSend(const int );                          // this function might not  be used
void Sen_DataReceive(const std::string);                // this one is received under Json type


// for the knob
void Knob_DataSend(const int);                          // this one will send order from the app
void Knob_DataReceive(const std::string);               // this one is received under Json type

// for App
void sendToApp();
void appSetup();
void appCallbacks();
void appOrder(const int );


//************************** All the communication functions***********************************8
// for App
BLYNK_APP_CONNECTED(){
	Serial.println("Blynk is ready for A.S.K");
}
BLYNK_APP_DISCONNECTED(){
	Serial.println("Blynk is not connected");
}

// call back for the Knob
class KnobServerCallbacks: public BLEServerCallbacks {
	void onConnect(BLEServer* pServer) {
		KnobConnected = true;
		BLEDevice::startAdvertising();
	};

	void onDisconnect(BLEServer* pServer) {
		KnobConnected = false;
	};
};
// callbacks for The Knob Characteristic
class KnobCallbacks: public BLECharacteristicCallbacks {
	void onWrite(BLECharacteristic* pChacracteristic1){
		std::string KnobSent = pCharacteristic1->getValue();
		// get data sent from KNob here
		Serial.println("From kNOB : position is-- ");
		Serial.println(KnobSent.c_str());
		Knob_DataReceive(KnobSent);
	};
};
// Call back for the Sensor
class SenServerCallbacks: public BLEServerCallbacks {
	// when it connect
	void onConnect(BLEServer* pServer) {
		SenConnected = true;
		BLEDevice::startAdvertising();
	};
	// when it disconnect
	void onDisconnect(BLEServer* pServer) {
		SenConnected = false;
	}
};

// callbacks for The Sensor Characteristic
class SenCallbacks: public BLECharacteristicCallbacks{
	void onWrite(BLECharacteristic* pCharacteristic2){
		std::string SenSent = pCharacteristic2->getValue();
		// get the Sensor Unit sent data here
		Serial.println("From the Sensor");
		Serial.println(SenSent.c_str());
		Sen_DataReceive(SenSent);
	};
};
// 07/11/20 done for receiving data from the sensor unit and theknob
// break the receiving data function into smaller module to use it better
void setup() {
	Serial.begin(115200);
	// BLE section: Setting up
	// Create the BLE Device
	BLEDevice::init("Maglt");
	// Create the BLE Server
	pServer = BLEDevice::createServer();
	pServer->setCallbacks(new KnobServerCallbacks());
	// Create the BLE Service
	// for The Knob
	BLEService *pService1 = pServer->createService(SERVICE_UUID1);
	// Create a BLE Characteristic
	pCharacteristic1 = pService1->createCharacteristic(
			CHARACTERISTIC_UUID1,
			BLECharacteristic::PROPERTY_READ   |
			BLECharacteristic::PROPERTY_WRITE  |
			BLECharacteristic::PROPERTY_NOTIFY |
			BLECharacteristic::PROPERTY_INDICATE
			);
	pCharacteristic1 ->setCallbacks(new KnobCallbacks());
	// Create a BLE Descriptor
	pCharacteristic1->addDescriptor(new BLE2902());
	// Start the service
	pService1->start();            // for The KNob
	// sensor unit settings about ble
	pServer->setCallbacks(new SenServerCallbacks());
	// Create the BLE Service
	// for App
	BLEService *pService2 = pServer->createService(SERVICE_UUID2);
	// Create a BLE Characteristic
	pCharacteristic2 = pService2->createCharacteristic(
			CHARACTERISTIC_UUID2,
			BLECharacteristic::PROPERTY_READ   |
			BLECharacteristic::PROPERTY_WRITE  |
			BLECharacteristic::PROPERTY_NOTIFY |
			BLECharacteristic::PROPERTY_INDICATE
			);

	pCharacteristic2 ->setCallbacks (new SenCallbacks());
	// Create a BLE Descriptor
	pCharacteristic2->addDescriptor(new BLE2902());
	// Start the service
	pService2->start();            // for Sen
	// Knob_Advertising()
	BLEAdvertising *pAdvertising1 = BLEDevice::getAdvertising();
	pAdvertising1->addServiceUUID(SERVICE_UUID1);
	pAdvertising1->setScanResponse(false);
	pAdvertising1->setMinPreferred(0x0);
	BLEDevice::startAdvertising();
	Serial.println("Wait for Knob");
	// Sen_Advertising();
	BLEAdvertising *pAdvertising2 = BLEDevice::getAdvertising();
	pAdvertising2->addServiceUUID(SERVICE_UUID2);
	pAdvertising2->setScanResponse(false);
	pAdvertising2->setMinPreferred(0x0);
	BLEDevice::startAdvertising();
	Serial.println("Wait for Sensor Unit");
	// for the BLYNK settings up
	appSetup();
}

//************************************** this middle section will be used for the writeback from the APP ********************8
//c=> as the command from the remote users
BLYNK_WRITE(V3){
	appCommand = param.asInt();
	Serial.println("command is");
	Serial.println(appCommand);
	appOrder(appCommand);

}
// This one is used to give the interatcion with the App (automatically)
BLYNK_WRITE(V2){
	// create some interation happens here
	String appTalk = param.asStr();
	if (appTalk=="Position" or appTalk=="position"){
		Blynk.virtualWrite(V2,"Position is "+ String(knob_position));
	}else if (appTalk=="status" or appTalk=="Status"){
		if(knob_position>0){
			Blynk.virtualWrite(V2,"Status is ON ");
		}else{
			Blynk.virtualWrite(V2,"Status is OFF");
		}
	}else{
		Blynk.virtualWrite(V2,"Yes,Master.");
	}

}
// Need another 2 functions to put the write back battery for Sensor and Knob back the App




void loop() {
	// BLYNK update
	// App activation
	Blynk.run();
	timer.run();
	// will be called as member function later
}
//****************Implementation for all the member functions*******************************
void Sen_DataSend(const int value){

	// need to ask Andrew about range but assume it is 1023
	int temp = map(value,0,1023,1,50);
	String dataSend = String(temp);                                       // base 10 Asc2 key
	// trying convert into char* => not very works
	size_t dataLength = dataSend.length();
	uint8_t dataValue[dataLength+1];                                      // convert it to int char
	dataSend.toCharArray((char *)dataValue, dataLength);                  // send string as char array
	// Successful by using C_string
	pCharacteristic2->setValue (dataSend.c_str());
	pCharacteristic2 -> notify();
	Serial.print("Sensor has sent to the Hub");
	Serial.println(dataSend);
}

void Sen_DataReceive(const std::string receivedData){
	Serial.print("Data received");
	// we can get data in here
	String message = receivedData.c_str();
	// *************Extract Json data types***********************
	DynamicJsonDocument doc(200);
	DeserializationError error = deserializeJson(doc, message);
	if (error) {
		Serial.print("deserializeJson() failed: ");
		Serial.println(error.c_str());
		return;
	}
	//"gas":"gas_data","motion":"motion_data","battery":"percent" from the sensor unit
	gas_data = doc["gas"];
	motion_data = doc["motion"];
	battery_sen = doc["battery"];
}

void Knob_DataSend(int value){
  	String dataSend = String(value);                                       // base 10 Asc2 key 
   	// Successful by using C_string
   	pCharacteristic1->setValue (dataSend.c_str());
	pCharacteristic1 -> notify();
	Serial.print("hub has sent to Knob ::");
	Serial.println(dataSend);
}

void Knob_DataReceive(const std::string receivedData){
	Serial.print("Data received from Knob");
	String message = receivedData.c_str();
	// we can get data in here
	// *************Extract Json data types***********************
	DynamicJsonDocument doc(200);
	DeserializationError error = deserializeJson(doc, message);
	if (error) {
		Serial.print("deserializeJson() failed: ");
		Serial.println(error.c_str());
		return;
	}
	//"knob":"pos","battery":"percent" from the the knob
	knob_position  = doc["knob"];
	battery_knob = doc["battery"];
}

// BLynk member function sections
// for App
void appSetup(){
	Blynk.begin(authenKey,magltWifi, passWord);
	timer.setInterval(1000L,appCallbacks);
}
void appCallbacks(){
	// pick V1 just for temporary
	Blynk.virtualWrite(V1, gas_data);
	// Write notification to the APP => have bugs
	/*
	   if (smokeValue > 2500){
	   Blynk.virtualWrite(V2," Smoke sensor has been activated at " + smokeValue);
	   }*/
}
void appOrder(const int command){
	if (command == 0){              // this one is killing me for one day
		Knob_DataSend(40);         // Tony want to test with data 20 instead of 0
		Serial.println(" after getting OFF from the App");
		Serial.println("");
		delayMicroseconds(20000);
		gas_data = 0;
		motion_data = 0;
		appCommand = 1;
	}
}
