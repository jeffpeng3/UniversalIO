/* SPDX-License-Identifier: GPL-2.0 */
#pragma once
#include <linux/module.h>
#include <linux/usb.h>

#define UniIO_VENDOR_ID  0xffff
#define UniIO_PRODUCT_ID 0xffff

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
