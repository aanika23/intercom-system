#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "video/display.h"
#include "cellular/sim7600.h"
#include "database/database.h"
#include "hardware/keypad.h"
#include "video/display.h"
#include "hardware/lock.h"
#include "cellular/phone.h"
#include "hardware/mode.h"
#include "hardware/directory.h"
#include "hardware/joystick.h"

#define ROWS 2
#define COLM 16

int main() {

    //KEYPAD_initialize();
    DISPLAY_initialize();
    LOCK_initialize();
    SIM7600_initialize();
    KEYPAD_initialize();
    DATABASE_initialize();
    JOYSTICK_initialize();
    MODE_initialize();
     PHONE_initialize();
     
    MODE_updateDatabase();
    
    char options[ROWS][COLM] = { "0xDEADC0DE", "ENSC351 FALL22" };
    
    DISPLAY_writeAll(options);
    
    lcd_mode test = DIRECTORY;
    MODE_setMode(test, 0);
    
    // PHONE_initialize();

    
   
   while(true) {
        // hits the watchdog every 5s by setting awake.txt to 1
        printf("\nMAIN: setting awake.txt to 1\n");
        FILE *pFile = fopen("awake.txt", "w");
        if (pFile == NULL)
        {
          perror("Error in opening awake.txt");	
        }

        char one = '1';
        fprintf(pFile,"%c", one);
        fclose(pFile);
        sleep(5);
    }
}


//    int test = DATABASE_insertPhoneInfo(147, "7788376114", "Anika");
//       int test2 =DATABASE_insertPhoneInfo(3, "7783223243", "Jeremy");
//      int test3 = DATABASE_insertPhoneInfo(148, "6048424372", "Anika");
//      int test4 = DATABASE_insertPhoneInfo(5, "7788376114", "Mehrad");