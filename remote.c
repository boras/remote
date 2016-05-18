#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/kfifo.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <linux/timer.h>
#include <linux/reboot.h>

#include <asm/mach-au1x00/au1000.h>
#include <asm/mach-db1x00/nuxitron.h>

//#define REMOTE_DEBUG
//#define CONFIG_EEPROM_TABLE_1
//#define CONFIG_EEPROM_TABLE_2
#define CONFIG_EEPROM_TABLE_3
//#define CONFIG_USE_FIFO
#define CONFIG_CPU_FLOWCHART

#include "remote.h"

extern struct i2c_adapter pb1550_board_adapter;

/* IRQ line coming from MCU */
unsigned int mcu_irq_line = AU1XXX_MCU_IRQ;
/* Debugging variables */
static unsigned int tv1, tv2;
static int mcu_irq_counter = 0;
/* Enables and disables run-time debugging */
static char remote_debug;

/* State variables */
#ifdef CONFIG_CPU_FLOWCHART
static int pc_tv_mode = TV_MODE;
static int panel_on_off = PANEL_OFF;
#endif

static int major = REMOTE_MAJOR;
static int minor = REMOTE_MINOR;
static int mode = REMOTE_MODE;
module_param(major, int, S_IRUGO);
module_param(minor, int, S_IRUGO);
module_param(mode, int, S_IRUGO);



/* Function definitions */

/* I2C bus access routines */
/**
 *	read_i2c_current - starts the I2C bus read from the current offset
 *	@dev_addr: I2C device address
 *	@buf: Buffer to place data
 *	@len: Buffer length
 *
 *	Called after the semaphore was taken.
 *
 *	This function is called from process context.
 */
static int read_i2c_current(u16 dev_addr, u8 *buf, u16 len)
{
	struct i2c_adapter *i2c = &pb1550_board_adapter;
	struct i2c_msg msg;
	int ret;

	FUNCTION;

	msg.addr = dev_addr / 2;
	msg.flags = I2C_M_RD;
	msg.buf = buf;
	msg.len = len;

	ret = i2c_transfer(i2c, &msg, 1);

	return ret;
}

/**
 *	read_i2c_random - starts the I2C bus read from the provided offset
 *	@dev_addr: I2C device address
 *	@reg: Device offset
 *	@buf: Buffer to place data
 *	@len: Buffer length
 *
 *	Called after the semaphore was taken.
 *
 *	This function is called from process context.
 */
static int read_i2c_random(u16 dev_addr, u8 *reg, u8 *buf, u16 len)
{
	struct i2c_adapter *i2c = &pb1550_board_adapter;
	struct i2c_msg msg[] = {{.addr = dev_addr / 2, .flags = 0, .buf = reg, .len = 1},
						{.addr = dev_addr / 2, .flags = I2C_M_RD, .buf = buf, .len = len}};
	int ret;

	FUNCTION;

	ret = i2c_transfer(i2c, msg, 2);

	return ret;
}

/**
 *	write_i2c - writes to the I2C bus
 *	@dev_addr: I2C device address
 *	@buf: Buffer to transmit
 *	@len: Buffer length
 *
 *	Offset is embedded within the device(current) or we provide
 *	it in the first byte(random).
 *
 *	Called after the semaphore was taken.
 *
 *	This function is called from process context.
 */
static int write_i2c(u16 dev_addr, u8 *buf, u16 len)
{
	struct i2c_adapter *i2c = &pb1550_board_adapter;
	struct i2c_msg msg;
	int ret;

	FUNCTION;

	msg.addr = dev_addr / 2;
	msg.flags = 0;
	msg.buf = buf;
	msg.len = len;

	ret = i2c_transfer(i2c, &msg, 1);

	return ret;
}

/* I2C chip accessors. We didnt optimize out them because of the little usage expectancy */
/**
 *	device_read_i2c_current - reads the I2C device from the current offset
 *	@addr: I2C device address
 *	@iop: User space parameters
 *
 *	Called after the semaphore was taken.
 *
 *	This function is called from process context.
 */
static int device_read_i2c_current(u16 addr, struct io_entry *iop)
{
	char *buffer;
	int ret = 0;

	FUNCTION;

	buffer = kmalloc(iop->len, GFP_KERNEL);
	if (!buffer) {
		printk(KERN_ERR "remote: cannot allocate memory\n");
		return -ENOMEM;
	}

	ret = read_i2c_current(addr, buffer, iop->len);
	//printk(KERN_INFO "remote: read_i2c_current : ret = %d\n", ret);
	if (ret < 0) {
		printk(KERN_ERR "remote: can't read I2C data from the device\n");
		goto fail;
	}
	//printk(KERN_INFO "remote: iop->len = %d\n", iop->len);
	//dump_data(buffer, iop->len);

	ret = copy_to_user(iop->buf, buffer, iop->len);
	if (ret) {
		printk(KERN_ERR "remote: cannot copy to user space\n");
		ret = -EFAULT;
	}

fail:
	kfree(buffer);

	return ret;
}

/**
 *	device_read_i2c_random - reads the I2C device from the provided offset
 *	@addr: I2C device address
 *	@iop: User space parameters
 *
 *	Called after the semaphore was taken.
 *
 *	This function is called from process context.
 */
