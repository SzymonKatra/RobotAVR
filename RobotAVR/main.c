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

#define SPEED 30
#define PULSES 10
#define PULSES_TURN 2

#define DISTANCE_LIMIT (20 * 58)
#define NEW_DISTANCE_LIMIT (22 * 58)

#define SIDE_DISTANCE_LIMIT (10 * 58)
#define SIDE_NEW_DISTANCE_LIMIT (15 * 58)

static uint8_t s_timerCounter;
static uint8_t s_started;

void mainInit();

void mainBluetooth();
void mainSensors();

void mainTimerTick();

uint8_t mainCheckModeChange();

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
	while(1)
	{
		uint8_t cmd;
		if (remotePoll(&cmd))
		{
			controllerStop();
			return;
		}

		controllerMove(CONTROLLER_FORWARD, SPEED, 0xFFFF, CONTROLLER_LEFT_AND_RIGHT_COMMON);
		while (sensorsGetFrontTime() > DISTANCE_LIMIT && sensorsGetLeftTime() > SIDE_DISTANCE_LIMIT && sensorsGetRightTime() > SIDE_DISTANCE_LIMIT) if(mainCheckModeChange()) return;
		controllerStop();

		if (sensorsGetFrontTime() <= DISTANCE_LIMIT)
		{
			uint8_t goLeft = (sensorsGetLeftTime() > sensorsGetRightTime());
			while (sensorsGetFrontTime() < NEW_DISTANCE_LIMIT)
			{
				if (goLeft)
				{
					controllerMove(CONTROLLER_BACKWARD, SPEED, 5, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
				}
				else
				{
					controllerMove(CONTROLLER_FORWARD, SPEED, 5, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
				}

				while(controllerIsBusy()) if (mainCheckModeChange()) return;
				_delay_ms(500);
			}
		}
		else if (sensorsGetLeftTime() <= SIDE_DISTANCE_LIMIT)
		{
			while (sensorsGetLeftTime() < SIDE_NEW_DISTANCE_LIMIT)
			{
				controllerMove(CONTROLLER_FORWARD, SPEED, 2, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
				while(controllerIsBusy()) if (mainCheckModeChange()) return;
				_delay_ms(500);
			}
		}
		else if (sensorsGetRightTime() <= SIDE_DISTANCE_LIMIT)
		{
			while (sensorsGetRightTime() < SIDE_NEW_DISTANCE_LIMIT)
			{
				controllerMove(CONTROLLER_BACKWARD, SPEED, 2, CONTROLLER_LEFT_AND_OPPOSITE_RIGHT);
				while(controllerIsBusy()) if (mainCheckModeChange()) return;
				_delay_ms(500);
			}
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
