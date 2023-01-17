#ifndef MODE_H
#define MODE_H

typedef enum{
    KEYPAD,
    DIRECTORY,
    CALLING,
    HOME,
    INSERT_USER
}lcd_mode;

void MODE_initialize();
void MODE_setMode(lcd_mode mode,int code);
lcd_mode MODE_getMode();
void MODE_updateDatabase();
#endif