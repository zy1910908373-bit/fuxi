#ifndef __BSP_LEDS_H__
#define __BSP_LEDS_H__

#include "gd32f4xx.h"
#include "systick.h"


void GPIO_config();

//打开所有灯
void turn_on_all();

//关闭所有灯
void turn_off_all();

//开单个灯
void led_turn_on(uint8_t i);

//关单个灯
void led_turn_off(uint8_t i);

#endif