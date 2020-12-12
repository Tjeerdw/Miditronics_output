#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "config.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

TwoWire display_I2C =  TwoWire(0);
TwoWire extenders_I2C =  TwoWire(1);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &display_I2C, OLED_RESET);

void setup() {
  //USB serial init
  Serial.begin(115200);

  //display init
  display_I2C.begin(DISPLAY_I2C_SDA, DISPLAY_I2C_SCL, 100000);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  } 

  //extender init
  extenders_I2C.begin(EXTENDERS_I2C_SDA, EXTENDERS_I2C_SCL, 100000);

  
  //first little text test
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30, 10);
  display.println("Hello     Church!");
  display.display(); 
}

void loop() {
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    extenders_I2C.beginTransmission(address);
    error = extenders_I2C.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }


  Serial.println("loop");
  delay(1000);
}