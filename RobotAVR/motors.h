#ifndef ROBOTAVR_MOTORS_H_
#define ROBOTAVR_MOTORS_H_

// Left motor - OC0A
// Right motor - OC0B

#include <avr/io.h>
#include <stdint.h>

#define MOTORS_FORWARD 0
#define MOTORS_BACKWARD 1

#define RIGHT_ADJ

void motorsInit();

void motorsSetLeftTorque(uint8_t torque);
void motorsSetLeftDirection(uint8_t direction);
void motorsSetLeft(uint8_t direction, uint8_t torque);
uint8_t motorsGetLeftTorque();
uint8_t motorsGetLeftDirection();
void motorsStopLeft();

void motorsSetRightTorque(uint8_t torque);
void motorsSetRightDirection(uint8_t direction);
void motorsSetRight(uint8_t direction, uint8_t torque);
uint8_t motorsGetRightTorque();
uint8_t motorsGetRightDirection();
void motorsStopRight();

void motorsStopAll();

void motorsSetAllSameDirection(uint8_t direction, uint8_t leftTorque, uint8_t rightTorque);
void motorsSetAllOppositeDirection(uint8_t direction, uint8_t leftTorque, uint8_t rightTorque);

void motorsSetAllTorque(uint8_t left, uint8_t right);
void motorsGetAllTorque(uint8_t* left, uint8_t* right);

#endif /* ROBOTAVR_MOTORS_H_ */
