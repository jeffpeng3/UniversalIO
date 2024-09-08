#include "usbd_core.h"
#include "usbd_cdc.h"
struct usbd_interface if_in;
struct usbd_interface if_out;

const int IN_EP = 0x81;
const int OUT_EP = 0x01;
const int INT_EP = 0x85;

const int CDC_MAX_MPS = 64;

const int USBD_VID = 0xffff;
const int USBD_PID = 0xffff;
const int USBD_MAX_POWER = 100;
const int USBD_LANGID_STRING = 0x0409;

const int USB_CONFIG_SIZE = (9 + CDC_ACM_DESCRIPTOR_LEN);

const uint8_t cdc_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xFF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, INT_EP, OUT_EP, IN_EP, CDC_MAX_MPS, 0x02),
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