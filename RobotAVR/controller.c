#include "controller.h"

#define HANDLER_TICKS 12 // ~ 50 ms

typedef struct
{
	uint8_t enabled;
	uint16_t pulsesRemaining;
	uint16_t previousPulses;
	uint8_t pulsesPerCalibrationPeriod;
	uint8_t timerCounter;
	uint8_t calibrateCounter;
	uint8_t wheelLocked;
}controllerInternalMoveData_t;

typedef struct
{
	uint8_t leftTorque;
	uint8_t rightTorque;
}controllerCalibration_t;

#define CALIBRATION_ARRAY_SIZE ((CONTROLLER_CALIBRATION_MAX_SPEED - CONTROLLER_CALIBRATION_MIN_SPEED) / CONTROLLER_CALIBRATION_SPEED_STEP + 1)

static controllerCalibration_t s_calibration[CALIBRATION_ARRAY_SIZE];

static volatile controllerInternalMoveData_t s_common;
static volatile controllerInternalMoveData_t s_left;
static volatile controllerInternalMoveData_t s_right;

static volatile controllerCalibration_t s_calibrating;
static volatile uint8_t s_calibrationEnabled;

static volatile timerTick_f s_timerTickFunction;

static void controllerResetData(volatile controllerInternalMoveData_t* data);
static void controllerFillOption(volatile controllerInternalMoveData_t* data, uint8_t speed, uint16_t pulses);

static void controllerCommonHandler();

// -1 if you don't need to do any action
// -2 if you must stop motor
// otherwis returns new torque and you must reset wheel counter
static int16_t controllerHandler(volatile controllerInternalMoveData_t* data, uint16_t pulses, uint8_t torque, volatile uint8_t* calibrating);

void controllerInit()
{
	controllerInitWithTimer(NULL);
}
void controllerInitWithTimer(timerTick_f timerTick)
{
	s_timerTickFunction = timerTick;

	controllerReset();

	eepromReadData(0, &s_calibration, sizeof(controllerCalibration_t) * CALIBRATION_ARRAY_SIZE);

	s_calibrationEnabled = 0;

	TCCR2B |= (1 << CS22) | (1 << CS21); // clk / 256 = 62.5 KHz
	TIMSK2 |= (1 << TOIE2); // interrupt on overflow ~ 244 interrupts per second
}

void controllerStop()
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		controllerResetData(&s_common);
		controllerResetData(&s_left);
		controllerResetData(&s_right);
	}

	motorsStopAll();
}
void controllerReset()
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		controllerResetData(&s_common);
		controllerResetData(&s_left);
		controllerResetData(&s_right);
		s_calibrationEnabled = 0;
	}
}

void controllerCalibrate()
{
	s_calibrationEnabled = 1;
	_delay_ms(2000);
	for(uint8_t speed = CONTROLLER_CALIBRATION_MIN_SPEED; speed <= CONTROLLER_CALIBRATION_MAX_SPEED; speed += CONTROLLER_CALIBRATION_SPEED_STEP)
	{
		controllerMoveStartTorque(CONTROLLER_FORWARD, speed, CONTROLLER_CALIBRATION_PULSES, CONTROLLER_LEFT_AND_RIGHT, CONTROLLER_CALIBRATION_INITIAL_TORQUE, CONTROLLER_CALIBRATION_INITIAL_TORQUE);
		while(controllerIsBusy());
		uint8_t index = (speed - CONTROLLER_CALIBRATION_MIN_SPEED) / CONTROLLER_CALIBRATION_SPEED_STEP;
		s_calibration[index] = s_calibrating;

		_delay_ms(1000);
		controllerMoveStartTorque(CONTROLLER_FORWARD, speed, 20, CONTROLLER_LEFT, CONTROLLER_CALIBRATION_INITIAL_TORQUE, CONTROLLER_CALIBRATION_INITIAL_TORQUE);
		controllerMoveStartTorque(CONTROLLER_BACKWARD, speed, 20, CONTROLLER_RIGHT, CONTROLLER_CALIBRATION_INITIAL_TORQUE, CONTROLLER_CALIBRATION_INITIAL_TORQUE);
		while(controllerIsBusy());

		_delay_ms(1000);
	}

	eepromWriteData(0, &s_calibration, sizeof(controllerCalibration_t) * CALIBRATION_ARRAY_SIZE);

	s_calibrationEnabled = 0;
}

