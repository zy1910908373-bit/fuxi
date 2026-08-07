#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "gd32f450i_eval.h"

#include "self_task.h"



// 钩子函数，回调
// callback
// 当发生对应事情，这个函数就会被调用

void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                        char * pcTaskName )
{
		// 内存溢出时，函数会被调用，stack
		// 
}

void vApplicationIdleHook( void )
{
		// 闲置 
		//sfeujfkshefiuehfios
}

void vApplicationTickHook( void )
{
		// tick 
}

void vApplicationMallocFailedHook( void )
{
	// 内存申请失败回调 heap
}

int main(void)
{
	//freertos 不需要systick
//		systick_config();
		//初始化
//		GPIO_config();
	
		// RTOS 初始化
	  // 创建起始任务
	xTaskCreate((TaskFunction_t)start_task,(const char*)"start_task",128,NULL,1,
		(TaskHandle_t*)&StartTask_Handler
	);
	vTaskStartScheduler();  // 打开调度器
		
    while(1) {

    }
}