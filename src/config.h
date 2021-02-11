#ifndef __CONFIG_H
#define __CONFIG_H

#include <Arduino.h>

//turn on serial debugging or midi, please choose one
//dont forget to change static const long BaudRate = 115200; in serialMIDI.h
#define SERIALDEBUG  
//#define SERIALMIDI

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

//Input Output module identification
#define IO_Identity 34 //high=output module, low = input module

//buttons
#define BUT_UP      35
#define BUT_DOWN    36
#define BUT_RIGHT   39
#define NAV_BUTTONS_INPUT_PULLUP
#define TOTAL_NAV_BUTTONS 4

//Menu
//Default font: lcd5x7
//#define LARGE_FONT Verdana12
#define MAX_DEPTH 2
#define fontX 5
#define fontY 10

//Midi-Serial
#define MIDI_IN_RX_PIN 18
#define MIDI_IN_TX_PIN 19
#define MIDI_IN_DE_PIN 27
#define MIDI_TH_RX_PIN 16
#define MIDI_TH_TX_PIN 17
#define MIDI_TH_DE_PIN 4

#endif
