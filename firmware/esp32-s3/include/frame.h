#include <stdint.h>
#include  "freertos/queue.h"
typedef struct _frame
{
    bool isConfig;
    uint8_t subsystem; // 0:uart, 1:i2c, 2:spi
    union
    {
        uint8_t *data;
        uint8_t *setting;
    };
    
} frame;
