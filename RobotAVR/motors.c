#include "motors.h"

#define LEFT_PWM_DDR DDRD
#define LEFT_PWM_BIT PD6

#define RIGHT_PWM_DDR DDRD
#define RIGHT_PWM_BIT PD5

#define LEFT_DIR1_DDR DDRB
#define LEFT_DIR1_PORT PORTB
#define LEFT_DIR1_BIT PB0

#define LEFT_DIR2_DDR DDRD
#define LEFT_DIR2_PORT PORTD
#define LEFT_DIR2_BIT PD7

#define RIGHT_DIR1_DDR DDRB
#define RIGHT_DIR1_PORT PORTB
#define RIGHT_DIR1_BIT PB1

#define RIGHT_DIR2_DDR DDRB
#define RIGHT_DIR2_PORT PORTB
#define RIGHT_DIR2_BIT PB2

void motorsInit()
{
	LEFT_PWM_DDR |= (1 << LEFT_PWM_BIT);
	RIGHT_PWM_DDR |= (1 << RIGHT_PWM_BIT);

	LEFT_DIR1_DDR |= (1 << LEFT_DIR1_BIT);
	LEFT_DIR2_DDR |= (1 << LEFT_DIR2_BIT);

	RIGHT_DIR1_DDR |= (1 << RIGHT_DIR1_BIT);
	RIGHT_DIR2_DDR |= (1 << RIGHT_DIR2_BIT);

	OCR0A = 0;
	OCR0B = 0;
	TCCR0A |= (1 << COM0A1) | (1 << COM0B1) | (1 << WGM01) | (1 << WGM00); // non-inverted, fast pwm
	TCCR0B |= (1 << CS02) | (1 << CS00); // clk / 1024 = 15.625 KHz
}

void motorsSetLeftTorque(uint8_t torque)
{
	OCR0A = torque;
}
void motorsSetLeftDirection(uint8_t direction)
{
	if (direction == MOTORS_FORWARD)
	{
		LEFT_DIR1_PORT &= ~(1 << LEFT_DIR1_BIT);
		LEFT_DIR2_PORT |= (1 << LEFT_DIR2_BIT);
	}
	else // MOTORS_BACKWARD
	{
		LEFT_DIR2_PORT &= ~(1 << LEFT_DIR2_BIT);
		LEFT_DIR1_PORT |= (1 << LEFT_DIR1_BIT);
	}
}
void motorsSetLeft(uint8_t direction, uint8_t torque)
{
	if (direction == MOTORS_FORWARD)
	{
		LEFT_DIR1_PORT &= ~(1 << LEFT_DIR1_BIT);
		OCR0A = torque;
		LEFT_DIR2_PORT |= (1 << LEFT_DIR2_BIT);
	}
	else // MOTORS_BACKWARD
	{
		LEFT_DIR2_PORT &= ~(1 << LEFT_DIR2_BIT);
		OCR0A = torque;
		LEFT_DIR1_PORT |= (1 << LEFT_DIR1_BIT);
	}
}
uint8_t motorsGetLeftTorque()
{
	return OCR0A;
}
uint8_t motorsGetLeftDirection()
{
	return (LEFT_DIR2_PORT & (1 << LEFT_DIR2_BIT)) ? MOTORS_FORWARD : MOTORS_BACKWARD;
}
void motorsStopLeft()
{
	LEFT_DIR1_PORT |= (1 << LEFT_DIR1_BIT);
	LEFT_DIR2_PORT |= (1 << LEFT_DIR2_BIT);
	OCR0A = 0;
}

