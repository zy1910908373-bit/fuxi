#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"

#include "FreeRTOS.h"
#include "task.h"
#include "self_task.h"

void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                        char * pcTaskName )
{
}

void vApplicationIdleHook( void )
{
}

void vApplicationTickHook( void )
{
}

void vApplicationMallocFailedHook( void )
{
}

int main(void)
{
	xTaskCreate((TaskFunction_t)start_task,(const char*)"start_task",128,NULL,1,
		(TaskHandle_t*)&StartTask_Handler
	);
	vTaskStartScheduler();  // 打开调度器
		
    while(1);
}