#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>  //copy_to/from_user()
#include <linux/gpio.h>     //GPIO

//LED is connected to this GPIO
#define GPIO_20 (20)

#define SET_GPIO_PIN _IO('a','a') 
#define CLEAR_GPIO_PIN _IO('a','b')
#define READ_GPIO_PIN _IOR('a','c',int32_t*)

int32_t data;
dev_t dev = 0;
static struct class *dev_class;
static struct cdev my_cdev;

static int __init my_driver_init(void);
static void __exit my_driver_exit(void);


static int my_open(struct inode *inode, struct file *file);
static int my_release(struct inode *inode, struct file *file);
static ssize_t my_read(struct file *filp, 
		char __user *buf, size_t len,loff_t * off);
static ssize_t my_write(struct file *filp, 
		const char *buf, size_t len, loff_t * off);
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg);


static struct file_operations fops =
{
	.owner          = THIS_MODULE,
	.read           = my_read,
	.write          = my_write,
	.open           = my_open,
	.release        = my_release,
	.unlocked_ioctl = my_ioctl,
};

/*
 ** This function will be called when we open the Device file
 */ 
static int my_open(struct inode *inode, struct file *file)
{
	pr_info("Device File Opened...!!!\n");
	return 0;
}

/*
 ** This function will be called when we close the Device file
 */
static int my_release(struct inode *inode, struct file *file)
{
	pr_info("Device File Closed...!!!\n");
	return 0;
}

/*
 ** This function will be called when we read the Device file
 */ 
static ssize_t my_read(struct file *filp, 
		char __user *buf, size_t len, loff_t *off)
{
	pr_info("Read function called,,,!!!!\n");
	return 0;
}

/*
 ** This function will be called when we write the Device file
 */ 
static ssize_t my_write(struct file *filp, 
		const char __user *buf, size_t len, loff_t *off)
{

	pr_info("Write Function called..!!! \n");
	return len;
}

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
         switch(cmd) {
                case SET_GPIO_PIN:
			//set the GPIO value to HIGH
			gpio_set_value(GPIO_20, 1);
			pr_info("GPIO value is set...!!!\n");
			break;
		case CLEAR_GPIO_PIN:
			//set the GPIO value to LOW
			gpio_set_value(GPIO_20, 0);
			pr_info("GPIO value is clear...!!!\n");
			break;
		case READ_GPIO_PIN:
			data=gpio_get_value(GPIO_20);
			if(copy_to_user((int32_t*)arg,&data,sizeof(data)))
                        {
                                pr_err("Data Read : Err!\n");
                        }
			pr_info("GPIO value is = %d\n",data);
			break;
                default:
                        pr_info("Default\n");
                        break;
        }
        return 0;
}

/*
 ** Module Init function
 */ 
static int __init my_driver_init(void)
{
	/*Allocating Major number*/
	if((alloc_chrdev_region(&dev, 0, 1, "char_Dev")) <0){
		pr_err("Cannot allocate major number\n");
		goto r_unreg;
	}
	pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

	/*Creating cdev structure*/
	cdev_init(&my_cdev,&fops);

	/*Adding character device to the system*/
	if((cdev_add(&my_cdev,dev,1)) < 0){
		pr_err("Cannot add the device to the system\n");
		goto r_del;
	}

	/*Creating struct class*/
	if((dev_class = class_create(THIS_MODULE,"char_class")) == NULL){
		pr_err("Cannot create the struct class\n");
		goto r_class;
	}

	/*Creating device*/
	if((device_create(dev_class,NULL,dev,NULL,"gpio_pin")) == NULL){
		pr_err( "Cannot create the Device \n");
		goto r_device;
	}

	//Checking the GPIO is valid or not
	if(gpio_is_valid(GPIO_20) == false){
		pr_err("GPIO %d is not valid\n", GPIO_20);
		goto r_device;
	}

	//Requesting the GPIO
	if(gpio_request(GPIO_20,"GPIO_20") < 0){
		pr_err("ERROR: GPIO %d request\n", GPIO_20);
		goto r_gpio;
	}

	//configure the GPIO as output
	gpio_direction_output(GPIO_20, 0);

	pr_info("Device Driver Insert...Done!!!\n");
	return 0;

r_gpio:
	gpio_free(GPIO_20);
r_device:
	device_destroy(dev_class,dev);
r_class:
	class_destroy(dev_class);
r_del:
	cdev_del(&my_cdev);
r_unreg:
	unregister_chrdev_region(dev,1);

	return -1;
}

/*
 ** Module exit function
 */ 
static void __exit my_driver_exit(void)
{
	gpio_free(GPIO_20);
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&my_cdev);
	unregister_chrdev_region(dev, 1);
	pr_info("Device Driver Remove...Done!!\n");
}

module_init(my_driver_init);
module_exit(my_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("harish panicker");
MODULE_DESCRIPTION("GPIO Driver");
