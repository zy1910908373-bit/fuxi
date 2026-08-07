#include "app_key.h"

// 关键点：用static关键字把队列私有化！外部绝对无法直接访问key_queue
static QueueHandle_t key_queue = NULL;

void app_key_init(void)
{
    // 在模块内部创建队列，外部无需知道队列的存在
    if(key_queue == NULL)
    {
        key_queue = xQueueCreate(5, sizeof(uint8_t));
    }
}

// 供生产者调用
bool app_key_send_event(uint8_t key_id)
{
    if (key_queue == NULL) return false;

    // 内部封装的 FreeRtos 的API
    BaseType_t res = xQueueSend(key_queue, &key_id, 0);
    return (res == pdPASS);
}

// 供消费者调用
bool app_key_wait_event(uint8_t *key_id, uint32_t timeout_ms)
{
    if (key_queue == NULL || key_id == NULL) return false;

    TickType_t ticks = (timeout_ms == 0xffffffff) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);

    //内部封装 xQueueReceive
    BaseType_t res = xQueueReceive(key_queue, key_id, ticks);
    return (res == pdPASS);
}