static int device_read_i2c_random(u16 addr, u8 reg, struct io_entry *iop)
{
	char *buffer;
	int ret = 0;

	FUNCTION;

	buffer = kmalloc(iop->len, GFP_KERNEL);
	if (!buffer) {
		printk(KERN_ERR "remote: cannot allocate memory\n");
		return -ENOMEM;
	}

	ret = read_i2c_random(addr, &reg, buffer, iop->len);
	//printk(KERN_INFO "remote: read_i2c_random : ret = %d\n", ret);
	if (ret < 0) {
		printk(KERN_ERR "remote: can't read I2C data from the device\n");
		goto fail;
	}
	//printk(KERN_INFO "remote: iop->len = %d\n", iop->len);
	//dump_data(buffer, iop->len);

	ret = copy_to_user(iop->buf, buffer, iop->len);
	if (ret) {
		printk(KERN_ERR "remote: cannot copy to user space\n");
		ret = -EFAULT;
	}

fail:
	kfree(buffer);

	return ret;
}

/**
 *	device_write_i2c_current - writes to the I2C device from the current offset
 *	@addr: I2C device address
 *	@iop: User space parameters
 *
 *	Offset is embedded within the device(current).
 *
 *	Called after the semaphore was taken.
 *
 *	This function is called from process context.
 */
static int device_write_i2c_current(u16 addr, struct io_entry *iop)
{
	char *buffer;
	int ret = 0;

	FUNCTION;

	buffer = kmalloc(iop->len, GFP_KERNEL);
	if (!buffer) {
		printk(KERN_ERR "remote: cannot allocate memory\n");
		return -ENOMEM;
	}

	ret = copy_from_user(buffer, iop->buf, iop->len);
	if (ret) {
		printk(KERN_ERR "remote: cannot copy from user space\n");
		ret = -EFAULT;
		goto fail;
	}
	//dump_data(buffer, iop->len);

	ret = write_i2c(addr, buffer, iop->len);
	//printk(KERN_INFO "remote: write_i2c : ret = %d\n", ret);
	if (ret < 0)
		printk(KERN_ERR "remote: can't write I2C data to the device\n");

fail:
	kfree(buffer);

	return ret;
}

/**
 *	device_write_i2c_random - writes to the I2C device from the provided offset
 *	@addr: I2C device address
 *	@iop: User space parameters
 *
 *	Offset is provided explicitly by the user space.
 *
 *	Called after the semaphore was taken.
 *
 *	This function is called from process context.
 */
static int device_write_i2c_random(u16 addr, u8 reg, struct io_entry *iop)
{
	char *buffer;
	int ret = 0;
	short len = iop->len + 1;

	FUNCTION;

	buffer = kmalloc(len, GFP_KERNEL);
	if (!buffer) {
		printk(KERN_ERR "remote: cannot allocate memory\n");
		return -ENOMEM;
	}

	ret = copy_from_user(&buffer[1], iop->buf, iop->len);
	if (ret) {
		printk(KERN_ERR "remote: cannot copy from user space\n");
		ret = -EFAULT;
		goto fail;
	}
	//dump_data(buffer, iop->len);

	buffer[0] = reg;
	ret = write_i2c(addr, buffer, len);
	//printk(KERN_INFO "remote: write_i2c : ret = %d\n", ret);
	if (ret < 0)
		printk(KERN_ERR "remote: can't write I2C data to the device\n");

fail:
	kfree(buffer);

	return ret;
}

