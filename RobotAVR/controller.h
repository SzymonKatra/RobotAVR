#ifndef ROBOTAVR_CONTROLLER_H_
#define ROBOTAVR_CONTROLLER_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdint.h>
#include <util/delay.h>
#include <string.h>
#include "motors.h"
#include "wheels.h"
#include "common.h"
#include "eeprom.h"

#define CONTROLLER_LEFT 1
#define CONTROLLER_RIGHT 2
#define CONTROLLER_LEFT_AND_RIGHT 3
#define CONTROLLER_LEFT_AND_OPPOSITE_RIGHT 4
#define CONTROLLER_LEFT_AND_RIGHT_COMMON 5
#define CONTROLLER_LEFT_AND_OPPOSITE_RIGHT_COMMON 6

#define CONTROLLER_FORWARD MOTORS_FORWARD
#define CONTROLLER_BACKWARD MOTORS_BACKWARD

#define CONTROLLER_OPPOSITE(x) (x == CONTROLLER_FORWARD ? CONTROLLER_BACKWARD : CONTROLLER_FORWARD)

#define CONTROLLER_START_ADDITIONAL_SPEED 0//20

#define CONTROLLER_CHANGE_FACTOR 2
#define CONTROLLER_MIN_PULSES 5
#define CONTROLLER_TO_LOCK_CALIBRATION_TICKS 6
#define CONTROLLER_TO_UNLOCK_CALIBRATION_TICKS 10

#define CONTROLLER_CALIBRATION_INITIAL_TORQUE 200
#define CONTROLLER_CALIBRATION_MIN_SPEED 10
#define CONTROLLER_CALIBRATION_MAX_SPEED 60
#define CONTROLLER_CALIBRATION_SPEED_STEP 5
#define CONTROLLER_CALIBRATION_PULSES 200

#define CONTROLLER_CALIBRATE_WHILE_MOVING 1

typedef void(*timerTick_f)();

void controllerInit();
void controllerInitWithTimer(timerTick_f timerTick);
void controllerStop();
void controllerReset();
void controllerCalibrate();
uint8_t controllerIsBusy();

void controllerMove(uint8_t direction, uint8_t speed, uint16_t pulses, uint8_t mode);
void controllerMoveNoAdditionalStartSpeed(uint8_t direction, uint8_t speed, uint16_t pulses, uint8_t mode);
void controllerMoveStartTorque(uint8_t direction, uint8_t speed, uint16_t pulses, uint8_t mode, uint8_t leftStartTorque, uint8_t rightStartTorque);

void controllerSetLeftPulses(uint16_t pulses);
void controllerSetRightPulses(uint16_t pulses);
void controllerSetCommonPulses(uint16_t pulses);

void controllerSetCalibrateWhileMoving(uint8_t state);
uint8_t controllerGetCalibrateWhileMoving();

#endif /* ROBOTAVR_CONTROLLER_H_ */
