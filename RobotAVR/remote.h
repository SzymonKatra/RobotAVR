#ifndef ROBOTAVR_REMOTE_H_
#define ROBOTAVR_REMOTE_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <string.h>
#include "asyncBuffer.h"

#define REMOTE_UBRR 103 // 9600 bps, 16 MHz

typedef void(*remoteReceived_f)(uint8_t);

void remoteInit();
void remoteInitAsync(remoteReceived_f receivedFunction);
uint8_t remotePoll(uint8_t* result);
void remoteSend(uint8_t data);
void remoteSendString(const char* data);

#endif /* ROBOTAVR_REMOTE_H_ */
