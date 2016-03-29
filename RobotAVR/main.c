#include <avr/io.h>
#include <util/delay.h>
#include "motors.h"
#include "wheels.h"
#include "common.h"
#include "controller.h"
#include "remote.h"
#include "sensors.h"
#include "api.h"
#include <stdio.h>
#include <util/atomic.h>

#define MIN_SPEED CONTROLLER_CALIBRATION_MIN_SPEED
#define MAX_SPEED CONTROLLER_CALIBRATION_MAX_SPEED
#define SPEED_STEP CONTROLLER_CALIBRATION_SPEED_STEP

#define PULSES_TURN 2

#define DISTANCE_LIMIT (14 * 58)
#define NEW_DISTANCE_LIMIT (17 * 58)

#define SIDE_DISTANCE_LIMIT (12 * 58)
#define SIDE_NEW_DISTANCE_LIMIT (14 * 58)

#define PRECISE_SIDE_DISTANCE_LIMIT (7 * 58)
#define PRECISE_SIDE_NEW_DISTANCE_LIMIT (7 * 58)

#define BETWEEN_WALL_DIFFERENCE (8 * 58)

//#define SIDE_SAME_TOLERANCE (3 * 58)

#define STRAIGHT_MOVE_MODE CONTROLLER_LEFT_AND_RIGHT

#define SENSORS_STEP 5
#define SENSORS_SMALL_STEP 2
#define SENSORS_90_STEP 10
#define SENSORS_180_STEP 20
#define SENSORS_MAX_SMALL_ROTATIONS 8
#define SENSORS_ENABLE_PRECISE_ROTATIONS 3

static uint8_t s_timerCounter;
static uint8_t s_started;
static volatile uint8_t s_buttonPressed;
static volatile uint8_t s_buttonLock;
static uint8_t s_speed;
static uint8_t s_straightMode;

#define CURRENT_PULSES (clamp(s_speed / 2, 5, 20))

void mainInit();

void mainBluetooth();
void mainSensors();

void mainTimerTick();

uint8_t mainCheckModeChange();
uint8_t mainCheckSpeedChange(uint8_t c);
uint8_t mainHandleButton();

int16_t abs(int16_t x)
{
	if (x < 0)
		return x * -1;
	else return x;
}
uint8_t clamp(uint8_t val, uint8_t min, uint8_t max)
{
	if (val < min) return min;
	else if (val > max) return max;

	return val;
}

int main()
{
	mainInit();

	if (buttonIsPressed()) controllerCalibrate();

	ledOn();
	s_started = 1;

	while(1)
	{
		mainBluetooth();
		mainSensors();
	}

	return 0;
}

