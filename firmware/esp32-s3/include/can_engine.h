#include <stdint.h>
#include "freertos/queue.h"
void can_init(QueueHandle_t pvParameters);
void can_dispatch(uint8_t *data, uint32_t len);
