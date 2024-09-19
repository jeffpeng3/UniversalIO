#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include <cdc_config.h>
#include <frame.h>
#include <usbd_wrapper.h>
#include <UIO_config.h>

#include "uart_engine.h"
#include "i2c_engine.h"
#include "spi_engine.h"
#include "can_engine.h"

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[2048];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[2048];
SemaphoreHandle_t readyToSend;

QueueHandle_t _writeQueue;

void dispatch(uint8_t *data, uint32_t len)
{
    struct data_header *header = (struct data_header *)data;
    switch (header->type)
    {
    case 0:
        uart_dispatch(data, len);
        break;
    case 1:
        // i2c_dispatch(data, len);
        break;
    case 2:
        // spi_dispatch(data, len);
        break;
    case 3:
        // can_dispatch(data, len);
        break;
    case 127:
        // request avaliable ports
        UIO_frame frame;
        frame.data = (uint8_t *)pvPortMalloc(5);
        frame.data[0] = UIO_AVAILABLE_UART;
        frame.data[1] = UIO_AVAILABLE_I2C;
        frame.data[2] = UIO_AVAILABLE_SPI;
        frame.data[3] = UIO_AVAILABLE_CAN;
        frame.len = 4;
        BaseType_t xTaskWokenByReceive = pdFALSE;
        xQueueSendToBackFromISR(_writeQueue, &frame, &xTaskWokenByReceive);
        break;
    default:
        break;
    }
}

void after_usbd_recv(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("%d actual get len:%d\n", ep, nbytes);
    uint8_t data[2048] = {0};
    memcpy(data, read_buffer, nbytes);
    usbd_ep_start_read(busid, OUT_EP, read_buffer, 2048);
    dispatch(data, nbytes);
}

void after_usbd_send(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("actual send len:%d\n", nbytes);
    if ((nbytes % CDC_MAX_MPS) == 0 && nbytes)
    {
        usbd_ep_start_write(busid, IN_EP, NULL, 0);
    }
    else
    {
        xSemaphoreGive(readyToSend);
    }
}

struct usbd_endpoint in_ep = {
    .ep_addr = IN_EP,
    .ep_cb = after_usbd_send};
struct usbd_endpoint out_ep = {
    .ep_addr = OUT_EP,
    .ep_cb = after_usbd_recv};

void usbd_event_handler(uint8_t busid, uint8_t event)
{
    switch (event)
    {
    case USBD_EVENT_RESET:
        break;
    case USBD_EVENT_CONNECTED:
        break;
    case USBD_EVENT_DISCONNECTED:
        break;
    case USBD_EVENT_RESUME:
        break;
    case USBD_EVENT_SUSPEND:
        break;
    case USBD_EVENT_CONFIGURED:
        usbd_ep_start_read(busid, OUT_EP, read_buffer, 2048);
        break;
    case USBD_EVENT_SET_REMOTE_WAKEUP:
        break;
    case USBD_EVENT_CLR_REMOTE_WAKEUP:
        break;
    default:
        break;
    }
}

void usbd_worker(void *pvParameters)
{
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    UIO_frame frame;
    USB_LOG_INFO("start usbd_worker\n");
    while (1)
    {
        if (xQueueReceive(queue, &frame, portMAX_DELAY) == pdTRUE)
        {
            memcpy(write_buffer, frame.data, frame.len);
            vPortFree(frame.data);

            xSemaphoreTake(readyToSend, portMAX_DELAY);
            USB_LOG_INFO("start send message: %d words\n", frame.len);
            usbd_ep_start_write(0, IN_EP, write_buffer, frame.len);
        }
    }
}

void UIO_cdc_acm_init(QueueHandle_t queue)
{
    _writeQueue = queue;
    readyToSend = xSemaphoreCreateBinary();
    if (readyToSend == NULL)
    {
        USB_LOG_ERR("Failed to create readyToSend\n");
        while (1)
            ;
    }
    xSemaphoreGive(readyToSend);

    uint8_t busid = 0;
    usbd_desc_register(busid, cdc_descriptor);

    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &if_in));
    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &if_out));
    usbd_add_endpoint(busid, &in_ep);
    usbd_add_endpoint(busid, &out_ep);

    usbd_initialize(busid, ESP_USBD_BASE, usbd_event_handler);

    xTaskCreate(usbd_worker, "usbd_worker", 2048, queue, 5, NULL);
}