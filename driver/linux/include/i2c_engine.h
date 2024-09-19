/* SPDX-License-Identifier: GPL-2.0 */
#pragma once
#include "lib.h"
#include <linux/i2c.h>
extern struct i2c_algorithm algo;
extern struct i2c_adapter adapter;
extern struct uio_queue *i2c_queue;
