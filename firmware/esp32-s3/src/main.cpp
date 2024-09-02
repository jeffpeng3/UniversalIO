#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "cdc_config.h"
#include "frame.h"

extern "C"
{
    void app_main(void);
}
SemaphoreHandle_t readyToSend;
QueueHandle_t writeQueue;

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[CDC_MAX_MPS];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[CDC_MAX_MPS];

static void usbd_event_handler(uint8_t busid, uint8_t event)
{
    switch (event)
    {
    case USBD_EVENT_RESET:
        USB_LOG_INFO("USBD_EVENT_RESET\n");
        break;
    case USBD_EVENT_CONNECTED:
        USB_LOG_INFO("USBD_EVENT_CONNECTED\n");
        break;
    case USBD_EVENT_DISCONNECTED:
        USB_LOG_INFO("USBD_EVENT_DISCONNECTED\n");
        break;
    case USBD_EVENT_RESUME:
        USB_LOG_INFO("USBD_EVENT_RESUME\n");
        break;
    case USBD_EVENT_SUSPEND:
        USB_LOG_INFO("USBD_EVENT_SUSPEND\n");
        break;
    case USBD_EVENT_CONFIGURED:
        USB_LOG_INFO("USBD_EVENT_CONFIGURED\n");
        usbd_ep_start_read(busid, OUT_EP, read_buffer, CDC_MAX_MPS);
        break;
    case USBD_EVENT_SET_REMOTE_WAKEUP:
        USB_LOG_INFO("USBD_EVENT_SET_REMOTE_WAKEUP\n");
        break;
    case USBD_EVENT_CLR_REMOTE_WAKEUP:
        USB_LOG_INFO("USBD_EVENT_CLR_REMOTE_WAKEUP\n");
        break;
    default:
        break;
    }
}

void usbd_cdc_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("%d actual out len:%d\n", ep, nbytes);
    /* setup next out ep read transfer */
    uint8_t data[65] = {0};
    memcpy(data, read_buffer, nbytes);
    USB_LOG_RAW("get message: %.64s\n", data);
    usbd_ep_start_read(busid, OUT_EP, read_buffer, CDC_MAX_MPS);
}

void usbd_cdc_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("actual in len:%d\n", nbytes);

    if ((nbytes % CDC_MAX_MPS) == 0 && nbytes)
    {
        /* send zlp */
        usbd_ep_start_write(busid, IN_EP, NULL, 0);
    }
    else
    {
        xSemaphoreGive(readyToSend);
    }
}

struct usbd_endpoint in_ep = {
    .ep_addr = IN_EP,
    .ep_cb = usbd_cdc_acm_bulk_in};
struct usbd_endpoint out_ep = {
    .ep_addr = OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out};

void usbd_cdc_acm_bulk_in_worker(void *pvParameters)
{
    frame f;
    USB_LOG_INFO("start usbd_cdc_acm_bulk_in_worker\n");
    while (1)
    {
        if (xQueueReceive(writeQueue, &f, portMAX_DELAY) == pdTRUE)
        {
            memcpy(write_buffer, f.data, CDC_MAX_MPS);
            vPortFree(f.data);

            xSemaphoreTake(readyToSend, portMAX_DELAY);
            USB_LOG_INFO("start send message: %.64s\n", write_buffer);
            usbd_ep_start_write(0, IN_EP, write_buffer, strlen((char *)write_buffer));
        }
    }
}

void app_main(void)
{

    USB_LOG_INFO("Hello CherryUSB!\n");
    uint8_t busid = 0;
    usbd_desc_register(busid, cdc_descriptor);

    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &if_in));
    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &if_out));
    usbd_add_endpoint(busid, &in_ep);
    usbd_add_endpoint(busid, &out_ep);

    usbd_initialize(busid, ESP_USBD_BASE, usbd_event_handler);
    writeQueue = xQueueCreate(20, sizeof(frame));
    if (writeQueue == NULL)
    {
        USB_LOG_ERR("Failed to create writeQueue\n");
        while (1)
            ;
    }

    readyToSend = xSemaphoreCreateBinary();
    if (readyToSend == NULL)
    {
        USB_LOG_ERR("Failed to create readyToSend\n");
        while (1)
            ;
    }
    xSemaphoreGive(readyToSend);

    xTaskCreate(usbd_cdc_acm_bulk_in_worker, "usbd_cdc_acm_bulk_in_worker", 2048, NULL, 5, NULL);

    frame f;
    f.isConfig = false;
    int i = 0;
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        i++;
        char *tmp = (char *)pvPortMalloc(20 * sizeof(char));
        memset(tmp, 0, 20);
        sprintf(tmp, "hello:%d\n", i);
        f.data = (uint8_t *)tmp;
        USB_LOG_INFO("Enqueue: %s\n", f.data);
        if (errQUEUE_FULL == xQueueSend(writeQueue, &f, 0))
        {
            vPortFree(tmp);
            USB_LOG_INFO("Queue is full\n");
        }
    }
}
