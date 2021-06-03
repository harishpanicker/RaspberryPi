#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/i2c.h>

static int BMP280_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg msgs[], int num)
{
	pr_info("I2C Transfer....\n");
	return 0;
}


static const struct i2c_algorithm BMP280_i2c_algo = {
	.master_xfer    = BMP280_i2c_xfer,
};

struct i2c_adapter adap = {
	.owner = THIS_MODULE,
	.class = I2C_CLASS_DEPRECATED,
	.algo  = &BMP280_i2c_algo,
	.name  = "i2c-dummy-bus",
};

static int __init bmp280_init(void)
{
	int ret;
	pr_info("Bus Driver Module Inserted..\n");
	ret = i2c_add_adapter(&adap);
	return 0;
}

static void __exit bmp280_exit(void)
{
	pr_info("Bus Driver Module Removed..\n");
	i2c_del_adapter(&adap);
}


module_init(bmp280_init);
module_exit(bmp280_exit);

MODULE_AUTHOR("HARISH");
MODULE_DESCRIPTION("I2C Bus Driver - Test");
MODULE_LICENSE("GPL");
