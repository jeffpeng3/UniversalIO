#include <stdint.h>
#include "freertos/queue.h"
void i2c_init(QueueHandle_t pvParameters);
void i2c_dispatch(uint8_t *data, uint32_t len);