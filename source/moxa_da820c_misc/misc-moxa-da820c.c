/*
 * This driver is for Moxa embedded computer programmble led, relay, power
 * indicator driver. It based on IT8786 GPIO hardware.
 *
 * History:
 * Date		Aurhor		Comment
 * 2018/08/23	Elvis Yao	First create for DA-820C.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/io.h>

#define SUPPORT_GPIOSYSFS

#include "gpio-it8786.c"
#include "dio_main.h"

#define LED_NUM			8
#define LED_PATTERN		"11111111"
#define LED_ON			0
#define LED_OFF			1

#define RELAY_NUM		1
#define RELAY_PATTERN		"1"
#define RELAY_ON		1
#define RELAY_OFF		0

#define DI_NUM			4
#define DI_PATTERN		"1111"
#define DI_ON			1
#define DI_OFF			0

#define DO_NUM			4
#define DO_PATTERN		"1111"
#define DO_ON			1
#define DO_OFF			0

#define USBPWR_NUM		3
#define USBPWR_PATTERN		"111"
#define USBPWR_ON		1
#define USBPWR_OFF		0

#define USBDET_NUM		3
#define USBDET_PATTERN		"111"
#define USBDET_ON		1
#define	USBDET_OFF		0

#define SIF_NUM			2
#define SIF_PATTERN		"12"

/* mknod /dev/pled c 10 100 for this module */
#define MOXA_PLED_MINOR		100
#define MOXA_SIF_MINOR		(MOXA_PLED_MINOR+1)
#define MOXA_RELAY_MINOR	(MOXA_SIF_MINOR+1)
#define MOXA_DI_MINOR		(MOXA_RELAY_MINOR+1)
#define MOXA_DO_MINOR		(MOXA_DI_MINOR+1)
#define MOXA_USBPWR_MINOR	(MOXA_DO_MINOR+1)
#define MOXA_USBDET_MINOR	(MOXA_USBPWR_MINOR+1)

#define PLED_NAME		"pled"
#define SIF_NAME		"uart"
#define RELAY_NAME		"relay"
#define DI_NAME			"di"
#define DO_NAME			"do"
#define USBPWR_NAME		"usbpwr"
#define USBDET_NAME		"usbdet"

/* Ioctl number */
#define MOXA			0x400
#define MOXA_SET_OP_MODE	(MOXA + 66)
#define MOXA_GET_OP_MODE	(MOXA + 67)

/* Debug message */
#ifdef DEBUG
#define dbg_printf(x...)	printk(x)
#else
#define dbg_printf(x...)
#endif

/*
 * module: Programable LED section
 */
static int pled_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int pled_release(struct inode *inode, struct file *file)
{
	return 0;
}

/*
 * Write function
 * Note: use echo 11111111 > /dev/pled
 * The order is [pled1, pled2, pled3, pled4, ..., pled8\n]
 */
ssize_t pled_write(struct file *filp, const char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	char stack_buf[LED_NUM];

	/* check input value */
	if(count != (sizeof(stack_buf)+1)) {
		printk( KERN_ERR "Moxa pled error! paramter should be %lu digits\
, like \"%s\" \n", sizeof(stack_buf)/sizeof(char), LED_PATTERN);
		return -EINVAL;
	}

	if(copy_from_user(stack_buf, buf, count-1)) {
		return 0;
	}

	for(i = 0; i < sizeof(stack_buf); i++) {
		if (stack_buf[i] == '1') {
			pled_set(i, LED_ON);
		} else if (stack_buf[i] == '0') {
			pled_set(i, LED_OFF);
		} else {
			printk("pled: error, the input is %s", stack_buf);
			break;
		}
	}

	return count;
}

ssize_t pled_read(struct file *filp, char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	int ret;
	char stack_buf[LED_NUM];

	for (i = 0; (i < sizeof(stack_buf)) && (i < count); i++) {
		if ( !pled_get(i, &ret) ) {
			if ( ret ) {
				stack_buf[i] = '0' + LED_ON;
			} else {
				stack_buf[i] = '0' + LED_OFF;
			}
		} else {
			return -EINVAL;
		}
	}

	ret = copy_to_user((void*)buf, (void*)stack_buf, i);
	if(ret < 0) {
		return -ENOMEM;
	}

	return i;
}

/* define which file operations are supported */
struct file_operations pled_fops = {
	.owner		= THIS_MODULE,
	.write		= pled_write,
	.read		= pled_read,
	.open		= pled_open,
	.release	= pled_release,
};

