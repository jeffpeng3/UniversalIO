/* SPDX-License-Identifier: <SPDX License Expression> */
#pragma once
#include <linux/module.h>
#include <linux/usb.h>

#define UniIO_VENDOR_ID  0xffff  // 替換為你的裝置的 Vendor ID
#define UniIO_PRODUCT_ID 0xffff  // 替換為你的裝置的 Product ID

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
