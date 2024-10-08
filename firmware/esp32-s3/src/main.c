#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "usbd_wrapper.h"
#include "uart_engine.h"
#include "usbd_core.h"
#include "string.h"
#include "frame.h"

QueueHandle_t writeQueue;
void idle_task(void *pvParameters)
{
    UIO_frame f;
    int count = 0;
    while (++count)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
        char *tmp = (char *)pvPortMalloc(80 * sizeof(char));
        memset(tmp, 0, 80);
        for (int i = 0; i < count; i++)
        {
            tmp[i] = 'A' + i % 26;
        }
        tmp[0] = 0;
        f.data = (uint8_t *)tmp;
        f.len = count;
        USB_LOG_INFO("Enqueue: %d words\n", count);
        if (errQUEUE_FULL == xQueueSend(writeQueue, &f, 0))
        {
            vPortFree(tmp);
            USB_LOG_INFO("Queue is full\n");
        }

        if (count > 70)
        {
            count = 0;
        }
    }
}

void app_main(void)
{
    writeQueue = xQueueCreate(20, sizeof(UIO_frame));
    if (writeQueue == NULL)
    {
        USB_LOG_ERR("Failed to create writeQueue\n");
        while (1)
            ;
    }
    uart_init(writeQueue);

    UIO_cdc_acm_init(writeQueue);

    // xTaskCreate(idle_task, "idle_task", 2048, NULL, 5, NULL);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
