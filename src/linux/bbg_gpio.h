#ifndef BBG_GPIO_H
#define BBG_GPIO_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum {
    KEY_0 = 80,
    KEY_1 = 79,
    KEY_2 = 77,
    KEY_3 = 75,
    KEY_4 = 73,
    KEY_5 = 71,
    KEY_6 = 44,
    KEY_7 = 68,
    KEY_8 = 67,
    KEY_9 = 45,
    KEY_STAR = 69,
    KEY_POUND = 23
} GPIO_t;

void GPIO_initialize();
void GPIO_cleanup();
int GPIO_buttonIsDown(GPIO_t GPIO);
int GPIO_risingEdge(GPIO_t GPIO);

#endif