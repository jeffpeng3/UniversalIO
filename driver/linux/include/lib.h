/* SPDX-License-Identifier: GPL-2.0 */
#pragma once
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/usb/cdc.h>

#define UniIO_VENDOR_ID  0xffff
#define UniIO_PRODUCT_ID 0xffff

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#define USB_RT_ACM (USB_TYPE_CLASS | USB_RECIP_INTERFACE)


struct uio {
    bool active;
    struct usb_device *dev;
    struct usb_interface *control;
    struct usb_interface *data;

    struct usb_endpoint_descriptor *epctrl;
	struct urb *ctrlurb;
    dma_addr_t ctrl_dma;
    u8 *ctrl_buffer;
    
	u8 *notification_buffer;			/* to reassemble fragmented notifications */
	unsigned int nb_index;
	unsigned int nb_size;


    struct usb_endpoint_descriptor *epread;
    struct usb_endpoint_descriptor *epwrite;

    
};