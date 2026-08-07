#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "gd32f4xx.h"
#include "systick.h"

void buzzer_init(void);
void buzzer_on();
void buzzer_off();

#endif