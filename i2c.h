#ifndef _I2C_H_
#define  _I2C_H_

/* Commands */
#define READ_DEVICE_CMD	0
#define READ_FILE_CMD	1
#define READ_POLL_CMD	2

#define CONFIG_DATA_WRITE_CMD	5
#define CONFIG_DATA_READ_CMD	6
//#define CONFIG_DATA_E2_READ_CMD	7

#define TEST_MCU_READ_CMD		10
#define TEST_MCU_WRITE_CMD	11
#define TEST_EEPROM_READ_CMD		12
#define TEST_EEPROM_WRITE_CMD		13

#define WELCOME_CMD	16
#define START_CMD	17
#define STOP_CMD	18
#define CONFIG_DATA_INVALID_CMD		19
#define CONFIG_DATA_VALID_CMD		20

#define LEARNING_CMD	23


/* Error codes */
#define ERR_PARSE_ARGS	-1
#define ERR_ALLOC_MEM		-2
#define ERR_OPEN_FILE		-3
#define ERR_READ_MCU		-4
#define ERR_WRITE_MCU		-5
#define ERR_ACCESS_KERNEL	-6
#define ERR_READ_EEPROM	-7
#define ERR_WRITE_EEPROM	-8
#define ERR_INVALID_ARG	-9


/* Some parameters */
#define FILE_NAME_SIZE	50
#define EEPROM_SIZE	0x200

/* EEPROM parameters */
#define SIGNATURE_SIZE		8
#define IRDA_CODING_SIZE	4
#define IRDA_KEY_SIZE		37
#define PANEL_OPTIONS_SIZE	4
#define ONE_TO_MUL_MAPPING_SIZE	12
#define ONE_TO_ONE_MAPPING_START	2
#define ONE_TO_ONE_MAPPING_LAST	23
#define ONE_TO_ONE_MAPPING_SIZE	21
//#define PANEL_ENTRY_SIZE	31
#define PANEL_ENTRY_SIZE	30

#define IRDA_CODING_LINE_START	0
#define IRDA_CODING_LINE_STOP	IRDA_CODING_SIZE
#define PANEL_OPTIONS_START	IRDA_CODING_LINE_STOP
#define PANEL_OPTIONS_STOP		((IRDA_CODING_LINE_STOP) + (PANEL_OPTIONS_SIZE))
#define PROCEDURE_LINE_START	PANEL_OPTIONS_STOP
#define PROCEDURE_LINE_STOP	((PROCEDURE_LINE_START) + (ONE_TO_MUL_MAPPING_SIZE))
#define PANEL_LINE_START		PROCEDURE_LINE_STOP
#define PANEL_LINE_STOP		((PANEL_LINE_START) + (PANEL_ENTRY_SIZE))

/* EEPROM parameters - Details */
#define IRDA_CODING_LENGTH_SHIFT	0
#define IRDA_CODING_PWM_OR_MANCH_SHIFT		6
#define IRDA_CODING_SYNC_PULSE_WIDTH		7
#define IRDA_CODING_REPEAT_ON_OFF		8
#define PANEL_OPTIONS_ON_STATE_SHIFT	0
#define PANEL_OPTIONS_START_TIMEOUT_SHIFT	1
#define PROCEDURE_COUNT_DEFAULT		0x20
#define PROCEDURE_COUNT_SHIFT		5
#define PROCEDURE_SIZE	4

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

/* irda_key indexes */
#define KEY_ON_OFF_IDX		0
#define KEY_TV_PC_IDX		1
#define KEY_VOL_UP_IDX		2
#define KEY_VOL_DOWN_IDX	3
#define KEY_MUTE_IDX		4
#define KEY_MENU_IDX	5
#define KEY_NUMBER_9_IDX	6
#define KEY_NUMBER_0_IDX	7
#define KEY_NUMBER_1_IDX	8
#define KEY_NUMBER_2_IDX	9
#define KEY_NUMBER_3_IDX	10
#define KEY_NUMBER_4_IDX	11
#define KEY_NUMBER_5_IDX	12
#define KEY_NUMBER_6_IDX	13
#define KEY_NUMBER_7_IDX	14
#define KEY_NUMBER_8_IDX	15
#define KEY_PROG_UP_IDX	16
#define KEY_PROG_DOWN_IDX	17
#define KEY_BACKWARD_IDX	18
#define KEY_FORWARD_IDX	19
#define KEY_OK_IDX		20
#define KEY_RESERVED_1_IDX	21
#define KEY_RESERVED_2_IDX	22
#define KEY_BACK_IDX	23
#define KEY_PAUSE_PLAY_IDX	24
#define KEY_VIDEO_IDX	25
#define KEY_MUSIC_IDX	26
#define KEY_INTERNET_IDX	27
#define KEY_NEWS_IDX	28
#define KEY_WEATHER_IDX	29
#define KEY_HOTEL_INFO_IDX	30
#define KEY_HELP_IDX	31
#define KEY_CLOCK_IDX	32
#define KEY_WAKE_UP_IDX	33
#define KEY_RESERVED_3_IDX	34
#define KEY_RESERVED_4_IDX	35
#define KEY_RESERVED_5_IDX	36


