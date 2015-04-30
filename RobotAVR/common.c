#include "common.h"

void delayMsLong(uint16_t ms)
{
	while(ms-- > 0) _delay_ms(1);
}
