#include "asyncBuffer.h"

asyncBuffer_t asyncBufferInit(uint8_t* buffer, uint8_t size)
{
	asyncBuffer_t result;

	result.pointer = buffer;
	result.size = size;
	result.readOffset = 0;
	result.writeOffset = 0;

	return result;
}

uint8_t asyncBufferAvailableDataLengthNoBlock(volatile asyncBuffer_t* asyncBuffer)
{
	if (asyncBuffer->readOffset == asyncBuffer->writeOffset)
	{
		return 0;
	}
	else if (asyncBuffer->writeOffset > asyncBuffer->readOffset)
	{
		return asyncBuffer->writeOffset - asyncBuffer->readOffset;
	}
	else // readOffset > writeOffset
	{
		return asyncBuffer->writeOffset + (asyncBuffer->size - asyncBuffer->readOffset);
	}
}
uint8_t asyncBufferAvailableDataLength(volatile asyncBuffer_t* asyncBuffer)
{
	uint8_t result;
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		result = asyncBufferAvailableDataLengthNoBlock(asyncBuffer);
	}
	return result;
}

void asyncBufferWriteUint8NoBlock(volatile asyncBuffer_t* asyncBuffer, uint8_t data)
{
	*(asyncBuffer->pointer + asyncBuffer->writeOffset) = data;

	asyncBuffer->writeOffset++;
	if (asyncBuffer->writeOffset >= asyncBuffer->size) asyncBuffer->writeOffset = 0;
}
void asyncBufferWriteUint8(volatile asyncBuffer_t* asyncBuffer, uint8_t data)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		asyncBufferWriteUint8NoBlock(asyncBuffer, data);
	}
}

uint8_t asyncBufferReadUint8NoBlock(volatile asyncBuffer_t* asyncBuffer)
{
	uint8_t result;
	result = *(asyncBuffer->pointer + asyncBuffer->readOffset);

	asyncBuffer->readOffset++;
	if (asyncBuffer->readOffset >= asyncBuffer->size) asyncBuffer->readOffset = 0;
	return result;
}
uint8_t asyncBufferReadUint8(volatile asyncBuffer_t* asyncBuffer)
{
	uint8_t result;
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		result = asyncBufferReadUint8NoBlock(asyncBuffer);
	}
	return result;
}
