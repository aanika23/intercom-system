#ifndef SIM7600_H
#define SIM7600_H

#include <stdbool.h>

#define UART_BUFFER_SIZE 4095

void SIM7600_initialize(void);
bool SIM7600_sendCommand(char* command, int commandSize, char* expectedReturn, int expectedReturnSize);
bool SIM7600_sendCommandGetBuffer(char* command, int commandSize, char* expectedReturn, char** returnBuffer);
void SIM7600_getBufferUART(char** bufferUART);
void SIM7600_cleanUp(void);

#endif