#include "outputs.h"
#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "Adafruit_MCP23017.h"

TwoWire extenders_I2C =  TwoWire(1);
Adafruit_MCP23017 ext1;
Adafruit_MCP23017 ext2;
Adafruit_MCP23017 ext3;
Adafruit_MCP23017 ext4;

void extendersInit(){
  extenders_I2C.begin(EXTENDERS_I2C_SDA, EXTENDERS_I2C_SCL, 100000);
  ext1.begin(ADDRESS_EXT1, &extenders_I2C);
  ext2.begin(ADDRESS_EXT2, &extenders_I2C);
  ext3.begin(ADDRESS_EXT3, &extenders_I2C);
  ext4.begin(ADDRESS_EXT4, &extenders_I2C);

  //TODO SET ALL OUTPUTs LOW

  for(int i =0;i<16;i++){ 
    ext1.pinMode(i, OUTPUT);
    Serial.println(i);}
  for(int i =0;i<16;i++){
    ext2.pinMode(i, OUTPUT);
    Serial.println(i);}
  for(int i =0;i<16;i++){
    ext3.pinMode(i, OUTPUT);
    Serial.println(i);}
  for(int i =0;i<16;i++){
    ext4.pinMode(i, OUTPUT);
    Serial.println(i);}
  }

void setOutput(uint8_t outputNumber, uint8_t outputValue){
    #ifdef SERIALDEBUG
    Serial.print("output ");
    Serial.print(outputNumber);
    Serial.print(" to ");
    Serial.println(outputValue); 
    #endif

    if (outputNumber < 8)
        ext1.digitalWrite(8-outputNumber,outputValue);
    else if(outputNumber < 16)
        ext1.digitalWrite(outputNumber-16,outputValue);

    else if(outputNumber < 24)
        ext2.digitalWrite(8-(outputNumber-16),outputValue);
    else if(outputNumber < 32)
        ext2.digitalWrite(outputNumber-16,outputValue);

    else if(outputNumber < 40)
        ext3.digitalWrite(8-(outputNumber-32),outputValue);
    else if(outputNumber < 48)
        ext3.digitalWrite(outputNumber-32,outputValue);

    else if(outputNumber < 56)
        ext4.digitalWrite(8-(outputNumber-48),outputValue);
    else if(outputNumber < 64)
        ext4.digitalWrite(outputNumber-48,outputValue);
}