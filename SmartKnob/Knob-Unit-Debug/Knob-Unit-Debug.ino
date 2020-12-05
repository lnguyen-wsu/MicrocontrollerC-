// Engineered by Tony Nguyen
// BLE Library obtained through the MIT license

#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>
//#include <TinyPICO.h>
//TinyPICO tp = TinyPICO();
//#include <TB6612FNG.h>

#define IN1 34
#define IN2 35
#define PWM 38
#define STBY 36
//Tb6612fng motor(STBY, IN1, IN2, PWM);

#define DOTSTAR_PWR 13
#define DOTSTAR_DATA 2
#define DOTSTAR_CLK 12

#define BAT_VOLTAGE 35

#define MOTOR1A 16
#define MOTOR2A 17

#define SETLOW 12
#define SETMED 14
#define SETHIGH 27

static BLEUUID serviceUUID("903eb116-84da-11ea-bc55-0242ac130003");
static BLEUUID    charUUID("b9f37d40-c452-11ea-87d0-0242ac130003");
static BLEAddress *pServerAddress;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
static boolean connected = false;
static const long convert = 1000000;

RTC_DATA_ATTR static int position = 10;
RTC_DATA_ATTR static int value = 0;
RTC_DATA_ATTR static int bootCount = 0;
RTC_DATA_ATTR static int bleFailure = 0;

void moveToHome();
void sleepTimer(int);
void triggerEvent(int);
String sendToServer(int,int);
void moveTo(int);
int checkPos();
int checkBattery();

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
	void onResult(BLEAdvertisedDevice advertisedDevice) {
		if (advertisedDevice.haveName()) {
			advertisedDevice.getScan()->stop();
			pServerAddress = new BLEAddress(advertisedDevice.getAddress());
			connected = true;
		}
	}
};

bool connectToServer(BLEAddress pAddress) {
	BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
	BLEClient* pClient = BLEDevice::createClient();
	Serial.println(" - Created client");
	pClient->connect(pAddress);
	Serial.println(" - Connected to server");
	if(!pClient->isConnected()) return false;
	BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
	if (pRemoteService == nullptr) return false;
	Serial.println(" - Found our service");
	pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
	if (pRemoteCharacteristic == nullptr) return false;
	Serial.println(" - Found our characteristic");
	return true;
}

void setup() {
	setCpuFrequencyMhz(80);
	Serial.begin(115200);
//	tp.DotStar_Clear();
//	tp.DotStar_SetPower(false);
	bootCount ++;
	Serial.println("Boot: " + String(bootCount));

	BLEDevice::init("KNOB");
	BLEScan* pBLEScan = BLEDevice::getScan();
	pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
	pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
	pBLEScan->setActiveScan(true);
	pBLEScan->start(10);

	// START OF MAIN

	if (connected && connectToServer(*pServerAddress)) {
		Serial.println("Bluetooth connected!!!");
		bleFailure = 0; 
    std::string text ="";
		text = pRemoteCharacteristic->readValue();
    Serial.println("Data got from the hub");
    Serial.print(text.c_str());
    int num = atoi(text.c_str());

		if (checkPos() != position || num != value) {
			//motor.begin();
			triggerEvent(num);
			//sendToServer(20,10);
      pRemoteCharacteristic->writeValue(sendToServer(20,10).c_str(), sendToServer(20,10).length());
			position = checkPos();
			value = num;
		}
		if (bootCount > 0 && bootCount < 10) sleepTimer(5); else sleepTimer(5);
	} else {
		bleFailure ++;
		if (bleFailure <= 3) {
			Serial.println("BLE failure #" + String(bleFailure));
			sleepTimer(1);
		}
		else {
			Serial.println("Turning off Oven"); //moveToHome();
			sleepTimer(5);
		}
	}
	esp_deep_sleep_start();

	// END OF MAIN
}

void loop() {pRemoteCharacteristic->writeValue(sendToServer(20,10).c_str());}

//int checkBattery() { return round((tp.GetBatteryVoltage() / 3.7) * 100); }

void sleepTimer(int time) { esp_sleep_enable_timer_wakeup(time * convert); }

void moveToHome() {
	//while (true) { motor.drive(-0.5, 500); }
//	motor.brake();
}

String sendToServer(int batt, int pos) {
	String JSON_Data = "{\"knob\":";
	JSON_Data += pos;
	JSON_Data += ",\"battery\":";
	JSON_Data += batt;
	JSON_Data += "}";
  return JSON_Data;
}

int checkPos() {
	pinMode(SETLOW, INPUT);
	pinMode(SETMED, INPUT);
	pinMode(SETHIGH, INPUT);
	if (!digitalRead(SETLOW)) return 25;
	else if (!digitalRead(SETMED)) return 50;
	else if (!digitalRead(SETHIGH)) return 100;
	else return 0;
}

void triggerEvent(int event) {
	switch(event) {
		case 0:
			Serial.println("Turning off");
			//moveToHome();
			break;
		case 25:
			Serial.println("Setting to HIGH");
			/*
			   while (!digitalRead(SETLOW)) {
			   if (checkPos() > 25) motor.drive(-0.5, 500);
			   if (checkPos() < 25) motor.drive(0.5, 500);
			   }
			   motor.brake();
			 */
			break;
		case 50:
			Serial.println("setting to MED");
			/*
			   while (!digitalRead(SETMED)) {
			   if (checkPos() > 50) motor.drive(-0.5, 500);
			   if (checkPos() < 50) motor.drive(0.5, 500);
			   }
			   motor.brake();
			 */
			break;
		case 100:
			Serial.println("setting to LOW");
			/*
			   while (!digitalRead(SETHIGH)) {
			   if (checkPos() > 100) motor.drive(-0.5, 500);
			   if (checkPos() < 100) motor.drive(0.5, 500);
			   }
			   motor.brake();
			 */
			break;
		default:
			Serial.println("No value recognised");
			//pRemoteCharacteristic->writeValue("NULL");
			break;
	}
}
