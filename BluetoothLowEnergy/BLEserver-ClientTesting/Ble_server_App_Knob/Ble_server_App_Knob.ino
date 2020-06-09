 /*
 BLE server for both Knob and The App
 There are couple problems have to figure out :
     1. How the app can connect with that assigned characteristic 
     2. function conversion on data 
     3. Test to see whether it work on multiple devices
 Note: 
 All services relates to Knob : 1
 All services relates to App : 2  
*/

// Programmer : Luan Nguyen 
// Purpose : Smart Knob - Internal connection 

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
// Chracteristic for Knob
BLECharacteristic* pCharacteristic1 = NULL;
// Characteristic for The app 
BLECharacteristic* pCharacteristic2 = NULL;
bool AppConnected = false;
bool KnobConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

// member function   =>>>> need to be done 
void App_DataSend(const int );
void App_Datareceive(const std::string);

// this function will take raw input from Sensor Unit and map it into 1->40 range
// then send it into client KNob 
void Knob_DataSend(const int);
void Knob_DataReceive(const std::string);
int StringToInt(const std::string);                     // convert String to Int but same value (not done yet)

void Knob_Advertising();
void App_Advertising();

void App_Setup();
void Knob_Setup();

// Main server 

// Service for The Knob   =>>> will change Service UUID at testing phase
#define SERVICE_UUID1        "903eb116-84da-11ea-bc55-0242ac130003"
// Service for the App 
#define SERVICE_UUID2        "903eb116-84da-11ea-bc55-0242ac130003"
// characteristic for KNob
#define CHARACTERISTIC_UUID1 "beb5483e-36e1-4688-b7f5-ea07361b26a8"
// characteristic for The app 
#define CHARACTERISTIC_UUID2 "beb5483e-36e1-4688-b7f5-ea07361b26a8"

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
    Serial.println(KnobSent.c_str());    
  };
};
// Call back for the App
class AppServerCallbacks: public BLEServerCallbacks {
     // when it connect
    void onConnect(BLEServer* pServer) {
      AppConnected = true;
      BLEDevice::startAdvertising();
    };
     // when it disconnect 
    void onDisconnect(BLEServer* pServer) {
      AppConnected = false;
    }    
};

// callbacks for The App Characteristic
class AppCallbacks: public BLECharacteristicCallbacks{
  void onWrite(BLECharacteristic* pCharacteristic2){
    std::string AppSent = pCharacteristic2->getValue();
    // get the App sent data here
    Serial.println(AppSent.c_str());
  };
};
void setup() {
  // Create the BLE Device
  BLEDevice::init("Maglt");
  // Set up service
  App_Setup();
  Knob_Setup();
  App_Advertising();
  Knob_Advertising();
}

void loop() {
    // will be called as member function later 
}
void App_DataSend(const int value){
  
  // need to ask Andrew about range but assume it is 1023
  int temp = map(temp,0,1023,1,50);
  String dataSend = String(temp);                                       // base 10 Asc2 key
  
  // trying convert into char* => not very works
  size_t dataLength = dataSend.length();            
  uint8_t dataValue[dataLength+1];                                      // convert it to int char
  dataSend.toCharArray((char *)dataValue, dataLength);                  // send string as char array 
      
      
   // Successful by using C_string
   pCharacteristic2->setValue (dataSend.c_str());
   pCharacteristic2 -> notify();
   Serial.println("Sensor has sent to App");
   Serial.print(dataSend);  
}

void App_Datareceive(const std::string receivedData){
  Serial.print("Data received from app");
  // we can get data in here 
  // data will be string type and from 0->49 from APP
  // will put StringToInt(receivedData) in here
}

void Knob_DataSend(const int value){
  // need to ask Andrew about range but assume it is 1023
  int temp = map(temp,0,1023,1,40);
  String dataSend = String(temp);                                       // base 10 Asc2 key
  
  // trying convert into char* => not very works
  size_t dataLength = dataSend.length();            
  uint8_t dataValue[dataLength+1];                                      // convert it to int char
  dataSend.toCharArray((char *)dataValue, dataLength);                  // send string as char array 
      
      
   // Successful by using C_string
   pCharacteristic1->setValue (dataSend.c_str());
   pCharacteristic1 -> notify();
   Serial.println("Sensor has sent to Knob");
   Serial.print(dataSend);
}

void Knob_DataReceive(const std::string receivedData){
  Serial.print("Data received from Knob");
  // we can get data in here 
  // data will be string type and from 0->39 from Knob
  // will put StringToInt(receivedData) in here
}

void Knob_Advertising(){
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID1);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);                               
  BLEDevice::startAdvertising();
  Serial.println("Wait for Knob");
}

void App_Advertising(){
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID2);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);                               
  BLEDevice::startAdvertising();
  Serial.println("Wait for App");
}

void Knob_Setup(){
  Serial.begin(115200);

  // Create the BLE Device
  // BLEDevice::init("Maglt");

  // Create the BLE Server
  // pServer = BLEDevice::createServer();
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
  // Create callback for Knob writeback to Sensor Unit
  BLECharacteristic* pDataCharacteristic1 = pService1->createCharacteristic(
                      CHARACTERISTIC_UUID1,
                      BLECharacteristic::PROPERTY_WRITE
                    );
  // define the KnobData callback to get that characteristics
  pDataCharacteristic1 ->setCallbacks(new KnobCallbacks());
  // Create a BLE Descriptor
  pCharacteristic1->addDescriptor(new BLE2902());
  
  // Start the service
  pService1->start();            // for The KNob
 
}
void App_Setup(){
  Serial.begin(115200);

  // Create the BLE Device
  // BLEDevice::init("Maglt");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new AppServerCallbacks());

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
  // Define callback so that Sensor Unit can receive the sentData from the APP
  BLECharacteristic* pDataCharacteristic2 = pService2->createCharacteristic(
                      CHARACTERISTIC_UUID2,
                      BLECharacteristic::PROPERTY_WRITE
                      );
  // define the APP DATA callback to get that characteristics
  pDataCharacteristic2 ->setCallbacks (new AppCallbacks());
  // Create a BLE Descriptor
  pCharacteristic2->addDescriptor(new BLE2902());
  
  
  // Start the service
  pService2->start();            // for App 
}
