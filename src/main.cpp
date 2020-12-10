#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "config.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

TwoWire display_I2C =  TwoWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &display_I2C, OLED_RESET);

void setup() {
  Serial.begin(115200);\

  display_I2C.begin(DISPLAY_I2C_SDA, DISPLAY_I2C_SCL, 100000);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  } 
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Hello     Church!");
  display.display(); 
}

void loop() {
  
  Serial.println("test");
  delay(1000);
}