/* register as misc driver */
static struct miscdevice pled_miscdev = {
	.minor = MOXA_PLED_MINOR,
	.name = PLED_NAME,
	.fops = &pled_fops,
};

/*
 * module: Relay output section
 */
static int relay_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int relay_release(struct inode *inode, struct file *file)
{
	return 0;
}

/* Write function
 * Note: use echo 1 > /dev/relay
 * The order is [realy1\n]
 */
ssize_t relay_write(struct file *filp, const char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	char stack_buf[RELAY_NUM];

	/* check input value */
	if(count != (sizeof(stack_buf)+1)) {
		printk( KERN_ERR "Moxa relay error! paramter should be %lu \
digits, like \"%s\" \n", sizeof(stack_buf), RELAY_PATTERN);
		return -EINVAL;
	}

	if(copy_from_user(stack_buf, buf, count-1)) {
		return 0;
	}

	for(i = 0; i < sizeof(stack_buf); i++) {
		if (stack_buf[i] == '1') {
			relay_set(i, RELAY_ON);
		} else if (stack_buf[i] == '0') {
			relay_set(i, RELAY_OFF);
		} else {
			printk("relay: error, you input is %s", stack_buf);
			break;
		}
	}

	return count;
}

ssize_t relay_read(struct file *filp, char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	int ret;
	char stack_buf[RELAY_NUM];

	for (i = 0; (i < sizeof(stack_buf)) && (i < count); i++) {
		if (!relay_get(i, &ret)) {
			if (ret) {
				stack_buf[i] = '0' + RELAY_ON;
			} else {
				stack_buf[i] = '0' + RELAY_OFF;
			}
		} else {
			return -EINVAL;
		}
	}

	ret = copy_to_user((void*)buf, (void*)stack_buf, sizeof(stack_buf));
	if(ret < 0) {
		printk(KERN_ERR "Moxa relay error! paramter should be %lu \
digits, like \"%s\" \n", sizeof(stack_buf), RELAY_PATTERN);
		return -ENOMEM;
	}

	return sizeof(stack_buf);
}

/* define which file operations are supported */
struct file_operations relay_fops = {
	.owner		= THIS_MODULE,
	.write		= relay_write,
	.read		= relay_read,
	.open		= relay_open,
	.release	= relay_release,
};

/* register as misc driver */
static struct miscdevice relay_miscdev = {
	.minor = MOXA_RELAY_MINOR,
	.name = RELAY_NAME,
	.fops = &relay_fops,
};


/*
 * module: DI section
 */
static int di_open(struct inode *inode, struct file *file)
{
	return 0;
}

/* Write function
 * Note: use echo 1111 > /dev/di
 * The order is [di1, di2, di3, di4\n]
 */
ssize_t di_write(struct file *filp, const char __user *buf, size_t count,
	loff_t *pos)
{
	printk("<1> %s[%d]\n", __FUNCTION__, __LINE__);
	return 0;
}

static int di_release(struct inode *inode, struct file *file)
{
	return 0;
}

ssize_t di_read(struct file *filp, char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	int ret;
	char stack_buf[DI_NUM];

	for (i = 0; (i < sizeof(stack_buf)) && (i < count); i++) {
		if ( !di_get(i, &ret) ) {
			if (ret) {
				stack_buf[i] = '0' + DI_ON;
			} else {
				stack_buf[i] = '0' + DI_OFF;
			}
		} else {
			return -EINVAL;
		}
	}

	ret = copy_to_user((void*)buf, (void*)stack_buf, i);
	if(ret < 0) {
		return -ENOMEM;
	}

	return i;
}

long di_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
	struct dio_set_struct   set;
	int i = 0;
	int di_state = 0;

	/* check data */
	if (copy_from_user(&set, (struct dio_set_struct *)arg,
		sizeof(struct dio_set_struct)) == sizeof(struct dio_set_struct))
		return -EFAULT;

	switch (cmd) {
		case IOCTL_GET_DIN :
			dp("get din io_number:%x\n",set.io_number);

			if(set.io_number == ALL_PORT) {
				set.mode_data = 0;
				for( i=0; i< DI_NUM; i++) {
					di_get(set.io_number, &di_state);
					set.mode_data |= (di_state<<i);
				}
				dp(KERN_DEBUG "all port: %x", set.mode_data);
			} else { 
				if(set.io_number < 0 || set.io_number >= DI_NUM)
					return -EINVAL;

				if(di_get(set.io_number, &set.mode_data) < 0)
					printk(KERN_ALERT "di_get(%d, %d) fail\n",
						set.io_number, set.mode_data);
			}

			if(copy_to_user((struct dio_set_struct *)arg, &set,
			sizeof(struct dio_set_struct)) == sizeof(struct dio_set_struct))
				return -EFAULT;

			dp("mode_data: %x\n", (unsigned int)set.mode_data);

			break;

		default :
			printk(KERN_DEBUG "ioctl:error\n");
			return -EINVAL;
	}

	return 0; /* if switch ok */
}

