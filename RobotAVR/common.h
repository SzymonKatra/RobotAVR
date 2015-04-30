#ifndef ROBOTAVR_COMMON_H_
#define ROBOTAVR_COMMON_H_

#include <avr/io.h>
#include <util/delay.h>

#define ledInit() DDRD |= (1 << PD4)
#define ledOn() PORTD |= (1 << PD4)
#define ledOff() PORTD &= ~(1 << PD4)
#define ledToggle() PORTD ^= (1 << PD4)

#define buttonInit() DDRB &= ~(1 << PB3); PORTB |= (1 << PB3)
#define buttonIsPressed() (!(PINB & (1 << PB3)))

void delayMsLong(uint16_t ms);

#endif /* ROBOTAVR_COMMON_H_ */
