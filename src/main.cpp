#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "config.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_MCP23017.h"

TwoWire display_I2C =  TwoWire(0);
TwoWire extenders_I2C =  TwoWire(1);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &display_I2C, OLED_RESET);
Adafruit_MCP23017 ext1;

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
  ext1.begin(0, &extenders_I2C);
  ext1.pinMode(0, OUTPUT);


  
  //first little text test
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30, 10);
  display.println("Hello     Church!");
  display.display(); 
}

void loop() {
  ext1.digitalWrite(0,HIGH);
  Serial.println("high");
  delay(1000);
  ext1.digitalWrite(0,LOW);
  Serial.println("low");
  delay(1000);
}