/* Driver methods */
/* The ioctl() implementation */
static int remote_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct remote_device *rc =  (struct remote_device *)filp->private_data;
	int err = 0;

	FUNCTION;

	/* Extract the type and the number */
	if (_IOC_TYPE(cmd) != REMOTE_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) > REMOTE_MAXNR)
		return -ENOTTY;
	/* VERIFY_WRITE catches R/W transfers */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err)
		return -EFAULT;
	if (down_interruptible(&rc->sem))
		return -ERESTARTSYS;

	err = -EPERM;

	switch (cmd) {
#if 0
	case REMOTE_CONFDAT_FROM_EEPROM_CMD:
		if (!capable(CAP_SYS_ADMIN))
			break;
		err = read_config_data(EEPROM_I2C_ADDRESS, rc);
		if (err > 0) {
			if (!__copy_to_user((struct eeprom_table __user *)arg, &e2prom_tab, sizeof(struct eeprom_table)))
				err = sizeof(struct eeprom_table);
			else
				err = -EFAULT;
		}
		//dump_config_data();
		break;
#endif
	case REMOTE_READ_CONFIG_DATA_CMD:
		if (!capable(CAP_SYS_ADMIN))
			break;
		//dump_config_data();
		if (!__copy_to_user((struct eeprom_table __user *)arg, &e2prom_tab, sizeof(struct eeprom_table)))
			err = sizeof(struct eeprom_table);
		else
			err = -EFAULT;
		break;
	case REMOTE_WELCOME_CMD:
		if (!capable(CAP_SYS_ADMIN))
			break;
		if (!mode)
			if ((err = send_cmd_to_mcu(ON_PROCEDURE)) > 0) {
				panel_on_off = PANEL_ON;
				pc_tv_mode = PC_MODE;
			}
		break;
	case REMOTE_START_CMD:
		if (!capable(CAP_SYS_ADMIN))
			break;
		if (!(rc->flag & CFG_STATE_SET)) {
			printk(KERN_ERR "remote: EEPROM contains stale data!\n");
			err = -EINVAL;
			break;
		}
		err = send_cmd_to_mcu(START);
		if (err < 0)
			break;
		rc->flag |= START_STATE_SET;		/* Start the PC */
		break;
	case REMOTE_STOP_CMD:
		if (!capable(CAP_SYS_ADMIN))
			break;
		if (!(rc->flag & START_STATE_SET)) {
			printk(KERN_ERR "remote: for this to work START bit have to be set\n");
			err = -EINVAL;	/* START bit is not set; we cant go into STOP mode */
			break;
		}
		/* System works as expected: valid and start bit is set */
		err = send_cmd_to_mcu(STOP);
		if (err < 0)
				break;
		rc->flag &= START_STATE_CLEAR;		/* Stop the PC */
		if (!mode) {
			panel_on_off = PANEL_OFF;
			pc_tv_mode = TV_MODE;
		}
		break;
	case REMOTE_CFG_INVALID_CMD:
		if (!capable(CAP_SYS_ADMIN))
			break;
		{
			char buf[SIGNATURE_SIZE + 1] = {0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

			if (rc->flag != PC_ENABLE) {
				printk(KERN_ERR "remote:  for this to work START and CFG_VALID bits have to be set\n");
				err = -EINVAL;	/* START and CFG bits are not set; we cant go into INVALID mode */
				break;
			}
			err = write_i2c(EEPROM_I2C_ADDRESS, buf, sizeof(buf));
			//printk(KERN_INFO "remote: write_i2c : ret = %d\n", ret);
			if (err < 0) {
				printk(KERN_ERR "remote: can't write I2C data to the EEPROM\n");
				break;
			}
			/* Stop the system */
			err = send_cmd_to_mcu(CONFIG_DATA_INVALID);	/* Tell MCU EEPROM is stale */
			if (err < 0)
				break;
			rc->flag = 0;		/* Stop the PC. config_data is invalid */
		}
		break;
	case REMOTE_CFG_VALID_CMD:
		if (!capable(CAP_SYS_ADMIN))
			break;
		/* VALID is now unused */
		if (rc->flag) {
			printk(KERN_ERR "remote: for this to work CFG_INVALID have to be set\n");
			err = -EINVAL;	/* CFG_INVALID is not set; we cant go into VALID mode */
			break;
		}
		err = send_cmd_to_mcu(CONFIG_DATA_VALID);		/* Tell MCU EEPROM is valid */
		if (err < 0)
				break;
		rc->flag |= CFG_STATE_SET;		/* config_data is valid */
		break;
	case REMOTE_READ_MCU_CMD:
		if (!capable(CAP_SYS_ADMIN))
			break;
		{
			struct io_entry io;

			if (__copy_from_user(&io, (struct io_entry __user *)arg, sizeof(struct io_entry))) {
				err = -EFAULT;
				break;
			}
			if (io.len > MCU_READ_SIZE_MAX || io.len == 0) {
				err = -EINVAL;
				break;
			}

			err = device_read_i2c_current(MCU_I2C_ADDRESS, &io);
		}
		break;
	case REMOTE_WRITE_MCU_CMD:
		if (!capable(CAP_SYS_ADMIN))
			break;
		{
			struct io_entry io;

			if (__copy_from_user(&io, (struct io_entry __user *)arg, sizeof(struct io_entry))) {
				err = -EFAULT;
				break;
			}
			if (io.len > MCU_WRITE_SIZE_MAX || io.len == 0) {
				err = -EINVAL;
				break;
			}

			err = device_write_i2c_current(MCU_I2C_ADDRESS, &io);
		}
		break;
	case REMOTE_READ_EEPROM_CMD:
		if (!capable(CAP_SYS_ADMIN))
			break;
		{
			struct io_entry io;
			u16 dev_addr = EEPROM_I2C_ADDRESS;
			u8 data_addr;

			if (__copy_from_user(&io, (struct io_entry __user *)arg, sizeof(struct io_entry))) {
				err = -EFAULT;
				break;
			}
			if (io.len > EEPROM_SIZE || io.len == 0) {
				err = -EINVAL;
				break;
			}
			dev_addr |= (io.addr & 0x0100) ? 0x2 : 0;
			data_addr = (u8)(io.addr & 0x00FF);

			err = device_read_i2c_random(dev_addr, data_addr, &io);
		}
		break;
	case REMOTE_WRITE_EEPROM_CMD:
		if (!capable(CAP_SYS_ADMIN))
			break;
		{
			struct io_entry io;
			u16 dev_addr = EEPROM_I2C_ADDRESS;
			u8 data_addr;

			if (__copy_from_user(&io, (struct io_entry __user *)arg, sizeof(struct io_entry))) {
				err = -EFAULT;
				break;
			}
			if (io.len > EEPROM_PAGE_WRITE_SIZE || io.len == 0 || io.addr < sizeof(struct eeprom_table)) {
				err = -EINVAL;
				break;
			}

			dev_addr |= (io.addr & 0x0100) ? 0x2 : 0;
			data_addr = (u8)(io.addr & 0x00FF);

			err = device_write_i2c_random(dev_addr, data_addr, &io);
		}
		break;
	default:
                err = -ENOTTY;
	}

	up(&rc->sem);

	return err;
}

/* Called after the semaphore was taken */
static void dump_data(u8 *buf, u16 count)
{
	int i;

	for (i = 0; i < count; i++)
		printk(KERN_INFO "%x ", buf[i]);
	printk(KERN_INFO "\n");
}