void controllerMove(uint8_t direction, uint8_t speed, uint16_t pulses, uint8_t mode)
{
	uint8_t index = (speed - CONTROLLER_CALIBRATION_MIN_SPEED) / CONTROLLER_CALIBRATION_SPEED_STEP;

	uint16_t left = s_calibration[index].leftTorque + CONTROLLER_START_ADDITIONAL_SPEED;
	if (left > 255) left = 255;
	uint16_t right = s_calibration[index].rightTorque + CONTROLLER_START_ADDITIONAL_SPEED;
	if (right > 255) right = 255;

	controllerMoveStartTorque(direction, speed, pulses, mode, (uint8_t)left, (uint8_t)right);
}
void controllerMoveNoAdditionalStartSpeed(uint8_t direction, uint8_t speed, uint16_t pulses, uint8_t mode)
{
	uint8_t index = (speed - CONTROLLER_CALIBRATION_MIN_SPEED) / CONTROLLER_CALIBRATION_SPEED_STEP;

		uint16_t left = s_calibration[index].leftTorque;
		if (left > 255) left = 255;
		uint16_t right = s_calibration[index].rightTorque;
		if (right > 255) right = 255;

		controllerMoveStartTorque(direction, speed, pulses, mode, (uint8_t)left, (uint8_t)right);
}
void controllerMoveStartTorque(uint8_t direction, uint8_t speed, uint16_t pulses, uint8_t mode, uint8_t leftStartTorque, uint8_t rightStartTorque)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		if (mode == CONTROLLER_LEFT_AND_RIGHT_COMMON)
		{
			controllerFillOption(&s_common, speed, pulses);
			wheelsResetCounters();
			motorsSetAllSameDirection(direction, leftStartTorque, rightStartTorque);
		}
		else if (mode == CONTROLLER_LEFT_AND_OPPOSITE_RIGHT_COMMON)
		{
			controllerFillOption(&s_common, speed, pulses);
			wheelsResetCounters();
			motorsSetAllOppositeDirection(direction, leftStartTorque, rightStartTorque);
		}
		else if (mode == CONTROLLER_LEFT_AND_RIGHT)
		{
			controllerFillOption(&s_left, speed, pulses);
			controllerFillOption(&s_right, speed, pulses);
			wheelsResetCounters();
			motorsSetLeft(direction, leftStartTorque);
			motorsSetRight(direction, rightStartTorque);
		}
		else if (mode == CONTROLLER_LEFT_AND_OPPOSITE_RIGHT)
		{
			controllerFillOption(&s_left, speed, pulses);
			controllerFillOption(&s_right, speed, pulses);
			wheelsResetCounters();
			motorsSetLeft(direction, leftStartTorque);
			motorsSetRight(CONTROLLER_OPPOSITE(direction), rightStartTorque);
		}
		else if (mode == CONTROLLER_LEFT)
		{
			controllerFillOption(&s_left, speed, pulses);
			wheelsResetLeftCounter();
			motorsSetLeft(direction, leftStartTorque);
		}
		else if (mode == CONTROLLER_RIGHT)
		{
			controllerFillOption(&s_right, speed, pulses);
			wheelsResetRightCounter();
			motorsSetRight(direction, rightStartTorque);
		}
	}
}

void controllerSetLeftPulses(uint16_t pulses)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		s_left.pulsesRemaining = pulses;
	}
}
void controllerSetRightPulses(uint16_t pulses)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		s_right.pulsesRemaining = pulses;
	}
}
void controllerSetCommonPulses(uint16_t pulses)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		s_common.pulsesRemaining = pulses;
	}
}

uint8_t controllerIsBusy()
{
	uint8_t tmp;
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		tmp = (s_common.enabled == 1 || s_left.enabled == 1 || s_right.enabled == 1);
	}
	return tmp;
}

static void controllerResetData(volatile controllerInternalMoveData_t* data)
{
	data->enabled = 0;
	data->pulsesRemaining = 0;
	data->previousPulses = 0;
	data->pulsesPerCalibrationPeriod = 0;
	data->timerCounter = 0;
	data->calibrateCounter = 0;
	data->wheelLocked = 0;
}
static void controllerFillOption(volatile controllerInternalMoveData_t* data, uint8_t speed, uint16_t pulses)
{
	controllerResetData(data);
	data->pulsesRemaining = pulses;
	data->pulsesPerCalibrationPeriod = speed / 2;
	data->enabled = 1;
}

