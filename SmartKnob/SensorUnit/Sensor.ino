// Project: Sensor unit in the smart knob system. This is my summer personal project
// Programmer : Luan Nguyen 
// Function: sensor unit will send the Json data  { "gas":"gas_data","motion":"motion_data"}
/*
Note for Team:
1. Need motion sensor.
2. LED indication 
3. power saving
4. Testing to figure out about data of smoke detector 
*/
#include "BLEDevice.h"
#include <BLEAdvertisedDevice.h>
//#include <Pangodream_18650_CL.h>               // Library for battery in esp32

using namespace std;
static BLEUUID serviceUUID("97920c4e-4b19-45c8-a526-330e5e543886");
static BLEUUID    charUUID("d6c6b454-c00f-4b1e-b180-1c7d7a983ebe");
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

// Global variables
#define CONV_FACTOR 1.7
#define READS 20
const int smokePin = 34;
const int motionPin = 0;    // still wait for pin for motion sensor
const int batteryPin = 4; 
int gas_data = 0;
int motion_data = 0;
// Battery section
int battery_life = 0;
float voltage;
float perc;


//Pangodream_18650_CL SenBattery (batteryPin, CONV_FACTOR, READS);
// member function 
void battery_Check();


static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("at the Sensor unit, from the hub");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    connected = true;
    Serial.print("Connected");
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
}
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());
    
    // We have found a device, let us now see if it contains the service we are looking for.
    // if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      if(advertisedDevice.haveName()){
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
      Serial.println("On Result");
    } // Found our server
  } // onResult
  void onRead (BLEAdvertisedDevice advertisedDevice){
    Serial.println("Luan");
    doConnect = true;
    doScan = true;
  }
}; // MyAdvertisedDeviceCallbacks


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.


// This is the Arduino main loop function.
void loop() {
   gas_data = analogRead(smokePin);
   motion_data = analogRead(motionPin);
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the Hub.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
// Battery Life
 battery_Check();
// connected to hub, time to shine
  if (connected) {    
    
    String JSON_Data = "{\"gas\":";
         JSON_Data += gas_data;
         JSON_Data += ",\"motion\":";
         JSON_Data += motion_data;
         JSON_Data += ",\"battery\":";
         JSON_Data += perc;
         JSON_Data += "}";
     // send by JSON data    
    pRemoteCharacteristic->writeValue(JSON_Data.c_str());    
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
  }
  
  delay(1000); // Delay a second between loops.
} // End of loop


void battery_Check(){
// Min = 4V max = 6.2V
  battery_life = analogRead(batteryPin);
  Serial.print("Battery Life");
  Serial.println(battery_life);
  voltage = battery_life * 6.2/4095;
  Serial.print("voltage");
  Serial.println(voltage);
  perc = (voltage - 4) * 100 / 2.2;
  Serial.print("Percent");
  Serial.println(perc);
}
