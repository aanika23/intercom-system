// Watchdog example
// by Brian Fraser
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "watchdog.h"	

// set watchdog timeout to 5 min (300s)
#define WATCHDOG_TIMEOUT 300

static int timeSinceLastHit = 0;
static int timeout = 0;

// int main(int argCount, char* args[])
// {
// 	// Help
// 	printf("Watchdog demo app. Use:\n");
// 	printf("  %s - Run app\n", args[0]);
// 	printf("  %s [timeout [s]] - Set timeout and run app\n", args[0]);
// 	printf("\n");

// 	// Read current timeout.
// 	timeout = readWatchdogTimeout();
// 	printf("Current watchdog interval is %d seconds.\n", timeout);

// 	// Change and re-read current timeout.
// 	if (argCount > 1) {
// 		int newTimeout = atoi(args[1]);
// 		changeWatchdogTimeout(newTimeout);
// 		timeout = readWatchdogTimeout();
// 		printf("Updated watchdog interval is %d seconds.\n", timeout);
// 	}

// 	// Start background thread to show time ticking
// 	pthread_t id;
// 	pthread_create(&id, NULL, &WATCHDOG_showTimeThread, NULL);

// 	// Allow user to experiment with watchdog hitting.
// 	hitWatchdogOnKeypress();

// 	return 0;
// }


static void WATCHDOG_dieOnError(_Bool successCondition, char *message);
static int WATCHDOG_readWatchdogTimeout();
static void WATCHDOG_changeWatchdogTimeout(int timeout_s);
static void* WATCHDOG_showTimeThread(void* notUsed);

int main()
{
	WATCHDOG_initialize();
}

void WATCHDOG_initialize()
{
    WATCHDOG_changeWatchdogTimeout(WATCHDOG_TIMEOUT);
	timeout = WATCHDOG_readWatchdogTimeout();
	printf("Watchdog time out is %d seconds.\n", timeout);

	// Start background thread to show time ticking
	pthread_t id;
	pthread_create(&id, NULL, &WATCHDOG_showTimeThread, NULL);

	WATCHDOG_hitWatchdogFromMain();
}

static int WATCHDOG_readWatchdogTimeout()
{
	int fd = open("/dev/watchdog", O_RDWR);
	WATCHDOG_dieOnError(fd != -1, "Unable to open WD.");

	int interval_s = 0;
	int result = ioctl(fd, WDIOC_GETTIMEOUT, &interval_s);
	WATCHDOG_dieOnError(result == 0, "Unable to read watchdog timeout.");

	close(fd);
	return interval_s;
}

static void WATCHDOG_changeWatchdogTimeout(int timeout_s)
{
	printf("Setting watchdog timout to %ds\n", timeout_s);

	int fd = open("/dev/watchdog", O_RDWR);
	WATCHDOG_dieOnError(fd != -1, "Unable to open WD.");

	int result = ioctl(fd, WDIOC_SETTIMEOUT, &timeout_s);
	WATCHDOG_dieOnError(result == 0, "Unable to set watchdog timout.");

	close(fd);
}

void WATCHDOG_hitWatchdogFromMain()
{
	int fd = open("/dev/watchdog", O_RDWR);
	WATCHDOG_dieOnError(fd != -1, "Unable to open WD.");

	while (1) {
		FILE *pFile = fopen("awake.txt", "r");
		if (pFile == NULL)
		{
			perror("Error in opening awake.txt");	
		}

		// read first character in file
		char textInFile = fgetc(pFile);
		fclose(pFile);
		
		if (textInFile == '1')
		{
			printf("\n\nWATCHDOG: main has set awake.txt to 1\n");
			printf("WATCHDOG: now setting awake.txt to 0\n");
			
			// write text to WD driver to keep it awake
			write(fd, "w", 1);

			// write 0 to awake.txt 
			FILE *pFileW = fopen("awake.txt", "w");
			if (pFileW == NULL)
			{
				printf("ERROR PFILEW IS NULL\n");
			}
			
			char zero = '0';
			fprintf(pFileW, "%c", zero);
			
			fclose(pFileW);
			
			// reset watchdog timer
			timeSinceLastHit = 0;
		}
	}

	// Close file to disable watchdog (or exit program auto-closes)
	close(fd);
}

static void WATCHDOG_dieOnError(_Bool successCondition, char *message)
{
	if (!successCondition) {
		fprintf(stderr, "ERROR: %s\n", message);
		fprintf(stderr, "Error string: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}


// Show the time on background thread
static void* WATCHDOG_showTimeThread(void* notUsed) {
	while (true) {
		sleep(1);
		timeSinceLastHit++;
		printf("%2ds since WD hit (timeout = %d).\n", timeSinceLastHit, timeout);
	}
	return 0;
}