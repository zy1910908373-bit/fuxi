#ifndef __TASK_H__
#define __TASK_H__

#include "bsp_led.h"
#include "buzzer.h"
#include "App.h"

#include "FreeRTOS.h"
#include "task.h"
extern  TaskHandle_t    StartTask_Handler;
extern  TaskHandle_t    Task1_Handler;
extern  TaskHandle_t    Task2_Handler;
extern  TaskHandle_t    Task3_Handler;


///////////////////////////////起始任务
void start_task(void *pvParameters);


///////////////////////////////流水灯

void task1(void *pvParameters);
void task2(void *pvParameters);


//////////////////////////////蜂鸣器
void buzzer_task(void *pvParameters);




////////////////////////按键
void task_key_scan(void *pvParameters);

#endif