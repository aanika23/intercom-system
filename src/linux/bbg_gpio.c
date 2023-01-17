#include <stdio.h>
#include <stdlib.h>

#include <errno.h>		// Errors
#include <string.h>

#include <sys/epoll.h>  // for epoll()
#include <fcntl.h>      // for open()
#include <unistd.h>     // for close()

#include "bbg_gpio.h"
#include "bbg_linux.h"

#define GPIO_PATH "/sys/class/gpio/gpio%d/"

#define BUFFER_SIZE 100

void GPIO_initialize(){
    //Mode Button 
    LINUX_runCommand("config-pin p8.15 gpio");
    //Base Drum Button 
    LINUX_runCommand("config-pin p8.16 gpio");
    //Snare drum Button 
    LINUX_runCommand("config-pin p8.17 gpio");
    //Hihat Button 
    LINUX_runCommand("config-pin p8.18 gpio");
}

void GPIO_cleanup() {
	// Empty
}

int GPIO_buttonIsDown(GPIO_t GPIO) {
    char buttonPath[BUFFER_SIZE/2];
    snprintf(buttonPath, BUFFER_SIZE/2, GPIO_PATH, GPIO);

    char valuePath[BUFFER_SIZE];
    snprintf(valuePath, BUFFER_SIZE, "%s%s", buttonPath, "value");

    FILE *pButtonFile =  fopen(valuePath, "r");

    if (pButtonFile == NULL) {
        printf("ERROR OPENING: %s\n", valuePath);
        exit(1); 
    }

    int value = 0;
    fscanf(pButtonFile,"%d", &value);

    fclose(pButtonFile);    

    return(value);
}

int GPIO_risingEdge(GPIO_t GPIO) 
{
	// If you want to wait for input on multiple file, you could change this function
	// to take an array of names, and then loop throug each, adding it to the 
	// epoll instance (accessed via epollfd) by calling epoll_ctl() numerous times.

    char buttonPath[BUFFER_SIZE/2];
    snprintf(buttonPath, BUFFER_SIZE/2, GPIO_PATH, GPIO);

	char edgePath[BUFFER_SIZE];
    snprintf(edgePath, BUFFER_SIZE, "%s%s", buttonPath, "edge");
	LINUX_writeToFile(edgePath, "rising");

	char directionPath[BUFFER_SIZE];
    snprintf(directionPath, BUFFER_SIZE, "%s%s", buttonPath, "direction");
	LINUX_writeToFile(edgePath, "in");

    char valuePath[BUFFER_SIZE];
    snprintf(valuePath, BUFFER_SIZE, "%s%s", buttonPath, "value");

	// create an epoll instance
	// .. kernel ignores size argument; must be > 0
	int epollfd = epoll_create(1);
	if (epollfd == -1) {
		fprintf(stderr, "ERROR: epoll_create() returned (%d) = %s\n", epollfd, strerror(errno));
		return -1;
	}

	// open GPIO value file:
	int fdLeft = open(valuePath, O_RDONLY | O_NONBLOCK);
	if (fdLeft == -1) {
		fprintf(stderr, "ERROR: unable to open() GPIO value file (%d) = %s\n", fdLeft, strerror(errno));
		return -1;
	}

	// configure epoll to wait for events: read operation | edge triggered | urgent data
	struct epoll_event epollStruct;
	epollStruct.events = EPOLLIN | EPOLLET | EPOLLPRI;
	epollStruct.data.fd = fdLeft;

	// register file descriptor on the epoll instance, see: man epoll_ctl
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fdLeft, &epollStruct) == -1) {
		fprintf(stderr, "ERROR: epoll_ctl() failed on add control interface (%d) = %s\n", fdLeft, strerror(errno));
		return -1;
	}

	// ignore first trigger
	for (int i = 0; i < 2; i++) {
		int waitRet = epoll_wait(
				epollfd, 
				&epollStruct, 
				1,                // maximum # events
				-1);              // timeout in ms, -1 = wait indefinite; 0 = returne immediately

		if (waitRet == -1){
			fprintf(stderr, "ERROR: epoll_wait() failed (%d) = %s\n", waitRet, strerror(errno));
			close(fdLeft);
			close(epollfd);
			return -1;
		}
	}

	// cleanup epoll instance:
	close(fdLeft);
	close(epollfd);
	return 0;
}
