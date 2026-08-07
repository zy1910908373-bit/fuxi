#include "self_task.h"

TaskHandle_t    StartTask_Handler;
TaskHandle_t    Task1_Handler;
TaskHandle_t    Task2_Handler;
TaskHandle_t    Task3_Handler;

///////////////////////////起始任务
void start_task(void *pvParameters)
{
	//外设初始化
	buzzer_init();
	//驱动初始化
	GPIO_config();

	taskENTER_CRITICAL();
	
	xTaskCreate((TaskFunction_t)task1,(const char*)"task1",50,NULL,2,
		(TaskHandle_t*)&Task1_Handler
	);
	xTaskCreate((TaskFunction_t)task2,(const char*)"task2",50,NULL,2,
		(TaskHandle_t*)&Task2_Handler
	);
	xTaskCreate((TaskFunction_t)buzzer_task,(const char*)"buzzer_task",50,NULL,2,
		(TaskHandle_t*)&Task3_Handler
	);
	vTaskDelete(StartTask_Handler);
	taskEXIT_CRITICAL();
}



/////////////////////////流水灯
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


////////////////////////蜂鸣器
void buzzer_task(void *pvParameters)
{
    // 1. 获取当前系统 Tick 计数作为基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
		while(1)
		{
			buzzer_on();
			 vTaskDelay(pdMS_TO_TICKS(50));  // 响 50ms (推荐用 pdMS_TO_TICKS 宏转换毫秒，防止 configTICK_RATE_HZ 改变)
			buzzer_off();
			 vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
		}
}
