/**
 ** i2c LCD test application
 **
 ** Author: Kaj-Michael Lang <milang@tal.org>
 ** Copyright 2014 - Under creative commons license 3.0 Attribution-ShareAlike CC BY-SA
 ** /

/***********************************************************************
 * Code listing from "Advanced Linux Programming," by CodeSourcery LLC  *
 * Copyright (C) 2001 by New Riders Publishing                          *
 * See COPYRIGHT for license information.                               *
 ***********************************************************************/

/***********************************************************************
 * Code from ENSC 351 Brian Fraser                                     *
 ***********************************************************************/

#include "LiquidCrystal_I2C.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#define ROWS 2
#define COLUMNS 16

#define MSG_MAX_LEN 1024
#define PORT 55934

	void writer(const char *message, int count, FILE *stream);
void reader(FILE *stream);

void listenToUDPAndWrite(FILE *stream)
{

	// GPIO chip i2c address
	uint8_t i2c = 0x27;
	// Control line PINs
	uint8_t en = 2;
	uint8_t rw = 1;
	uint8_t rs = 0;

	// Data line PINs
	uint8_t d4 = 4;
	uint8_t d5 = 5;
	uint8_t d6 = 6;
	uint8_t d7 = 7;

	// Backlight PIN
	uint8_t bl = 3;

	int adapter_nr = 1;
	char filename[20];

	snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);

	printf("Using i2c device: %s, Rows: %u, Cols: %u\n", filename, ROWS, COLUMNS);

	LiquidCrystal_I2C lcd(filename, i2c, en, rw, rs, d4, d5, d6, d7, bl, POSITIVE);

	// Initialize
	lcd.begin(COLUMNS, ROWS);
	lcd.on();
	lcd.clear();

	lcd.print("Host use command:");
	lcd.setCursor(0, 1);
	lcd.print("netcat -u");
	lcd.setCursor(0, 2);
	lcd.print("192.168.7.2 55934");

	// Address
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;				 // Connection may be from network
	sin.sin_addr.s_addr = htonl(INADDR_ANY); // Host to Network long
	sin.sin_port = htons(PORT);				 // Host to Network short

	// Create the socket for UDP
	int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

	// Bind the socket to the port (PORT) that we specify
	bind(socketDescriptor, (struct sockaddr *)&sin, sizeof(sin));

	// Check for errors (-1)

	while (1)
	{
		// Get the data (blocking)
		// Will change sin (the address) to be the address of the client.
		// Note: sin passes information in and out of call!
		struct sockaddr_in sinRemote;
		unsigned int sin_len = sizeof(sinRemote);
		char messageRx[MSG_MAX_LEN];

		// Pass buffer size - 1 for max # bytes so room for the null (string data)
		int bytesRx = recvfrom(socketDescriptor,
							   messageRx, MSG_MAX_LEN - 1, 0,
							   (struct sockaddr *)&sinRemote, &sin_len);

		// Check for errors (-1)

		// Make it null terminated (so string functions work)
		// - recvfrom given max size - 1, so there is always room for the null
		messageRx[bytesRx] = 0;
		printf("[CHILD] Message received (%d bytes): \n\n[CHILD]'%s'\n", bytesRx, messageRx);
		lcd.clear();
		lcd.setCursor(0,2);
		lcd.print(messageRx);
		//lcd.setCursor(0,1);
		printf("HERE\n");
			// Process received data
			if (strncmp(messageRx, "quit", strlen("quit")) == 0)
		{
			printf("That's it! Lecture is done!\n");
			break;
		}
		else
		{
			// Relay received data into pipe
			// Write the message to the stream, and send it off immediately.
			fprintf(stream, "%s", messageRx);
			fflush(stream);
		}
	}

	// Close
	lcd.clear();
	close(socketDescriptor);
}

int main()
{

	// Create a pipe. File descriptors for two ends of pipe are placed in fds
	int fds[2];
	pipe(fds);

	// Fork a child process.
	pid_t pid = fork();
	printf("My PID is %d.\n", getpid());
	if (pid == (pid_t)0)
	{
		// This is the child process.
		printf("We are the child, time to liste and write\n");
		// Close our copy of the read end of the file descriptor.
		close(fds[0]);

		// Convert write file descriptor to a FILE object, and write to it
		FILE *stream = fdopen(fds[1], "w");
		listenToUDPAndWrite(stream);
		close(fds[0]);
	}
	else
	{
		// This is the parent process.
		printf("We are the parent! It's time to read from the stream!\n");
		// Close our copy of the write end of the file descriptor.
		close(fds[1]);

		// Convert the read file descriptor to a FILE object, and read from it
		FILE *stream = fdopen(fds[0], "r");

		// READ!
		reader(stream);

		close(fds[0]);
	}

	return 0;
}

// Read random strings from the stream as long as possible.
void reader(FILE *stream)
{
	char buffer[1024];
	// Read until we hit the end of the stream.
	// fgets reads until either a newline or the end-of-file.
	while (!feof(stream) && !ferror(stream) && fgets(buffer, sizeof(buffer), stream) != NULL)
	{
		printf("[PARENT] %s\n", buffer);
		// fputs (buffer, stdout);
	}
}
