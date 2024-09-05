// SPDX-License-Identifier: GPL-2.0
#define DEBUG

#include "i2c_engine.h"
#include "lib.h"
#include "spi_engine.h"
#include "uart_engine.h"

static int usb_cdc_channel_active(struct uio *uio, bool active)
{
	int val;
	int retval = 0;

	int actual_length;

	uio->control->needs_remote_wakeup = 1;

	if (active) retval = usb_submit_urb(uio->ctrlurb, GFP_KERNEL);
	if (retval) {
		dev_err(&uio->control->dev, "%s - usb_submit_urb(ctrl irq) failed\n", __func__);
		// goto error_submit_urb;
		return retval;
	}

	if (active)
		val = USB_CDC_CTRL_DTR | USB_CDC_CTRL_RTS;
	else
		val = 0;
	retval = usb_control_msg(uio->dev, usb_sndctrlpipe(uio->dev, 0), USB_CDC_REQ_SET_CONTROL_LINE_STATE, USB_RT_ACM,
							 val, uio->control->altsetting[0].desc.bInterfaceNumber, NULL, 0, USB_CTRL_SET_TIMEOUT);
	if (retval) {
		pr_err("Failed to set control line state\n");
	} else {
		uio->active = active;
	}

	// usb_autopm_put_interface(acm->control);

	return retval;
}

static void acm_ctrl_irq(struct urb *urb)
{
	struct uio *acm = urb->context;
	struct usb_cdc_notification *dr = urb->transfer_buffer;
	unsigned int current_size = urb->actual_length;
	unsigned int expected_size, copy_size, alloc_size;
	int retval;
	int status = urb->status;

	switch (status) {
	case 0:
		/* success */
		break;
	case -ECONNRESET:
	case -ENOENT:
	case -ESHUTDOWN:
		/* this urb is terminated, clean up */
		dev_info(&acm->control->dev, "%s - urb shutting down with status: %d\n", __func__, status);
		return;
	default:
		dev_info(&acm->control->dev, "%s - nonzero urb status received: %d\n", __func__, status);
		goto exit;
	}

	usb_mark_last_busy(acm->dev);

	if (acm->nb_index) dr = (struct usb_cdc_notification *)acm->notification_buffer;

	/* size = notification-header + (optional) data */
	expected_size = sizeof(struct usb_cdc_notification) + le16_to_cpu(dr->wLength);

	if (current_size < expected_size) {
		/* notification is transmitted fragmented, reassemble */
		if (acm->nb_size < expected_size) {
			u8 *new_buffer;
			alloc_size = roundup_pow_of_two(expected_size);
			/* Final freeing is done on disconnect. */
			new_buffer = krealloc(acm->notification_buffer, alloc_size, GFP_ATOMIC);
			if (!new_buffer) {
				acm->nb_index = 0;
				goto exit;
			}

			acm->notification_buffer = new_buffer;
			acm->nb_size = alloc_size;
			dr = (struct usb_cdc_notification *)acm->notification_buffer;
		}

		copy_size = min(current_size, expected_size - acm->nb_index);

		memcpy(&acm->notification_buffer[acm->nb_index], urb->transfer_buffer, copy_size);
		acm->nb_index += copy_size;
		current_size = acm->nb_index;
	}

	if (current_size >= expected_size) {
		/* notification complete */
		// acm_process_notification(acm, (unsigned char *)dr);
		acm->nb_index = 0;
	}

exit:
	retval = usb_submit_urb(urb, GFP_ATOMIC);
	if (retval && retval != -EPERM && retval != -ENODEV)
		dev_err(&acm->control->dev, "%s - usb_submit_urb failed: %d\n", __func__, retval);
	else
		dev_vdbg(&acm->control->dev, "control resubmission terminated %d\n", retval);
}

struct api_context {
	struct completion done;
	int status;
};