/* Called after the semaphore was taken */
static void dump_config_data(void)
{
	int i;

	for (i = 0; i < SIGNATURE_SIZE; i++)
		printk(KERN_INFO "remote: e2prom.reserved[%d] = %x\n", i, e2prom_tab.reserved[i]);
	for (i = 0; i < PANEL_ENTRY_SIZE; i++) {
#if defined(CONFIG_EEPROM_TABLE_1) || defined(CONFIG_EEPROM_TABLE_2)
		printk(KERN_INFO "remote: e2prom_tab.panel_tab[%d].panel_key = %x\n", i, e2prom_tab.panel_tab[i].panel_key);
		printk(KERN_INFO "remote: e2prom_tab.panel_tab[%d].wait_time = %x\n", i, e2prom_tab.panel_tab[i].wait_time);
#endif
#ifdef CONFIG_EEPROM_TABLE_3
		printk(KERN_INFO "remote: e2prom_tab.panel_irda_key[%d] = %x\n", i, e2prom_tab.panel_irda_key[i]);
		printk(KERN_INFO "remote: e2prom_tab.panel_wait_time[%d] = %x\n", i, e2prom_tab.panel_wait_time[i]);
#endif
	}
	for (i = 0; i < ONE_TO_MUL_MAPPING_SIZE; i++)
		printk(KERN_INFO "remote: e2prom.procedure[%d] = %x\n", i, e2prom_tab.procedure[i]);
	for (i = 0; i < IRDA_CODING_SIZE; i++)
		printk(KERN_INFO "remote: e2prom.irda_coding[%d] = %x\n", i, e2prom_tab.irda_coding[i]);
	for (i = 0; i < IRDA_KEY_SIZE; i++)
		printk(KERN_INFO "remote: e2prom.irda_tab[%d] = %x\n", i, e2prom_tab.irda_tab[i]);
}

static int remote_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	struct remote_device *rc = (struct remote_device *)data;
	int len = 0;

	if (down_interruptible(&rc->sem))
		return -ERESTARTSYS;

	len += sprintf(page, "%d\n", remote_debug);

	up(&rc->sem);

	return len;
}

static int remote_proc_write(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	struct remote_device *rc = (struct remote_device *)data;
	char debug;

	if (down_interruptible(&rc->sem))
		return -ERESTARTSYS;

	if (get_user(debug, buffer)) {
		printk(KERN_ERR "remote: cannot copy from user space\n");
		return -EFAULT;
	}

	if (debug != '0' && debug != '1')
		goto fail;

	remote_debug = debug - '0';

fail:
	up(&rc->sem);

	return count;
}

/* Data management: read and write */
/**	If config_table is not valid, returns 0.
*
*	If there is no data, blocks the calling process.
*
*	Supports multiple reader.
*
*/
static ssize_t remote_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct remote_device *rc = (struct remote_device *)filp->private_data;
#ifdef CONFIG_USE_FIFO
	__u32 irda_data;
#endif

	FUNCTION;

	if (filp->f_flags & O_NONBLOCK)
		return -EAGAIN;
	if (count != sizeof(__u32))
		return -EINVAL;

	if (down_interruptible(&rc->sem))
		return -ERESTARTSYS;

	/* Test if config_data is valid or written */
	if (rc->flag != PC_ENABLE) {
		up(&rc->sem);
		return 0;
	}

#ifdef CONFIG_USE_FIFO
	up(&rc->sem);

	if (wait_event_interruptible(rc->keyq, kfifo_get(rc->ckfifo, (unsigned char *)&irda_data, sizeof(irda_data)) != 0))
		return -ERESTARTSYS;	/* signal: tell the fs layer to handle it */

	//printk(KERN_INFO "remote: irda_data = %X\n", irda_data);

	if (put_user(irda_data, (__u32 __user *)buf)) {
		printk(KERN_ERR "remote: cannot copy to user space\n");
		return -EFAULT;
	}
#else
	while (rc->irda_data == 0) {
		up(&rc->sem);
		if (wait_event_interruptible(rc->keyq, rc->irda_data != 0))
			return -ERESTARTSYS;
		if (down_interruptible(&rc->sem))
			return -ERESTARTSYS;
	}

	//printk(KERN_INFO "remote: remote.irda_data = %X\n", rc->irda_data);

	if (put_user(rc->irda_data, (__u32 __user *)buf)) {
		printk(KERN_ERR "remote: cannot copy to user space\n");
		return -EFAULT;
	}
	rc->irda_data = 0;

	up(&rc->sem);
#endif /* CONFIG_USE_FIFO */

	//tv2 = read_c0_count();
	//printk(KERN_INFO "timeval = %d\n", tv2 - tv1);
	//printk(KERN_INFO "tv1 = %u\n", tv1);
	//printk(KERN_INFO "tv2 = %u\n", tv2);

	return count;
}

/**
 *	read_config_data - reads config_data from the EEPROM
 *	@addr: EEPROM address
 *	@rc: remote device struct
 *
 *	This is triggered at the module_init time or in the remote_ioctl routine
 *	after the newly created config_data is written successfully.
 *
 *	Called only module_init time.
 *
 *	This function is called from process context.
 */
static int read_config_data(u16 dev_addr, struct remote_device *rc)
{
	int table_size = sizeof(struct eeprom_table);
	u8 data_addr = 0;
	int ret;
	char signature[] = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};

	FUNCTION;

	//tv1 = read_c0_count();
	ret = read_i2c_random(dev_addr, &data_addr, (u8 *)&e2prom_tab, SIGNATURE_SIZE);
	//printk(KERN_INFO "remote: read_i2c_random : ret = %d\n", ret);
	if (ret < 0) {
		printk(KERN_ERR "remote: can't read I2C data from the EEPROM\n");
		return ret;
	}
	ret = memcmp(signature, (u8 *)&e2prom_tab, SIGNATURE_SIZE);
	if (ret) {
		printk(KERN_ERR "remote: unknown config data or first working!\n");
		return 0;
	}

	data_addr = SIGNATURE_SIZE;
	ret = read_i2c_random(dev_addr, &data_addr, ((u8 *)&e2prom_tab + SIGNATURE_SIZE), table_size - SIGNATURE_SIZE);
	//printk(KERN_INFO "remote: read_i2c_random : ret = %d\n", ret);
	if (ret < 0) {
		printk(KERN_ERR "remote: can't read I2C data from the EEPROM\n");
		return ret;
	}
	//tv2 = read_c0_count();
	//dump_config_data();

	//printk(KERN_INFO "tv1 = %u\n", tv1);
	//printk(KERN_INFO "tv2 = %u\n", tv2);

	/* Start the system */
	send_cmd_to_mcu(START);	/* Start the MCU */
	rc->flag |= CFG_STATE_SET;
	rc->timer.expires = jiffies + msecs_to_jiffies(START_SEEN_TIMEOUT);
	add_timer(&rc->timer);

	return ret;
}

