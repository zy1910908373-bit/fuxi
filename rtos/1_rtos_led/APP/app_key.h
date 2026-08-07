#ifndef __APP_KEY_H__
#define __APP_KEY_H__

#include <stdint.h>
#include <stdbool.h>

/* 初始化按键模块(内部创建队列) */
void app_key_init(void);

/* 发送按键事件(供中断或任务调用) */
bool app_key_send_event(uint8_t key_event);

/* 接收按键事件 */
bool app_key_wait_event(uint8_t *p_key_id, uint32_t timeout_ms);

#endif