static void usb_api_blocking_completion(struct urb *urb)
{
	char *buf = urb->context;
	if (urb->status) {
		pr_err("Bulk message failed with error: %d\n", urb->status);
		return;
	}
	if (buf == NULL) {
		pr_err("Bulk message buffer is NULL!\n");
		return;
	}
	char bb[10] = {};
	for (int j = 0; j < urb->actual_length; j++) {
		sprintf(bb + j, "%c", buf[j]);
	}
	pr_info("Received Bulk message, actual length in urb: %d bytes %s\n", urb->actual_length, bb);
	int retval = usb_submit_urb(urb, GFP_KERNEL);
	if (retval) {
		pr_err("Failed to submit URB\n");
		// return retval;
	}
	// return 0;
}

static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id)
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

	if (data_interface->cur_altsetting->desc.bInterfaceClass != USB_CLASS_CDC_DATA) {
		if (control_interface->cur_altsetting->desc.bInterfaceClass == USB_CLASS_CDC_DATA) {
			dev_info(&usb_dev->dev, "Your device has switched interfaces.\n");
			swap(control_interface, data_interface);
		} else {
			return -EINVAL;
		}
	}

	// wer only initialize contril interface
	if (interface != control_interface) return -ENODEV;

	char sendbuf[100] = "HelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHello";

	epctrl = &control_interface->cur_altsetting->endpoint[0].desc;
	epread = &data_interface->cur_altsetting->endpoint[0].desc;
	epwrite = &data_interface->cur_altsetting->endpoint[1].desc;

	if (!usb_endpoint_dir_in(epread)) {
		/* descriptors are swapped */
		dev_info(&usb_dev->dev, "The data interface has switched endpoints\n");
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

	u8 *buf = usb_alloc_coherent(usb_dev, ctrlsize, GFP_KERNEL, &uio->ctrl_dma);
	if (!buf) {
		pr_err("Failed to allocate buffer for control endpoint\n");
		return -ENOMEM;
	}
	uio->ctrl_buffer = buf;

	usb_fill_int_urb(uio->ctrlurb, usb_dev, usb_rcvintpipe(usb_dev, epctrl->bEndpointAddress), uio->ctrl_buffer,
					 ctrlsize, acm_ctrl_irq, uio, epctrl->bInterval ? epctrl->bInterval : 16);
	uio->ctrlurb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	uio->ctrlurb->transfer_dma = uio->ctrl_dma;
	uio->ctrlurb->dev = uio->dev;

	uio->notification_buffer = NULL;
	uio->nb_index = 0;
	uio->nb_size = 0;

	usb_cdc_channel_active(uio, true);

	// 发送 Bulk 消息
	retval = usb_bulk_msg(usb_dev, usb_sndbulkpipe(usb_dev, 0x01), sendbuf, 5, &actual_length, 1000);
	if (retval)
		pr_err("Sending Bulk message failed with error: %d %d\n", retval, actual_length);
	else
		pr_info("Sent Bulk message successfully, actual length: %d bytes\n", actual_length);
	for (int i = 0; i < 1; i++) {
		char *rx_buf = kzalloc(100, GFP_KERNEL);
		struct urb *urb;
		urb = usb_alloc_urb(0, GFP_KERNEL);
		if (!urb) {
			pr_err("Failed to allocate URB\n");
			return -ENOMEM;
		}
		usb_fill_bulk_urb(urb, usb_dev, usb_rcvbulkpipe(usb_dev, 0x81), rx_buf, 64, usb_api_blocking_completion, rx_buf);
		retval = usb_submit_urb(urb, GFP_KERNEL);
		if (retval) {
			pr_err("Failed to submit URB\n");
			return retval;
		}
	}

	usb_cdc_channel_active(uio, 0);
	pr_info("USB device %04X:%04X init done.\n", id->idVendor, id->idProduct);
	return 0;
}

static void usb_disconnect(struct usb_interface *interface)
{
	pr_info("UniversalIO device unplugged\n");
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