/**
 *	remote_timer_fn - timer routine
 *	@data: remote device struct
 *
 *	Because i2c_core system takes a semaphore we cant read
 *	it from softirq context so we schedule a bottom halve.
 *	Also i2c_transfer takes a long time for softirq context.
 *
 *	This function is called from softirq context.(TIMER_SOFTIRQ)
 */
static void remote_timer_fn(unsigned long data)
{
	struct remote_device *rc = (struct remote_device *)data;

	queue_work(remote_wq, &rc->remote_work);
}

/**
 *	write_config_data - writes config_data to the EEPROM
 *	@addr: EEPROM address
 *	@rc: remote device struct
 *
 *	Triggered from the driver write method.
 *
 *	Called after the semaphore was taken.
 *
 *	This function is called from process context.
 */
static int write_config_data(u16 dev_addr, struct remote_device *rc)
{
	int table_size = sizeof(struct eeprom_table);
	int i, ret;
	int len = EEPROM_PAGE_TRANSACTION_SIZE;
	char buf[EEPROM_PAGE_TRANSACTION_SIZE];

	FUNCTION;

	if (!(rc->flag  == 0) && !(rc->flag == PC_ENABLE)) {
		printk(KERN_ERR "remote: for this to work we have to be in invalid or normal mode\n");
		return -EINVAL;
	}
	/* Stop the system */
	rc->flag = 0;	/* Stop the PC. config_data is invalid */
	send_cmd_to_mcu(CONFIG_DATA_INVALID);	/* Tell MCU EEPROM is stale */
	if (!mode) {
		panel_on_off = PANEL_OFF;
		pc_tv_mode = TV_MODE;
	}

	//tv1 = read_c0_count();
	for (i = 0; i < table_size; i += EEPROM_PAGE_WRITE_SIZE) {
		/* 512 byte == 4Kbit EEPROM needs 9 addr pin */
		if (i == 0x100)
			dev_addr |= 2;
		/* The place where to write */
		buf[0] = (u8)(i & 0x00FF);
		/* The last slice of the table_size is calculated */
		if ((i + EEPROM_PAGE_WRITE_SIZE) > table_size)
			len = (table_size % EEPROM_PAGE_WRITE_SIZE) + 1;
		memcpy(&buf[1], &(((u8 *)&e2prom_tab)[i]), len);
		//dump_data(buf, len);
		ret = write_i2c(dev_addr, buf, len);
		//printk(KERN_INFO "remote: write_i2c : ret = %d\n", ret);
		if (ret < 0) {
			printk(KERN_ERR "remote: can't write I2C data to the EEPROM\n");
			return ret;
		}
		mdelay(5);
	}
	//tv2 = read_c0_count();
	//printk(KERN_INFO "tv1 = %u\n", tv1);
	//printk(KERN_INFO "tv2 = %u\n", tv2);

	/* Start the system */
	send_cmd_to_mcu(CONFIG_DATA_VALID);	/* Tell MCU EEPROM is valid */
	mdelay(500);
	send_cmd_to_mcu(START);	/* Start the MCU */
	rc->flag |= CFG_STATE_SET;
	rc->timer.expires = jiffies + msecs_to_jiffies(START_SEEN_TIMEOUT);
	add_timer(&rc->timer);

	return ret;
}

/* Triggers write_config_data */
static ssize_t remote_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct remote_device *rc =  (struct remote_device *)filp->private_data;
	int ret;
	char signature[] = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};

	FUNCTION;

	//printk(KERN_INFO "remote: sizeof(struct eeprom_table) = %d\n", sizeof(struct eeprom_table));

	if (count != sizeof(struct eeprom_table))
		return -EINVAL;

	if (down_interruptible(&rc->sem))
		return -ERESTARTSYS;

	ret = copy_from_user(&e2prom_tab, buf, count);
	if (ret) {
		printk(KERN_ERR "remote: cannot copy from user space\n");
		ret = -EFAULT;
		goto fail;
	}

	memcpy((char *)&e2prom_tab, signature, SIGNATURE_SIZE);

	ret = write_config_data(EEPROM_I2C_ADDRESS, rc);
	if (ret > 0)
		ret = count;

	//dump_config_data();

fail:
	up(&rc->sem);

	return ret;
}

/**
 *	mcu_interrupt - IRQ routine
 *	@irq: IRQ line - GPIO[15](Nuxitron RevD)
 *	@dev_id: Device identity
 *	@pt_regs: Registers when IRQ is happened
 *
 *	Because i2c_core system takes a semaphore we cant read
 *	it from IRQ context so we schedule a bottom halve.
 *	Also i2c_transfer takes a long time for IRQ context.
 *
 *	This function is called from IRQ context.
 */
static irqreturn_t mcu_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	struct remote_device *rc =  (struct remote_device *)dev_id;
	//tv1 = read_c0_count();
	mcu_irq_counter++;
	queue_work(remote_wq, &rc->remote_work);

	return IRQ_HANDLED;
}

/**
 *	send_cmd_to_mcu - send commands to the mcu
 *	@cmd: command to send
 *
 *	Called after the semaphore was taken.
 *
 *	This function is called from process context.
 */
