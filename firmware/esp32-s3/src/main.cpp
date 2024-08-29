#include "freertos/FreeRTOS.h"
#include  "freertos/queue.h"
#include "freertos/task.h"
#include "cdc_config.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "frame.h"

extern "C"
{
    void app_main(void);
}

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[CDC_MAX_MPS];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[CDC_MAX_MPS];

static void usbd_event_handler(uint8_t busid, uint8_t event)
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
        usbd_ep_start_read(busid, OUT_EP, read_buffer, CDC_MAX_MPS);
        break;
    case USBD_EVENT_SET_REMOTE_WAKEUP:
        break;
    case USBD_EVENT_CLR_REMOTE_WAKEUP:
        break;
    default:
        break;
    }
}

void usbd_cdc_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    static frame* f = NULL;

    

    USB_LOG_RAW("%d actual out len:%d\n", ep, nbytes);
    /* setup next out ep read transfer */
    memcpy(write_buffer,read_buffer,nbytes);
    USB_LOG_RAW("get message: %.64s\n", write_buffer);
    usbd_ep_start_write(busid, IN_EP, write_buffer, nbytes);
    usbd_ep_start_read(busid, ep, read_buffer, CDC_MAX_MPS);
}

void usbd_cdc_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("actual in len:%d\n", nbytes);

    if ((nbytes % CDC_MAX_MPS) == 0 && nbytes)
    {
        /* send zlp */
        usbd_ep_start_write(busid, ep, NULL, 0);
    }
}

struct usbd_endpoint in_ep = {
    .ep_addr = IN_EP,
    .ep_cb = usbd_cdc_acm_bulk_in};
struct usbd_endpoint out_ep = {
    .ep_addr = OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out};


void app_main(void)
{
    USB_LOG_INFO("Hello CherryUSB!\n");
    uint8_t busid = 0;
    usbd_desc_register(busid, cdc_descriptor);

    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &if_in));
    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &if_out));
    usbd_add_endpoint(busid, &out_ep);
    usbd_add_endpoint(busid, &in_ep);

    usbd_initialize(busid, ESP_USBD_BASE, usbd_event_handler);
    while (1)
    {
        vTaskDelay(10);
    }
}