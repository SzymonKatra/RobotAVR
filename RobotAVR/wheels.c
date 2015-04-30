#include "wheels.h"

#define LEFT_DDR DDRD
#define LEFT_PORT PORTD
#define LEFT_BIT PD2

#define RIGHT_DDR DDRD
#define RIGHT_PORT PORTD
#define RIGHT_BIT PD3

static volatile uint16_t s_leftCount;
static volatile uint16_t s_rightCount;

void wheelsInit()
{
	s_leftCount = 0;
	s_rightCount = 0;

	// set as inputs with pull-ups
	LEFT_DDR &= ~(1 << LEFT_BIT);
	LEFT_PORT |= (1 << LEFT_BIT);

	RIGHT_DDR &= ~(1 << RIGHT_BIT);
	RIGHT_PORT |= (1 << RIGHT_BIT);

	// interrupt on falling edge
	EICRA |= (1 << ISC11) | (1 << ISC01);
	EIMSK |= (1 << INT1) | (1 << INT0);
}

void wheelsResetLeftCounter()
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		s_leftCount = 0;
	}
}
void wheelsResetRightCounter()
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		s_rightCount = 0;
	}
}
void wheelsResetCounters()
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		s_leftCount = 0;
		s_rightCount = 0;
	}
}

uint16_t wheelsGetLeftCount()
{
	uint16_t tmp;
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		tmp = s_leftCount;
	}
	return tmp;
}
uint16_t wheelsGetRightCount()
{
	uint16_t tmp;
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		tmp = s_rightCount;
	}
	return tmp;
}

void wheelsGetAllCount(uint16_t* left, uint16_t* right)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		*left = s_leftCount;
		*right = s_rightCount;
	}
}

ISR(INT0_vect)
{
	s_rightCount++;
	/*if(s_rightCount >= 20)
	{
		s_rightCount = 0;
		ledToggle();
	}*/
}
ISR(INT1_vect)
{
	s_leftCount++;
	/*if(s_leftCount >= 20)
	{
		s_leftCount = 0;
		ledToggle();
	}*/
}
