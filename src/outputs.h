#ifndef __OUTPUTS_H
#define __OUTPUTS_H

#include <Arduino.h>

void extendersI2Cinit();
void extendersInit(uint8_t channelNumbers, uint8_t isThisAnOutputModule);
uint8_t extendersCount();
void setOutput(uint8_t outputNumber, uint8_t outputValue);
void readInputs(uint8_t channelNumbers, uint16_t inputbuffertje[]);
uint8_t bitToGPIO(uint8_t bit);

#endif