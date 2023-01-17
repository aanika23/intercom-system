#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "../linux/bbg_linux.h"
#include "../linux/bbg_time.h"

#define LOCK_DIRECTION "/sys/class/gpio/gpio70/direction"
#define LOCK_VALUE "/sys/class/gpio/gpio70/value"

pthread_t lockThread;
static bool isLockRunning = true;
static bool isLocked = false;
static long long lockTime = 0;

static void* LOCK_timerUnlocked(void * _args);
static void LOCK_lockDoor();

void LOCK_initialize()
{
    LINUX_writeToFile(LOCK_DIRECTION, "out");
    LINUX_runCommand("config-pin p8.45 gpio");
    LOCK_lockDoor();
    pthread_create(&lockThread, NULL, &LOCK_timerUnlocked, NULL);
}

void LOCK_cleanup()
{
    isLockRunning = false;
    pthread_join(lockThread, NULL);
    LINUX_writeToFile(LOCK_DIRECTION, "out");
    printf("LOCK Shutdown...\n");
}

static void* LOCK_timerUnlocked(void * _args) {

    while(isLockRunning) {

        if(TIME_getTimeInMs() > lockTime) {
            if(!isLocked) {
                LOCK_lockDoor();
            }
        }
        TIME_sleepForMs(100);
    }

    return 0;

}

static void LOCK_lockDoor()
{
    printf("DOOR: Locking\n");
    isLocked = true;
    LINUX_writeToFile(LOCK_VALUE, "1");
}

void LOCK_unlockDoor(int time_MS)
{
    printf("DOOR: Unlocking for %dms\n", time_MS);
    lockTime = TIME_getTimeInMs() + (long long)time_MS;
    // lockTime = 7;
    LINUX_writeToFile(LOCK_VALUE, "0");
    isLocked = false;
}