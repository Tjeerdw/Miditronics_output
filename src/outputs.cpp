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

void extendersI2Cinit(){
   extenders_I2C.begin(EXTENDERS_I2C_SDA, EXTENDERS_I2C_SCL, 100000);
}

uint8_t extendersCount(){
    byte error, address;
    int nDevices = 0;
    for(address = 1; address < 127; address++ ) {
        extenders_I2C.beginTransmission(address);
        error = extenders_I2C.endTransmission();
        if (error == 0) {
            nDevices++;
        }
    }
    return nDevices;
}


void extendersInit(uint8_t channelNumbers, uint8_t isThisAnOutputModule){
    
    uint8_t IOsetting = INPUT; //default to input
    if (isThisAnOutputModule){
        IOsetting = OUTPUT;}
    
    if(channelNumbers>=32){
        ext1.begin(ADDRESS_EXT1, &extenders_I2C);
        ext2.begin(ADDRESS_EXT2, &extenders_I2C);
        for(int i =0;i<16;i++){ 
            ext1.pinMode(i, IOsetting);
            if (!isThisAnOutputModule){
                ext1.pullUp(i,HIGH);
            }
        }
        for(int i =0;i<16;i++){
            ext2.pinMode(i, IOsetting);
            if (!isThisAnOutputModule){
                ext2.pullUp(i,HIGH);
            }
        }
    }
    if(channelNumbers>=64){
        ext3.begin(ADDRESS_EXT3, &extenders_I2C);
        ext4.begin(ADDRESS_EXT4, &extenders_I2C);
        for(int i =0;i<16;i++){
            ext3.pinMode(i, IOsetting);
            if (!isThisAnOutputModule){
                ext3.pullUp(i,HIGH);
            }
        }
        for(int i =0;i<16;i++){
            ext4.pinMode(i, IOsetting);
            if (!isThisAnOutputModule){
                ext4.pullUp(i,HIGH);
            }
        }
    }

    if(isThisAnOutputModule){ //if it is an output module, set al outputs to 0
        
        for (int i=1; i<channelNumbers; i++){
            setOutput(i,0);
        }
    }
    
}

void setOutput(uint8_t outputNumber, uint8_t outputValue){
    #ifdef SERIALDEBUG
    Serial.print("output ");
    Serial.print(outputNumber);
    Serial.print(" to ");
    Serial.println(outputValue); 
    #endif

    if (outputNumber <= 8)
        ext1.digitalWrite(8-outputNumber,outputValue);
    else if(outputNumber <= 16)
        ext1.digitalWrite(outputNumber-16,outputValue);
    else if(outputNumber <= 24)
        ext2.digitalWrite(8-(outputNumber-16),outputValue);
    else if(outputNumber <= 32)
        ext2.digitalWrite(outputNumber-16,outputValue);
    else if(outputNumber <= 40)
        ext3.digitalWrite(8-(outputNumber-32),outputValue);
    else if(outputNumber <= 48)
        ext3.digitalWrite(outputNumber-32,outputValue);
    else if(outputNumber <= 56)
        ext4.digitalWrite(8-(outputNumber-48),outputValue);
    else if(outputNumber <= 64)
        ext4.digitalWrite(outputNumber-48,outputValue);
}

void readInputs(uint8_t channelNumbers, uint16_t inputbuffertje[]){
    inputbuffertje[0]  = ~ext1.readGPIOAB();
    inputbuffertje[1]  = ~ext2.readGPIOAB();
    inputbuffertje[2]  = ~ext3.readGPIOAB();
    inputbuffertje[3]  = ~ext4.readGPIOAB();

}

uint8_t bitToGPIO(uint8_t bit){
    uint8_t GPIO=0;
    if     (bit <= 7)
        GPIO = 8-bit;
    else if(bit <= 15)
        GPIO = bit+1;
    else if(bit <= 23)
        GPIO = 8-(bit-16)+16;
    else if(bit <= 31)
        GPIO = bit+1;
    else if(bit <= 39)
        GPIO =8-(bit-32)+32;
    else if(bit <= 47)
        GPIO = bit+1;
    else if(bit <= 55)
        GPIO = 8-(bit-48)+48;
    else if(bit <= 63)
        GPIO = bit+1;
    return GPIO;
}