#include <stdint.h>

// 0:uart, 1:i2c, 2:spi, 3:can 127:request avaliable ports
// first byte is the type of the frame
struct data_header
{
    uint8_t is_config : 1;
    uint8_t type : 7;
};

typedef struct _UIO_frame
{
    uint8_t len;
    union
    {
        uint8_t *data;
        struct data_header *header;
    };

} UIO_frame;

typedef struct _UIO_uart_setting
{
    uint8_t type; // must be 0
    uint8_t uart_num;
    uint32_t baud_rate;
    uint8_t data_bits;
    uint8_t stop_bits;
    uint8_t parity;
    uint8_t flow_control;
} UIO_uart_setting;

typedef struct _UIO_i2c_setting
{
    uint8_t type; // must be 1
    uint8_t i2c_num;
    uint32_t clock_speed;
    uint8_t address_mode;
    uint8_t address;
} UIO_i2c_setting;

typedef struct _UIO_spi_setting
{
    uint8_t type; // must be 2
    uint8_t spi_num;
    uint32_t clock_speed;
    uint8_t data_mode;
    uint8_t data_order;
    uint8_t cs_pin;
} UIO_spi_setting;

typedef struct _UIO_can_setting
{
    uint8_t type; // must be 3
    uint8_t can_num;
    uint32_t baud_rate;
    uint8_t mode;
} UIO_can_setting;