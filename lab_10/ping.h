#ifndef PING_H_
#define PING_H_

#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"

void ping_init(void);

void ping_trigger(void);

void TIMER3B_Handler(void);

float ping_getDistance(void);

#endif
