#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "usbd_wrapper.h"
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
        vTaskDelay(pdMS_TO_TICKS(1000));
        char *tmp = (char *)pvPortMalloc(80 * sizeof(char));
        memset(tmp, 1, 80);
        tmp[0] = 0;
        for (int i = 1; i < count; i++)
        {
            tmp[i] = 'A' + i % 26;
        }
        f.data = (uint8_t *)tmp;
        f.len = count + 1;
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
    writeQueue = xQueueCreate(20, sizeof(UIO_frame *));
    if (writeQueue == NULL)
    {
        USB_LOG_ERR("Failed to create writeQueue\n");
        while (1)
            ;
    }
    UIO_cdc_acm_init(writeQueue);

    xTaskCreate(idle_task, "idle_task", 2048, NULL, 5, NULL);

}
