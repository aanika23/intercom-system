#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <stdbool.h>

//#include "../private.h"
//#include "../private.h"
#include "database.h"

#define PHONE_NUM_LENGTH 10
#define NAME_LENGTH 50
#define USER_CODE_LENGTH 4
#define MAX_QUERY_LENGTH 200

static sqlite3 *phoneDB;

static char phoneNum[PHONE_NUM_LENGTH];
static char name[NAME_LENGTH];

//Funcion prototype for callback function that is used in conjuction with sqlite3_exec()
static int callbackPhoneNum(void *, int, char **, char **);
static int callbackName(void *, int, char **, char **);


static void DATABASE_executeGetSQLStatement(int userCode, int(*callback)(void *, int, char **, char **), char * getElement){
    char getStatement[44];
    if(strstr(getElement, "name") != NULL){
        strcpy(getStatement, "SELECT userName FROM phone WHERE code = ");
    } else if(strstr(getElement, "phone") != NULL){
        strcpy(getStatement, "SELECT phoneNumber FROM phone WHERE code = ");
    } else {
        strcpy(getStatement, "");
    }
    
    char codeStr[USER_CODE_LENGTH];
    char *err_msg = 0;

    snprintf(codeStr,USER_CODE_LENGTH,"%03d", userCode);
    strcat(getStatement, codeStr);
    //printf("%s\n",getStatement);

    int retrivePhoneNumStatus = sqlite3_exec(phoneDB, getStatement, callback, 0, &err_msg);
    if (retrivePhoneNumStatus != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(phoneDB);
    }
}


void DATABASE_initialize(){
    int openDBStatus = sqlite3_open("intercom.db", &phoneDB);
    if(openDBStatus != SQLITE_OK){
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(phoneDB));
        sqlite3_close(phoneDB);
    }
    //printf("Done here\n");
}

char * DATABASE_getPhoneNumFromCode(int userCode){
    static char phoneNumTmp[PHONE_NUM_LENGTH];
    DATABASE_executeGetSQLStatement(userCode, callbackPhoneNum, "phone");
    strcpy(phoneNumTmp, phoneNum);
    strcpy(phoneNum, "");
    return phoneNumTmp;
}

char * DATABASE_getNameFromCode(int userCode){
    static char nameTmp[NAME_LENGTH];
    DATABASE_executeGetSQLStatement(userCode, callbackName, "name");
    strcpy(nameTmp, name);
    strcpy(name, "");
    return nameTmp;
}

int DATABASE_updateUserPhoneInfoWithCode(int userCode, char * phoneNumber, char * userName){
    bool userExists = strlen(DATABASE_getNameFromCode(userCode)) != 0;
    if (userExists){

        char *err_msg = 0;
        char userCodeStr[USER_CODE_LENGTH];
        char query[MAX_QUERY_LENGTH] = "UPDATE phone SET phoneNumber = '";
        snprintf(userCodeStr,USER_CODE_LENGTH,"%03d", userCode);

        strcat(query, phoneNumber);
        strcat(query, "', userName = '");
        strcat(query, userName);
        strcat(query, "' WHERE code = ");
        strcat(query, userCodeStr);
        int updateStatus = sqlite3_exec(phoneDB, query, 0, 0, &err_msg);
        if (updateStatus != SQLITE_OK ) {
            
            fprintf(stderr, "Failed to update table\n");
            fprintf(stderr, "SQL error: %s\n", err_msg);
            sqlite3_free(err_msg);
            
        }

        return userCode;
    } else {
        return -1;
    }
}

int DATABASE_deleteUserPhoneInfoWithCode(int userCode){
    bool userExists = strlen(DATABASE_getNameFromCode(userCode)) != 0;
    if (userExists){
        char *err_msg = 0;
        char query[MAX_QUERY_LENGTH] = "DELETE FROM phone where code = ";
        char userCodeStr[USER_CODE_LENGTH];

        snprintf(userCodeStr,USER_CODE_LENGTH,"%03d", userCode);

        strcat(query, userCodeStr);

        int deleteStatus = sqlite3_exec(phoneDB, query, 0, 0, &err_msg);
        if (deleteStatus != SQLITE_OK ) {
            
            fprintf(stderr, "Failed to delete row from table\n");
            fprintf(stderr, "SQL error: %s\n", err_msg);
            sqlite3_free(err_msg);
            
        }

        return userCode;
    } else {
        return -1;
    }

}

int DATABASE_insertPhoneInfo(int userCode, char * phoneNumber, char * userName){
    bool userExists = strlen(DATABASE_getNameFromCode(userCode)) != 0;
    if (!userExists){
        char *err_msg = 0;
        char query[MAX_QUERY_LENGTH] = "INSERT INTO phone (code, userName, phoneNumber) VALUES (";
        
        char userCodeStr[USER_CODE_LENGTH];

        snprintf(userCodeStr,USER_CODE_LENGTH,"%03d", userCode);

        strcat(query, userCodeStr);
        strcat(query, ", '");
        strcat(query, userName);
        strcat(query, "', '");
        strcat(query, phoneNumber);
        strcat(query, "');");

        printf(query);
        int insertStatus = sqlite3_exec(phoneDB, query, 0, 0, &err_msg);
        if (insertStatus != SQLITE_OK ) {
            
            fprintf(stderr, "Failed to update table\n");
            fprintf(stderr, "SQL error: %s\n", err_msg);
            sqlite3_free(err_msg);
            
        }
        return userCode;
    } else {
        return -1;
    }
   
}

int callbackPhoneNum(void *NotUsed, int argc, char **argv, char **azColName) {
    strcpy(phoneNum,argv[0]);
    return 0;
}

int callbackName(void *NotUsed, int argc, char **argv, char **azColName) {
    strcpy(name,argv[0]);
    return 0;
}

bool DATABASE_getPhoneNumFromDirId(char** getPhoneNumber, unsigned short keyCode){

    // Temp
    printf("pn length: %s", DATABASE_getPhoneNumFromCode((int)keyCode));
    if(strlen(DATABASE_getPhoneNumFromCode((int)keyCode))){
        *getPhoneNumber = DATABASE_getPhoneNumFromCode((int)keyCode);
        return true;
    } else {
        return false;
    }
}