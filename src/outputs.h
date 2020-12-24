#ifndef __OUTPUTS_H
#define __OUTPUTS_H

#include <Arduino.h>

void extendersI2Cinit();
void extendersInit();
uint8_t extendersCount();
void setOutput(uint8_t outputNumber, uint8_t outputValue);

#endif