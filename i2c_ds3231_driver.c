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

#define I2C_ADDRESS 0x68
#define I2C_NAME "i2c-slave"

#define RD_DATE 	_IOR('a','a',int32_t*)
#define RD_CALENDER 	_IOR('a','b',int32_t*)


dev_t dev = 0;
static struct class *i2c_class;
static struct cdev i2c_cdev;
static struct i2c_client *ds3231_client = NULL;
static struct i2c_adapter *adap = NULL;
int32_t value = 0;

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

	pr_info("Driver Read Function Called...!!!\n");
	return 0;
}


/*
 ** This function will be called when we write the Device file
 */
static ssize_t i2c_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	pr_info("Driver Write Function Called...!!!\n");
	//i2c_smbus_write_byte_data(ds3231_client, 0x00, );
	return len;
}

/*
 ** This function will be called when we write IOCTL on the Device file
 */
static long i2c_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	u8 sec,min,hr;

	switch(cmd) {
		case RD_DATE:

			sec = i2c_smbus_read_byte_data(ds3231_client, 0x00);
			min = i2c_smbus_read_byte_data(ds3231_client, 0x01);
			hr  = i2c_smbus_read_byte_data(ds3231_client, 0x02);
			printk("hr:min:sec =  %d:%d:%d\n", hr, min, sec);
			value = hr*10000 + min*100 + sec;
			if( copy_to_user((int32_t*) arg, &value, sizeof(value)) )
			{
				pr_err("Data Read : Err!\n");
			}
			pr_info("Value = %d\n", value);
			break;
		case RD_CALENDER:
			if( copy_to_user((int32_t*) arg, &value, sizeof(value)) )
			{
				pr_err("Data Read : Err!\n");
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
	.read       = i2c_read,
	.write      = i2c_write,
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
