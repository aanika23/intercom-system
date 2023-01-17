#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#include "keypad.h"
#include "../database/database.h"
#include "../cellular/phone.h"
#include "../linux/bbg_time.h"
#include "../linux/bbg_linux.h"
#include "../linux/bbg_gpio.h"

#define LOW "0"
#define HIGH "1"
#define IN "in"
#define OUT "out"

#define STAR 10
#define POUND 11

#define NUMBER_OF_KEYS 12

#define MAX_KEYPAD_ENTRIES 3

// Locks
static pthread_mutex_t buttonLock;
static bool pollKeypad = true;
static bool killButtons = false;
static bool busyButton = false;

pthread_t keyPressedThread;
pthread_t key0Thread;
pthread_t key1Thread;
pthread_t key2Thread;
pthread_t key3Thread;
pthread_t key4Thread;
pthread_t key5Thread;
pthread_t key6Thread;
pthread_t key7Thread;
pthread_t key8Thread;
pthread_t key9Thread;
pthread_t keyStarThread;
pthread_t keyPoundThread;

static int keyCodeDigit[MAX_KEYPAD_ENTRIES];
static int keyPressCount = 0;


static int KEYPAD_calculateKeyCode(int digits);
static void KEYPAD_initGPIO(int gpio);
static void* RHYTHM_buttons(void *arg);


static int KEYPAD_calculateKeyCode(int digits) {
    int keyCode = 0;
    for (int i = 0; i < digits; i++) {
        int keyIndex = pow(10, i);
        keyCode += (keyCodeDigit[digits - i - 1] * keyIndex);
    }

    return keyCode;
} 

void KEYPAD_initialize(void)
{
    LINUX_runCommand("config-pin p8.13 gpio");
    LINUX_runCommand("config-pin p8.09 gpio");
    LINUX_runCommand("config-pin p8.08 gpio");
    LINUX_runCommand("config-pin p8.10 gpio");
    LINUX_runCommand("config-pin p8.12 gpio");
    LINUX_runCommand("config-pin p8.11 gpio");
    
    LINUX_runCommand("config-pin p8.46 gpio");

    // LINUX_runCommand("config-pin p9.31 gpio");
    // LINUX_runCommand("config-pin p9.29 gpio");
    // LINUX_runCommand("config-pin p9.27 gpio");
    // LINUX_runCommand("config-pin p9.25 gpio");
    // LINUX_runCommand("config-pin p9.30 gpio");
    // LINUX_runCommand("config-pin p9.28 gpio");
    
    // KEYPAD_initGPIO(KEY_0);
    // KEYPAD_initGPIO(KEY_1);
    // KEYPAD_initGPIO(KEY_2);
    // KEYPAD_initGPIO(KEY_3);
    // KEYPAD_initGPIO(KEY_4);
    // KEYPAD_initGPIO(KEY_5);
    KEYPAD_initGPIO(KEY_6);
    KEYPAD_initGPIO(KEY_7);
    KEYPAD_initGPIO(KEY_8);
    KEYPAD_initGPIO(KEY_9);
    KEYPAD_initGPIO(KEY_STAR);
    KEYPAD_initGPIO(KEY_POUND);

    // pthread_create(&key0Thread, NULL, &RHYTHM_buttons, (void*)KEY_0);
    // pthread_create(&key1Thread, NULL, &RHYTHM_buttons, (void*)KEY_1);
    // pthread_create(&key2Thread, NULL, &RHYTHM_buttons, (void*)KEY_2);
    // pthread_create(&key3Thread, NULL, &RHYTHM_buttons, (void*)KEY_3);
    // pthread_create(&key4Thread, NULL, &RHYTHM_buttons, (void*)KEY_4);
    // pthread_create(&key5Thread, NULL, &RHYTHM_buttons, (void*)KEY_5);
    pthread_create(&key6Thread, NULL, &RHYTHM_buttons, (void*)KEY_6);
    pthread_create(&key7Thread, NULL, &RHYTHM_buttons, (void*)KEY_7);
    pthread_create(&key8Thread, NULL, &RHYTHM_buttons, (void*)KEY_8);
    pthread_create(&key9Thread, NULL, &RHYTHM_buttons, (void*)KEY_9);
    pthread_create(&keyStarThread, NULL, &RHYTHM_buttons, (void*)KEY_STAR);
    pthread_create(&keyPoundThread, NULL, &RHYTHM_buttons, (void*)KEY_POUND);
    
}