/* define which file operations are supported */
struct file_operations di_fops = {
	.owner		= THIS_MODULE,
	.read		= di_read,
	.open		= di_open,
	.write		= di_write,
	.unlocked_ioctl	= di_ioctl,
	.release	= di_release,
};

/* register as misc driver */
static struct miscdevice di_miscdev = {
	.minor = MOXA_DI_MINOR,
	.name = DI_NAME,
	.fops = &di_fops,
};

/*
 * module: DO section
 */
static int do_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int do_release(struct inode *inode, struct file *file)
{
	return 0;
}

/* Write function
 * Note: use echo 1111 > /dev/do
 * The order is [do1, do2, do3, do4\n]
 */
ssize_t do_write(struct file *filp, const char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	char stack_buf[DO_NUM];

	/* check input value */
	if (count != (sizeof(stack_buf)+1)) {
		printk(KERN_ERR "Moxa do error! paramter should be %lu \
digits, like \"%s\" \n", sizeof(stack_buf), DO_PATTERN);
		return -EINVAL;
	}

	if (copy_from_user(stack_buf, buf, count-1)) {
		return 0;
	}

	for (i = 0; i < sizeof(stack_buf); i++) {
		if (stack_buf[i] == '1') {
			do_set(i, DO_ON);
		} else if (stack_buf[i] == '0') {
			do_set(i, DO_OFF);
		} else {
			printk("do: error, you input is %s", stack_buf);
			break;
		}
	}

	return count;
}

ssize_t do_read(struct file *filp, char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	int ret;
	char stack_buf[DO_NUM];

	for (i = 0; (i < sizeof(stack_buf)) && (i < count); i++) {
		if ( !do_get(i, &ret) ) {
			if ( ret ) {
				stack_buf[i] = '0' + DO_ON;
			} else {
				stack_buf[i] = '0' + DO_OFF;
			}
		} else {
			return -EINVAL;
		}
	}

	ret = copy_to_user((void*)buf, (void*)stack_buf, sizeof(stack_buf));
	if(ret < 0) {
		printk( KERN_ERR "Moxa do error! paramter should be %lu \
digits, like \"%s\" \n", sizeof(stack_buf), DO_PATTERN);
		return -ENOMEM;
	}

	return sizeof(stack_buf);
}

long do_ioctl (struct file *filp, unsigned int cmd, unsigned long arg) {
	struct dio_set_struct   set;
	int i = 0;
	int do_state = 0;

	/* check data */
	if(copy_from_user(&set, (struct dio_set_struct *)arg, sizeof(struct dio_set_struct)) == sizeof(struct dio_set_struct))
		return -EFAULT;

	switch (cmd) {
		case IOCTL_SET_DOUT :
			dp("set dio:%x\n",set.io_number);
			if (set.io_number < 0 || set.io_number >= DO_NUM)
				return -EINVAL;

			if (set.mode_data != DIO_HIGH && set.mode_data != DIO_LOW)
				return -EINVAL;

			do_set(set.io_number, set.mode_data);
			break;
		case IOCTL_GET_DOUT :
			dp("get do io_number:%x\n",set.io_number);

			if (set.io_number == ALL_PORT) {
				set.mode_data = 0;
				for( i=0; i< DO_NUM; i++) {
					do_get(set.io_number, &do_state);
					set.mode_data |= (do_state<<i);
				}
				dp(KERN_DEBUG "all port: %x", set.mode_data);
			} else { 
				if(set.io_number < 0 || set.io_number >= DO_NUM)
					return -EINVAL;

				if(do_get(set.io_number, &set.mode_data) < 0)
					printk(KERN_ALERT "di_get(%d, %d) fail\n", set.io_number, set.mode_data);
			}

			if(copy_to_user((struct dio_set_struct *)arg, &set, sizeof(struct dio_set_struct)) == sizeof(struct dio_set_struct))
				return -EFAULT;

			dp("mode_data: %x\n", (unsigned int)set.mode_data);
			break;
		default :
			printk(KERN_DEBUG "ioctl:error\n");
			return -EINVAL;
	}

	return 0; /* if switch ok */
}

