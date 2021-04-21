/* How to connect: 
 *  LRC - 25
 *  din-22
 *  blck - 26
 * gnd -gnd
 * vin
 */
#include "AudioGeneratorAAC.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourcePROGMEM.h"
#include "sampleaac.h"

AudioFileSourcePROGMEM *in;
AudioGeneratorAAC *aac;
AudioOutputI2S *out;
void makeSound(){
  if (aac->isRunning()) {aac->loop();}
}
void setup(){
  Serial.begin(115200);

  in = new AudioFileSourcePROGMEM(sampleaac, sizeof(sampleaac));
  aac = new AudioGeneratorAAC();
  out = new AudioOutputI2S();
  out -> SetGain(0.125);
   // out -> SetPinout(26,25,22);   // this is based on recommened connection as the head
  out -> SetPinout(32,25,33);   // BLck -32,lrc-25, din- 33
  aac->begin(in, out);
}

void loop(){
  if (aac->isRunning()) {
    aac->loop();
  } else {
    aac -> stop();
    Serial.printf("Sound Generator\n");
    delay(1000);
  }
}
