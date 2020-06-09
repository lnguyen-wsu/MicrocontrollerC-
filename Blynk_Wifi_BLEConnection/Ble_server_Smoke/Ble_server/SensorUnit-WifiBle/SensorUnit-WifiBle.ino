// Project: Smart Knob 
// Purpose: This is start over project by Luan Nguyen as individual for whole project's summer 
// only for summer so that I can test some points of communication
// 1. WIfi and BLE at the same time
// 2. Data between App and Sensor Unit
// 3. Connect with the Knob
// 4. connect with the App
// 5. Two way communication between the app and Sensor unit
// Sensor Unit includes Gas detector, buzzer
// App will include data tracking of Gas values, and notifications
// Note for the app : V1,V2,V3 are virtual pins in this summer test
// V1 is used for get smoke value
// V2 is used for get the notification in terminal 
// V3 is used for writeback as button to turn off the Knob through send data to Sensor Unit and to KNob 
// Programmer : Luan Nguyen

// For BLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#define SERVICE_UUID        "903eb116-84da-11ea-bc55-0242ac130003"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// For Wifi 
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

//int turn = 0;
int appCommand;   // command get from the App 
// Variables for BLE
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

// Variables for Smoke sensor 
const int smokePin = 34;
const int smokeLed = 2;

// For the App => should out the credential into different .h 
BlynkTimer timer;
const char* authenKey = "Y7haI6NX6wssIJMC50S3-mYoac_SdQje";
const char* magltWifi = "ATT2NP4529";
const char* passWord = "36k3bs73a62w";
int smokeValue;

// Variable for KNob coming data
int knobPosition;

// Member functions Declaration
// for BLE
void Ble_setup();
void Ble_send(const String);
void Ble_reconnect();

// for Wifi

// for Sensor

// for App
void appSetup();
void appCallbacks();
void appOrder(const int );
// writeback data to the app



// All the communication functions
// for App
BLYNK_APP_CONNECTED(){
  Serial.println("App is connected");
}
BLYNK_APP_DISCONNECTED(){
  Serial.println("App is disconnected");
}

// for BLE
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyReceivingCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pKnob){
    std::string knobData = pKnob -> getValue();
    /*
    Serial.println("Knob position is : ");
    for ( int i = 0; i < knobData.length();i++){
      Serial.print(knobData[i]);
    }
    */
    Serial.println();
    // we need convert function here
    knobPosition = atoi(knobData.c_str());
    Serial.println("Knob position is ");
    Serial.print(knobData.c_str());
    Serial.println();
  }
};

void setup() {
  Serial.begin(115200);
  // Set up for BLE
  Ble_setup();
  // Set up for Wifi

  // set up for App
  appSetup();
  
}
// this middle section will be used for the writeback from the APP => as the command from the remote users
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
    Blynk.virtualWrite(V2,"Position is "+ String(knobPosition));
  }else if (appTalk=="status" or appTalk=="Status"){
    if(knobPosition>0){
      Blynk.virtualWrite(V2,"Status is ON "); 
    }else{
      Blynk.virtualWrite(V2,"Status is OFF"); 
    }
  }else{
    Blynk.virtualWrite(V2,"Yes,Master.");
  }
  
}
void loop() {
    // App activation
    Blynk.run();
    timer.run();
    
       // App will get data from here and update to the app 
    smokeValue= analogRead(34);
    // Sensor Unit will send to Knob here
   
    if (deviceConnected) {
      // here will be waited for testing
      /*
        if (appCommand = 0){
          Ble_send("0");
          Serial.println("Sensor Units send 0 position");
          Serial.println(" after getting OFF from the App");
          Serial.println("");
          delayMicroseconds(20000);
          smokeValue = 0;
          appCommand = 1;
      }*/
      
        if (smokeValue > 3000){
          Ble_send("0"); // send to the KNob
          Serial.println("Sensor Units automatic send 0 position");
          Serial.println(" after get the data from the sensor ");
          Serial.println("");
          delay(1000); 
          // reset the smoke value again 
          smokeValue = 0; 
        }      
    }
    
    Ble_reconnect();
    
    
    // Let consider about how many delay should be in here
 //*** NOte for this test : It has been succesffully for both wifi and BLE work together
 // Require to put more delay time for between send to knob and app
    
}

//*********** Member functions Implementation

// for BLE

  //set up
void Ble_setup(){
  // Create the BLE Device
  BLEDevice::init("Maglt");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  // callbacks to get data
  pCharacteristic ->setCallbacks(new MyReceivingCallbacks());
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());
  BLEDescriptor VariableDescriptor(BLEUUID((uint16_t)0x2901));
  VariableDescriptor.setValue("It is from MAGLT");
  pCharacteristic->addDescriptor(&VariableDescriptor);

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a Knob connection to notify...");
  
}

  // Ble send to Knob
void Ble_send (const String positionKnob){
  
  //String result = String(millis()% 41);
  //pCharacteristic -> setValue (result.c_str());
  pCharacteristic ->setValue(positionKnob.c_str());
  //pCharacteristic->setValue((uint8_t*)&value, 4);           // send as characters
  pCharacteristic->notify();                                  // notify to client
}
void Ble_reconnect(){
  // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
// for Wifi

// for Sensor

// for App
void appSetup(){
  Blynk.begin(authenKey,magltWifi, passWord);
  timer.setInterval(1000L,appCallbacks);
}
void appCallbacks(){
  // pick V1 just for temporary
  Blynk.virtualWrite(V1, smokeValue);
  // Write notification to the APP => have bugs 
  /*
  if (smokeValue > 2500){
    Blynk.virtualWrite(V2," Smoke sensor has been activated at " + smokeValue);
  }*/
}
void appOrder(const int command){
  if (command == 0){              // this one is killing me for one day
        Ble_send("0");
        Serial.println("Sensor Units send 0 position");
        Serial.println(" after getting OFF from the App");
        Serial.println("");
        delayMicroseconds(20000);
        smokeValue = 0;
        appCommand = 1;
      }
}
