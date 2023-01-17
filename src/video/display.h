#ifndef DISPLAY_H
#define DISPLAY_H


#define DISPLAY_ROWS 2
#define DISPLAY_COLUMNS 16

void DISPLAY_initialize();
void DISPLAY_cleanUp();
void DISPLAY_writeAll(char inputStr[DISPLAY_ROWS][DISPLAY_COLUMNS]);


#endif