#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>



#include "phone.h"
#include "sim7600.h"
#include "../linux/bbg_time.h"
#include "../database/database.h"
#include "../hardware/lock.h"
#include "../hardware/mode.h"
#include "../video/display.h"

#define BUFFER_SIZE 40
#define UART_BUFFER 4095
#define WAIT_TIME_MS 30000

pthread_t callEventThread;
bool stopCallEvent = false;
bool isCallOver = false;

static void * PHONE_callEvent(void* _args);
static void PHONE_callLimit(int callLimit);
static void PHONE_textToDatabase(char* AT_textMessage);

void PHONE_initialize(void){
    SIM7600_sendCommand("AT+CMGD=,4\r\n", BUFFER_SIZE, "OK", BUFFER_SIZE);
    pthread_create(&callEventThread, NULL, &PHONE_callEvent, NULL);

}

void PHONE_cleanup(void){

    isCallOver = true;
    stopCallEvent = true;
    printf("PHONE Shutdown...\n");

    pthread_cancel(callEventThread);
    pthread_join(callEventThread, NULL);
}

static void PHONE_textToDatabase(char* AT_textMessage) {

    int i = 0;
    int codeIndexStart = -1;
    int codeIndexEnd = -1;
    int phoneIndexStart = -1;
    int phoneIndexEnd = -1;
    int nameIndexStart = -1;
    int nameIndexEnd = -1;

    //                  Code Phone Num  Name
    // Message format = <###[##########]NAME>
    while (i < strlen(AT_textMessage))
    {
        if(AT_textMessage[i] == '<') {
            codeIndexStart = i+1;
        }
        if(AT_textMessage[i] == '[') {
            codeIndexEnd = i;
            phoneIndexStart = i + 1;
        }  
        if(AT_textMessage[i] == ']') {
            phoneIndexEnd = i;
            nameIndexStart = i + 1;
        }  
        if(AT_textMessage[i] == '>') {
            nameIndexEnd = i;
        }
        i++;
    }

    if(codeIndexStart == -1 ||
        codeIndexEnd == -1 ||
        phoneIndexStart == -1 ||
        phoneIndexEnd == -1 ||
        nameIndexStart == -1 ||
        nameIndexEnd == -1) {
            return;
    }

    if(codeIndexStart >= codeIndexEnd ||
        phoneIndexStart >= phoneIndexEnd ||
        nameIndexStart >= nameIndexEnd) {
            return;
    }

    char userCode_s[4];
    char userPhone_s[11];
    char userName_s[16];

    strncpy(userCode_s, AT_textMessage + codeIndexStart, codeIndexEnd - codeIndexStart);
    strncpy(userPhone_s, AT_textMessage + phoneIndexStart, phoneIndexEnd - phoneIndexStart);
    strncpy(userName_s, AT_textMessage + nameIndexStart, nameIndexEnd - nameIndexStart);
    userCode_s[3] = '\0';
    userPhone_s[10] = '\0';
    userName_s[nameIndexEnd - nameIndexStart] = '\0';
    int userCode = atoi(userCode_s);
    
    printf("Text Message Code: %d\n", userCode);
    printf("Text Message Phone: %s\n", userPhone_s);
    printf("Text Message Name: %s\n", userName_s);
    
    DATABASE_insertPhoneInfo(userCode, userPhone_s, userName_s);
    lcd_mode textInsert = INSERT_USER;
    MODE_setMode(textInsert, userCode);
    
    MODE_updateDatabase();

    lcd_mode directory = HOME;
    MODE_setMode(directory, 0);


}

static void * PHONE_callEvent(void* _args) {

    while (!stopCallEvent)
    {
        // Time to fill buffer
        TIME_sleepForMs(100);

        char* bufferUART;
        SIM7600_getBufferUART(&bufferUART);

        if(!bufferUART) {
            continue;
        }

        if(strstr(bufferUART, "+RXDTMF: 5")) {
            printf("PHONE: Comand 5\n");
            LOCK_unlockDoor(10000);
        }

        if (strstr(bufferUART, "VOICE CALL: END")) {
            printf("PHONE: Ending call\n");
            isCallOver = true;
            TIME_sleepForMs(5000);
        }

        if (strstr(bufferUART, "+CMTI: \"SM\"")) {
            char * returnBuffer;
            // Read SMS
            SIM7600_sendCommandGetBuffer("AT+CMGR=0\r\n", BUFFER_SIZE, "+CMGR:", &returnBuffer);

            printf("TEST2\n");

            PHONE_textToDatabase(returnBuffer);

            // Delete SMS
            SIM7600_sendCommand("AT+CMGD=0\r\n", BUFFER_SIZE, "OK", BUFFER_SIZE);
            
        }
    }

    return 0;
}

static void PHONE_callLimit(int callLimit_ms) {

    isCallOver = false;
    long long startTime = TIME_getTimeInMs();

    while (!isCallOver)
    {
        long long endTimer = TIME_getTimeInMs() - startTime;
        if(endTimer > WAIT_TIME_MS) {
            isCallOver = true;
        }

        TIME_sleepForMs(1);
    }

    printf("PHONE: Hang up\n");
    SIM7600_sendCommand("AT+CHUP\r\n", BUFFER_SIZE, "OK", BUFFER_SIZE);
}

bool PHONE_call(int keyCode) {

    char* phoneNumber;

    lcd_mode call = CALLING;
    lcd_mode directory = HOME;
    MODE_setMode(call, keyCode);

    printf("KEYCODE: %d", keyCode);

    // No Phone Number associated with Key Code
    if (!DATABASE_getPhoneNumFromDirId(&phoneNumber, keyCode)) {
        printf("CALL FAILED: KeyCode has no phone number.\n");
        return false;
    }

    // Formats the phone number
    static char AT_phoneNumber[100];
    strcpy(AT_phoneNumber, "ATD");
    strcat(AT_phoneNumber, phoneNumber);
    strcat(AT_phoneNumber, ";\r\n");

    printf("CALLING: %s\n", AT_phoneNumber);

    // Call Phone Number
    if(!SIM7600_sendCommand(AT_phoneNumber, BUFFER_SIZE, "OK", BUFFER_SIZE)) {
        printf("CALL FAILED: Did not receive OK from SIM7600.\n");
        return false;
    }

    PHONE_callLimit(30000);

    MODE_setMode(directory, 0);
    return true;
}