// SPDX-License-Identifier: GPL-2.0
#include "i2c_engine.h"
#include "lib.h"
#include "spi_engine.h"
#include "uart_engine.h"

static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_device *usb_dev = interface_to_usbdev(interface);
	int actual_length;
	int retval;
	char buf[100] = "HelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHello";

	pr_info("USB device (%04X:%04X)-%d plugged\n", id->idVendor, id->idProduct, id->bInterfaceNumber);

	// 发送 Bulk 消息
	retval = usb_bulk_msg(usb_dev, usb_sndbulkpipe(usb_dev, 0x01), buf, 81, &actual_length, 1000);
	if (retval)
		pr_err("Sending Bulk message failed with error: %d\n", retval);
	else
		pr_info("Sent Bulk message successfully, actual length: %d bytes\n", actual_length);

	// 接收 Bulk 消息
	retval = usb_bulk_msg(usb_dev, usb_rcvbulkpipe(usb_dev, 0x85), buf, 64, &actual_length, 1000);
	if (retval)
		pr_err("Receiving Bulk message failed with error: %d\n", retval);
	else
		pr_info("Received Bulk message successfully, actual length: %d bytes\n", actual_length);

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