char *keys[] = {
	"OFF",
	"TV_PC",
	"VOL_UP",
	"VOL_DOWN",
	"MUTE",
	"MENU",
	"9",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"PROG_UP",
	"PROG_DOWN",
	"BACKWARD",
	"FORWARD",
	"OK",
	"RESERVED_1",
	"RESERVED_2",
	"ON",
	"PC",
	"TV",
	"MULTIMEDIA",
	"SOURCE",
	"RESERVED_3",
	"RESERVED_4",
};

char *irda_coding_str[] = {
	"receive_1",
	"receive_2",
	"send_1",
	"send_2",
};

char *procedure_str[] = {
	"ON_1",
	"ON_2",
	"ON_3",
	"ON_4",
	"TV_to_PC_1",
	"TV_to_PC_2",
	"TV_to_PC_3",
	"TV_to_PC_4",
	"PC_to_TV_1",
	"PC_to_TV_2",
	"PC_to_TV_3",
	"PC_to_TV_4",
};

struct eeprom_table;

/* Function prototypes */
static int read_device_cmd(int device_fd);
static int read_file_cmd(int device_fd);
static int read_poll_cmd(int device_fd);
static int config_data_write_cmd(int device_fd);
static int config_data_read_cmd(int device_fd);
static int config_data_read_e2_cmd(int device_fd);
static int test_mcu_read_cmd(int device_fd);
static int test_mcu_write_cmd(int device_fd);
static int test_eeprom_read_cmd(int device_fd);
static int test_eeprom_write_cmd(int device_fd);
static int welcome_cmd(int device_fd);
static int start_cmd(int device_fd);
static int stop_cmd(int device_fd);
static int cfg_invalid_cmd(int device_fd);
static int learning_cmd(int device_fd);

static void dump_key(int idx);
static void lookup(__u32 irda_data);
static int open_file(FILE **fp, char *filename);
static int cache_file(FILE **fp, unsigned char **buf);
static int parse_irda_key_file(struct eeprom_table *e2prom_tab_p);
static int parse_panel_file(struct eeprom_table *e2prom_tab_p);
static void first_stage(int device_fd, struct eeprom_table *new_e2prom_tab_p, int dump_mode);
static void first_stage_dump(struct eeprom_table *e2prom_tab_p);
static void second_stage(int device_fd, struct eeprom_table *new_e2prom_tab_p, int dump_mode);
static void second_stage_dump(struct eeprom_table *e2prom_tab_p);
static void third_stage(int device_fd, struct eeprom_table *new_e2prom_tab_p, int dump_mode);
static void third_stage_dump(struct eeprom_table *e2prom_tab_p);
static void on_and_tv_to_pc_procedure(int device_fd, struct eeprom_table *new_e2prom_tab_p, int onflag);
static int magic_pc_transition_key(int device_fd, struct eeprom_table *new_e2prom_tab_p);
static void pc_to_tv_procedure(int device_fd, struct eeprom_table *new_e2prom_tab_p);
static int wait_key_and_grab(int device_fd, struct eeprom_table *new_e2prom_tab_p, char *str);
static int strtoidx(char **strp, const char *str, int len);
static char *idxtostr(char **strp, int idx);
static unsigned int get_value(const char *fmt, unsigned int min, unsigned int size);
static int stage_wrapper(char *str, void (*stage)(int device_fd, struct eeprom_table *new_e2prom_tab_p, int dump_mode), void (*stage_dump)(struct eeprom_table *new_e2prom_tab_p), int device_fd, struct eeprom_table *new_e2prom_tab_p, int dump_mode);
static int epilogue(char *str);
static int prologue(char *str);

static char *alloc_mem(short length);
static void free_mem(char *buf);

static void dump_config_data(struct eeprom_table *e2prom_tab_p);
static void dump_data_hex(unsigned char *buf, int size);
static void dump_data_dec(unsigned char *buf, int size);

static int parse_cmd(int device_fd);
static int parse_args(int argc, char *argv[]);
static void show_help(void);

#endif	/* _I2C_H_ */
