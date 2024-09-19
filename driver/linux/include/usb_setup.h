/* SPDX-License-Identifier: GPL-2.0 */
#pragma once
#include "lib.h"
void usb_bluk_on_recv(struct urb *urb);
int usb_cdc_channel_active(struct uio *uio);
void acm_ctrl_irq(struct urb *urb);
