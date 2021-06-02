#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/i2c.h>

#define I2C_ADDRESS 0x40 	//0x76
#define I2C_NAME "i2c-slave"

dev_t dev = 0;
static struct class *i2c_class;
static struct cdev i2c_cdev;
struct i2c_client *bmp280_client = NULL;
struct i2c_adapter *adap = NULL;


/* Variables for temperature calculation */
s32 dig_T1, dig_T2, dig_T3;

/* Read current temperature from BMP280 sensor and return temperature in degree */

s32 read_temperature(void) {
	int var1, var2;
	s32 raw_temp;
	s32 d1, d2, d3;

	/* Read Temperature */
	d1 = i2c_smbus_read_byte_data(bmp280_client, 0xFA);
	d2 = i2c_smbus_read_byte_data(bmp280_client, 0xFB);
	d3 = i2c_smbus_read_byte_data(bmp280_client, 0xFC);
	raw_temp = ((d1<<16) | (d2<<8) | d3) >> 4;

	/* Calculate temperature in degree */
	var1 = ((((raw_temp >> 3) - (dig_T1 << 1))) * (dig_T2)) >> 11;

	var2 = (((((raw_temp >> 4) - (dig_T1)) * ((raw_temp >> 4) - (dig_T1))) >> 12) * (dig_T3)) >> 14;
	return ((var1 + var2) *5 +128) >> 8;
}

/*
** This function will be called when we open the Device file
*/
static int i2c_open(struct inode *inode, struct file *file)
{
        pr_info("Driver Open Function Called...!!!\n");
        return 0;
}

/*
** This function will be called when we close the Device file
*/
static int i2c_release(struct inode *inode, struct file *file)
{
        pr_info("Driver Release Function Called...!!!\n");
        return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t i2c_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta;
	char out_string[20];
	int temperature;

	/* Get amount of bytes to copy */
	to_copy = min(sizeof(out_string), count);

	/* Get temperature */
	temperature = read_temperature();
	snprintf(out_string, sizeof(out_string), "%d.%d\n", temperature/100, temperature%100);

	/* Copy Data to user */
	not_copied = copy_to_user(user_buffer, out_string, to_copy);

	/* Calculate delta */
	delta = to_copy - not_copied;

	return delta;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t i2c_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Driver Write Function Called...!!!\n");
        return len;
}

static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = i2c_read,
    .write      = i2c_write,
    .open       = i2c_open,
    .release    = i2c_release,
};


int bmp280_probe(struct i2c_client *i2c_client, const struct i2c_device_id *device_id)
{

	u8 id;

	pr_info("Probe called Successfully....\n");

       	/* Read Chip ID */
	id = i2c_smbus_read_byte_data(bmp280_client, 0xD0);
	printk("ID: 0x%x\n", id);

	/* Read Calibration Values */
	dig_T1 = i2c_smbus_read_word_data(bmp280_client, 0x88);
	dig_T2 = i2c_smbus_read_word_data(bmp280_client, 0x8a);
	dig_T3 = i2c_smbus_read_word_data(bmp280_client, 0x8c);

	if(dig_T2 > 32767)
		dig_T2 -= 65536;

	if(dig_T3 > 32767)
		dig_T3 -= 65536;

	/* Initialice the sensor */
	i2c_smbus_write_byte_data(bmp280_client, 0xf5, 5<<5);
	i2c_smbus_write_byte_data(bmp280_client, 0xf4, ((5<<5) | (5<<2) | (3<<0)));
	return 0;

}

static const struct i2c_device_id dev_id[]={
	{ I2C_NAME, I2C_ADDRESS
	},
	{}
};
MODULE_DEVICE_TABLE(i2c,dev_id);

static struct i2c_driver myclient_i2c = {
	.class          = I2C_CLASS_DEPRECATED,
	.driver = {
		.owner = THIS_MODULE,
		.name   = I2C_NAME,
	},
	.probe          = bmp280_probe,
	.id_table       = dev_id,
};

static struct i2c_board_info bmp280_i2c_board_info = {
	I2C_BOARD_INFO(I2C_NAME, I2C_ADDRESS)
};

static int __init i2c_client_init(void)
{
	pr_info("Welcome to I2C Client Driver\n");
	adap = i2c_get_adapter(6);
	if(adap == NULL)
		printk("adapter error\n");
	
	bmp280_client = i2c_new_device(adap, &bmp280_i2c_board_info);

	if(bmp280_client!=NULL )
        {
		printk("Registered new device to i2c-core\n");
		i2c_add_driver(&myclient_i2c);
        }
	i2c_put_adapter(adap);
	return 0;
}

static void __exit i2c_client_exit(void)
{
	device_destroy(i2c_class,dev);
	class_destroy(i2c_class);
	cdev_del(&i2c_cdev);
	unregister_chrdev_region(dev, 1);	
	i2c_del_driver(&myclient_i2c);	
	i2c_unregister_device(bmp280_client);    		
	adap=NULL;
	pr_info("client Module Removed ..successfully\n");

}


module_init(i2c_client_init);
module_exit(i2c_client_exit);


MODULE_AUTHOR("HARISH");
MODULE_DESCRIPTION("I2C Client Device Driver");
MODULE_LICENSE("GPL");
