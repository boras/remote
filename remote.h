#ifndef _REMOTE_DRIVER_H_
#define _REMOTE_DRIVER_H_

/* Debugging options */
#ifdef REMOTE_DEBUG
#define FUNCTION	printk(KERN_INFO "remote: %s\n", __FUNCTION__)
#else
#define FUNCTION do { } while (0)
#endif

#define KEY_ACK				0x00
#define ON_PROCEDURE			0x20
#define TV_TO_PC_PROCEDURE	0x40
#define PC_TO_TV_PROCEDURE	0x60
#define CHANGE_REMOTE		0x80
#define CONFIG_DATA_INVALID		0xA0
#define CONFIG_DATA_VALID		0xC0
#define START			0xE0
#define STOP			0x5F

#define START_STATE_SET	(0x2)
#define START_STATE_CLEAR	(~0x2)
#define CFG_STATE_SET	(0x1)
#define CFG_STATE_CLEAR	(~0x1)
#define PC_ENABLE	((START_STATE_SET) | (CFG_STATE_SET))

#define RECV_MCU_CMD_IDX	0
#define RECV_IR_DATA_IDX	1

/* Module parameters */
#define REMOTE_NR_DEVS	1
#define REMOTE_MAJOR		241
#define REMOTE_MINOR		0
#define REMOTE_MODE		0

#ifdef CONFIG_CPU_FLOWCHART	/* CONFIG_CPU_FLOWCHART */

#define KEY_ON_OFF_IDX		0
#define KEY_TV_PC_IDX		1
#define KEY_VOL_UP_IDX		2
#define KEY_VOL_DOWN_IDX	3
#define KEY_MUTE_IDX		4
#define KEY_MENU_IDX	5
#define KEY_NUMBER_9_IDX	6
#define KEY_NUMBER_START_IDX		6
#define KEY_NUMBER_STOP_IDX		((KEY_NUMBER_START_IDX) + (10))

#define ONE_TO_ONE_MASK	0x1F
//#define IDX_MASK	0x1F

#define RECV_ON_OFF_IDX	5

#define PANEL_OFF	0
#define PANEL_ON	1
#define TV_MODE		0
#define PC_MODE		1

#define TRUE	1
#define FALSE	0

#define PASS_ENTRY_CHR_NUMBER		4

#endif	/* CONFIG_CPU_FLOWCHART */

/* ioctl implementation */
/* Magic number is 'k' */
#define REMOTE_MAGIC  'k'

//#define REMOTE_CONFDAT_FROM_EEPROM_CMD	_IOR(REMOTE_MAGIC, 0, int)
//#define REMOTE_CONFDAT_FROM_MEM_CMD	_IOR(REMOTE_MAGIC, 1, int)
#define REMOTE_READ_CONFIG_DATA_CMD     _IOR(REMOTE_MAGIC, 1, int)
#define REMOTE_WELCOME_CMD	_IO(REMOTE_MAGIC, 2)
#define REMOTE_START_CMD	_IO(REMOTE_MAGIC, 3)
#define REMOTE_STOP_CMD	_IO(REMOTE_MAGIC, 4)
#define REMOTE_CFG_INVALID_CMD	_IO(REMOTE_MAGIC, 5)
#define REMOTE_CFG_VALID_CMD	_IO(REMOTE_MAGIC, 6)

#define REMOTE_READ_MCU_CMD	_IOR(REMOTE_MAGIC, 9, int)
#define REMOTE_WRITE_MCU_CMD	_IOW(REMOTE_MAGIC, 10, int)
#define REMOTE_READ_EEPROM_CMD	_IOR(REMOTE_MAGIC, 11, int)
#define REMOTE_WRITE_EEPROM_CMD	_IOW(REMOTE_MAGIC, 12, int)

#define REMOTE_MAXNR 15

/* EEPROM parameters */
#define SIGNATURE_SIZE		8
#define IRDA_CODING_SIZE	4
#define IRDA_KEY_SIZE		37
#define ONE_TO_MUL_MAPPING_SIZE	12
#define ONE_TO_ONE_MAPPING_FIRST	2
#define ONE_TO_ONE_MAPPING_LAST	23
#define ONE_TO_ONE_MAPPING_SIZE	21
#define PANEL_ENTRY_SIZE	30

/* I2C device addresses */
#define MCU_I2C_ADDRESS	0x10
#define EEPROM_I2C_ADDRESS	0xA0

/* I2C transaction sizes */
#define MCU_READ_SIZE	8
#define MCU_READ_SIZE_MAX	200
#define MCU_WRITE_SIZE_MAX	200
#define EEPROM_SIZE	0x200
#define EEPROM_PAGE_WRITE_SIZE	16
/* +1 because of the leading data word address requirement */
#define EEPROM_PAGE_TRANSACTION_SIZE	((EEPROM_PAGE_WRITE_SIZE) + (1))

