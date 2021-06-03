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
#include "i2c_sensor.h"

#define I2C_ADDRESS 0x76
#define I2C_NAME "i2c-slave"

dev_t dev = 0;
static struct class *i2c_class;
static struct cdev i2c_cdev;
static struct i2c_client *bmp280_client = NULL;
static struct i2c_adapter *adap = NULL;


/*
 * Returns temperature in DegC, resolution is 0.01 DegC.  Output value of
 * "5123" equals 51.23 DegC.  t_fine carries fine temperature as global
 * value.
 *
 * Taken from datasheet, Section 3.11.3, "Compensation formula".
 */
s32 i2c_read_temperature(void) {
	int var1, var2;
	s32 raw_temp;
	s32 d1, d2, d3;

	/* Read Temperature */
	d1 = i2c_smbus_read_byte_data(bmp280_client, BMP280_REG_TEMP_MSB);
	d2 = i2c_smbus_read_byte_data(bmp280_client, BMP280_REG_TEMP_LSB);
	d3 = i2c_smbus_read_byte_data(bmp280_client, BMP280_REG_TEMP_XLSB);
	raw_temp = ((d1<<16) | (d2<<8) | d3) >> 4;

	/* Calculate temperature in degree */
	var1 = ((((raw_temp >> 3) - (dig_T1 << 1))) * (dig_T2)) >> 11;

	var2 = (((((raw_temp >> 4) - (dig_T1)) * ((raw_temp >> 4) - (dig_T1))) >> 12) * (dig_T3)) >> 14;
	return ((var1 + var2) *5 +128) >> 8;
}

/*
 * Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24
 * integer bits and 8 fractional bits).  Output value of "24674867"
 * represents 24674867/256 = 96386.2 Pa = 963.862 hPa
 *
 * Taken from datasheet, Section 3.11.3, "Compensation formula".
 */

u32 i2c_read_pressure(void) {

	s64 var1, var2,p;
	s64 raw_temp;
	s64 d1, d2, d3;

	/* Read Pressure */
	d1 = i2c_smbus_read_byte_data(bmp280_client, BMP280_REG_PRESS_MSB);
	d2 = i2c_smbus_read_byte_data(bmp280_client, BMP280_REG_PRESS_LSB);
	d3 = i2c_smbus_read_byte_data(bmp280_client, BMP280_REG_PRESS_XLSB);
	raw_temp = ((d1<<16) | (d2<<8) | d3) >> 4;

	var1 = raw_temp - 128000;
	var2 = var1 * var1 * dig_P6;
	var2 += (var1 * dig_P5) << 17;
	var2 += (dig_P4) << 35;
	var1 = ((var1 * var1 * dig_P3) >> 8) +
		((var1 * dig_P2) << 12);
	var1 = ((((s64)1) << 47) + var1) * (dig_P1) >> 33;

	if (var1 == 0)
		return 0;

	p = ((((s64)1048576 - raw_temp) << 31) - var2) * 3125;
	p = div64_s64(p, var1);
	var1 = ((dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = ((dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + ((dig_P7) << 4);

	return (u32)p;

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
	int temperature,pressure;

	/* Get amount of bytes to copy */
	to_copy = min(sizeof(out_string), count);

	/* Get temperature */
	temperature = i2c_read_temperature();
	snprintf(out_string, sizeof(out_string), "%d.%d\n", temperature/100, temperature%100);

	/*Get pressure*/
	pressure = i2c_read_pressure();
	pr_info("pressure value = %d.%d\n",pressure/100, pressure%100);

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

	/*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "I2c_Dev")) <0){
                pr_err("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

        /*Creating cdev structure*/
        cdev_init(&i2c_cdev,&fops);

        /*Adding character device to the system*/
        if((cdev_add(&i2c_cdev,dev,1)) < 0){
            pr_err("Cannot add the device to the system\n");
            goto r_class;
        }

        /*Creating struct class*/
        if((i2c_class = class_create(THIS_MODULE,"i2c_class")) == NULL){
            pr_err("Cannot create the struct class\n");
            goto r_class;
        }

        /*Creating device*/
        if((device_create(i2c_class,NULL,dev,NULL,"i2c_device")) == NULL){
            pr_err("Cannot create the Device 1\n");
            goto r_device;
        }
        
	/* Read Chip ID */
	id = i2c_smbus_read_byte_data(bmp280_client, 0xD0);
	printk("ID: 0x%x\n", id);

	/* Read Calibration Values */
	dig_T1 = i2c_smbus_read_word_data(bmp280_client, BMP280_REG_TEMP_CALIB1);
	dig_T2 = i2c_smbus_read_word_data(bmp280_client, BMP280_REG_TEMP_CALIB2);
	dig_T3 = i2c_smbus_read_word_data(bmp280_client, BMP280_REG_TEMP_CALIB3);

	if(dig_T2 > 32767)
		dig_T2 -= 65536;

	if(dig_T3 > 32767)
		dig_T3 -= 65536;

	/* Read Calibration Values for pressure */
	dig_P1 = i2c_smbus_read_word_data(bmp280_client, BMP280_REG_PRESS_CALIB1);
	dig_P2 = i2c_smbus_read_word_data(bmp280_client, BMP280_REG_PRESS_CALIB2);
	dig_P3 = i2c_smbus_read_word_data(bmp280_client, BMP280_REG_PRESS_CALIB3);
	dig_P4 = i2c_smbus_read_word_data(bmp280_client, BMP280_REG_PRESS_CALIB4);
	dig_P5 = i2c_smbus_read_word_data(bmp280_client, BMP280_REG_PRESS_CALIB5);
	dig_P6 = i2c_smbus_read_word_data(bmp280_client, BMP280_REG_PRESS_CALIB6);
	dig_P7 = i2c_smbus_read_word_data(bmp280_client, BMP280_REG_PRESS_CALIB7);
	dig_P8 = i2c_smbus_read_word_data(bmp280_client, BMP280_REG_PRESS_CALIB8);
	dig_P9 = i2c_smbus_read_word_data(bmp280_client, BMP280_REG_PRESS_CALIB9);

	if(dig_P2 > 32767)
		dig_P2 -= 65536;

	if(dig_P3 > 32767)
		dig_P3 -= 65536;

	if(dig_P4 > 32767)
		dig_T2 -= 65536;

	if(dig_P5 > 32767)
		dig_P5 -= 65536;

	if(dig_P6 > 32767)
		dig_P6 -= 65536;

	if(dig_P7 > 32767)
		dig_P7 -= 65536;

	if(dig_P8 > 32767)
		dig_P8 -= 65536;

	if(dig_P9 > 32767)
		dig_P9 -= 65536;

	/* Initialice the sensor */
	i2c_smbus_write_byte_data(bmp280_client, 0xf5, 5<<5);
	i2c_smbus_write_byte_data(bmp280_client, 0xf4, ((5<<5) | (5<<2) | (3<<0)));
	return 0;

r_device:
        class_destroy(i2c_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
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
	/*Bus number 1 according to raspberrypi board bus availability*/
	adap = i2c_get_adapter(1);
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
	i2c_unregister_device(bmp280_client);
    i2c_del_driver(&myclient_i2c);	
	pr_info("client Module Removed ..successfully\n");

}


module_init(i2c_client_init);
module_exit(i2c_client_exit);


MODULE_AUTHOR("HARISH");
MODULE_DESCRIPTION("I2C Client Device Driver");
MODULE_LICENSE("GPL");
