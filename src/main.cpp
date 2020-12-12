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
Adafruit_MCP23017 ext2;
Adafruit_MCP23017 ext3;
Adafruit_MCP23017 ext4;

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
  ext1.begin(ADDRESS_EXT1, &extenders_I2C);
  ext2.begin(ADDRESS_EXT2, &extenders_I2C);
  ext3.begin(ADDRESS_EXT3, &extenders_I2C);
  ext4.begin(ADDRESS_EXT4, &extenders_I2C);
  //set all pins to output
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
  
  
  //first little text test
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30, 10);
  display.println("Hello     Church!");
  display.display(); 
}

void loop() {
  ext2.digitalWrite(0,HIGH);
  Serial.println("high");
  delay(1000);
  ext2.digitalWrite(0,LOW);
  Serial.println("low");
  delay(1000);
}