static int send_cmd_to_mcu(u8 cmd)
{
	int ret = 0, i;

	FUNCTION;

	if (remote_debug)
		printk(KERN_INFO "remote: send_cmd_to_mcu:  cmd = 0x%X\n", cmd);

	for (i = 0; i < I2C_TRY_COUNT; i++) {
		ret = write_i2c(MCU_I2C_ADDRESS, &cmd, 1);
		//printk(KERN_INFO "remote: write_i2c : ret = %d\n", ret);
		if (ret > 0)
			return ret;
		//udelay(500);
	}
	printk(KERN_ERR "remote: can't write I2C data to the MCU\n");

	return ret;
}

EXPORT_SYMBOL(send_cmd_to_mcu);

/**
 *	remote_do_work - Main producer function
 *	@data: (struct remote_device *)
 *
 *	Reads irda_data from the MCU and operates the flowchart.
 *
 *	This function is called from process context.
 */
static void remote_do_work(void *data)
{
	struct remote_device *rc = (struct remote_device *)data;
	u8 buf[MCU_READ_SIZE];
	int ret, i;
	u8 cmd;

	FUNCTION;

	for (i = 0; i < I2C_TRY_COUNT; i++) {
		ret = read_i2c_current(MCU_I2C_ADDRESS, buf, MCU_READ_SIZE);
		//printk(KERN_INFO "remote: read_i2c_current : ret = %d\n", ret);
		if (ret > 0)
			break;
		//udelay(500);
	}
	if (i >= I2C_TRY_COUNT) {
		printk(KERN_ERR "remote: can't read I2C data from the MCU\n");
		return;
	}
	//tv2 = read_c0_count();

	down(&rc->sem);

	if (!(rc->flag & START_STATE_SET)) {
		cmd = *((u8 *)(&buf[RECV_MCU_CMD_IDX]));
		if (cmd == START) {
			printk(KERN_INFO "remote: remote_timer_fn <=> START is lost\n");
			rc->timer.expires = jiffies + msecs_to_jiffies(START_SEEN_TIMEOUT);
			add_timer(&rc->timer);
		}
		else
			rc->flag |= START_STATE_SET;
		goto out;
	}

	if (!mode )		//#ifdef CONFIG_CPU_FLOWCHART
	{
		int i;
		static int change_remote_counter, is_change_remote;
		u32 irda_data;
		u8 on_off;

		irda_data = *((u32 *)(&buf[RECV_IR_DATA_IDX]));
		on_off = *((u8 *)(&buf[RECV_ON_OFF_IDX]));

		//down(&rc->sem);

		/* MCU side: if (prev_panel_state != panel_state ){ send_to_CPU( on_off ); prev_panel_state = panel_state;} */

		if (remote_debug) {
			printk(KERN_INFO "remote: remote_do_work: on_off = %d\n", on_off);
			printk(KERN_INFO "remote: remote_do_work: irda_data = 0x%X\n", irda_data);
		}

		/* Manuel panel on/off control */
		if (on_off != panel_on_off) {
			if (on_off == PANEL_ON) {	/* Manuel panel open */
				if (send_cmd_to_mcu(TV_TO_PC_PROCEDURE) > 0) {	/* TV-to-PC */
					panel_on_off = PANEL_ON;
					pc_tv_mode = PC_MODE;
					goto out;
				}
			}
			else { 	/* Manuel panel close */
				panel_on_off = PANEL_OFF;
				pc_tv_mode = TV_MODE;
				change_remote_counter = 0;
				is_change_remote = FALSE;
				if (change_remote_counter != PASS_ENTRY_CHR_NUMBER) {
					rc->irda_data = e2prom_tab.irda_tab[KEY_ON_OFF_IDX];
					goto to_pc;
				}
				goto out;
			}
		}

		if (is_change_remote) {
			if (irda_data == e2prom_tab.irda_tab[KEY_NUMBER_9_IDX]) {
				change_remote_counter++;
				if (change_remote_counter == PASS_ENTRY_CHR_NUMBER)
					send_cmd_to_mcu(CHANGE_REMOTE);
				goto out;
			}
			else {
				change_remote_counter = 0;
				is_change_remote = FALSE;
			}
		}

		if (!pc_tv_mode) {	/* TV mode */
			if (panel_on_off == PANEL_OFF) {
				if (irda_data == e2prom_tab.irda_tab[KEY_ON_OFF_IDX])	/* Index 0 - KEY_ON_OFF_IDX */
					goto on_procedure;
				else {
					for (i = KEY_NUMBER_START_IDX; i < KEY_NUMBER_STOP_IDX; i++)
						if (irda_data == e2prom_tab.irda_tab[i])
							goto on_procedure;
				}
				goto out;
on_procedure:
				if (send_cmd_to_mcu(ON_PROCEDURE) > 0) { /* Open the panel */
					panel_on_off = PANEL_ON;
					pc_tv_mode = PC_MODE;
				}
				goto out;
			}
			else if (irda_data == e2prom_tab.irda_tab[KEY_ON_OFF_IDX]) {	/* Index 0 - KEY_ON_OFF_IDX */
				if (send_cmd_to_mcu((KEY_ON_OFF_IDX + 1) & ONE_TO_ONE_MASK) > 0) { /* Close the panel */
					panel_on_off = PANEL_OFF;
					pc_tv_mode = TV_MODE;
				}
				goto out;
			}

			else if (irda_data == e2prom_tab.irda_tab[KEY_TV_PC_IDX]) {	/* Index 1 - KEY_TV_PC_IDX */
				if (send_cmd_to_mcu(TV_TO_PC_PROCEDURE) > 0)	/* TV-to-PC */
					pc_tv_mode = PC_MODE;
				goto out;
			}

			for (i = ONE_TO_ONE_MAPPING_FIRST; i < ONE_TO_ONE_MAPPING_LAST; i++)
				if (irda_data == e2prom_tab.irda_tab[i])
					break;
			if (i < ONE_TO_ONE_MAPPING_LAST) {	/* && panel_tab.key_tab[i].wait_time != 0 -> && e2prom_tab.panel_tab.key_tab.wait_time != 0 */
				send_cmd_to_mcu((i + 1) & ONE_TO_ONE_MASK);
				if (irda_data == e2prom_tab.irda_tab[KEY_MENU_IDX])
					is_change_remote = TRUE;
				goto out;
			}
			if (i >= ONE_TO_ONE_MAPPING_LAST) {	/*  && i < IRDA_KEY_SIZE */
				//printk(KERN_INFO "remote: Not on/off!\n");
				goto out;
			}
		}
		else {	/* PC mode */
			if (irda_data == e2prom_tab.irda_tab[KEY_ON_OFF_IDX]) {	/* Close the panel - Index 0 */
				if (send_cmd_to_mcu((KEY_ON_OFF_IDX + 1) & ONE_TO_ONE_MASK) > 0) {
					panel_on_off = PANEL_OFF;
					pc_tv_mode = TV_MODE;
				}
				rc->irda_data = irda_data;
				//goto out;
			}
			else if (irda_data == e2prom_tab.irda_tab[KEY_TV_PC_IDX]) {	/* PC-to-TV - Index 1 */
				if (send_cmd_to_mcu(PC_TO_TV_PROCEDURE) > 0)
					pc_tv_mode = TV_MODE;
				rc->irda_data = irda_data;
				//goto out;
			}
			else if (irda_data == e2prom_tab.irda_tab[KEY_VOL_UP_IDX]) {	/* VOL_UP - Index 2 */
				send_cmd_to_mcu((KEY_VOL_UP_IDX + 1) & ONE_TO_ONE_MASK);
				goto out;
			}
			else if (irda_data == e2prom_tab.irda_tab[KEY_VOL_DOWN_IDX]) {	/* VOL_DOWN - Index 3 */
				send_cmd_to_mcu((KEY_VOL_DOWN_IDX + 1) & ONE_TO_ONE_MASK);
				goto out;
			}
			else if (irda_data == e2prom_tab.irda_tab[KEY_MUTE_IDX]) {	/* MUTE - Index 4 */
				send_cmd_to_mcu((KEY_MUTE_IDX + 1) & ONE_TO_ONE_MASK);
				goto out;
			}
		}
to_pc:
		{
			/* toward PC */
			//printk(KERN_INFO "remote: irda_data = %x\n", irda_data);
			if ((rc->flag == PC_ENABLE) && rc->reader_thread_count)
			#ifdef CONFIG_USE_FIFO
				__kfifo_put(rc->ckfifo, (unsigned char *)&irda_data, sizeof(irda_data));
			#else
				rc->irda_data = irda_data;
			#endif

			up(&rc->sem);

			//tv1 = read_c0_count();

			wake_up_interruptible(&rc->keyq);

			return;
		}
out:
		up(&rc->sem);
		return;
	}
	else		//#else /* CONFIG_CPU_FLOWCHART */
	{
		u32 irda_data;

		/* toward PC */
		irda_data = *((u32 *)(&buf[RECV_IR_DATA_IDX]));

		//down(&rc->sem);

		if (remote_debug)
			printk(KERN_INFO "remote: remote_do_work: irda_data = 0x%X\n", irda_data);

		if ((rc->flag == PC_ENABLE) && rc->reader_thread_count)
		#ifdef CONFIG_USE_FIFO
			__kfifo_put(rc->ckfifo, (unsigned char *)&irda_data, sizeof(irda_data));
			//__kfifo_put(rc->ckfifo, irda_key, sizeof(irda_key));
		#else
			rc->irda_data = irda_data;
		#endif

		up(&rc->sem);

		wake_up_interruptible(&rc->keyq);

		return;
	}
//#endif /* CONFIG_CPU_FLOWCHART */
}

