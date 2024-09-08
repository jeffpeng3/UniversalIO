#include <stdint.h>
#include "freertos/queue.h"
void uart_init(QueueHandle_t pvParameters);
void uart_dispatch(uint8_t *data, uint32_t len);