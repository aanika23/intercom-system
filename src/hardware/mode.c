#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <string.h>

#include "directory.h"
#include "mode.h"
#include "joystick.h"
#include "../database/database.h"
#include "../linux/bbg_time.h"
#include "../video/display.h"

#define MAX_CUSTOMER 999
#define CODE_MAX 30
#define DIRECTORY 1

#define NONE 0
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

#define ROWS 2
#define COLM 16

static lcd_mode CURRENT_MODE;
static bool killMode = false;
pthread_t modeThread;
pthread_mutex_t databaseMutex;
static int callerCode = 0;
static char databaseCpy[1000][COLM];
static int size = 0;

#define ROWS 2
#define COLM 16


lcd_mode MODE_getMode(){
    return CURRENT_MODE;
}

void MODE_setMode(lcd_mode mode,int code){
    CURRENT_MODE = mode;
    callerCode = code;
}

void MODE_updateDatabase(){
    pthread_mutex_lock(&databaseMutex);
    int dataBaseidx = 0;
    size = 0;
    
    //copy data base
    printf("DIRECTORY: Copying Database\n");
    for(int i=0; i < MAX_CUSTOMER; i++) 
    {
        
        if(strlen(DATABASE_getNameFromCode(i)) != 0)
        {
           // printf("name: %s \n", DATABASE_getNameFromCode(i));
            char nameCode[COLM];
            char num[CODE_MAX];
            snprintf(num, CODE_MAX, "%03d", i);
            strcpy(nameCode, num);
            strcat(nameCode, " ");
            strcat(nameCode, DATABASE_getNameFromCode(i));
            printf("name: %s \n", nameCode);

            strcpy(databaseCpy[dataBaseidx], nameCode);

            size++;
            dataBaseidx++;
        }
    }
    printf("DIRECTORY: Database Updated\n");
    pthread_mutex_unlock(&databaseMutex);
}

static void* MODE_controlMode(void* arg){

    int counter = 1;
    if(CURRENT_MODE == DIRECTORY){
         
    }
    while(!killMode){
        switch (CURRENT_MODE)
        {
        case CALLING:
            if(strlen(DATABASE_getNameFromCode(callerCode)) == 0){
                char callingStr[ROWS][COLM] = {"Call Failed!", "Unknown Code"};
                DISPLAY_writeAll(callingStr);
            } else {
                char callingStr[ROWS][COLM];
                strcpy(callingStr[0], "Calling ...");
                strcpy(callingStr[1], DATABASE_getNameFromCode(callerCode));
                DISPLAY_writeAll(callingStr);
            } 

            TIME_sleepForMs(2000);
            break;
        case INSERT_USER:
            if (true) {
                char insertStr[ROWS][COLM];
                strcpy(insertStr[0], "Inserted User: ");
                strcpy(insertStr[1], DATABASE_getNameFromCode(callerCode));
                DISPLAY_writeAll(insertStr);
                TIME_sleepForMs(3000);
            }
            
            break;
        case HOME:
            if (true) {
                lcd_mode dir = DIRECTORY;
                MODE_setMode(dir, 0);
                char nameCodeTop[COLM];
                strcpy(nameCodeTop, databaseCpy[0]);

                char nameCodeBottom[COLM];
                strcpy(nameCodeBottom, databaseCpy[1]);

                char options[ROWS][COLM];//{"nameCodeTop", "nameCodeBottom"};
                strcpy(options[0],nameCodeTop);
                strcpy(options[1],nameCodeBottom);
                DISPLAY_writeAll(options);
                TIME_sleepForMs(3000);
            }
            
            break;
        case DIRECTORY:
            if(JOYSTICK_getDirection() == UP){
                if(counter < 2) {
                    counter = size;
                }
                counter--;
                char nameCodeTop[COLM];
                strcpy(nameCodeTop, databaseCpy[(counter - 1) % size]);

                char nameCodeBottom[COLM];
                strcpy(nameCodeBottom, databaseCpy[counter % size]);

                char options[ROWS][COLM];
                strcpy(options[0],nameCodeTop);
                strcpy(options[1],nameCodeBottom);
                DISPLAY_writeAll(options);
                
            }else if(JOYSTICK_getDirection() == DOWN){
                if (counter > size - 1){
                    counter = 1;
                }
                counter++;
                char nameCodeTop[COLM];
                strcpy(nameCodeTop, databaseCpy[(counter - 1) % size]);

                char nameCodeBottom[COLM];
                strcpy(nameCodeBottom, databaseCpy[counter % size]);

                char options[ROWS][COLM];//{"nameCodeTop", "nameCodeBottom"};
                strcpy(options[0],nameCodeTop);
                strcpy(options[1],nameCodeBottom);
                DISPLAY_writeAll(options);
                
            }
            TIME_sleepForMs(300);
            break;
        default:
            TIME_sleepForMs(10);
            break;
        }

        
        
    }
    return 0;
}

void MODE_initialize(){
    pthread_create(&modeThread, NULL, &MODE_controlMode, NULL);

}

void MODE_cleanUp(){

}