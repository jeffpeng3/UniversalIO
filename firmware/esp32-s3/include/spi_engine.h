#include <stdint.h>
#include "freertos/queue.h"
void spi_init(QueueHandle_t pvParameters);
void spi_dispatch(uint8_t *data, uint32_t len);