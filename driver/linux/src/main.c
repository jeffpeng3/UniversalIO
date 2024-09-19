// SPDX-License-Identifier: GPL-2.0
#define DEBUG

#include "i2c_engine.h"
#include "lib.h"
#include "spi_engine.h"
#include "uart_engine.h"
#include "usb_setup.h"

static void usb_bluk_on_send(struct urb *urb)
{
	char bb[65] = {};
	uint8_t *buf = urb->transfer_buffer;

	static int count;

	memcpy(bb, buf, urb->actual_length);

	pr_info("Transmit Bulk message, actual length in urb: %d bytes\n",
		urb->actual_length);
	print_hex_dump(KERN_INFO, "Transmit Bulk message: ", DUMP_PREFIX_OFFSET,
		       16, 1, bb, urb->actual_length, false);
	// delay(3);
	if (*buf == 255) {
		buf[0] = 0;
		buf[1] = 'A';
		buf[2] = 'b';
		buf[3] = '*';
		buf[4] = '@';
		// buf[5] = 0;
		urb->transfer_buffer_length = 5;
		usb_submit_urb(urb, GFP_KERNEL);
		count = 0;
	} else if (count < 5) {
		count++;
		usb_submit_urb(urb, GFP_KERNEL);
	} else {
		kfree(buf);
		usb_free_urb(urb);
	}
}

static int usb_probe(struct usb_interface *interface,
		     const struct usb_device_id *id)
{
	struct usb_device *usb_dev = interface_to_usbdev(interface);

	struct usb_interface *control_interface;
	struct usb_interface *data_interface;

	struct usb_endpoint_descriptor *epctrl;
	struct usb_endpoint_descriptor *epread;
	struct usb_endpoint_descriptor *epwrite;

	int actual_length;
	int retval;

	control_interface = usb_ifnum_to_if(usb_dev, 0);
	data_interface = usb_ifnum_to_if(usb_dev, 1);

	if (data_interface->cur_altsetting->desc.bInterfaceClass !=
	    USB_CLASS_CDC_DATA) {
		if (control_interface->cur_altsetting->desc.bInterfaceClass ==
		    USB_CLASS_CDC_DATA) {
			dev_info(&usb_dev->dev,
				 "Your device has switched interfaces.\n");
			swap(control_interface, data_interface);
		} else {
			return -EINVAL;
		}
	}

	// we only initialize contril interface
	if (interface != control_interface)
		return -ENODEV;

	uint8_t data[100] = { 255, 0 };

	epctrl = &control_interface->cur_altsetting->endpoint[0].desc;
	epread = &data_interface->cur_altsetting->endpoint[0].desc;
	epwrite = &data_interface->cur_altsetting->endpoint[1].desc;

	if (!usb_endpoint_dir_in(epread)) {
		/* descriptors are swapped */
		dev_info(&usb_dev->dev,
			 "The data interface has switched endpoints\n");
		swap(epread, epwrite);
	}

	struct uio *uio = kzalloc(sizeof(struct uio), GFP_KERNEL);

	uio->active = false;
	uio->dev = usb_dev;
	uio->control = control_interface;
	uio->data = data_interface;
	uio->epctrl = epctrl;
	uio->epread = epread;
	uio->epwrite = epwrite;
	uio->ctrlurb = usb_alloc_urb(0, GFP_KERNEL);

	int ctrlsize = usb_endpoint_maxp(epctrl);

	u8 *ctrlBuf = usb_alloc_coherent(usb_dev, ctrlsize, GFP_KERNEL,
					 &uio->ctrl_dma);

	if (!ctrlBuf) {
		pr_err("Failed to allocate buffer for control endpoint\n");
		return -ENOMEM;
	}
	uio->ctrl_buffer = ctrlBuf;

	usb_fill_int_urb(uio->ctrlurb, usb_dev,
			 usb_rcvintpipe(usb_dev, epctrl->bEndpointAddress),
			 uio->ctrl_buffer, ctrlsize, acm_ctrl_irq, uio,
			 epctrl->bInterval ? epctrl->bInterval : 16);
	uio->ctrlurb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	uio->ctrlurb->transfer_dma = uio->ctrl_dma;
	uio->ctrlurb->dev = uio->dev;

	uio->notification_buffer = NULL;
	uio->nb_index = 0;
	uio->nb_size = 0;

	usb_cdc_channel_active(uio);

	struct urb *urb = usb_alloc_urb(0, GFP_KERNEL);
	char *tx_buf = kzalloc(64, GFP_KERNEL);

	memcpy(tx_buf, data, 64);
	usb_fill_bulk_urb(urb, usb_dev,
			  usb_sndbulkpipe(usb_dev, epwrite->bEndpointAddress),
			  tx_buf, 1, usb_bluk_on_send, tx_buf);
	retval = usb_submit_urb(urb, GFP_KERNEL);
	if (retval) {
		pr_err("Failed to submit URB\n");
		// return retval;
	}

	for (int i = 0; i < 2; i++) {
		char *rx_buf = kzalloc(64, GFP_KERNEL);
		struct urb *urb = usb_alloc_urb(0, GFP_KERNEL);

		if (!urb) {
			pr_err("Failed to allocate URB\n");
			return -ENOMEM;
		}
		usb_fill_bulk_urb(urb, usb_dev,
				  usb_rcvbulkpipe(usb_dev,
						  epread->bEndpointAddress),
				  rx_buf, 64, usb_bluk_on_recv, uio);
		retval = usb_submit_urb(urb, GFP_KERNEL);
		if (retval) {
			pr_err("Failed to submit URB\n");
			return retval;
		}
	}

	pr_info("USB device %04X:%04X init done.\n", id->idVendor,
		id->idProduct);
	return 0;
}

static void usb_disconnect(struct usb_interface *interface)
{
	pr_info("UniversalIO device unplugged\n");
	// 這裡可以添加清理代碼
}

static const struct usb_device_id UniIO_device_ids[] = {
	{ USB_DEVICE(UniIO_VENDOR_ID, UniIO_PRODUCT_ID) },
	{}
};

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
