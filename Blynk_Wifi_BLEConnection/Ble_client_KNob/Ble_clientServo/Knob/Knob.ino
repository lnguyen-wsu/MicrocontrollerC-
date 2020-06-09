/* This it my individual summer reseach by Luan Nguyen
 *  Purpose : 
 *  1/ This test is used for showing about working concept
 *     Knob will get command from the Sensor unit from the App
 *     It will come back to O position 
 *  2/ Test about BLE and Wifi can work together
 *  3/ test about how reliable in the system
 *         
 *  Summer project : Working A.S.K concept
 * 
 */
//Programmer: Luan Nguyen
//Purpose : This knob will get data from sensor Unit as BLE connection to turn off the servo motor 
//BLe
#include "BLEDevice.h"
#include <BLEAdvertisedDevice.h>
//Servo 
#include<Servo.h>


// The remote service we wish to connect to.
static BLEUUID serviceUUID("903eb116-84da-11ea-bc55-0242ac130003");
static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
// Variable for BLE and Knob
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
int knobIndicator = 0;
// Variables for Servo
static const int servoPin = 4;
Servo Knob;
// Member functions declarations
// For Ble
void bleSetup();
void knobPosition(const int );


// For Servo
void servoSetup();
void servoAppOn();           // turn on from App
void servoGetHome();         // turn off from App 


static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Data from Sensor Unit........ ");
    //Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    
    Serial.println((char*)pData);
    /// stuck in here to get the incomming data
    // get data 
/*
    std::string value = pBLERemoteCharacteristic->readValue();
    Serial.println("value is..");
    int finalValue = atoi(value.c_str());
    Serial.println(finalValue);
    if (finalValue == 0){
      servoGetHome();
    }
  */  
    
    
    /*else{
      servoAppOn();
    }*/
    
    
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    connected = true;
    Serial.print("Luan");
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
      // set condition to come home
      /*
      int finalValue = atoi(value.c_str());
      if (finalValue == 0){
          servoGetHome();
      }*/
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
    bleSetup();
    servoSetup();
    servoAppOn();
    
} // End of setup.


void loop() {
  
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("This is after Server has been pull off ............");
    }
    doConnect = false;
  }
  /*
  if (knobIndicator < 180){
    servoAppOn();
  }*/
  knobPosition(knobIndicator);
  
  if (connected && pRemoteCharacteristic->canRead()){
    std::string value = pRemoteCharacteristic->readValue();
    int finalValue = atoi(value.c_str());
    Serial.println("Knob Current position is .. ");
    Serial.println(finalValue);
    if (finalValue == 0){
      servoGetHome();
      delay(10000);
    }
    
  }
  
  delay(1000); // Delay a second between loops.
 // End of loop
}

// Member functions implementation
// For BLE
void bleSetup(){
  Serial.begin(115200);
  Serial.println("Knob as Client");
  BLEDevice::init("");

  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void knobPosition(const int whereKnob){
  // convert whereknob into String newValue
  // Below will be send the home position back to Sensor Units
  if (connected) {
    //String newValue = String(millis()% 41);
    String newValue = String(whereKnob);
    //String newValue = int 
    pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
    delay(1000);
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just Sample to start scan after disconnect, most likely there is better way to do it in arduino
  }
}
// For Servo
void servoSetup(){
    Knob.attach(
        servoPin, 
        Servo::CHANNEL_NOT_ATTACHED, 
        45,
        120
    );
}
void servoAppOn(){
  for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
        Knob.write(posDegrees);
        Serial.println(posDegrees);
        delay(20);
    }
  Serial.println("On");
  knobIndicator = 180;
  // need to send back to Sensor UNits => need to modify , it is only for testing 
  //knobPosition(knobIndicator);
}
void servoGetHome(){
  for(int posDegrees = knobIndicator; posDegrees >= 0; posDegrees--) {
        Knob.write(posDegrees);
        Serial.println(posDegrees);
        delay(20);
    }
  Serial.println("Knob is Off by App's Order");
  knobIndicator = 0;
  // Send back to Sensor UNit about its home position => need to modify 
  if (connected) {
    String newValue = "0";
    //String newValue = int 
    pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just Sample to start scan after disconnect, most likely there is better way to do it in arduino
  }
}
