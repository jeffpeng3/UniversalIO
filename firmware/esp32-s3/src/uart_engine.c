#include "driver/uart.h"
#include "esp_log.h"
#include "frame.h"
#include "string.h"

#define UART_PORT_NUM UART_NUM_1
#define UART_BAUD_RATE 115200
#define UART_TX_PIN 17
#define UART_RX_PIN 16

void uart_task(void *pvParameters)
{
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    uint8_t data[2047] = {0};
    int len = 0;
    UIO_frame frame;
    vTaskDelay(pdMS_TO_TICKS(1000));
    while (1)
    {
        if ((len = uart_read_bytes(UART_PORT_NUM, data + 1, 2047, pdMS_TO_TICKS(1000))) > 0)
        {
            frame.len = len + 1;
            uint8_t *tmp = calloc(frame.len, sizeof(uint8_t));
            memcpy(tmp, data, frame.len);
            frame.data = tmp;
            esp_rom_printf("uart read %d bytes", len);

            xQueueSend(queue, &frame, portMAX_DELAY);
        }
    }
}

void uart_init(QueueHandle_t pvParameters)
{
    const uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    // Configure UART parameters
    uart_param_config(UART_PORT_NUM, &uart_config);

    // Set UART pins
    uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Install UART driver
    if (ESP_OK != uart_driver_install(UART_PORT_NUM, 1024 * 2, 0, 0, NULL, 0)){
        esp_rom_printf("uart_driver_install failed");
    }

    xTaskCreate(uart_task, "uart_task", 4096, pvParameters, 10, NULL);
}

void uart_dispatch(uint8_t *data, uint32_t len)
{
    uart_write_bytes(UART_PORT_NUM, data + 1, len - 1);
}
