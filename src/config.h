#ifndef __CONFIG_H
#define __CONFIG_H
#endif

#include <Arduino.h>

//OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1

//I2C
#define DISPLAY_I2C_SDA 25
#define DISPLAY_I2C_SCL 26
#define EXTENDERS_I2C_SDA 21
#define EXTENDERS_I2C_SCL 22

//Extenders
#define ADDRESS_EXT1 0
#define ADDRESS_EXT2 1
#define ADDRESS_EXT3 2
#define ADDRESS_EXT4 3