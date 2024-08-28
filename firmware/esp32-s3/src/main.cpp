/*
 * Copyright (c) 2024, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "cdc_config.h"

extern "C"
{
    void app_main(void);
}

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[2][CDC_MAX_MPS];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[2][CDC_MAX_MPS];
volatile bool ep_tx_busy_flag = false;

static void usbd_event_handler(uint8_t busid, uint8_t event)
{
    switch (event)
    {
    case USBD_EVENT_RESET:
        USB_LOG_RAW("USBD_EVENT_RESET\r\n");
        break;
    case USBD_EVENT_CONNECTED:
        USB_LOG_RAW("USBD_EVENT_CONNECTED\r\n");
        break;
    case USBD_EVENT_DISCONNECTED:
        USB_LOG_RAW("USBD_EVENT_DISCONNECTED\r\n");
        break;
    case USBD_EVENT_RESUME:
        USB_LOG_RAW("USBD_EVENT_DISCONNECTED\r\n");
        break;
    case USBD_EVENT_SUSPEND:
        USB_LOG_RAW("USBD_EVENT_DISCONNECTED\r\n");
        break;
    case USBD_EVENT_CONFIGURED:
        ep_tx_busy_flag = false;
        /* setup first out ep read transfer */
        usbd_ep_start_read(busid, CDC_UART_OUT_EP, read_buffer[0], CDC_MAX_MPS);
        usbd_ep_start_read(busid, CDC_I2C_OUT_EP, read_buffer[1], CDC_MAX_MPS);
        // usbd_ep_start_read(busid, CDC_SPI_OUT_EP, read_buffer[2], CDC_MAX_MPS);
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
    USB_LOG_RAW("%d actual out len:%d\r\n", ep, nbytes);
    /* setup next out ep read transfer */
    // char buf[2048] = {0};
    int idx = ep & 0x7;
    memcpy(write_buffer[idx],read_buffer[idx],nbytes);
    // USB_LOG_RAW("message from channel <%d>:%.64s\r\n", idx, buf);
    usbd_ep_start_write(busid, CDC_I2C_IN_EP, write_buffer[idx], nbytes);
    usbd_ep_start_read(busid, ep, read_buffer[idx], CDC_MAX_MPS);
}

void usbd_cdc_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("actual in len:%d\r\n", nbytes);

    if ((nbytes % CDC_MAX_MPS) == 0 && nbytes)
    {
        /* send zlp */
        usbd_ep_start_write(busid, ep, NULL, 0);
    }
    else
    {
        ep_tx_busy_flag = false;
    }
}

struct usbd_endpoint cdc_uart_in_ep = {
    .ep_addr = CDC_UART_IN_EP,
    .ep_cb = usbd_cdc_acm_bulk_in};
struct usbd_endpoint cdc_uart_out_ep = {
    .ep_addr = CDC_UART_OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out};

struct usbd_endpoint cdc_i2c_in_ep = {
    .ep_addr = CDC_I2C_IN_EP,
    .ep_cb = usbd_cdc_acm_bulk_in};
struct usbd_endpoint cdc_i2c_out_ep = {
    .ep_addr = CDC_I2C_OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out};

// struct usbd_endpoint cdc_spi_in_ep = {
//     .ep_addr = CDC_SPI_IN_EP,
//     .ep_cb = usbd_cdc_acm_bulk_in};
// struct usbd_endpoint cdc_spi_out_ep = {
//     .ep_addr = CDC_SPI_OUT_EP,
//     .ep_cb = usbd_cdc_acm_bulk_out};

void app_main(void)
{
    USB_LOG_INFO("Hello CherryUSB!\n");
    uint8_t busid = 0;
    usbd_desc_register(busid, cdc_descriptor);

    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &uart_interface_in));
    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &uart_interface_out));
    usbd_add_endpoint(busid, &cdc_uart_out_ep);
    usbd_add_endpoint(busid, &cdc_uart_in_ep);

    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &i2c_interface_in));
    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &i2c_interface_out));
    usbd_add_endpoint(busid, &cdc_i2c_out_ep);
    usbd_add_endpoint(busid, &cdc_i2c_in_ep);

    // usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &spi_interface_in));
    // usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &spi_interface_out));
    // usbd_add_endpoint(busid, &cdc_spi_out_ep);
    // usbd_add_endpoint(busid, &cdc_spi_in_ep);

    usbd_initialize(busid, ESP_USBD_BASE, usbd_event_handler);
    while (1)
    {
        vTaskDelay(10);
    }
}