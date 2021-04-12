/*
  This is test about how to display content in I2C
  mapping pins:
  pin 21 = SDA
  pin 22 = SCL
  5v = VCC
  GND = GND
  Please : Check the attach link 
  https://randomnerdtutorials.com/esp32-esp8266-i2c-lcd-arduino-ide/
  
*/



/********************************
// version code 1
// this version is working also
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
  
String messageToScroll = "void loop Robotech & Automation";

void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i=0; i < lcdColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}
void setup(){

  lcd.init();     // initialize LCD 
  //lcd.clear();                
  lcd.backlight();  // turn on Backlight
}
void loop(){

  lcd.setCursor(5, 0);
  lcd.print("WELCOME");
  scrollText(1, messageToScroll, 250, 16);
}
*****************/



// Version 2 :this below version is working 
/*
#include <LiquidCrystal_I2C.h>

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

void setup(){
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
}

void loop(){
  // set cursor to first column, first row
  lcd.setCursor(0, 0);
  // print message
  lcd.print("Hello, World!");
  delay(1000);
  // clears the display to print new message
  lcd.clear();
  // set cursor to first column, second row
  lcd.setCursor(0,1);
  lcd.print("Hello, World!");
  delay(1000);
  lcd.clear(); 
}*/
