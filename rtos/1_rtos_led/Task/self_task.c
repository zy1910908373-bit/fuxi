#include "self_task.h"

TaskHandle_t    StartTask_Handler;
TaskHandle_t    Task1_Handler;
TaskHandle_t    Task2_Handler;
TaskHandle_t    Task3_Handler;
TaskHandle_t TaskKey_Handler;

///////////////////////////起始任务
void start_task(void *pvParameters)
{
	//外设初始化
	buzzer_init();
	app_key_init();
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
	xTaskCreate((TaskFunction_t)task_key_scan,(const char*)"task_key_scan",50,NULL,1,
		(TaskHandle_t*)&TaskKey_Handler
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
	uint8_t key_id = 0;


	while(1)
	{
		// 死等队列
		if (app_key_wait_event(&key_id, 0xffffffff))
		{
			if (key_id == 1)
			{
				buzzer_on();
				vTaskDelay(pdMS_TO_TICKS(50));
				buzzer_off();
				vTaskDelay(pdMS_TO_TICKS(1000));
			}
		}
	}
    // // 1. 获取当前系统 Tick 计数作为基准时间
    // TickType_t xLastWakeTime = xTaskGetTickCount();
		// while(1)
		// {
		// 	buzzer_on();
		// 	 vTaskDelay(pdMS_TO_TICKS(50));  // 响 50ms (推荐用 pdMS_TO_TICKS 宏转换毫秒，防止 configTICK_RATE_HZ 改变)
		// 	buzzer_off();
		// 	 vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
		// }
}


////////////////////////按键
void task_key_scan(void *pvParameters)
{
    // 初始化按键 GPIO（假设按键在 PC0，低电平有效）
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_0);

    while(1)
    {
        if (gpio_input_bit_get(GPIOC, GPIO_PIN_0) == RESET)
        {
            vTaskDelay(pdMS_TO_TICKS(20)); // 消抖
            if (gpio_input_bit_get(GPIOC, GPIO_PIN_0) == RESET)
            {
                // 把按键事件发送进 APP 模块的队列
                app_key_send_event(1);

                while(gpio_input_bit_get(GPIOC, GPIO_PIN_0) == RESET) {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}