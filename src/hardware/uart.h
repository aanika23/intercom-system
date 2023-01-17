#ifndef UART_H
#define UART_H

#include <termios.h>

typedef enum{
    UART0 = 0,
    UART1,
    UART2,
    UART3,
    UART4,
    UART5
} serialPort_t;

void UART_initialize(serialPort_t serialPort, int baudRate);
void UART_cleanUp(void);
void UART_write(serialPort_t serialPort, char *message, int messageSize);
void UART_read(serialPort_t serialPort, char *message, int messageSize);

#endif
