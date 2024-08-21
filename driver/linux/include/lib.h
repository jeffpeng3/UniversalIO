/* SPDX-License-Identifier: <SPDX License Expression> */
#pragma once
#include <linux/module.h>

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