/* define which file operations are supported */
struct file_operations do_fops = {
	.owner		= THIS_MODULE,
	.write		= do_write,
	.read		= do_read,
	.unlocked_ioctl	= do_ioctl,
	.open		= do_open,
	.release	= do_release,
};

/* register as misc driver */
static struct miscdevice do_miscdev = {
	.minor = MOXA_DO_MINOR,
	.name = DO_NAME,
	.fops = &do_fops,
};

/*
 * module: USB power section
 */
static int usbpwr_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int usbpwr_release(struct inode *inode, struct file *file)
{
	return 0;
}

/* Write function
 * Note: use echo 111 > /dev/usbpwr
 * The order is [usbpwr_rear, usbpwr_wafer, usbpwr_front\n]
 */
ssize_t usbpwr_write(struct file *filp, const char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	char stack_buf[USBPWR_NUM];

	/* check input value */
	if(count != (sizeof(stack_buf)+1)) {
		printk( KERN_ERR "Moxa do error! paramter should be %lu \
digits, like \"%s\" \n", sizeof(stack_buf), USBPWR_PATTERN);
		return -EINVAL;
	}

	if(copy_from_user(stack_buf, buf, count-1)) {
		return 0;
	}

	for (i = 0; i < sizeof(stack_buf); i++) {
		if (stack_buf[i] == '1') {
			do_set(i, USBPWR_ON);
		} else if (stack_buf[i] == '0') {
			do_set(i, USBPWR_OFF);
		} else {
			printk("USB power: error, you input is %s", stack_buf);
			break;
		}
	}

	return count;
}

ssize_t usbpwr_read(struct file *filp, char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	int ret;
	char stack_buf[USBPWR_NUM];

	for (i = 0; (i < sizeof(stack_buf)) && (i < count); i++) {
		if (!do_get(i, &ret)) {
			if (ret) {
				stack_buf[i] = '0' + USBPWR_ON;
			} else {
				stack_buf[i] = '0' + USBPWR_OFF;
			}
		} else {
			return -EINVAL;
		}
	}

	ret = copy_to_user((void*)buf, (void*)stack_buf, sizeof(stack_buf));
	if(ret < 0) {
		printk(KERN_ERR "Moxa USB power error! paramter should be %lu \
digits, like \"%s\" \n", sizeof(stack_buf), USBPWR_PATTERN);
		return -ENOMEM;
	}

	return sizeof(stack_buf);
}

/* define which file operations are supported */
struct file_operations usbpwr_fops = {
	.owner		= THIS_MODULE,
	.write		= usbpwr_write,
	.read		= usbpwr_read,
	.open		= usbpwr_open,
	.release	= usbpwr_release,
};

/* register as misc driver */
static struct miscdevice usbpwr_miscdev = {
	.minor = MOXA_USBPWR_MINOR,
	.name = USBPWR_NAME,
	.fops = &usbpwr_fops,
};

/*
 * module: USB detection section
 */
static int usbdet_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int usbdet_release(struct inode *inode, struct file *file)
{
	return 0;
}

/* Write function
 * Note: use echo 111 > /dev/usbdet
 * The order is [usbdet_rear, usbdet_wafer, usbdet_front\n]
 */
ssize_t usbdet_write(struct file *filp, const char __user *buf, size_t count,
	loff_t *pos)
{
	printk("<1> %s[%d]\n", __FUNCTION__, __LINE__);
	return 0;
}

ssize_t usbdet_read(struct file *filp, char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	int ret;
	char stack_buf[USBDET_NUM];

	for (i = 0; (i < sizeof(stack_buf)) && (i < count); i++) {
		if (!do_get(i, &ret)) {
			if ( ret ) {
				stack_buf[i] = '0' + USBDET_ON;
			} else {
				stack_buf[i] = '0' + USBDET_OFF;
			}
		} else {
			return -EINVAL;
		}
	}

	ret = copy_to_user((void*)buf, (void*)stack_buf, sizeof(stack_buf));
	if(ret < 0) {
		printk(KERN_ERR "Moxa do error! paramter should be %lu \
digits, like \"%s\" \n", sizeof(stack_buf), USBDET_PATTERN);
		return -ENOMEM;
	}

	return sizeof(stack_buf);
}

