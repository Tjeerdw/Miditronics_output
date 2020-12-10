#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "config.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  
}

void loop() {
  Serial.println("test");
  delay(1000);
}