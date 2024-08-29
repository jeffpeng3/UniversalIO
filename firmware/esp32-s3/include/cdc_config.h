static struct usbd_interface uart_interface_in;
static struct usbd_interface uart_interface_out;
static struct usbd_interface i2c_interface_in;
static struct usbd_interface i2c_interface_out;
static struct usbd_interface spi_interface_in;
static struct usbd_interface spi_interface_out;

static inline const int CDC_UART_IN_EP = 0x81;
static inline const int CDC_UART_OUT_EP = 0x01;
static inline const int CDC_UART_INT_EP = 0x85;

static inline const int CDC_I2C_IN_EP = 0x82;
static inline const int CDC_I2C_OUT_EP = 0x02;
static inline const int CDC_I2C_INT_EP = 0x86;

static inline const int CDC_SPI_IN_EP = 0x21;
static inline const int CDC_SPI_OUT_EP = 0x22;
static inline const int CDC_SPI_INT_EP = 0x23;

static inline const int CDC_MAX_MPS = 64;

static inline const int USBD_VID = 0xffff;
static inline const int USBD_PID = 0xffff;
static inline const int USBD_MAX_POWER = 100;
static inline const int USBD_LANGID_STRING = 0x0409;

static inline const int USB_CONFIG_SIZE = (9 + CDC_ACM_DESCRIPTOR_LEN * 2);

const uint8_t cdc_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xFF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x04, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_UART_INT_EP, CDC_UART_OUT_EP, CDC_UART_IN_EP, CDC_MAX_MPS, 0x02),
    CDC_ACM_DESCRIPTOR_INIT(0x02, CDC_I2C_INT_EP, CDC_I2C_OUT_EP, CDC_I2C_IN_EP, CDC_MAX_MPS, 0x02),
    // CDC_ACM_DESCRIPTOR_INIT(0x04, CDC_SPI_INT_EP, CDC_SPI_OUT_EP, CDC_SPI_IN_EP, CDC_MAX_MPS, 0x02),
    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'J', 0x00,                  /* wcChar0 */
    'e', 0x00,                  /* wcChar1 */
    'f', 0x00,                  /* wcChar2 */
    'f', 0x00,                  /* wcChar3 */
    'p', 0x00,                  /* wcChar4 */
    'e', 0x00,                  /* wcChar5 */
    'n', 0x00,                  /* wcChar6 */
    'g', 0x00,                  /* wcChar7 */
    '3', 0x00,                  /* wcChar8 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x28,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'U', 0x00,                  /* wcChar0 */
    'n', 0x00,                  /* wcChar1 */
    'i', 0x00,                  /* wcChar2 */
    'v', 0x00,                  /* wcChar3 */
    'e', 0x00,                  /* wcChar4 */
    'r', 0x00,                  /* wcChar5 */
    's', 0x00,                  /* wcChar6 */
    'a', 0x00,                  /* wcChar7 */
    'l', 0x00,                  /* wcChar8 */
    'I', 0x00,                  /* wcChar9 */
    'O', 0x00,                  /* wcChar10 */
    '-', 0x00,                  /* wcChar11 */
    'E', 0x00,                  /* wcChar12 */
    'S', 0x00,                  /* wcChar13 */
    'P', 0x00,                  /* wcChar14 */
    '3', 0x00,                  /* wcChar15 */
    '2', 0x00,                  /* wcChar16 */
    'S', 0x00,                  /* wcChar17 */
    '3', 0x00,                  /* wcChar18 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x12,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '4', 0x00,                  /* wcChar3 */
    '0', 0x00,                  /* wcChar4 */
    '8', 0x00,                  /* wcChar5 */
    '2', 0x00,                  /* wcChar6 */
    '7', 0x00,                  /* wcChar7 */
    0x00};