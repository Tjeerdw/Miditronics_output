#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "config.h"
#include "outputs.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


TwoWire display_I2C =  TwoWire(0);

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

  //extenders init
  extendersInit();
    
  //first little text test
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30, 10);
  display.println("Hello     Church!");
  display.display(); 
}

void loop() {
  setOutput(17,1);
  delay(1000);

  setOutput(17,0);
  delay(1000);
}