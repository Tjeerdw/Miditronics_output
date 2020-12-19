#ifndef __CONFIG_H
#define __CONFIG_H

#include <Arduino.h>

//turn on serial debugging
#define SERIALDEBUG 

//OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1

//I2C
#define DISPLAY_I2C_SDA     25
#define DISPLAY_I2C_SCL     26
#define EXTENDERS_I2C_SDA   21
#define EXTENDERS_I2C_SCL   22

//Extenders
#define ADDRESS_EXT1 0
#define ADDRESS_EXT2 1
#define ADDRESS_EXT3 2
#define ADDRESS_EXT4 3

//buttons
#define BUT_UP      36
#define BUT_DOWN    35
#define BUT_LEFT    34
#define BUT_RIGHT   39
#define NAV_BUTTONS_INPUT_PULLUP
#define TOTAL_NAV_BUTTONS 4

//Menu
//Default font: lcd5x7
//#define LARGE_FONT Verdana12
#define MAX_DEPTH 2
#define fontX 7
#define fontY 9


#endif
