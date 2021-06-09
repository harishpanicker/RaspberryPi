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
#include <linux/ioctl.h>
#include "rtc_header.h"

#define RD_TIME 	_IOR('a','a',int32_t*)
#define RD_DATE 	_IOR('a','b',int32_t*)
#define RD_DAY	 	_IOR('a','c',uint8_t*)
#define MEM_SIZE	10  

dev_t dev = 0;
static struct class *i2c_class;
static struct cdev i2c_cdev;
static struct i2c_client *ds3231_client = NULL;
static struct i2c_adapter *adap = NULL;
int32_t *value = NULL;
uint8_t *kernel_buffer;

/*
 ** This function will be called when we to read the time
 */
static int Rtc_get_time(void){

	u8 sec,min,hr;

	sec = i2c_smbus_read_byte_data(ds3231_client, DS3231_REG_SECONDS);
	min = i2c_smbus_read_byte_data(ds3231_client, DS3231_REG_MINUTES);
	hr  = i2c_smbus_read_byte_data(ds3231_client, DS3231_REG_HOURS);
	printk("hr:min:sec =  %x:%x:%x\n", hr, min, sec);

	return hr*10000 + min*100 + sec;

}

/*
 ** This function will be called when we want to read the date
 */
static int Rtc_get_date(void){

	u8 dd,mm,yy;

	dd  = i2c_smbus_read_byte_data(ds3231_client, DS3231_REG_DATE);
	mm  = i2c_smbus_read_byte_data(ds3231_client, DS3231_REG_MONTH);
	yy  = i2c_smbus_read_byte_data(ds3231_client, DS3231_REG_YEAR);

	printk("dd/mm/yy =  %x/%x/%x\n", dd, mm, yy);

	return dd*10000 + mm*100 + yy;

}

/*
 ** This function will be called when we want to read which day today
 */
static uint8_t* Rtc_get_day(void){

	u8 day;

	day = i2c_smbus_read_byte_data(ds3231_client, DS3231_REG_DAY);

	switch(day){
		case 1 : return "MONDAY";
		case 2 : return "TUESDAY";
		case 3 : return "WEDNESDAY";
		case 4 : return "THURSDAY";
		case 5 : return "FRIDAY";
		case 6 : return "SATURDAY";
		case 7 : return "SUNDAY";
	}
	return 0;
}

//i2c_smbus_write_byte_data(ds3231_client, 0x00, );

/*
 ** This function will be called when we open the Device file
 */
static int i2c_open(struct inode *inode, struct file *file)
{
	if((value = kmalloc(sizeof(*value) , GFP_KERNEL)) == 0){
		pr_info("Cannot allocate memory in kernel\n");
		return -1;
	}
	if((kernel_buffer = kmalloc(MEM_SIZE , GFP_KERNEL)) == 0){
		pr_info("Cannot allocate memory in kernel\n");
		return -1;
	}	
	pr_info("Driver Open Function Called...!!!\n");
	return 0;
}

/*
 ** This function will be called when we close the Device file
 */
static int i2c_release(struct inode *inode, struct file *file)
{
	kfree(kernel_buffer);
	kfree(value);	
	pr_info("Driver Release Function Called...!!!\n");
	return 0;
}

/*
 ** This function will be called when we write IOCTL on the Device file
 */
static long i2c_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

	switch(cmd) {
		case RD_TIME:
			*value = Rtc_get_time();
			if( copy_to_user((int32_t*) arg, value, sizeof(value)) )
			{
				pr_err("Data Read for Time : Err!\n");
			}
			break;
		case RD_DATE:
			*value = Rtc_get_date();
			if( copy_to_user((int32_t*) arg, value, sizeof(value)) )
			{
				pr_err("Data Read for Calender : Err!\n");
			}
			break;
		case RD_DAY:
			kernel_buffer = Rtc_get_day();
			if( copy_to_user((uint8_t*) arg, kernel_buffer, sizeof(kernel_buffer)) )
			{
				pr_err("Data Read for day : Err!\n");
			}
			break;
		default:
			pr_info("Default\n");
			break;
	}
	return 0;
}

static struct file_operations fops =
{
	.owner      = THIS_MODULE,
	.open       = i2c_open,
	.release    = i2c_release,
	.unlocked_ioctl = i2c_ioctl,
};

int ds3231_probe(struct i2c_client *i2c_client, const struct i2c_device_id *device_id)
{

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

	return 0;

r_device:
	class_destroy(i2c_class);
r_class:
	unregister_chrdev_region(dev,1);
	return -1;	
}

static const struct i2c_device_id dev_id[]={
	{ I2C_NAME, 0
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
	.probe          = ds3231_probe,
	.id_table       = dev_id,
};

static struct i2c_board_info ds3231_i2c_board_info = {
	I2C_BOARD_INFO(I2C_NAME, I2C_ADDRESS)
};

static int __init ds3231_init(void)
{
	pr_info("Welcome to I2C Client Driver\n");
	/*Bus number 1 according to raspberrypi board bus availability*/
	adap = i2c_get_adapter(1);
	if(adap == NULL)
		printk("adapter error\n");

	ds3231_client = i2c_new_device(adap, &ds3231_i2c_board_info);

	if(ds3231_client!=NULL )
	{
		printk("Registered new device to i2c-core\n");
		i2c_add_driver(&myclient_i2c);
	}
	return 0;
}

static void __exit ds3231_exit(void)
{	
	device_destroy(i2c_class,dev);
	class_destroy(i2c_class);
	cdev_del(&i2c_cdev);
	unregister_chrdev_region(dev, 1);	
	i2c_unregister_device(ds3231_client);
	i2c_del_driver(&myclient_i2c);	
	pr_info("client Module Removed ..successfully\n");

}


module_init(ds3231_init);
module_exit(ds3231_exit);


MODULE_AUTHOR("HARISH");
MODULE_DESCRIPTION("I2C Client Device Driver");
MODULE_LICENSE("GPL");
