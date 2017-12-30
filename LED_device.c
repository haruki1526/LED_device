#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/io.h>
#include <linux/timer.h>


MODULE_AUTHOR("Haruki Takamura");
MODULE_DESCRIPTION("driver for LED control");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");



static volatile u32 *gpio_base =NULL; //gpioのアドレスを入れる　volatileはアドレスを変えられないように固定する
static dev_t dev;
static struct cdev cdv;

static struct class *cls = NULL;

struct timer_list mytimer;

char set_timer = 1;
char check = 1;

static ssize_t led_timer(struct file* flip, const char* buf, size_t count, loff_t* pos)
{

	char t;
	
	if(copy_from_user(&t, buf,sizeof(char)))
		return -EFAULT;
	printk(KERN_INFO "deba:%d\n",t);
    if((int)t!=10){
	    del_timer(&mytimer);
	    set_timer = t-48 ;

	    mod_timer(&mytimer, jiffies + set_timer*HZ);
    }

	return 1;
}

static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.write = led_timer
};

static void led_write(unsigned long arg)
{
	if( check == 1)	{
		gpio_base[10] = 1 << 25;
		check = 0;
	}
	else if(check == 0){
		gpio_base[7] = 1 << 25;
		check = 1;
	}
	
	mod_timer(&mytimer, jiffies + set_timer*HZ);

}

static int __init init_mod(void)
{
	int retval;
	
	gpio_base = ioremap_nocache(0x3f200000, 0xA0); //先頭のアドレスが0x3f20　で　0xA0個場所をとる
	
	const u32 led = 25; //25番目
	const u32 index = led/10;//GPFSEL2
	const u32 shift = (led%10)*3;//15bit
	const u32 mask = ~(0x7 << shift);//111111111111000111111111111となる
	gpio_base[index] = (gpio_base[index] & mask ) | (0x1 << shift);//001: output flag//111111111111111100111111111111


	init_timer(&mytimer);
	mytimer.expires = jiffies + set_timer*HZ;
	mytimer.data = 0;
	mytimer.function = led_write;
	add_timer(&mytimer);



	
	retval = alloc_chrdev_region(&dev,0,1,"myled");
	if(retval < 0){
		printk(KERN_ERR " alloc_chrdev_region failed.\n");
		return retval;
	}
	printk(KERN_INFO "%s is losded. major:%d\n",__FILE__,MAJOR(dev));
	
	cdev_init(&cdv, &led_fops);
	retval = cdev_add(&cdv, dev, 1);
	if(retval <0){
		printk(KERN_ERR "cdev_add failed. major:%d, minor:%d",MAJOR(dev),MINOR(dev));
		return retval;
	}

	cls = class_create(THIS_MODULE,"myled");
	if(IS_ERR(cls)){
		printk(KERN_ERR "class_create failed.");
		return PTR_ERR(cls);
	}
	
	device_create(cls, NULL, dev, NULL, "myled%d",MINOR(dev));



		
	return 0;
	
}




static void __exit cleanup_mod(void)
{
	cdev_del(&cdv);
	device_destroy(cls, dev);
	class_destroy(cls);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "%s is unloaded. major:%d\n", __FILE__,MAJOR(dev));		
	
	del_timer(&mytimer);
}

module_init(init_mod);
module_exit(cleanup_mod);







