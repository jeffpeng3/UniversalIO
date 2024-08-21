// SPDX-License-Identifier: GPL-2.0
#include "lib.h"
#include "i2c_engine.h"
#include "spi_engine.h"
#include "uart_engine.h"

static int __init my_i2c_init(void)
{
	snprintf(adapter.name, sizeof(adapter.name), "My I2C Adapter");

	if (i2c_add_adapter(&adapter)) {
		// i2c_put_adapter(&adapter);
		return -ENODEV;
	}
	pr_info("hello i2c-%d!\n", adapter.nr);

	return 0;
}

static void __exit my_i2c_exit(void)
{
	pr_info("goodbye i2c-%d!\n", adapter.nr);
	i2c_del_adapter(&adapter);
}

module_init(my_i2c_init);
module_exit(my_i2c_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Author");
MODULE_DESCRIPTION("A simple I2C adapter driver");