/* flush routine - for tracking down the reader thread count */
static int remote_flush(struct file *filp)
{
	struct remote_device *rc =  (struct remote_device *)filp->private_data;

	if (filp->f_mode & FMODE_READ) {
		if (down_interruptible(&rc->sem))
			return -ERESTARTSYS;
		rc->reader_thread_count--;
		up(&rc->sem);
	}

	return 0;
}

/* poll method */
static unsigned int remote_poll(struct file *filp, poll_table *wait)
{
	struct remote_device *rc = (struct remote_device *)filp->private_data;
	unsigned int mask = 0;

	poll_wait(filp, &rc->keyq,  wait);

#ifdef CONFIG_USE_FIFO
	if (kfifo_len(rc->ckfifo) != 0)
		mask |= POLLIN | POLLRDNORM;	/* readable */
#else
	if (down_interruptible(&rc->sem))
		return -ERESTARTSYS;
	if (rc->irda_data != 0)
		mask |= POLLIN | POLLRDNORM;	/* readable */
	up(&rc->sem);
#endif

	return mask;
}

/* Open and release */
static int remote_release(struct inode *inode, struct file *filp)
{
	FUNCTION;

	return 0;
}

static int remote_open(struct inode *inode, struct file *filp)
{
	FUNCTION;

	filp->private_data = &remote;

	/* If one thread opens the device for reading then remote control irda key
	events to the PC are enabled */
	if (filp->f_mode & FMODE_READ) {
		if (down_interruptible(&remote.sem))
			return -ERESTARTSYS;
		remote.reader_thread_count++;
		up(&remote.sem);
	}

	return nonseekable_open(inode, filp);
}