static void controllerCommonHandler()
{
	uint16_t leftPulses, rightPulses;
	wheelsGetAllCount(&leftPulses, &rightPulses);

	uint16_t commonCount = (leftPulses + rightPulses) / 2;
	uint16_t delta = commonCount - s_common.previousPulses;
	s_common.previousPulses = commonCount;

	if (s_common.pulsesRemaining < delta) s_common.pulsesRemaining = 0; else s_common.pulsesRemaining -= delta;

	if (s_common.pulsesRemaining <= 0)
	{
		motorsStopLeft();
		motorsStopRight();
		s_common.enabled = 0;
		return;
	}

	if (s_common.calibrateCounter++ >= 10)
	{
		s_common.calibrateCounter = 0;

		uint8_t leftTmp, rightTmp;
		int16_t leftTorque, rightTorque;

		motorsGetAllTorque(&leftTmp, &rightTmp);

		leftTorque = (int16_t)leftTmp;
		rightTorque = (int16_t)rightTmp;

		if (CONTROLLER_CALIBRATE_WHILE_MOVING || s_calibrationEnabled)
		{
			int16_t leftPulsesDelta = (leftPulses - s_common.pulsesPerCalibrationPeriod) * CONTROLLER_CHANGE_FACTOR;
			leftTorque -= leftPulsesDelta;

			int16_t rightPulsesDelta = (rightPulses - s_common.pulsesPerCalibrationPeriod) * CONTROLLER_CHANGE_FACTOR;
			rightTorque -= rightPulsesDelta;

			if (leftTorque > 255) leftTorque = 255; else if (leftTorque < 0) leftTorque = 0;
			if (rightTorque > 255) rightTorque = 255; else if (rightTorque < 0) rightTorque = 0;

			s_calibrating.leftTorque = (uint8_t)leftTorque;
			s_calibrating.rightTorque = (uint8_t)rightTorque;
		}

		if ((leftPulses < CONTROLLER_MIN_PULSES || rightPulses < CONTROLLER_MIN_PULSES) && (leftTorque == 255 || rightTorque == 255))
		{
			if(s_common.wheelLocked == 0) s_common.wheelLocked = 1;
		}
		else if (s_common.wheelLocked < CONTROLLER_TO_LOCK_CALIBRATION_TICKS) s_common.wheelLocked = 0;
		if (s_common.wheelLocked >= 1) s_common.wheelLocked++;
		if (s_common.wheelLocked >= CONTROLLER_TO_LOCK_CALIBRATION_TICKS)
		{
			leftTorque = 0;
			rightTorque = 0;
		}
		if (s_common.wheelLocked >= CONTROLLER_TO_LOCK_CALIBRATION_TICKS + CONTROLLER_TO_UNLOCK_CALIBRATION_TICKS)
		{
			s_common.wheelLocked = 0;
			leftTorque = 255;
			rightTorque = 255;
		}

		motorsSetAllTorque((uint8_t)leftTorque, (uint8_t)rightTorque);

		s_common.previousPulses = 0;
		wheelsResetCounters();
	}
}

static int16_t controllerHandler(volatile controllerInternalMoveData_t* data, uint16_t pulses, uint8_t torque, volatile uint8_t* calibrating)
{
	if (data->timerCounter++ >= HANDLER_TICKS)
	{
		data->timerCounter = 0;

		uint16_t delta = pulses - data->previousPulses;
		data->previousPulses = pulses;

		if (data->pulsesRemaining < delta) data->pulsesRemaining = 0; else data->pulsesRemaining -= delta;

		if (data->pulsesRemaining <= 0)
		{
			data->enabled = 0;
			return -2;
		}

		if (data->calibrateCounter++ >= 10)
		{
			data->calibrateCounter = 0;

			int16_t newTorque = (int16_t)torque;

			if (CONTROLLER_CALIBRATE_WHILE_MOVING || s_calibrationEnabled)
			{
				int16_t pulsesDelta = (pulses - data->pulsesPerCalibrationPeriod) * CONTROLLER_CHANGE_FACTOR;
				newTorque -= pulsesDelta;

				if (newTorque > 255) newTorque = 255; else if (newTorque < 0) newTorque = 0;

				*calibrating = (uint8_t)newTorque;
			}

			data->previousPulses = 0;

			if (pulses < CONTROLLER_MIN_PULSES && newTorque == 255)
			{
				if(data->wheelLocked == 0) data->wheelLocked = 1;
			}
			else if (data->wheelLocked < CONTROLLER_TO_LOCK_CALIBRATION_TICKS) data->wheelLocked = 0;
			if (data->wheelLocked >= 1) data->wheelLocked++;
			if (data->wheelLocked >= CONTROLLER_TO_LOCK_CALIBRATION_TICKS) newTorque = 0;
			if (data->wheelLocked >= CONTROLLER_TO_LOCK_CALIBRATION_TICKS + CONTROLLER_TO_UNLOCK_CALIBRATION_TICKS)
			{
				data->wheelLocked = 0;
				newTorque = 255;
			}

			return newTorque;
		}
	}

	return -1;
}

ISR(TIMER2_OVF_vect)
{
	if (s_timerTickFunction != NULL) s_timerTickFunction();

	if (s_common.enabled)
	{
		if (s_common.timerCounter++ >= HANDLER_TICKS)
		{
			s_common.timerCounter = 0;

			controllerCommonHandler();
		}


	}
	else
	{
		if (s_left.enabled)
		{
			int16_t result = controllerHandler(&s_left, wheelsGetLeftCount(), motorsGetLeftTorque(), &(s_calibrating.leftTorque));
			switch (result)
			{
			case -2:
				motorsStopLeft();
				break;

			case -1:
				break;

			default:
				motorsSetLeftTorque((uint8_t)result);
				wheelsResetLeftCounter();
				break;
			}
		}

		if (s_right.enabled)
		{
			int16_t result = controllerHandler(&s_right, wheelsGetRightCount(), motorsGetRightTorque(), &(s_calibrating.rightTorque));
			switch (result)
			{
			case -2:
				motorsStopRight();
				break;

			case -1:
				break;

			default:
				motorsSetRightTorque((uint8_t)result);
				wheelsResetRightCounter();
				break;
			}
		}
	}
}