/* define which file operations are supported */
struct file_operations usbdet_fops = {
	.owner		= THIS_MODULE,
	.write		= usbdet_write,
	.read		= usbdet_read,
	.open		= usbdet_open,
	.release	= usbdet_release,
};

/* register as misc driver */
static struct miscdevice usbdet_miscdev = {
	.minor = MOXA_USBDET_MINOR,
	.name = USBDET_NAME,
	.fops = &usbdet_fops,
};

/*
 * module: Serial interface section
 */
static int sif_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int sif_release(struct inode *inode, struct file *file)
{
	return 0;
}

/* Write function
 * Note: use echo 11 > /dev/uart
 * The order is [uart1, uart2\n]
 */
ssize_t sif_write(struct file *filp, const char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	char stack_buf[SIF_NUM];

	/* check input value */
	if(count != (sizeof(stack_buf)+1)) {
		printk( KERN_ERR "Moxa uart error! paramter should be %lu digits\
, like \"%s\" \n", sizeof(stack_buf)/sizeof(char), SIF_PATTERN);
		return -EINVAL;
	}

	if(copy_from_user(stack_buf, buf, count-1)) {
		return 0;
	}

	for (i = 0; i < sizeof(stack_buf); i++) {
		if (stack_buf[i] >= '0' && stack_buf[i] <= '3') {
			if(0 != uartif_set( i, stack_buf[i]-'0')) {
				printk("uart: the mode is not supported, \
the input is %s", stack_buf);
				break;
			}
		} else {
			printk("uart: error, the input is %s", stack_buf);
			break;
		}
	}

	return count;
}

ssize_t sif_read(struct file *filp, char __user *buf, size_t count,
	loff_t *pos)
{
	int i;
	int ret;
	char stack_buf[SIF_NUM];

	for (i = 0; (i < sizeof(stack_buf)) && (i < count); i++) {
		if (!uartif_get(i, &ret)) {
			stack_buf[i] = '0' + ret;
			dbg_printf("%s", stackbuf[i] );
		} else {
			return -EINVAL;
		}
	}

	ret = copy_to_user((void*)buf, (void*)stack_buf, sizeof(stack_buf));
	if(ret < 0) {
		printk(KERN_ERR "Moxa uart error! paramter should be %lu digits\
, like \"%s\" \n", sizeof(stack_buf), SIF_PATTERN);
		return -ENOMEM;
	}

	return sizeof(stack_buf);
}

/* 
 * ioctl - I/O control
 */
