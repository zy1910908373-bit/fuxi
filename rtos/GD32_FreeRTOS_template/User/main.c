#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "gd32f450i_eval.h"
#include "bsp_led.h"

#include "FreeRTOS.h"
#include "task.h"

TaskHandle_t    StartTask_Handler;
TaskHandle_t    Task1_Handler;
TaskHandle_t    Task2_Handler;


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
		
}

void vApplicationTickHook( void )
{
		// tick 
}

void vApplicationMallocFailedHook( void )
{
	// 内存申请失败回调 heap
}

void task1(void *pvParameters)
{
		// 流水灯 1234
		while(1)
		{
				led_turn_on(0);
				vTaskDelay(500);
				led_turn_on(1);
				vTaskDelay(500);
				led_turn_on(2);
				vTaskDelay(500);
				led_turn_on(3);
				vTaskDelay(500);
			
			led_turn_off(0);
			led_turn_off(1);
			led_turn_off(2);
			led_turn_off(3);
			vTaskDelay(500);
		}
}

void task2(void *pvParameters)
{
		// 流水灯 5678
		while(1)
		{
				led_turn_on(4);
				vTaskDelay(500);
				led_turn_on(5);
				vTaskDelay(500);
				led_turn_on(6);
				vTaskDelay(500);
				led_turn_on(7);
				vTaskDelay(500);
			
			led_turn_off(4);
			led_turn_off(5);
			led_turn_off(6);
			led_turn_off(7);
			vTaskDelay(500);
		}
}

void start_task(void *pvParameters)
{
	//外设初始化
	
	//驱动初始化
	GPIO_config();

	taskENTER_CRITICAL();
	
	xTaskCreate((TaskFunction_t)task1,(const char*)"task1",50,NULL,2,
		(TaskHandle_t*)&Task1_Handler
	);
	xTaskCreate((TaskFunction_t)task2,(const char*)"task2",50,NULL,2,
		(TaskHandle_t*)&Task2_Handler
	);
	vTaskDelete(StartTask_Handler);
	taskEXIT_CRITICAL();
}

//void GPIO_config(){
//	//1、时钟初始化
//	rcu_periph_clock_enable(RCU_GPIOB);
//	
//	//2、配置GPIO的输入输出模式
//	gpio_mode_set(GPIOB,GPIO_MODE_OUTPUT,GPIO_PUPD_PULLDOWN,GPIO_PIN_2);
//	
//	//3、配置GPIO模式的操作方式
//	gpio_output_options_set(GPIOB,GPIO_OTYPE_PP,GPIO_OSPEED_2MHZ,GPIO_PIN_2);
//	
//}

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