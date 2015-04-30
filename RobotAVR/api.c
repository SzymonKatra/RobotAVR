#include "api.h"

static void correctMove(uint8_t leftDirection, uint8_t rightDirection);

void rotateRight(uint16_t angle)
{
	controllerMove(CONTROLLER_FORWARD, API_SPEED, angle / 9, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
	correctMove(CONTROLLER_FORWARD, CONTROLLER_BACKWARD);
}
void rotateLeft(uint16_t angle)
{
	controllerMove(CONTROLLER_BACKWARD, API_SPEED, angle / 9, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
	correctMove(CONTROLLER_BACKWARD, CONTROLLER_FORWARD);
}

static void correctMove(uint8_t leftDirection, uint8_t rightDirection)
{
	leftDirection = CONTROLLER_OPPOSITE(leftDirection);
	rightDirection = CONTROLLER_OPPOSITE(rightDirection);

	int16_t left, right;
	do
	{
		while(controllerIsBusy());
		wheelsResetCounters();
		_delay_ms(500);
		uint16_t lt, rt;
		wheelsGetAllCount(&lt, &rt);
		left = (int16_t)lt;
		right = (int16_t)rt;

		if (left > 0)
		{
			controllerMove(leftDirection, API_CORRECTION_SPEED, (uint16_t)left, CONTROLLER_LEFT);
		}
		if (right > 0)
		{
			controllerMove(rightDirection, API_CORRECTION_SPEED, (uint16_t)left, CONTROLLER_RIGHT);
		}

	} while (left != 0 && right != 0);
}
