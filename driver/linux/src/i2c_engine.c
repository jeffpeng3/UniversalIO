// SPDX-License-Identifier: GPL-2.0
#include "i2c_engine.h"
// static int i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num);
// static u32 i2c_func(struct i2c_adapter *adap);

static int i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	pr_info("[i2c-%d] xfer!\n", adap->nr);
	pr_info("[i2c-%d] xfer num %d!\n", adap->nr, num);
	pr_info("[i2c-%d] xfer len %d!\n", adap->nr, msgs->len);
	// int len = msgs->len;
	return 1;
}

static u32 i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL_ALL;
}

struct i2c_algorithm algo = {
	.functionality = i2c_func,
	.master_xfer = i2c_xfer,
	.smbus_xfer = NULL};

struct i2c_adapter adapter = {
	.class = I2C_CLASS_HWMON,
	.owner = THIS_MODULE,
	.algo = &algo};