void mainBluetooth()
{
	uint8_t cmd, lastcmd;

	cmd = lastcmd = 0;
	while(1)
	{
		if (mainHandleButton()) return;
		if (remotePoll(&cmd))
		{
			mainCheckSpeedChange(cmd);
			if (cmd == 'x')
			{
				controllerStop();
				return;
			}

			if (lastcmd == cmd && controllerIsBusy())
			{
				if (cmd == 'w' || cmd == 's')
				{
					if (s_straightMode == CONTROLLER_LEFT_AND_RIGHT_COMMON)
					{
						controllerSetCommonPulses(CURRENT_PULSES);
					}
					else
					{
						controllerSetLeftPulses(CURRENT_PULSES / 2);
						controllerSetRightPulses(CURRENT_PULSES / 2);
					}
				}
				else if (cmd == 'a' || cmd == 'd')
				{
					controllerSetLeftPulses(PULSES_TURN);
					controllerSetRightPulses(PULSES_TURN);
				}
			}
			else
			{
				controllerStop();
				switch (cmd)
				{
				case 'w':
					controllerMove(CONTROLLER_FORWARD, s_speed, CURRENT_PULSES, s_straightMode);
					break;

				case 'a':
					controllerMove(CONTROLLER_BACKWARD, s_speed, PULSES_TURN, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
					break;

				case 's':
					controllerMove(CONTROLLER_BACKWARD, s_speed, CURRENT_PULSES, s_straightMode);
					break;

				case 'd':
					controllerMove(CONTROLLER_FORWARD, s_speed, PULSES_TURN, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
					break;
				}
			}
			lastcmd = cmd;
		}
	}
}
void mainSensors()
{
	uint8_t s_rotations = 0;

	while(1)
	{
		/*uint8_t cmd;
		if (remotePoll(&cmd))
		{
			controllerStop();
			return;
		}*/

		controllerMove(CONTROLLER_FORWARD, s_speed, 0xFFFF, s_straightMode);
		while (sensorsGetFrontTime() > DISTANCE_LIMIT && sensorsGetLeftTime() > SIDE_DISTANCE_LIMIT && sensorsGetRightTime() > SIDE_DISTANCE_LIMIT)
		{
			s_rotations = 0;
			if(mainCheckModeChange()) return;
		}
		controllerStop();

		uint16_t sideDistanceLimit = (s_rotations >= SENSORS_ENABLE_PRECISE_ROTATIONS ? PRECISE_SIDE_DISTANCE_LIMIT : SIDE_DISTANCE_LIMIT);
		uint16_t sideNewDistanceLimit = (s_rotations >= SENSORS_ENABLE_PRECISE_ROTATIONS ? PRECISE_SIDE_NEW_DISTANCE_LIMIT : SIDE_NEW_DISTANCE_LIMIT);

		if (sensorsGetFrontTime() <= DISTANCE_LIMIT)
		{
			uint8_t goLeft = (sensorsGetLeftTime() > sensorsGetRightTime());
			while (sensorsGetFrontTime() < NEW_DISTANCE_LIMIT)
			{
				s_rotations++;
				if (goLeft)
				{
					controllerMove(CONTROLLER_BACKWARD, s_speed, SENSORS_STEP, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
				}
				else
				{
					controllerMove(CONTROLLER_FORWARD, s_speed, SENSORS_STEP, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
				}

				while(controllerIsBusy()) if (mainCheckModeChange()) return;
			}
		}
		else if (abs((int16_t)sensorsGetLeftTime() - (int16_t)sensorsGetRightTime()) <= BETWEEN_WALL_DIFFERENCE && sensorsGetFrontTime() > DISTANCE_LIMIT)
		{
			controllerMove(CONTROLLER_FORWARD, s_speed, 10, s_straightMode);
			while(controllerIsBusy()) if (mainCheckModeChange()) return;
		}
		else if (sensorsGetLeftTime() <= sideDistanceLimit)
		{
			s_rotations++;
			while (sensorsGetLeftTime() < sideNewDistanceLimit)
			{
				controllerMove(CONTROLLER_FORWARD, s_speed, SENSORS_SMALL_STEP, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
				while(controllerIsBusy()) if (mainCheckModeChange()) return;
			}
		}
		else if (sensorsGetRightTime() <= sideDistanceLimit)
		{
			s_rotations++;
			while (sensorsGetRightTime() < sideNewDistanceLimit)
			{
				controllerMove(CONTROLLER_BACKWARD, s_speed, SENSORS_SMALL_STEP, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
				while(controllerIsBusy()) if (mainCheckModeChange()) return;
			}
		}
		else
		{
			controllerMove((sensorsGetLeftTime() > sensorsGetRightTime() ? CONTROLLER_BACKWARD : CONTROLLER_FORWARD),
								       s_speed, SENSORS_180_STEP, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
			while(controllerIsBusy()) if (mainCheckModeChange()) return;
			s_rotations = 0;
		}

		if (s_rotations >= SENSORS_MAX_SMALL_ROTATIONS)
		{
			controllerMove((sensorsGetLeftTime() > sensorsGetRightTime() ? CONTROLLER_BACKWARD : CONTROLLER_FORWARD),
					       s_speed, SENSORS_180_STEP, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
			while(controllerIsBusy()) if (mainCheckModeChange()) return;
			s_rotations = 0;
		}
	}
}

uint8_t mainCheckModeChange()
{
	uint8_t cmd;
	uint8_t polled = remotePoll(&cmd);
	if (polled) mainCheckSpeedChange(cmd);
	if ((polled && cmd == 'x') || mainHandleButton())
	{
		controllerStop();
		return 1;
	}
	return 0;
}
uint8_t mainCheckSpeedChange(uint8_t c)
{
	uint8_t changed = 0;
	int16_t tmpSpeed = (int16_t)s_speed;
	if (c == '+')
	{
		tmpSpeed += SPEED_STEP;
		changed = 1;
	}
	else if (c == '-')
	{
		tmpSpeed -= SPEED_STEP;
		changed = 1;
	}
	else if (c == 'm')
	{
		s_straightMode = (s_straightMode == CONTROLLER_LEFT_AND_RIGHT ? CONTROLLER_LEFT_AND_RIGHT_COMMON : CONTROLLER_LEFT_AND_RIGHT);
		changed = 1;
	}
	else if (c == 'c')
	{
		uint8_t current = controllerGetCalibrateWhileMoving();
		controllerSetCalibrateWhileMoving(current == 0 ? 1 : 0);
	}

	if (tmpSpeed < MIN_SPEED) tmpSpeed = MIN_SPEED;
	else if (tmpSpeed > MAX_SPEED) tmpSpeed = MAX_SPEED;
	s_speed = (uint8_t)tmpSpeed;

	return changed;
}

uint8_t mainHandleButton()
{
	uint8_t pressed;
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		pressed = s_buttonPressed;
		s_buttonPressed = 0;
	}

	return pressed;
}

void mainTimerTick()
{
	if (s_started)
	{
		if (s_timerCounter++ >= 244)
		{
			s_timerCounter = 0;
			ledToggle();
		}

		if (buttonIsPressed())
		{
			if (!s_buttonLock)
			{
				s_buttonPressed = 1;
				s_buttonLock = 1;
			}
		}
		else
		{
			s_buttonPressed = 0;
			s_buttonLock = 0;
		}
	}
}

void mainInit()
{
	// not used pins as inputs with pull-ups
	DDRB &= ~(1 << PB4);
	PORTB |= (1 << PB4);
	DDRB &= ~(1 << PB5);
	PORTB |= (1 << PB5);

	s_timerCounter = 0;
	s_started = 0;
	s_buttonPressed = 0;
	s_buttonLock = 0;
	s_speed = 40;
	s_straightMode = CONTROLLER_LEFT_AND_RIGHT;

	sei();

	motorsInit();
	wheelsInit();
	controllerInitWithTimer(&mainTimerTick);
	remoteInit();
	sensorsInit();
	ledInit();
	buttonInit();
}
