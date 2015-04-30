#include "remote.h"

#define RECEIVE_BUFFER_SIZE 128
#define TRANSMIT_BUFFER_SIZE 128

static uint8_t s_receiveBuffer[RECEIVE_BUFFER_SIZE];
static uint8_t s_transmitBuffer[TRANSMIT_BUFFER_SIZE];
static volatile asyncBuffer_t s_receiveBufferState;
static volatile asyncBuffer_t s_transmitBufferState;
static remoteReceived_f s_receivedFunction;

static void remoteInitInternal();

void remoteInit()
{
	s_receiveBufferState = asyncBufferInit(s_receiveBuffer, RECEIVE_BUFFER_SIZE);
	s_receivedFunction = NULL;

	remoteInitInternal();
}
void remoteInitAsync(remoteReceived_f receivedFunction)
{
	s_receivedFunction = receivedFunction;

	remoteInitInternal();
}

uint8_t remotePoll(uint8_t* result)
{
	if (asyncBufferAvailableDataLength(&s_receiveBufferState) > 0)
	{
		*result = asyncBufferReadUint8(&s_receiveBufferState);
		return 1;
	}
	else return 0;
}
void remoteSend(uint8_t data)
{
	asyncBufferWriteUint8(&s_transmitBufferState, data);
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		while(asyncBufferAvailableDataLength(&s_transmitBufferState) > 0)
		{
			while (!( UCSR0A & (1<<UDRE0)));
			UDR0 = asyncBufferReadUint8NoBlock(&s_transmitBufferState);
		}
		/*if ((UCSR0A & UDRE0) && asyncBufferAvailableDataLength(&s_transmitBufferState) > 0)
		{
			UDR0 = asyncBufferReadUint8NoBlock(&s_transmitBufferState);
		}*/
	}
}
void remoteSendString(const char* data)
{
	while (data++ != '\0') remoteSend(*data);
}

static void remoteInitInternal()
{
	s_transmitBufferState = asyncBufferInit(s_transmitBuffer, TRANSMIT_BUFFER_SIZE);

	UBRR0H = (uint8_t)(REMOTE_UBRR>>8);
	UBRR0L = (uint8_t)REMOTE_UBRR;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0)/* | (1 << UDRIE0)*/; // receiver, transmitter, receive interrupt, buffer empty (ready to transmit) interrupt
	UCSR0C = (3 << UCSZ00); // 8data, 1stop bit
}

ISR(USART_RX_vect)
{
	uint8_t data = UDR0;
	if (s_receivedFunction == NULL)
	{
		asyncBufferWriteUint8NoBlock(&s_receiveBufferState, data);
	}
	else
	{
		s_receivedFunction(data);
	}
}

/*ISR(USART_UDRE_vect)
{
	if (asyncBufferAvailableDataLengthNoBlock(&s_transmitBufferState) > 0)
	{
		UDR0 = asyncBufferReadUint8NoBlock(&s_transmitBufferState);
	}
}*/
