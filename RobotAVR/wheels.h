#ifndef ROBOTAVR_WHEELS_H_
#define ROBOTAVR_WHEELS_H_

#include <avr/interrupt.h>
#include <util/atomic.h>
#include "common.h"

void wheelsInit();

void wheelsResetLeftCounter();
void wheelsResetRightCounter();
void wheelsResetCounters();

uint16_t wheelsGetLeftCount();
uint16_t wheelsGetRightCount();
void wheelsGetAllCount(uint16_t* left, uint16_t* right);

#endif /* ROBOTAVR_WHEELS_H_ */
