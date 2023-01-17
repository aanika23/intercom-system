#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "uart.h"
#include "../linux/bbg_linux.h"
#include "../linux/bbg_pinout.h"

#define UART_TTY "/dev/ttyS%d"
#define CONFIG_PIN_UART "config-pin %s uart"
#define UART_MAX 5

#define BUFFER_SIZE 100

struct UART_options {
    int fileDescriptor;
    int portAttributes;
    int baudRate;
};

struct termios options;
struct UART_options UART[UART_MAX];

static void UART_configurePins(char* TX, char* RX);
static void UART_configurePort(serialPort_t serialPort);
static void UART_open(serialPort_t serialPort);
// static void UART_setSerialPath(serialPort_t serialPort);
static void UART_setBaudRate(serialPort_t serialPort, int baudRate);

static void UART_configurePins(char* TX, char* RX) {
    char bufferTX[BUFFER_SIZE];
    char bufferRX[BUFFER_SIZE];
    snprintf(bufferTX, BUFFER_SIZE, CONFIG_PIN_UART, TX);
    snprintf(bufferRX, BUFFER_SIZE, CONFIG_PIN_UART, RX);
    LINUX_runCommand(bufferTX);
    LINUX_runCommand(bufferRX);
}

// static void UART_setSerialPath(serialPort_t serialPort) {
//     char buffer[BUFFER_SIZE];
//     char* serialPath = snprintf(buffer, BUFFER_SIZE, UART_TTY, serialPort);
// }

static void UART_setBaudRate(serialPort_t serialPort, int baudRate) {
    UART[serialPort].baudRate = baudRate;
}

static void UART_open(serialPort_t serialPort) {

    char serialPath[BUFFER_SIZE];
    snprintf(serialPath, BUFFER_SIZE, UART_TTY, serialPort);
    
    UART[serialPort].fileDescriptor = open(serialPath, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
    if(UART[serialPort].fileDescriptor < 0) {
        printf("ERROR: %s was unable to open. File Descriptor: %d\r\n",  serialPath, UART[serialPort].fileDescriptor);
        exit(EXIT_FAILURE);
    }
}

static void UART_configurePort(serialPort_t serialPort) {

    char serialPath[BUFFER_SIZE];
    snprintf(serialPath, BUFFER_SIZE, UART_TTY, serialPort);

    UART[serialPort].portAttributes = tcgetattr(UART[serialPort].fileDescriptor, &options);
    
    if(UART[serialPort].portAttributes != 0) {
        printf("ERROR: %s was unable to set port attributes. Port Attributes: %d\r\n", 
            serialPath, UART[serialPort].portAttributes);
        exit(EXIT_FAILURE);
    }

    fcntl (UART[serialPort].fileDescriptor, F_SETFL, O_RDWR);
    tcgetattr(UART[serialPort].fileDescriptor, &options);
	cfmakeraw(&options);
	cfsetispeed (&options, B115200);
	cfsetospeed (&options, B115200);

    options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;

    tcsetattr (UART[serialPort].fileDescriptor, TCSANOW, &options);

    if(tcsetattr(UART[serialPort].fileDescriptor, TCSANOW, &options)) {
        printf("ERROR: %s was unable to set port attributes. Port Attributes: %d\r\n", 
            serialPath, UART[serialPort].portAttributes);
        exit(EXIT_FAILURE);
    }
}

void UART_initialize(serialPort_t serialPort, int baudRate) {

    switch (serialPort)
    {
    case UART0:
        UART_configurePins(UART0_TX, UART0_RX);
        break;
    case UART1:
        UART_configurePins(UART1_TX, UART1_RX);
        break;
    case UART2:
        UART_configurePins(UART2_TX, UART2_RX);
        break;
    // UART 3 DOES NOT HAVE GPIO    
    case UART4:
        UART_configurePins(UART4_TX, UART4_RX);
        break;
    case UART5:
        UART_configurePins(UART5_TX, UART5_RX);
        break;
    default:
        printf("ERROR: Unknown pins for UART%d, please implement this functionalty in the file uart.c\n", serialPort);
        exit(-1);
        break;
    }

    // UART_setSerialPath(serialPort);
    UART_setBaudRate(serialPort, baudRate);
    UART_open(serialPort);
    UART_configurePort(serialPort);
}

void UART_cleanUp(void) {

    for(int i = 0; i < UART_MAX; i++) {
        close(UART[i].fileDescriptor);
    }
}

void UART_write(serialPort_t serialPort, char* message, int messageSize) {

    write(UART[serialPort].fileDescriptor, message, messageSize);
}

void UART_read(serialPort_t serialPort, char* message, int messageSize) {
    read(UART[serialPort].fileDescriptor, message, messageSize);
}