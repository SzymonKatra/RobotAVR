#include "sensors.h"

#define LEFT_TRIG_DDR DDRC
#define LEFT_TRIG_PORT PORTC
#define LEFT_TRIG_BIT PC3

#define RIGHT_TRIG_DDR DDRC
#define RIGHT_TRIG_PORT PORTC
#define RIGHT_TRIG_BIT PC1

#define FRONT_TRIG_DDR DDRC
#define FRONT_TRIG_PORT PORTC
#define FRONT_TRIG_BIT PC5

// all ECHOS belong to PCMSK1 and C segment

#define LEFT_ECHO_BIT PC2
#define LEFT_ECHO_PCINT PCINT10

#define RIGHT_ECHO_BIT PC0
#define RIGHT_ECHO_PCINT PCINT8

#define FRONT_ECHO_BIT PC4
#define FRONT_ECHO_PCINT PCINT12

#define MEASURE_TICKS 3 // measure every ~ 100 ms

static volatile uint16_t s_leftTime;
static volatile uint16_t s_rightTime;
static volatile uint16_t s_frontTime;

static uint16_t s_leftBeginPulseTick;
static uint16_t s_rightBeginPulseTick;
static uint16_t s_frontBeginPulseTick;

static uint8_t s_PINCState;

static uint8_t s_currentSensor;

static void sensorsEchoChangedHandler(volatile uint16_t* time, uint16_t* beginPulseTick);

void sensorsInit()
{
	// outputs
	LEFT_TRIG_DDR |= (1 << LEFT_TRIG_BIT);
	RIGHT_TRIG_DDR |= (1 << RIGHT_TRIG_BIT);
	FRONT_TRIG_DDR |= (1 << FRONT_TRIG_BIT);

	// inputs witout pull-ups
	PORTC &= ~(1 << LEFT_ECHO_BIT);
	PORTC &= ~(1 << RIGHT_ECHO_BIT);
	PORTC &= ~(1 << FRONT_ECHO_BIT);

	DDRC &= ~(1 << LEFT_ECHO_BIT);
	DDRC &= ~(1 << RIGHT_ECHO_BIT);
	DDRC &= ~(1 << FRONT_ECHO_BIT);

	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << LEFT_ECHO_PCINT) | (1 << RIGHT_ECHO_PCINT) | (1 << FRONT_ECHO_PCINT);

	s_leftTime = 0;
	s_rightTime = 0;
	s_frontTime = 0;

	s_currentSensor = 0;
	OCR1A = 22;
	TIMSK1 |= (1 << TOIE1); // interrupt every 32.768 ms, each sensor updates every ~ 100ms alternately
	TIMSK1 |= (1 << OCIE1A); // interrupt on OCR1A compare match (22 ticks = 11 us) to end pulse for sensors
	TCCR1B |= (1 << CS11); // clk / 8 = 2 MHz
	TCNT1 = 0;
}

uint16_t sensorsGetLeftTime()
{
	uint16_t result;
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		result = s_leftTime;
	}
	return result;
}
uint16_t sensorsGetRightTime()
{
	uint16_t result;
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		result = s_rightTime;
	}
	return result;
}
uint16_t sensorsGetFrontTime()
{
	uint16_t result;
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		result = s_frontTime;
	}
	return result;
}

static void sensorsEchoChangedHandler(volatile uint16_t* time, uint16_t* beginPulseTick)
{
	if (*beginPulseTick > 0)
	{
		*time = (TCNT1 - *beginPulseTick) / 2;

		*beginPulseTick = 0;
	}
	else
	{
		*beginPulseTick = TCNT1;
	}
}

ISR(PCINT1_vect)
{
	uint8_t changed = PINC ^ s_PINCState;
	s_PINCState = PINC;

	switch(s_currentSensor)
	{
	case 0:
		if (changed & (1 << LEFT_ECHO_BIT)) sensorsEchoChangedHandler(&s_leftTime, &s_leftBeginPulseTick);
		break;
	case 1:
		if (changed & (1 << RIGHT_ECHO_BIT)) sensorsEchoChangedHandler(&s_rightTime, &s_rightBeginPulseTick);
		break;
	case 2:
		if (changed & (1 << FRONT_ECHO_BIT)) sensorsEchoChangedHandler(&s_frontTime, &s_frontBeginPulseTick);
		break;
	}
}

ISR(TIMER1_OVF_vect)
{
	// TCNT1 = 0 due to overflow

	++s_currentSensor;
	if(s_currentSensor > 2) s_currentSensor = 0;
	switch (s_currentSensor)
	{
	case 0:
		if (s_leftBeginPulseTick > 0)
		{
			s_leftBeginPulseTick = 0;
			s_leftTime = 0xFFFF;
		}
		LEFT_TRIG_PORT |= (1 << LEFT_TRIG_BIT);
		break;
	case 1:
		if (s_rightBeginPulseTick > 0)
		{
			s_rightBeginPulseTick = 0;
			s_rightTime = 0xFFFF;
		}
		RIGHT_TRIG_PORT |= (1 << RIGHT_TRIG_BIT);
		break;
	case 2:
		if (s_frontBeginPulseTick > 0)
		{
			s_frontBeginPulseTick = 0;
			s_frontTime = 0xFFFF;
		}
		FRONT_TRIG_PORT |= (1 << FRONT_TRIG_BIT);
		break;
	}
}

ISR(TIMER1_COMPA_vect)
{
	switch(s_currentSensor)
	{
	case 0:
		LEFT_TRIG_PORT &= ~(1 << LEFT_TRIG_BIT);
		break;
	case 1:
		RIGHT_TRIG_PORT &= ~(1 << RIGHT_TRIG_BIT);
		break;
	case 2:
		FRONT_TRIG_PORT &= ~(1 << FRONT_TRIG_BIT);
		break;
	}
}