void motorsSetRightTorque(uint8_t torque)
{
	OCR0B = torque;
}
void motorsSetRightDirection(uint8_t direction)
{
	if (direction == MOTORS_FORWARD)
	{
		RIGHT_DIR1_PORT &= ~(1 << RIGHT_DIR1_BIT);
		RIGHT_DIR2_PORT |= (1 << RIGHT_DIR2_BIT);
	}
	else // MOTORS_BACKWARD
	{
		RIGHT_DIR2_PORT &= ~(1 << RIGHT_DIR2_BIT);
		RIGHT_DIR1_PORT |= (1 << RIGHT_DIR1_BIT);
	}
}
void motorsSetRight(uint8_t direction, uint8_t torque)
{
	if (direction == MOTORS_FORWARD)
	{
		RIGHT_DIR1_PORT &= ~(1 << RIGHT_DIR1_BIT);
		OCR0B = torque;
		RIGHT_DIR2_PORT |= (1 << RIGHT_DIR2_BIT);
	}
	else // MOTORS_BACKWARD
	{
		RIGHT_DIR2_PORT &= ~(1 << RIGHT_DIR2_BIT);
		OCR0B = torque;
		RIGHT_DIR1_PORT |= (1 << RIGHT_DIR1_BIT);
	}
}
uint8_t motorsGetRightTorque()
{
	return OCR0B;
}
uint8_t motorsGetRightDirection()
{
	return (RIGHT_DIR2_PORT & (1 << RIGHT_DIR2_BIT)) ? MOTORS_FORWARD : MOTORS_BACKWARD;
}
void motorsStopRight()
{
	RIGHT_DIR1_PORT |= (1 << RIGHT_DIR1_BIT);
	RIGHT_DIR2_PORT |= (1 << RIGHT_DIR2_BIT);
	OCR0B = 0;
}

void motorsStopAll()
{
	OCR0A = 0;
	OCR0B = 0;
	LEFT_DIR1_PORT |= (1 << LEFT_DIR1_BIT);
	LEFT_DIR2_PORT |= (1 << LEFT_DIR2_BIT);
	RIGHT_DIR1_PORT |= (1 << RIGHT_DIR1_BIT);
	RIGHT_DIR2_PORT |= (1 << RIGHT_DIR2_BIT);
}

void motorsSetAllSameDirection(uint8_t direction, uint8_t leftTorque, uint8_t rightTorque)
{
	if (direction == MOTORS_FORWARD)
	{
		LEFT_DIR1_PORT &= ~(1 << LEFT_DIR1_BIT);
		RIGHT_DIR1_PORT &= ~(1 << RIGHT_DIR1_BIT);
		OCR0A = leftTorque;
		OCR0B = rightTorque;
		LEFT_DIR2_PORT |= (1 << LEFT_DIR2_BIT);
		RIGHT_DIR2_PORT |= (1 << RIGHT_DIR2_BIT);
	}
	else // MOTORS_BACKWARD
	{
		LEFT_DIR2_PORT &= ~(1 << LEFT_DIR2_BIT);
		RIGHT_DIR2_PORT &= ~(1 << RIGHT_DIR2_BIT);
		OCR0A = leftTorque;
		OCR0B = rightTorque;
		LEFT_DIR1_PORT |= (1 << LEFT_DIR1_BIT);
		RIGHT_DIR1_PORT |= (1 << RIGHT_DIR1_BIT);
	}
}
void motorsSetAllOppositeDirection(uint8_t direction, uint8_t leftTorque, uint8_t rightTorque)
{
	if (direction == MOTORS_FORWARD)
	{
		LEFT_DIR1_PORT &= ~(1 << LEFT_DIR1_BIT);
		RIGHT_DIR2_PORT &= ~(1 << RIGHT_DIR2_BIT);
		OCR0A = leftTorque;
		OCR0B = rightTorque;
		LEFT_DIR2_PORT |= (1 << LEFT_DIR2_BIT);
		RIGHT_DIR1_PORT |= (1 << RIGHT_DIR1_BIT);
	}
	else // MOTORS_BACKWARD
	{
		LEFT_DIR2_PORT &= ~(1 << LEFT_DIR2_BIT);
		RIGHT_DIR1_PORT &= ~(1 << RIGHT_DIR1_BIT);
		OCR0A = leftTorque;
		OCR0B = rightTorque;
		LEFT_DIR1_PORT |= (1 << LEFT_DIR1_BIT);
		RIGHT_DIR2_PORT |= (1 << RIGHT_DIR2_BIT);
	}
}

void motorsSetAllTorque(uint8_t left, uint8_t right)
{
	OCR0A = left;
	OCR0B = right;
}
void motorsGetAllTorque(uint8_t* left, uint8_t* right)
{
	*left = OCR0A;
	*right = OCR0B;
}

