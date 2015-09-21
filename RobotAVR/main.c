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

#define SPEED 40
#define PULSES 10
#define PULSES_TURN 2

#define DISTANCE_LIMIT (13 * 58)
#define NEW_DISTANCE_LIMIT (16 * 58)

#define SIDE_DISTANCE_LIMIT (11 * 58)
#define SIDE_NEW_DISTANCE_LIMIT (13 * 58)

#define PRECISE_SIDE_DISTANCE_LIMIT (6 * 58)
#define PRECISE_SIDE_NEW_DISTANCE_LIMIT (6 * 58)

#define BETWEEN_WALL_DIFFERENCE (8 * 58)

//#define SIDE_SAME_TOLERANCE (3 * 58)

#define SENSORS_STEP 5
#define SENSORS_SMALL_STEP 2
#define SENSORS_90_STEP 10
#define SENSORS_180_STEP 20
#define SENSORS_MAX_SMALL_ROTATIONS 8
#define SENSORS_ENABLE_PRECISE_ROTATIONS 3

static uint8_t s_timerCounter;
static uint8_t s_started;

void mainInit();

void mainBluetooth();
void mainSensors();

void mainTimerTick();

uint8_t mainCheckModeChange();

int16_t abs(int16_t x)
{
	if (x < 0)
		return x * -1;
	else return x;
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
		if (remotePoll(&cmd))
		{
			if (cmd == 'x')
			{
				controllerStop();
				return;
			}

			if (lastcmd == cmd && controllerIsBusy())
			{
				if (cmd == 'w' || cmd == 's')
				{
					controllerSetCommonPulses(PULSES);
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
					controllerMove(CONTROLLER_FORWARD, SPEED, PULSES, CONTROLLER_LEFT_AND_RIGHT_COMMON);
					break;

				case 'a':
					controllerMove(CONTROLLER_BACKWARD, SPEED, PULSES_TURN, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
					break;

				case 's':
					controllerMove(CONTROLLER_BACKWARD, SPEED, PULSES, CONTROLLER_LEFT_AND_RIGHT_COMMON);
					break;

				case 'd':
					controllerMove(CONTROLLER_FORWARD, SPEED, PULSES_TURN, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
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
		uint8_t cmd;
		if (remotePoll(&cmd))
		{
			controllerStop();
			return;
		}

		controllerMove(CONTROLLER_FORWARD, SPEED, 0xFFFF, CONTROLLER_LEFT_AND_RIGHT_COMMON);
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
					controllerMove(CONTROLLER_BACKWARD, SPEED, SENSORS_STEP, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
				}
				else
				{
					controllerMove(CONTROLLER_FORWARD, SPEED, SENSORS_STEP, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
				}

				while(controllerIsBusy()) if (mainCheckModeChange()) return;
			}
		}
		else if (abs((int16_t)sensorsGetLeftTime() - (int16_t)sensorsGetRightTime()) <= BETWEEN_WALL_DIFFERENCE && sensorsGetFrontTime() > DISTANCE_LIMIT)
		{
			controllerMove(CONTROLLER_FORWARD, SPEED, 10, CONTROLLER_LEFT_AND_RIGHT_COMMON);
			while(controllerIsBusy()) if (mainCheckModeChange()) return;
		}
		else if (sensorsGetLeftTime() <= sideDistanceLimit)
		{
			s_rotations++;
			while (sensorsGetLeftTime() < sideNewDistanceLimit)
			{
				controllerMove(CONTROLLER_FORWARD, SPEED, SENSORS_SMALL_STEP, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
				while(controllerIsBusy()) if (mainCheckModeChange()) return;
			}
		}
		else if (sensorsGetRightTime() <= sideDistanceLimit)
		{
			s_rotations++;
			while (sensorsGetRightTime() < sideNewDistanceLimit)
			{
				controllerMove(CONTROLLER_BACKWARD, SPEED, SENSORS_SMALL_STEP, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
				while(controllerIsBusy()) if (mainCheckModeChange()) return;
			}
		}
		else
		{
			controllerMove((sensorsGetLeftTime() > sensorsGetRightTime() ? CONTROLLER_BACKWARD : CONTROLLER_FORWARD),
								       SPEED, SENSORS_180_STEP, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
			while(controllerIsBusy()) if (mainCheckModeChange()) return;
			s_rotations = 0;
		}

		if (s_rotations >= SENSORS_MAX_SMALL_ROTATIONS)
		{
			controllerMove((sensorsGetLeftTime() > sensorsGetRightTime() ? CONTROLLER_BACKWARD : CONTROLLER_FORWARD),
					       SPEED, SENSORS_180_STEP, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
			while(controllerIsBusy()) if (mainCheckModeChange()) return;
			s_rotations = 0;
		}
	}
}

uint8_t mainCheckModeChange()
{
	uint8_t cmd;
	if (remotePoll(&cmd) && cmd == 'x')
	{
		controllerStop();
		return 1;
	}
	return 0;
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

	sei();

	motorsInit();
	wheelsInit();
	controllerInitWithTimer(&mainTimerTick);
	remoteInit();
	sensorsInit();
	ledInit();
	buttonInit();
}
