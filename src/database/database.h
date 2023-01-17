#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>

void DATABASE_initialize();


//get functions
char * DATABASE_getPhoneNumFromCode(int userCode);
char * DATABASE_getNameFromCode(int userCode);

//insert user; return -1 if user already exists
int DATABASE_insertPhoneInfo(int userCode, char * phoneNumber, char * userName);

//update table: phone; return -1 if user does not exist
int DATABASE_updateUserPhoneInfoWithCode(int userCode, char * phoneNumber, char * userName);

//delete from table: phone; return -1 if user does not exist
int DATABASE_deleteUserPhoneInfoWithCode(int userCode);

bool DATABASE_getPhoneNumFromDirId(char** getPhoneNumber, unsigned short keyCode);


#endif
