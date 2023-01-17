#ifndef JOYSTICK_H
#define JOYSTICK_H

typedef enum{
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT
} JOYSTICK_direction;

void JOYSTICK_initialize();
void JOYSTICK_cleanUp();

JOYSTICK_direction JOYSTICK_getDirection();

#endif