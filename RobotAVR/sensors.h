#ifndef ROBOTAVR_SENSORS_H_
#define ROBOTAVR_SENSORS_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "common.h"
#include <stdio.h>

void sensorsInit();

uint16_t sensorsGetLeftTime();
uint16_t sensorsGetRightTime();
uint16_t sensorsGetFrontTime();

#endif /* ROBOTAVR_SENSORS_H_ */
