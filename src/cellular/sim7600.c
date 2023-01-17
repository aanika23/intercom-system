#include <stdio.h>
#include <string.h>

#include "sim7600.h"
#include "../hardware/uart.h"
#include "../linux/bbg_time.h"

void SIM7600_initialize(void) {
    UART_initialize(UART1, B1152000);
}

void SIM7600_cleanUp(void) {
     UART_cleanUp();
}

bool SIM7600_sendCommand(char* command, int commandSize, char* expectedReturn, int expectedReturnSize) {
    char buffer[UART_BUFFER_SIZE] = {0};

    printf("TX: %s\n", command);
    int attempt = 0;
    while(attempt < 5) {
        printf("RX Attempt %d\n", attempt);
        UART_write(UART1, command, commandSize);
        TIME_sleepForMs(100);
        UART_read(UART1, buffer, UART_BUFFER_SIZE);
        printf("RX: %s\n", buffer);
        char* inString = strstr(buffer, expectedReturn);

        if(inString) {
            return true;
        }

        attempt++;
    }

    return false;
}

bool SIM7600_sendCommandGetBuffer(char* command, int commandSize, char* expectedReturn, char** returnBuffer) {
    char buffer[UART_BUFFER_SIZE] = {0};

    printf("TX: %s\n", command);
    int attempt = 0;
    while(attempt < 5) {
        printf("RX Attempt %d\n", attempt);
        UART_write(UART1, command, commandSize);
        TIME_sleepForMs(100);
        UART_read(UART1, buffer, UART_BUFFER_SIZE);
        printf("RX: %s\n", buffer);
        char* inString = strstr(buffer, expectedReturn);

        if(inString) {
            *returnBuffer = buffer;
            return true;
        }

        attempt++;
    }

    return false;
}


void SIM7600_getBufferUART(char** bufferUART) {
    char buffer[UART_BUFFER_SIZE] = {0};
    UART_read(UART1, buffer, UART_BUFFER_SIZE);
    printf("RX: %s\n", buffer);
    buffer[UART_BUFFER_SIZE-1] = '\0';
    *bufferUART = buffer;

}
