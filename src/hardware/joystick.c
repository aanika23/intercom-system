#include <pthread.h>
#include <stdio.h>

#include "joystick.h"
#include "../linux/bbg_a2d.h"

#define X_VOLTAGE_FILE_PATH "/sys/bus/iio/devices/iio:device0/in_voltage3_raw"
#define Y_VOLTAGE_FILE_PATH "/sys/bus/iio/devices/iio:device0/in_voltage2_raw"
#define UPPER_VOLTAGE_TRIGGER 1.4
#define LOWER_VOLTAGE_TRIGGER 0.4

// Locks
pthread_mutex_t mutexJoystick;

static void JOYSTICK_readXY(double* x, double* y) {
    *x = A2D_getVoltageReading(X_VOLTAGE_FILE_PATH);
    *y = A2D_getVoltageReading(Y_VOLTAGE_FILE_PATH);
}

JOYSTICK_direction JOYSTICK_getDirection() {
    double x = 0;
    double y = 0; 

    JOYSTICK_readXY(&x, &y);
    // printf("x: %f y:%f\n", x, y);

    if (y < LOWER_VOLTAGE_TRIGGER) {
        return UP;
    } else if(y > UPPER_VOLTAGE_TRIGGER) {
        return DOWN;
    } else if (x > UPPER_VOLTAGE_TRIGGER) {
        return LEFT;
    } else if (x < LOWER_VOLTAGE_TRIGGER) {
        return RIGHT;
    } else {
        return NONE;
    }   
}


void JOYSTICK_initialize(){

}
void JOYSTICK_cleanUp(){
    
}