static void KEYPAD_initGPIO(int gpio)
{
    char fileDirectory1[50] = "/sys/class/gpio/gpio";
    char fileDirectory2[50] = "/sys/class/gpio/gpio";
    char gpioString[2];

    sprintf(gpioString, "%d", gpio);
    strcat(fileDirectory1, gpioString);
    strcat(fileDirectory2, gpioString);
    strcat(fileDirectory1, "/direction");
    strcat(fileDirectory2, "/active_low");

    FILE *pFile = fopen(fileDirectory1, "w");
    printf("before\n");
    fprintf(pFile, IN);
    printf("after\n");
    pFile = fopen(fileDirectory2, "w");
    fprintf(pFile, HIGH);
    fclose(pFile);
}

int KEYPAD_checkCX(int gpio)
{
    char fileDirectory[50] = "/sys/class/gpio/gpio";
    char gpioString[2];
    char buffer[1024];

    sprintf(gpioString, "%d", gpio);
    strcat(fileDirectory, gpioString);
    strcat(fileDirectory, "/value");
    FILE *pFile = fopen(fileDirectory, "r");
    fgets(buffer, sizeof(buffer), pFile);
    fclose(pFile);

    return atoi(buffer);
}

static void* RHYTHM_buttons(void *arg) {

    GPIO_t gpio = (GPIO_t)arg;
    
    while(!killButtons) {

        // Activate the buttons only on rising edges
        int ret = GPIO_risingEdge(gpio);
		if (ret == -1) {
			printf("ERROR: Cannot read GPIO%d\n", gpio);
            continue;
        }

        if(busyButton) {
            continue;
        }

        pthread_mutex_lock(&buttonLock);
        int currrentKey = -1;

        busyButton = true;
        switch (gpio)
        {
        case KEY_0:
            currrentKey = 0;
            break;
        case KEY_1:
            currrentKey = 1;
            break;
        case KEY_2:
            currrentKey = 2;
            break;
        case KEY_3:
            currrentKey = 3;
            break;
        case KEY_4:
            currrentKey = 4;
            break;
        case KEY_5:
            currrentKey = 5;
            break;
        case KEY_6:
            currrentKey = 6;
            break;
        case KEY_7:
            currrentKey = 7;
            break;
        case KEY_8:
            currrentKey = 8;
            break;
        case KEY_9:
            currrentKey = 9;
            break;
        case KEY_STAR:
            currrentKey = STAR;
            break;
        case KEY_POUND:
            currrentKey = POUND;
            break;
        default:
            currrentKey = -1;
            break;
        }

        printf("KEY PRESSED: %d\n", currrentKey);

        // Reset Keypresses
        if(currrentKey == POUND) {
            keyPressCount = 0;
            for(int i = 0; i < MAX_KEYPAD_ENTRIES; i++) {
                keyCodeDigit[i] = 0;
            }
        }

        if (currrentKey >= 0 && currrentKey <= STAR) {
            keyCodeDigit[keyPressCount] = currrentKey;
            if ((currrentKey >= 0 && currrentKey <= 9)) {
                keyPressCount++;
            }

            // Call number
            // If star is pressed call immediatly
            if(keyPressCount >= MAX_KEYPAD_ENTRIES || currrentKey == STAR) {
                int keyCode = KEYPAD_calculateKeyCode(keyPressCount);
                printf("KEY CODE: %d\n", keyCode);
                PHONE_call(keyCode);
                keyPressCount = 0;
            } 
        }

        TIME_sleepForMs(200);
        busyButton = false;
        pthread_mutex_unlock(&buttonLock);
    }

    return 0;
}


void KEYPAD_cleanup(void)
{
    pollKeypad = false;

    // Cancel the edge triggered threads
    pthread_cancel(key0Thread);
    pthread_cancel(key1Thread);
    pthread_cancel(key2Thread);
    pthread_cancel(key3Thread);
    pthread_cancel(key4Thread);
    pthread_cancel(key5Thread);
    pthread_cancel(key6Thread);
    pthread_cancel(key7Thread);
    pthread_cancel(key8Thread);
    pthread_cancel(key9Thread);
    pthread_cancel(keyStarThread);
    pthread_cancel(keyPoundThread);

    // Join threads
    pthread_join(key0Thread, NULL);
    pthread_join(key1Thread, NULL);
    pthread_join(key2Thread, NULL);
    pthread_join(key3Thread, NULL);
    pthread_join(key4Thread, NULL);
    pthread_join(key5Thread, NULL);
    pthread_join(key6Thread, NULL);
    pthread_join(key7Thread, NULL);
    pthread_join(key8Thread, NULL);
    pthread_join(key9Thread, NULL);
    pthread_join(keyStarThread, NULL); 
    pthread_join(keyPoundThread, NULL);         

    printf("KEYPAD Shutdown...\n");
}

