#ifndef PHONE_H
#define PHONE_H

#include <stdbool.h>

void PHONE_initialize(void);
void PHONE_cleanup(void);
bool PHONE_call(int keyCode);

#endif