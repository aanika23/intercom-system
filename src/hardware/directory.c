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

pthread_t directoryThread;
static bool killDirectory = false;
static char databaseCpy[1000][COLM];
static int size = 0;

void DATABASE_updateDatabase(){
    int dataBaseidx = 0;
    
    //copy data base
    printf("DIRECTORY: Copying Database111\n");
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
}
static void* DIRECTORY_display(void* arg) {
    
    int counter = 1;
    
    DATABASE_updateDatabase();
    printf("DATABASE update done\n");

    char nameCodeTop[COLM];
    strcpy(nameCodeTop, databaseCpy[0]);

    char nameCodeBottom[COLM];
    strcpy(nameCodeBottom, databaseCpy[1]);

    char options[ROWS][COLM];//{"nameCodeTop", "nameCodeBottom"};
    strcpy(options[0],nameCodeTop);
    strcpy(options[1],nameCodeBottom);
    DISPLAY_writeAll(options);

    while(!killDirectory) {

        // printf("Mode: %d\n", MODE_getMode());
        // printf("Mode: %d\n", JOYSTICK_getDirection());

        if(MODE_getMode() == DIRECTORY){
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
        }
        TIME_sleepForMs(100);
    }
    
    return 0;
}

void DIRECTORY_initialize(){
    pthread_create(&directoryThread, NULL, &DIRECTORY_display, NULL);
    // pthread_join(directoryThread, NULL);
}

void DIRECTORY_cleanup(){
    // pthread_create(&directoryThread, NULL, &DIRECTORY_display, NULL);
    pthread_join(directoryThread, NULL);
}