/* CKFIFO size if need be */
#define CKFIFO_SIZE	16

/* Trial number for read_config_data */
#define CONFIG_READ_TIMEOUT	3
#define START_SEEN_TIMEOUT		100

/* Trial number for i2c bus transactions */
#define I2C_TRY_COUNT	3


/* It's here to suppress compiler warnings */
struct io_entry {
	short len;
	short val;
	char __user *buf;
	short addr;
};

static struct remote_device {
	u8 flag; /* Keeps config_data valid / invalid and start / stop state */
	int reader_thread_count;	/* PC enable variable together with flag */
#ifdef CONFIG_USE_FIFO
	struct kfifo *ckfifo;
#else
	u32 irda_data;
#endif
	struct semaphore sem; /* Protects this structure against concurreny problems */
	struct timer_list timer; /* Checks START is seen by the MCU */
	wait_queue_head_t keyq;
	struct work_struct remote_work;
	struct cdev cdev;
} remote;

static struct workqueue_struct *remote_wq;

#ifdef CONFIG_EEPROM_TABLE_1
/* Dont leave out the alignment to the compiler */
struct eeprom_table {
	__u8 reserved[8];
	struct {
		__u32 panel_key;	/* Panel irda key. Must be 32 bit */
		__u16 wait_time;	/* Panel wait time. Must be 16 bit */
		__u8 padding[2];
	} __attribute__ ((packed)) panel_tab[PANEL_ENTRY_SIZE];
	__u8 procedure[ONE_TO_MUL_MAPPING_SIZE];		/* one_to_mul_index_array */
	__u8 irda_coding[IRDA_CODING_SIZE];
	__u8 panel_options[4];
	__u32 irda_tab[IRDA_KEY_SIZE];	/* iNUX irda_key table */
} __attribute__ ((packed)) e2prom_tab;
#endif	/* CONFIG_EEPROM_TABLE_1 */

#ifdef CONFIG_EEPROM_TABLE_2
/* Dont leave out the alignment to the compiler */
struct eeprom_table {
	__u8 reserved[8];
	struct {
		__u32 panel_key;	/* Panel irda key. Must be 32 bit */
		__u16 wait_time;	/* Panel wait time. Must be 16 bit */
	} __attribute__ ((packed)) panel_tab[PANEL_ENTRY_SIZE];
	__u8 procedure[ONE_TO_MUL_MAPPING_SIZE];		/* one_to_mul_index_array */
	__u8 irda_coding[IRDA_CODING_SIZE];
	__u8 panel_options[4];
	__u32 irda_tab[IRDA_KEY_SIZE];	/* iNUX irda_key table */
} __attribute__ ((packed)) e2prom_tab;
#endif	/* CONFIG_EEPROM_TABLE_2 */

#ifdef CONFIG_EEPROM_TABLE_3
/* Dont leave out the alignment to the compiler */
struct eeprom_table {
	__u8 reserved[8];
	__u32 panel_irda_key[PANEL_ENTRY_SIZE];
	__u16 panel_wait_time[PANEL_ENTRY_SIZE];
	__u8 procedure[ONE_TO_MUL_MAPPING_SIZE];		/* one_to_mul_index_array */
	__u8 irda_coding[IRDA_CODING_SIZE];
	__u8 panel_options[4];
	__u32 irda_tab[IRDA_KEY_SIZE];	/* iNUX irda_key table */
} __attribute__ ((packed)) e2prom_tab;
#endif	/* CONFIG_EEPROM_TABLE_3 */

/* Function prototypes */
static int read_i2c_current(u16 dev_addr, u8 *buf, u16 len);
static int read_i2c_random(u16 dev_addr, u8 *reg, u8 *buf, u16 len);
static int write_i2c(u16 dev_addr, u8 *buf, u16 len);
static int device_read_i2c_current(u16 addr, struct io_entry *iop);
static int device_write_i2c_current(u16 addr, struct io_entry *iop);
static int device_read_i2c_random(u16 addr, u8 reg, struct io_entry *iop);
static int device_write_i2c_random(u16 addr, u8 reg, struct io_entry *iop);

static void dump_data(u8 *buf, u16 count);
static void dump_config_data(void);

static void remote_do_work(void *data);
static int send_cmd_to_mcu(u8 cmd);
static int write_config_data(u16 dev_addr, struct remote_device *rp);
static int read_config_data(u16 dev_addr, struct remote_device *rp);
static void remote_timer_fn(unsigned long data);

#endif	/* _REMOTE_DRIVER_H_ */