static struct file_operations remote_fops = {
	.owner =  THIS_MODULE,
	.llseek		= no_llseek,
	.poll = remote_poll,
	.read = remote_read,
	.write = remote_write,
	.ioctl = remote_ioctl,
	.flush = remote_flush,
	.open = remote_open,
	.release = remote_release,
};

/* Module stuff */
static void __exit remote_cleanup_module(void)
{
	dev_t devno = MKDEV(major, minor);

	printk(KERN_INFO "remote: Stopping remote controller interface!\n");

	/* Get rid of our char_dev entry */
	cdev_del(&remote.cdev);

	/* Cancel the irq after informing the MCU */
	down(&remote.sem);
	send_cmd_to_mcu(STOP);
	up(&remote.sem);
	free_irq(mcu_irq_line, &remote);
	if (remote_debug)
		printk(KERN_INFO "remote: mcu_irq_counter = %d\n", mcu_irq_counter);

	/* Destroy worker thread */
	flush_workqueue(remote_wq);
	destroy_workqueue(remote_wq);

	del_timer_sync(&remote.timer);

#ifdef CONFIG_USE_FIFO
	kfifo_free(remote.ckfifo);
#endif

	remove_proc_entry("remote_debug", NULL);

	unregister_chrdev_region(devno, REMOTE_NR_DEVS);
}

static int __init remote_init_module(void)
{
	dev_t devno = 0;
	int ret, i;
	struct proc_dir_entry *entry;

	printk(KERN_INFO "remote: Starting remote controller interface!\n");

	/*
	 * Get the minor number to work with, asking for a dynamic
	 * major unless directed otherwise at load time
	 */
	if (major) {
		devno = MKDEV(major, minor);
		ret = register_chrdev_region(devno, REMOTE_NR_DEVS, "remote");
	}
	else {
		ret = alloc_chrdev_region(&devno, minor, REMOTE_NR_DEVS, "remote");
		major = MAJOR(devno);
	}
	if (ret < 0) {
		printk(KERN_ERR "remote: can't get major %d\n", major);
		return ret;
	}

	/* Create a proc entry for run-time debug purposes */
	entry = create_proc_entry("remote_debug", 0, NULL);
	/* Failure is an unlikely situation */
	entry->read_proc = remote_proc_read;
	entry->write_proc = remote_proc_write;
	entry->data = &remote;

#ifdef CONFIG_USE_FIFO
	/* Allocate the circular buffer */
	remote.ckfifo = kfifo_alloc(CKFIFO_SIZE, GFP_KERNEL, NULL);
	if (!remote.ckfifo) {
		printk(KERN_ERR "remote: cannot allocate memory\n");
		ret = -ENOMEM;
		goto fail1;
	}
#else
	remote.irda_data = 0;
#endif

	/* Initialize "struct remote_device" protector */
	init_MUTEX(&remote.sem);

	/* Register the timer */
	init_timer(&remote.timer);
	remote.timer.data = (unsigned long)&remote;
	remote.timer.function = remote_timer_fn;

	/* Initialize the "read method" waitqueue */
	init_waitqueue_head(&remote.keyq);

	/* irq is requested before char_dev routines are registered because system start time and
	application start time is different. System starts when the power is applied but application
	starts only when the Linux booted and calls it */
	ret = request_irq(mcu_irq_line, mcu_interrupt, SA_INTERRUPT, "remote", &remote);
	if (ret) {
		printk(KERN_ERR "remote: can't get assigned irq %i\n", mcu_irq_line);
		goto fail2;
	}

	/* Create the bottom half handler */
	remote_wq = create_singlethread_workqueue("remote_wq");
	if (!remote_wq) {
		printk(KERN_ERR "remote: cannot create remote_wq worker thread\n");
		ret = -ENOMEM;
		goto fail3;
	}
	INIT_WORK(&remote.remote_work, remote_do_work, &remote);

	/* EEPROM is read first because workqueue relies on heavily */
	/* We dont need semaphore protection because cdev routines
	arent initialized yet */
	for (i = 0; i < CONFIG_READ_TIMEOUT; i++) {
		ret = read_config_data(EEPROM_I2C_ADDRESS, &remote);
		if (ret >= 0)
			break;
		mdelay(500);
	}

	/* Set up the char_dev structure for this device */
	cdev_init(&remote.cdev, &remote_fops);	/* struct cdev *my_cdev = cdev_alloc(); my_cdev->ops = &remote_fops; */
	remote.cdev.owner = THIS_MODULE;
	ret = cdev_add(&remote.cdev, devno, 1);
	/*
	 * We disabled tearing down in the case of an error return value from
	 * cdev registration because system should be in a workable state
	 * even if PC side wouldnt work.
	 */
	if (ret)
		printk(KERN_ERR "remote: error %d when adding remote", ret);
#if 0
		goto fail4;
	}
#endif

	return 0;	/* Succeed */

fail4:
	destroy_workqueue(remote_wq);
fail3:
	free_irq(mcu_irq_line, NULL);
fail2:
	del_timer_sync(&remote.timer);
#ifdef CONFIG_USE_FIFO
	kfifo_free(remote.ckfifo);
#endif
fail1:
	remove_proc_entry("remote_debug", NULL);
	unregister_chrdev_region(devno, REMOTE_NR_DEVS);
	return ret;
}

MODULE_AUTHOR("Bora Sahin <borasah@gmail.com>");
MODULE_DESCRIPTION("Remote Controller Device Driver");
MODULE_LICENSE("GPL");

module_init(remote_init_module);
module_exit(remote_cleanup_module);
