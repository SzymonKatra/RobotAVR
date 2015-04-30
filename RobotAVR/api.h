#ifndef ROBOTAVR_API_H_
#define ROBOTAVR_API_H_

#include <util/delay.h>
#include "controller.h"
#include "wheels.h"

#define API_SPEED 30
#define API_CORRECTION_SPEED 30

void rotateRight(uint16_t angle);
void rotateLeft(uint16_t angle);

#endif /* ROBOTAVR_API_H_ */
