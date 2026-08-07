#include "app_key.h"
#include "FreeRTOS.h"
#include "queue.h"

/* 用 static 把队列私有化，外部无法直接访问 key_queue */
static QueueHandle_t key_queue = NULL;

void app_key_init(void)
{
    if (key_queue == NULL) {
        key_queue = xQueueCreate(5, sizeof(uint8_t));
    }
}

bool app_key_send_event(uint8_t key_id)
{
    BaseType_t res;

    if (key_queue == NULL) {
        return false;
    }

    res = xQueueSend(key_queue, &key_id, 0);
    return (res == pdPASS);
}

bool app_key_wait_event(uint8_t *p_key_id, uint32_t timeout_ms)
{
    TickType_t ticks;
    BaseType_t res;

    if (key_queue == NULL || p_key_id == NULL) {
        return false;
    }

    ticks = (timeout_ms == 0xffffffffU) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    res = xQueueReceive(key_queue, p_key_id, ticks);
    return (res == pdPASS);
}