static long sif_ioctl(struct file *file,unsigned int cmd, unsigned long arg)
{
	unsigned char port;
	unsigned char opmode;
	int val;

	switch (cmd) {
	case MOXA_SET_OP_MODE:
		if (copy_from_user(&opmode, (unsigned char *) arg, sizeof(opmode))) {
			return -EFAULT;
		}
		port = opmode >> 4 ;
		opmode = opmode & 0xf;
		if (0 != uartif_set(opmode >> 4, opmode & 0xf)) {
			printk("uart: the mode is not supported, \
the input is %x\n", opmode);
			return -EFAULT;
		}

		break;

	case MOXA_GET_OP_MODE:
		if(copy_from_user(&port, (unsigned char *)arg, sizeof(port))){
			return -EFAULT;
		}

		if(0 != uartif_get(port, &val)) {
			return -EINVAL;
		}
		opmode = val;
		if(copy_to_user((unsigned char*)arg, &opmode, sizeof(opmode))) {
			return -EFAULT;
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/* define which file operations are supported */
struct file_operations sif_fops = {
	.owner		= THIS_MODULE,
	.write		= sif_write,
	.read		= sif_read,
	.unlocked_ioctl	= sif_ioctl,
	.open		= sif_open,
	.release	= sif_release,
};

/* register as misc driver */
static struct miscdevice sif_miscdev = {
	.minor = MOXA_SIF_MINOR,
	.name = SIF_NAME,
	.fops = &sif_fops,
};


#ifdef SUPPORT_GPIOSYSFS
/*
 * Support sysfs
 */

/*
 * module: DO chip-section
 */
static int moxa_gpio_do_get(struct gpio_chip *gc, unsigned gpio_num)
{
	int val;

	if (0==do_get(gpio_num, &val)) {
		if (DO_ON==val)
			return 1;
	}
	return 0;
}

static void moxa_gpio_do_set(struct gpio_chip *gc, unsigned gpio_num, int val)
{
	if ( val )
		val = DO_ON;
	else
		val = DO_OFF;
	do_set(gpio_num, val);
}

const char *gpio_do_names[] = {
	"do1",
	"do2",
	"do3",
	"do4",
};

static struct gpio_chip moxa_gpio_do_chip = {
	.label		= "moxa-gpio-do",
	.owner		= THIS_MODULE,
	.get		= moxa_gpio_do_get,
	.set		= moxa_gpio_do_set,
	.base		= -1,
	.ngpio		= sizeof(do_pin_def)/2,
	.names		= gpio_do_names
};

/*
 * module: DI chip-section
 */
static int moxa_gpio_di_get(struct gpio_chip *gc, unsigned gpio_num)
{
	int val;

	if (0==di_get(gpio_num, &val)) {
		if (DI_ON==val)
			return 1;
	}
	return 0;
}

static void moxa_gpio_di_set(struct gpio_chip *gc, unsigned gpio_num, int val)
{
}

const char *gpio_di_names[] = {
	"di1",
	"di2",
	"di3",
	"di4",
};

static struct gpio_chip moxa_gpio_di_chip = {
	.label		= "moxa-gpio-di",
	.owner		= THIS_MODULE,
	.get		= moxa_gpio_di_get,
	.set		= moxa_gpio_di_set,
	.base		= -1,
	.ngpio		= sizeof(di_pin_def)/2,
	.names		= gpio_di_names
};

/*
 * module: Relay output chip-section
 */
static int moxa_gpio_relay_get(struct gpio_chip *gc, unsigned gpio_num)
{
	int val;

	if (0==relay_get(gpio_num, &val)) {
		if (RELAY_ON==val)
			return 1;
	}
	return 0;
}

static void moxa_gpio_relay_set(struct gpio_chip *gc, unsigned gpio_num, int val)
{
	if ( val )
		val = RELAY_ON;
	else
		val = RELAY_OFF;
	relay_set(gpio_num, val);
}

const char *gpio_relay_names[] = {
	"relay1",
};

static struct gpio_chip moxa_gpio_relay_chip = {
	.label		= "moxa-gpio-relay",
	.owner		= THIS_MODULE,
	.get		= moxa_gpio_relay_get,
	.set		= moxa_gpio_relay_set,
	.base		= -1,
	.ngpio		= sizeof(relay_pin_def)/2,
	.names		= gpio_relay_names
};

/*
 * module: USB power chip-section
 */
static int moxa_gpio_usbpwr_get(struct gpio_chip *gc, unsigned gpio_num)
{
	int val;

	if (0==usbpwr_get(gpio_num, &val)) {
		if (USBPWR_ON==val)
			return 1;
	}
	return 0;
}

static void moxa_gpio_usbpwr_set(struct gpio_chip *gc, unsigned gpio_num, int val)
{
	if ( val )
		val = USBPWR_ON;
	else
		val = USBPWR_OFF;
	usbpwr_set(gpio_num, val);
}

const char *gpio_usbpwr_names[] = {
	"usbpwr_rear",
	"usbpwr_wafer",
	"usbpwr_front",
};

static struct gpio_chip moxa_gpio_usbpwr_chip = {
	.label		= "moxa-gpio-usbpwr",
	.owner		= THIS_MODULE,
	.get		= moxa_gpio_usbpwr_get,
	.set		= moxa_gpio_usbpwr_set,
	.base		= -1,
	.ngpio		= sizeof(usbpwr_pin_def)/2,
	.names		= gpio_usbpwr_names
};

/*
 * module: USB detection chip-section
 */
static int moxa_gpio_usbdet_get(struct gpio_chip *gc, unsigned gpio_num)
{
	int val;

	if (0==usbdet_get(gpio_num, &val)) {
		if (USBDET_ON==val)
			return 1;
	}
	return 0;
}

static void moxa_gpio_usbdet_set(struct gpio_chip *gc, unsigned gpio_num, int val)
{
}

const char *gpio_usbdet_names[] = {
	"usbdet_rear",
	"usbdet_wafer",
	"usbdet_front",
};

static struct gpio_chip moxa_gpio_usbdet_chip = {
	.label		= "moxa-gpio-usbdet",
	.owner		= THIS_MODULE,
	.get		= moxa_gpio_usbdet_get,
	.set		= moxa_gpio_usbdet_set,
	.base		= -1,
	.ngpio		= sizeof(usbdet_pin_def)/2,
	.names		= gpio_usbdet_names
};

/*
 * module: Programable LED chip-section
 */
static int moxa_gpio_pled_get(struct gpio_chip *gc, unsigned gpio_num)
{
	int val;

	if (0==pled_get(gpio_num, &val)) {
		if (LED_ON==val)
			return 1;
	}
	return 0;
}

static void moxa_gpio_pled_set(struct gpio_chip *gc, unsigned gpio_num, int val)
{
	if ( val )
		val = LED_ON;
	else
		val = LED_OFF;
	pled_set(gpio_num, val);
}

const char *gpio_pled_names[] = {
	"pled1",
	"pled2",
	"pled3",
	"pled4",
	"pled5",
	"pled6",
	"pled7",
	"pled8",
};

static struct gpio_chip moxa_gpio_pled_chip = {
	.label		= "moxa-gpio-pled",
	.owner		= THIS_MODULE,
	.get		= moxa_gpio_pled_get,
	.set		= moxa_gpio_pled_set,
	.base		= -1,
	.ngpio		= sizeof(pled_pin_def)/2,
	.names		= gpio_pled_names
};

/*
 * module: Serial interface chip-section
 */
static int moxa_gpio_uartif_get(struct gpio_chip *gc, unsigned gpio_num)
{
	int val;

	if (0==uartif_get(gpio_num/3, &val)) {
		if (val==(gpio_num%3)) {
			return 1;
		}
	}

	return 0;
}

static void moxa_gpio_uartif_set(struct gpio_chip *gc, unsigned gpio_num, int val)
{
	if (val) {
		uartif_set(gpio_num/3, gpio_num%3);
	}
}

const char *gpio_uartif_names[] = {
	"uart1_232",
	"uart1_485",
	"uart1_422",
	"uart2_232",
	"uart2_485",
	"uart2_422",
};

static struct gpio_chip moxa_gpio_uartif_chip = {
	.label		= "moxa-gpio-uartif",
	.owner		= THIS_MODULE,
	.get		= moxa_gpio_uartif_get,
	.set		= moxa_gpio_uartif_set,
	.base		= -1,
	.ngpio		= sizeof(gpio_uartif_names)/sizeof(char*),
	.names		= gpio_uartif_names
};
#endif /* SUPPORT_GPIOSYSFS */


/* initialize module (and interrupt) */
static int __init moxa_misc_init_module (void)
{
	int retval=0;

	printk("initializing MOXA misc. device module\n");

	// register misc driver
	retval = misc_register(&pled_miscdev);
	if(retval != 0) {
		printk("Moxa pled driver: Register misc fail !\n");
		goto moxa_misc_init_module_err1;
	}

	retval = misc_register(&relay_miscdev);
	if(retval != 0) {
		printk("Moxa relay driver: Register misc fail !\n");
		goto moxa_misc_init_module_err2;
	}

	retval = misc_register(&do_miscdev);
	if(retval != 0) {
		printk("Moxa DO driver: Register misc fail !\n");
		goto moxa_misc_init_module_err3;
	}

	retval = misc_register(&di_miscdev);
	if(retval != 0) {
		printk("Moxa DI driver: Register misc fail !\n");
		goto moxa_misc_init_module_err4;
	}

	retval = misc_register(&sif_miscdev);
	if(retval != 0) {
		printk("Moxa uart interface driver: Register misc fail !\n");
		goto moxa_misc_init_module_err5;
	}

	retval = misc_register(&usbpwr_miscdev);
	if(retval != 0) {
		printk("Moxa USB power driver: Register misc fail !\n");
		goto moxa_misc_init_module_err6;
	}

	retval = misc_register(&usbdet_miscdev);
	if(retval != 0) {
		printk("Moxa USB detection driver: Register misc fail !\n");
		goto moxa_misc_init_module_err7;
	}

	retval = gpio_init();
	if(retval != 0) {
		printk("Moxa GPIO init driver: gpio_init() fail !\n");
		goto moxa_misc_init_module_err8;
	}

#ifdef SUPPORT_GPIOSYSFS
	retval = gpiochip_add(&moxa_gpio_relay_chip);
	if(retval < 0) {
		printk("Moxa realy driver: gpiochip_add(&moxa_gpio_relay_chip) fail !\n");
		goto moxa_misc_init_module_err9;
	}

	retval = gpiochip_add(&moxa_gpio_pled_chip);
	if(retval < 0) {
		printk("Moxa pled driver: gpiochip_add(&moxa_gpio_pled_chip) fail !\n");
		goto moxa_misc_init_module_err10;
	}

	retval = gpiochip_add(&moxa_gpio_uartif_chip);
	if(retval < 0) {
		printk("Moxa uart interface driver: gpiochip_add(&moxa_gpio_uartif_chip) fail !\n");
		goto moxa_misc_init_module_err11;
	}

	retval = gpiochip_add(&moxa_gpio_do_chip);
	if(retval < 0) {
		printk("Moxa DO driver: gpiochip_add(&moxa_gpio_do_chip) fail !\n");
		goto moxa_misc_init_module_err12;
	}

	retval = gpiochip_add(&moxa_gpio_di_chip);
	if(retval < 0) {
		printk("Moxa DI driver: gpiochip_add(&moxa_gpio_di_chip) fail !\n");
		goto moxa_misc_init_module_err13;
	}

	retval = gpiochip_add(&moxa_gpio_usbpwr_chip);
	if(retval < 0) {
		printk("Moxa USB power driver: gpiochip_add(&moxa_gpio_usbpwr_chip) fail !\n");
		goto moxa_misc_init_module_err14;
	}

	retval = gpiochip_add(&moxa_gpio_usbdet_chip);
	if(retval < 0) {
		printk("Moxa USB detection driver: gpiochip_add(&moxa_gpio_usbdet_chip) fail !\n");
		goto moxa_misc_init_module_err15;
	}
#endif /* SUPPORT_GPIOSYSFS */

	return 0;
#ifdef CONFIG_GPIO_SYSFS
moxa_misc_init_module_err15:
	gpiochip_remove(&moxa_gpio_usbpwr_chip);
moxa_misc_init_module_err14:
	gpiochip_remove(&moxa_gpio_di_chip);
moxa_misc_init_module_err13:
	gpiochip_remove(&moxa_gpio_do_chip);
moxa_misc_init_module_err12:
	gpiochip_remove(&moxa_gpio_uartif_chip);
moxa_misc_init_module_err11:
	gpiochip_remove(&moxa_gpio_pled_chip);
moxa_misc_init_module_err10:
	gpiochip_remove(&moxa_gpio_relay_chip);
#endif
moxa_misc_init_module_err9:
	gpio_exit();
moxa_misc_init_module_err8:
	misc_deregister(&usbdet_miscdev);
moxa_misc_init_module_err7:
	misc_deregister(&usbpwr_miscdev);
moxa_misc_init_module_err6:
	misc_deregister(&sif_miscdev);
moxa_misc_init_module_err5:
	misc_deregister(&di_miscdev);
moxa_misc_init_module_err4:
	misc_deregister(&do_miscdev);
moxa_misc_init_module_err3:
	misc_deregister(&relay_miscdev);
moxa_misc_init_module_err2:
	misc_deregister(&pled_miscdev);
moxa_misc_init_module_err1:
	return retval;
}

/* close and cleanup module */
static void __exit moxa_misc_cleanup_module (void)
{
	printk("cleaning up module\n");
	misc_deregister(&pled_miscdev);
	misc_deregister(&relay_miscdev);
	misc_deregister(&sif_miscdev);
	misc_deregister(&di_miscdev);
	misc_deregister(&do_miscdev);
	misc_deregister(&usbpwr_miscdev);
	misc_deregister(&usbdet_miscdev);
#ifdef SUPPORT_GPIOSYSFS
	gpiochip_remove(&moxa_gpio_pled_chip);
	gpiochip_remove(&moxa_gpio_relay_chip);
	gpiochip_remove(&moxa_gpio_uartif_chip);
	gpiochip_remove(&moxa_gpio_di_chip);
	gpiochip_remove(&moxa_gpio_do_chip);
	gpiochip_remove(&moxa_gpio_usbpwr_chip);
	gpiochip_remove(&moxa_gpio_usbdet_chip);
#endif /* SUPPORT_GPIOSYSFS */
	gpio_exit();
}

module_init(moxa_misc_init_module);
module_exit(moxa_misc_cleanup_module);
MODULE_AUTHOR("ElvsiCW.Yao@moxa.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MOXA misc. device driver");
