#ifndef ROBOTAVR_ASYNCBUFFER_H_
#define ROBOTAVR_ASYNCBUFFER_H_

#include <stdint.h>
#include <util/atomic.h>

typedef struct
{
	uint8_t* pointer;
	uint8_t size;
	uint8_t readOffset;
	uint8_t writeOffset;
}asyncBuffer_t;

asyncBuffer_t asyncBufferInit(uint8_t* buffer, uint8_t size);

uint8_t asyncBufferAvailableDataLength(volatile asyncBuffer_t* asyncBuffer);
uint8_t asyncBufferAvailableDataLengthNoBlock(volatile asyncBuffer_t* asyncBuffer);

void asyncBufferWriteUint8(volatile asyncBuffer_t* asyncBuffer, uint8_t data);
void asyncBufferWriteUint8NoBlock(volatile asyncBuffer_t* asyncBuffer, uint8_t data);

uint8_t asyncBufferReadUint8(volatile asyncBuffer_t* asyncBuffer);
uint8_t asyncBufferReadUint8NoBlock(volatile asyncBuffer_t* asyncBuffer);

#endif /* ROBOTAVR_ASYNCBUFFER_H_ */
