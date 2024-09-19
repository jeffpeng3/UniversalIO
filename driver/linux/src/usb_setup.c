// SPDX-License-Identifier: GPL-2.0
#include "usb_setup.h"

int usb_cdc_channel_active(struct uio *uio)
{
	int val;
	int retval = 0;

	uio->control->needs_remote_wakeup = 1;

	retval = usb_submit_urb(uio->ctrlurb, GFP_KERNEL);
	if (retval) {
		dev_err(&uio->control->dev,
			"%s - usb_submit_urb(ctrl irq) failed\n", __func__);
		// goto error_submit_urb;
		return retval;
	}

	val = USB_CDC_CTRL_DTR | USB_CDC_CTRL_RTS;
	retval = usb_control_msg(
		uio->dev, usb_sndctrlpipe(uio->dev, 0),
		USB_CDC_REQ_SET_CONTROL_LINE_STATE, USB_RT_ACM, val,
		uio->control->altsetting[0].desc.bInterfaceNumber, NULL, 0,
		USB_CTRL_SET_TIMEOUT);
	if (retval)
		pr_err("Failed to set control line state\n");
	else
		uio->active = true;

	// usb_autopm_put_interface(acm->control);

	return retval;
}

void acm_ctrl_irq(struct urb *urb)
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
		dev_info(&acm->control->dev,
			 "%s - urb shutting down with status: %d\n", __func__,
			 status);
		return;
	default:
		dev_info(&acm->control->dev,
			 "%s - nonzero urb status received: %d\n", __func__,
			 status);
		goto exit;
	}

	usb_mark_last_busy(acm->dev);

	if (acm->nb_index)
		dr = (struct usb_cdc_notification *)acm->notification_buffer;

	/* size = notification-header + (optional) data */
	expected_size =
		sizeof(struct usb_cdc_notification) + le16_to_cpu(dr->wLength);

	if (current_size < expected_size) {
		/* notification is transmitted fragmented, reassemble */
		if (acm->nb_size < expected_size) {
			u8 *new_buffer;

			alloc_size = roundup_pow_of_two(expected_size);
			/* Final freeing is done on disconnect. */
			new_buffer = krealloc(acm->notification_buffer,
					      alloc_size, GFP_ATOMIC);
			if (!new_buffer) {
				acm->nb_index = 0;
				goto exit;
			}

			acm->notification_buffer = new_buffer;
			acm->nb_size = alloc_size;
			dr = (struct usb_cdc_notification *)
				     acm->notification_buffer;
		}

		copy_size = min(current_size, expected_size - acm->nb_index);

		memcpy(&acm->notification_buffer[acm->nb_index],
		       urb->transfer_buffer, copy_size);
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
		dev_err(&acm->control->dev, "%s - usb_submit_urb failed: %d\n",
			__func__, retval);
	else
		dev_vdbg(&acm->control->dev,
			 "control resubmission terminated %d\n", retval);
}

void usb_bluk_on_recv(struct urb *urb)
{
	char *buf = urb->transfer_buffer;

	if (urb->status) {
		pr_err("Bulk message failed with error: %d\n", urb->status);
		return;
	}
	if (buf == NULL) {
		pr_err("Bulk message buffer is NULL!\n");
		return;
	}
	char bb[65] = {};
	int actual_length;

	memcpy(bb, buf, urb->actual_length);
	pr_info("Received Bulk message, actual length in urb: %d bytes\n",
		urb->actual_length);
	pr_info("Received Bulk message: %s\n", bb);
	print_hex_dump(KERN_INFO, "Received Bulk message: ", DUMP_PREFIX_OFFSET,
		       16, 1, bb, urb->actual_length, false);
	int retval = usb_submit_urb(urb, GFP_KERNEL);

	if (retval) {
		pr_err("Failed to submit URB\n");
		// return retval;
	}
	// return 0;
}
