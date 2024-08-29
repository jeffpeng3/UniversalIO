// SPDX-License-Identifier: GPL-2.0
#include "i2c_engine.h"
#include "lib.h"
#include "spi_engine.h"
#include "uart_engine.h"

static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	pr_info("CDC-ACM device (%04X:%04X)-%d plugged\n", id->idVendor, id->idProduct, id->bInterfaceNumber);
	// 這裡可以添加初始化和設置 CDC-ACM 裝置的代碼
	return 0;
}
static void usb_disconnect(struct usb_interface *interface)
{
	pr_info("CDC-ACM device unplugged\n");
	// 這裡可以添加清理代碼
}

static const struct usb_device_id UniIO_device_ids[] = {{USB_DEVICE(UniIO_VENDOR_ID, UniIO_PRODUCT_ID)}, {}};

static struct usb_driver UniIO_driver = {
	.name = "UniversalIO",
	.id_table = UniIO_device_ids,
	.probe = usb_probe,
	.disconnect = usb_disconnect,
};

module_usb_driver(UniIO_driver);
MODULE_DEVICE_TABLE(usb, UniIO_device_ids);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeff");
MODULE_DESCRIPTION("UniversalIO linux driver");
