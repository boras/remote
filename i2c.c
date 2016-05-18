#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <asm/types.h>

#include "i2c.h"

//#define CONFIG_EEPROM_TABLE_1
//#define CONFIG_EEPROM_TABLE_2
#define CONFIG_EEPROM_TABLE_3

/* Global variables */
static const char *filename = "/dev/remote";
struct parameter {
	struct {
		short length;
		short value;
		char *buf;
		short addr;
		short count;
	} io;
	char irda_key_file[FILE_NAME_SIZE];
	char panel_file[FILE_NAME_SIZE];
	int cmd;
};
static struct parameter param;

#if 0
__u32 irda_tab[] =
	{
		0x00FF50AF,	// ON/OFF
		0x00FF30CF,	// TV/PC
		0x00FF40BF,	// V+
		0x00FFC03F,	// V-
		0x00FFB847,	// Mute
		0x00FF08F7,	// 0
		0x00FF8877, 	// 1
		0x00FF48B7,	// 2
		0x00FFC837,	// 3
		0x00FF28D7,	// 4
		0x00FFA857,	// 5
		0x00FF6897,	// 6
		0x00FFE817,	// 7
		0x00FF18E7,	// 8
		0x00FF9867,	// 9
		0x00FF00FF,	// P+
		0x00FF807F,	// P-
		0x00FF58A7,	// Backward(-/--)
		0x00FFD827,	// Forward(-/--)
		0x00FFF807,	// OK
		0x00FF38C7,	// MENU
		0x00000000,	// reserved
		0x00000000,	// reserved
		0x00FFA05F,	// Back
		0x00FFE01F,	// Pause/Play
		0x00FF906F,	// Video
		0x00FF20DF,	// Music
		0x00FF10EF,	// Internet
		0x00FFF00F,	// News
		0x00FFD02F,	// Weather
		0x00FF609F,	// Hotel info
		0x00FF02FD,	// Help
		0x00FF708F,	// Clock
		0x00FF7887,	// Wake up
		0x00000000,	// reserved
		0x00000000,	// reserved
		0x00000000,	// reserved
	};
#else
#ifdef CONFIG_EEPROM_TABLE_1
/* Dont leave out the alignment to the compiler */
struct eeprom_table {
	__u8 reserved[8];
	struct {
		__u32 panel_key;	// Panel irda key. Must be 32 bit
		__u16 wait_time;	// Panel wait time. Must be 16 bit
		__u8 padding[2];
	} __attribute__ ((packed)) panel_tab[PANEL_ENTRY_SIZE];
	__u8 procedure[ONE_TO_MUL_MAPPING_SIZE];		// one_to_mul_index_array
	__u8 irda_coding[IRDA_CODING_SIZE];
	__u8 panel_options[4];
	__u32 irda_tab[IRDA_KEY_SIZE];	// iNUX irda_key table
} __attribute__ ((packed)) e2prom_tab;
#endif	/* CONFIG_EEPROM_TABLE_1 */

#ifdef CONFIG_EEPROM_TABLE_2
/* Dont leave out the alignment to the compiler */
struct eeprom_table {
	__u8 reserved[8];
	struct {
		__u32 panel_key;	// Panel irda key. Must be 32 bit
		__u16 wait_time;	// Panel wait time. Must be 16 bit
	} __attribute__ ((packed)) panel_tab[PANEL_ENTRY_SIZE];
	__u8 procedure[ONE_TO_MUL_MAPPING_SIZE];		// one_to_mul_index_array
	__u8 irda_coding[IRDA_CODING_SIZE];
	__u8 panel_options[4];
	__u32 irda_tab[IRDA_KEY_SIZE];	// iNUX irda_key table
} __attribute__ ((packed)) e2prom_tab;
#endif	/* CONFIG_EEPROM_TABLE_2 */

#ifdef CONFIG_EEPROM_TABLE_3
/* Dont leave out the alignment to the compiler */
struct eeprom_table {
	__u8 reserved[8];
	__u32 panel_irda_key[PANEL_ENTRY_SIZE];
	__u16 panel_wait_time[PANEL_ENTRY_SIZE];
	__u8 procedure[ONE_TO_MUL_MAPPING_SIZE];		// one_to_mul_index_array
	__u8 irda_coding[IRDA_CODING_SIZE];
	__u8 panel_options[4];
	__u32 irda_tab[IRDA_KEY_SIZE];	// iNUX irda_key table
} __attribute__ ((packed)) e2prom_tab;
#endif	/* CONFIG_EEPROM_TABLE_3 */
#endif

/* Function definitions */
int main(int argc, char *argv[])
{
	int device_fd, result;

	result = parse_args(argc, argv);
	if (result < 0)
		exit(EXIT_FAILURE);

	device_fd = open(filename, O_RDWR);
	if (device_fd == -1) {
		fprintf(stderr, "i2c: cannot open %s file\n", filename);
		perror("i2c: main");
		exit(EXIT_FAILURE);
	}

	result = parse_cmd(device_fd);
	if (result)
		exit(EXIT_FAILURE);

	close(device_fd);

	return 0;
}

static void dump_key(int idx)
{
	switch (idx) {
		case KEY_ON_OFF_IDX:
			printf("KEY_ON_OFF\n"); break;
		case KEY_TV_PC_IDX:
			printf("KEY_TV_PC\n"); break;
		case KEY_VOL_UP_IDX:
			printf("KEY_VOL_UP\n"); break;
		case KEY_VOL_DOWN_IDX:
			printf("KEY_VOL_DOWN\n"); break;
		case KEY_MUTE_IDX:
			printf("KEY_MUTE\n"); break;
		case KEY_NUMBER_0_IDX:
			printf("KEY_NUMBER_0\n"); break;
		case KEY_NUMBER_1_IDX:
			printf("KEY_NUMBER_1\n"); break;
		case KEY_NUMBER_2_IDX:
			printf("KEY_NUMBER_2\n"); break;
		case KEY_NUMBER_3_IDX:
			printf("KEY_NUMBER_3\n"); break;
		case KEY_NUMBER_4_IDX:
			printf("KEY_NUMBER_4\n"); break;
		case KEY_NUMBER_5_IDX:
			printf("KEY_NUMBER_5\n"); break;
		case KEY_NUMBER_6_IDX:
			printf("KEY_NUMBER_6\n"); break;
		case KEY_NUMBER_7_IDX:
			printf("KEY_NUMBER_7\n"); break;
		case KEY_NUMBER_8_IDX:
			printf("KEY_NUMBER_8\n"); break;
		case KEY_NUMBER_9_IDX:
			printf("KEY_NUMBER_9\n"); break;
		case KEY_PROG_UP_IDX:
			printf("KEY_PROG_UP\n"); break;
		case KEY_PROG_DOWN_IDX:
			printf("KEY_PROG_DOWN\n"); break;
		case KEY_BACKWARD_IDX:
			printf("KEY_BACKWARD\n"); break;
		case KEY_FORWARD_IDX:
			printf("KEY_FORWARD\n"); break;
		case KEY_OK_IDX:
			printf("KEY_OK\n"); break;
		case KEY_MENU_IDX:
			printf("KEY_MENU\n"); break;
		case KEY_RESERVED_1_IDX:
			printf("KEY_RESERVED_1\n"); break;
		case KEY_RESERVED_2_IDX:
			printf("KEY_RESERVED_2\n"); break;
		case KEY_BACK_IDX:
			printf("KEY_BACK\n"); break;
		case KEY_PAUSE_PLAY_IDX:
			printf("KEY_PAUSE_PLAY\n"); break;
		case KEY_VIDEO_IDX:
			printf("KEY_VIDEO\n"); break;
		case KEY_MUSIC_IDX:
			printf("KEY_MUSIC\n"); break;
		case KEY_INTERNET_IDX:
			printf("KEY_INTERNET\n"); break;
		case KEY_NEWS_IDX:
			printf("KEY_NEWS\n"); break;
		case KEY_WEATHER_IDX:
			printf("KEY_WEATHER\n"); break;
		case KEY_HOTEL_INFO_IDX:
			printf("KEY_HOTEL_INFO\n"); break;
		case KEY_HELP_IDX:
			printf("KEY_HELP\n"); break;
		case KEY_CLOCK_IDX:
			printf("KEY_CLOCK\n"); break;
		case KEY_WAKE_UP_IDX:
			printf("KEY_WAKE_UP\n"); break;
		case KEY_RESERVED_3_IDX:
			printf("KEY_RESERVED_3\n"); break;
		case KEY_RESERVED_4_IDX:
			printf("KEY_RESERVED_4\n"); break;
		case KEY_RESERVED_5_IDX:
			printf("KEY_RESERVED_5\n"); break;
	}
}

static void lookup(__u32 irda_data)
{
	int i;

	for (i = 0; i < IRDA_KEY_SIZE; i++)
		if (irda_data == e2prom_tab.irda_tab[i]) {
			printf("i2c: Key found!\n");
			dump_key(i);
			return;
		}
	printf("i2c: Key not found!\n");
}

static int open_file(FILE **fp, char *filename)
{
	*fp = fopen(filename, "r");

	if (!*fp) {
		fprintf(stderr, "i2c: cannot open '%s' file\n", filename);
		perror("i2c: open_file");
		return ERR_OPEN_FILE;
	}

	return 0;
}

static int cache_file(FILE **fp, unsigned char **buf)
{
	size_t size = 0;

	fseek(*fp, 0, SEEK_END);
	size = ftell(*fp);
	//printf("i2c: file_size = %d\n", size);
	fseek(*fp, 0, SEEK_SET);
	*buf = alloc_mem(size + 1);
	if (!*buf)
		return ERR_ALLOC_MEM;

	while (!feof(*fp))
		fread(*buf, size, 1, *fp);
	//dump_data_dec(*buf, size);

	return size;
}

static int parse_irda_key_file(struct eeprom_table *e2prom_tab_p)
{
#define IRDA_READ_SIZE	20
	FILE *irda_fp;
	int result, i, idx, j;
	unsigned char *buf;
	size_t size = 0;

	result = open_file(&irda_fp, param.irda_key_file);
	if (result)
		return result;

	result = cache_file(&irda_fp, &buf);
	if (result < 0)
		return ERR_ALLOC_MEM;
	size = result;

	idx = 0;	// irda_tab index
	for (i = 0; i < size; i++) {
		if (!strncmp(&buf[i], "0x", 2)) {
			//printf("i2c: i = %d\n", i);
			//for (j = i + 2; j < i + 2 + 8; j++)
			//	printf("%c", buf[j]);
			//printf("\n");
			e2prom_tab_p->irda_tab[idx] = strtoul(&buf[i], NULL, 16);
			//printf("i2c: irda_tab[%d] = %x\n", idx, e2prom.irda_tab[idx]);
			i += 9;	// 2 + 8 - 1 -> -1 because of i++
		}
		if (buf[i] == '\n')
			idx++;
	}

	fclose(irda_fp);
	free_mem(buf);

	return 0;
}

static int parse_panel_file(struct eeprom_table *e2prom_tab_p)
{
	FILE *panel_fp;
	int result, i;
	int line, fidx, sidx, tidx;	// first, second and third idx
	__u8 *buf;
	size_t size = 0;

	//printf("i2c: parse_panel_file function\n");

	result = open_file(&panel_fp, param.panel_file);
	if (result)
		return result;

	result = cache_file(&panel_fp, &buf);
	if (result < 0)
		return ERR_ALLOC_MEM;
	size = result;

	//dump_data_dec(buf, size);

	line = fidx = sidx = tidx = 0;
	for (i = 0; i < size; i++) {
		if (buf[i] == '\n')
			line++;
		if (!strncmp(&buf[i], "0x", 2)) {
			//printf("i2c: i = %d ", i);
			if (line == IRDA_CODING_LINE_START || line < IRDA_CODING_LINE_STOP) {
				fidx = line - IRDA_CODING_LINE_START;
				e2prom_tab_p->irda_coding[fidx] = strtoul(&buf[i], NULL, 16);
				i += 3;	// 2 + 2 - 1 -> -1 because of i++
				continue;
			}
			if (line == PANEL_OPTIONS_START || line < PANEL_OPTIONS_STOP) {
				fidx = line - PANEL_OPTIONS_START;
				e2prom_tab_p->panel_options[fidx] = strtoul(&buf[i], NULL, 16);
				i += 3;	// 2 + 2 - 1 -> -1 because of i++
				continue;
			}
			if (line == PROCEDURE_LINE_START || line < PROCEDURE_LINE_STOP) {
				sidx = line - PROCEDURE_LINE_START;
				e2prom_tab_p->procedure[sidx] = strtoul(&buf[i], NULL, 16);
				i += 3;	// 2 + 2 - 1 -> -1 because of i++
				continue;
			}

			if (line == PANEL_LINE_START || line < PANEL_LINE_STOP) {
				int j;

				tidx = line - PANEL_LINE_START;
#if defined(CONFIG_EEPROM_TABLE_1) || defined(CONFIG_EEPROM_TABLE_2)
				e2prom_tab_p->panel_tab[tidx].panel_key = strtoul(&buf[i], NULL, 16);
#else
				e2prom_tab_p->panel_irda_key[tidx] = strtoul(&buf[i], NULL, 16);
#endif
				i += 9;	// 2 + 8 - 1 -> -1 because of i++
				for (j = i; buf[j] != '\n'; j++)
					if (!strncmp(&buf[j], "0x", 2)) {
#if defined(CONFIG_EEPROM_TABLE_1) || defined(CONFIG_EEPROM_TABLE_2)
						e2prom_tab_p->panel_tab[tidx].wait_time = strtoul(&buf[j], NULL, 16);
#else
						e2prom_tab_p->panel_wait_time[tidx] = strtoul(&buf[j], NULL, 16);
#endif
						break;
					}
				i = j + 5;	// 2 + 4 - 1 -> -1 because of i++
			}
		}
	}

	fclose(panel_fp);
	free_mem(buf);

	return 0;
}

static int config_data_write_cmd(int device_fd)
{
	int result, i;
	int counter = 0;

	printf("i2c: sizeof(struct eeprom_table) = %d\n", sizeof(struct eeprom_table));

	result = parse_irda_key_file(&e2prom_tab);
	if (result)
		return result;

	result = parse_panel_file(&e2prom_tab);
	if (result)
		return result;

	//dump_config_data(&e2prom_tab);

	for (i = 0; i < 1; i++) {
		result = write(device_fd, (const void *)(&e2prom_tab), sizeof(struct eeprom_table));
		printf("i2c: result = %d\n", result);
		if (result < 0) {
			fprintf(stderr, "i2c: cannot write to the device\n");
			counter++;
			printf("i2c: err : counter = %d\n", counter);
			//return ERR_READ_DEV;
		}
	}

	return 0;
}

static int config_data_read_cmd(int device_fd)
{
	int result, i;

	for (i = 0; i < 1; i++) {
		result = ioctl(device_fd, REMOTE_READ_CONFIG_DATA_CMD, &e2prom_tab);
		printf("i2c: result = %d\n", result);
		if (result < 0) {
			fprintf(stderr, "i2c: cannot read the CPU EEPROM table\n");
			return ERR_ACCESS_KERNEL;
		}
	}

	dump_config_data(&e2prom_tab);

	return 0;
}

/*
static int config_data_read_e2_cmd(int device_fd)
{
	int result, i;

	for (i = 0; i < 1; i++) {
		result = ioctl(device_fd, REMOTE_CONFDAT_FROM_EEPROM_CMD, &e2prom_tab);
		printf("i2c: result = %d\n", result);
		if (result < 0) {
			fprintf(stderr, "i2c: cannot reach EEPROM\n");
			return ERR_READ_EEPROM;
		}
		else if (result == 0) {
			fprintf(stderr, "i2c: EEPROM was reached but config_data is invalid\n");
			return ERR_READ_EEPROM;
		}
	}

	dump_config_data(&e2prom_tab);

	return 0;
}
*/

static int read_device_cmd(int device_fd)
{
	int result;
	int counter = 0;
	__u32 irda_data = 0;

	result = ioctl(device_fd, REMOTE_READ_CONFIG_DATA_CMD, &e2prom_tab);
	//printf("i2c: result = %d\n", result);
	if (result < 0) {
		fprintf(stderr, "i2c: cannot read the CPU EEPROM table\n");
		return ERR_ACCESS_KERNEL;
	}

	for (;;) {
		result = read(device_fd, &irda_data, sizeof(irda_data));
		//printf("i2c: result = %d\n", result);
		if (result < 0) {
			fprintf(stderr, "i2c: cannot read the device\n");
			counter++;
			printf("i2c: err : counter = %d\n", counter);
			//return ERR_READ_DEV;
		}
		else if (result == 0) {
			fprintf(stderr, "i2c: there is no data\n");
			break;
		}
		else {
			printf("i2c: irda_data = %X\n", irda_data);
			lookup(irda_data);
		}
	}

	return 0;
}

static int read_file_cmd(int device_fd)
{
	int result;
	int counter = 0;
	__u32 irda_data = 0;

	result = parse_irda_key_file(&e2prom_tab);
	if (result)
		return result;

	for (;;) {
		result = read(device_fd, &irda_data, sizeof(irda_data));
		//printf("i2c: result = %d\n", result);
		if (result < 0) {
			fprintf(stderr, "i2c: cannot read the device\n");
			counter++;
			printf("i2c: err : counter = %d\n", counter);
			//return ERR_READ_DEV;
		}
		else if (result == 0) {
			fprintf(stderr, "i2c: there is no data\n");
			break;
		}
		else {
			printf("i2c: irda_data = %X\n", irda_data);
			lookup(irda_data);
		}
	}

	return 0;
}

static int read_poll_cmd(int device_fd)
{
	int result;
	int counter = 0;
	__u32 irda_data = 0;

	result = ioctl(device_fd, REMOTE_READ_CONFIG_DATA_CMD, &e2prom_tab);
	//printf("i2c: result = %d\n", result);
	if (result < 0) {
		fprintf(stderr, "i2c: cannot read the CPU EEPROM table\n");
		return ERR_ACCESS_KERNEL;
	}

	for (;;) {
		fd_set rfds;
		struct timeval tv;

		/* Watch device_fd(/dev/remote) to see when it has input */
		FD_ZERO(&rfds);
		FD_SET(device_fd, &rfds);
		/* Wait up to five seconds. */
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		result = select((device_fd + 1), &rfds, NULL, NULL, &tv);
		/* Don’t rely on the value of tv now! */

		if (result == -1)
			perror("select()");
		else {
			if (result) {
				printf("Data is available now.\n");
				/* FD_ISSET(device_fd, &rfds) will be true. */
				if (FD_ISSET(device_fd, &rfds)) {
					result = read(device_fd, &irda_data, sizeof(irda_data));
					//printf("i2c: result = %d\n", result);
					 if (result == 0) {
						fprintf(stderr, "i2c: there is no data\n");
						break;
					}
					else {
						printf("i2c: irda_data = %X\n", irda_data);
						lookup(irda_data);
					}
				}
			}
			else
				printf("No data within five seconds.\n");
		}
	}

	return 0;
}

static int welcome_cmd(int device_fd)
{
	int result, i;

//	printf("i2c: welcome_cmd\n");

	for (i = 0; i < 1; i++) {
		result = ioctl(device_fd, REMOTE_WELCOME_CMD, 0);
		//printf("i2c: result = %d\n", result);
		if (result < 0) {
			fprintf(stderr, "i2c: cannot access to the MCU\n");
			return ERR_WRITE_MCU;
		}
	}

	return 0;
}

static int start_cmd(int device_fd)
{
	int result, i;

//	printf("i2c: start_cmd\n");

	for (i = 0; i < 1; i++) {
		result = ioctl(device_fd, REMOTE_START_CMD, 0);
		//printf("i2c: result = %d\n", result);
		if (result < 0) {
			fprintf(stderr, "i2c: cannot access to the MCU\n");
			return ERR_WRITE_MCU;
		}
	}

	return 0;
}

static int stop_cmd(int device_fd)
{
	int result, i;

//	printf("i2c: stop_cmd\n");

	for (i = 0; i < 1; i++) {
		result = ioctl(device_fd, REMOTE_STOP_CMD, 0);
		//printf("i2c: result = %d\n", result);
		if (result < 0) {
			fprintf(stderr, "i2c: cannot access to the MCU\n");
			return ERR_WRITE_MCU;
		}
	}

	return 0;
}

static int cfg_invalid_cmd(int device_fd)
{
	int result, i;

//	printf("i2c: cfg_invalid_cmd\n");

	for (i = 0; i < 1; i++) {
		result = ioctl(device_fd, REMOTE_CFG_INVALID_CMD, 0);
		//printf("i2c: result = %d\n", result);
		if (result < 0) {
			fprintf(stderr, "i2c: cannot access to the MCU\n");
			return ERR_WRITE_MCU;
		}
	}

	return 0;
}

#if 0
static int cfg_valid_cmd(int device_fd)
{
	int result, i;

//	printf("i2c: cfg_valid_cmd\n");

	for (i = 0; i < 1; i++) {
		result = ioctl(device_fd, REMOTE_CFG_VALID_CMD, 0);
		//printf("i2c: result = %d\n", result);
		if (result < 0) {
			fprintf(stderr, "i2c: cannot access to the MCU\n");
			return ERR_WRITE_MCU;
		}
	}

	return 0;
}
#endif

static int learning_cmd(int device_fd)
{
	struct eeprom_table new_e2prom_tab;
	int ret;

	memset(&new_e2prom_tab, 0, sizeof(new_e2prom_tab));
#if 0
	first_stage(&new_e2prom_tab, 1);
	configwrite(&new_e2prom_tab);
	second_stage(device_fd, &new_e2prom_tab);
	third_stage(device_fd, &new_e2prom_tab);
	first_stage(&new_e2prom_tab, 0);
	parse_irda_key_file(&new_e2prom_tab);
	configwrite(&new_e2prom_tab);
#else
	ret = stage_wrapper("first_stage", first_stage, first_stage_dump, device_fd, &new_e2prom_tab, 1);
	if (!ret) {
		ret = write(device_fd, (const void *)(&new_e2prom_tab), sizeof(struct eeprom_table));
		//printf("i2c: ret = %d\n", ret);
		if (ret < 0) {
			fprintf(stderr, "i2c: cannot write to the device\n");
			return ERR_WRITE_EEPROM;
		}
		sleep(1);
	}

	stage_wrapper("second_stage", second_stage, second_stage_dump, device_fd, &new_e2prom_tab, 1);

	stage_wrapper("third_stage", third_stage, third_stage_dump, device_fd, &new_e2prom_tab, 1);

	stage_wrapper("fourth_stage", first_stage, first_stage_dump, device_fd, &new_e2prom_tab, 0);

	parse_irda_key_file(&new_e2prom_tab);
	//dump_config_data(&new_e2prom_tab);
	ret = write(device_fd, (const void *)(&new_e2prom_tab), sizeof(struct eeprom_table));
	//printf("i2c: ret = %d\n", ret);
	if (ret < 0) {
		fprintf(stderr, "i2c: cannot write to the device\n");
		return ERR_WRITE_EEPROM;
	}
#endif

	return 0;
}

static void third_stage_dump(struct eeprom_table *e2prom_tab_p)
{
	int i;

	for (i = 0; i < PANEL_ENTRY_SIZE; i++) {
#if defined(CONFIG_EEPROM_TABLE_1) || defined(CONFIG_EEPROM_TABLE_2)
		printf("i2c: e2prom_tab_p->panel_tab[%s].panel_key = %x\n", idxtostr(keys, i), e2prom_tab_p->panel_tab[i].panel_key);
		printf("i2c: e2prom_tab_p->panel_tab[%s].wait_time = %x\n",  idxtostr(keys, i), e2prom_tab_p->panel_tab[i].wait_time);
#endif
#ifdef CONFIG_EEPROM_TABLE_3
		printf("i2c: e2prom_tab_p->panel_irda_key[%s] = %x\n",  idxtostr(keys, i), e2prom_tab_p->panel_irda_key[i]);
		printf("i2c: e2prom_tab_p->panel_wait_time[%s] = %x\n",  idxtostr(keys, i), e2prom_tab_p->panel_wait_time[i]);
#endif
	}
	for (i = 0; i < ONE_TO_MUL_MAPPING_SIZE; i++)
		printf("i2c: e2prom_tab_p->procedure[%s] = %x\n", idxtostr(procedure_str, i), e2prom_tab_p->procedure[i]);
}

static void third_stage(int device_fd, struct eeprom_table *new_e2prom_tab_p, int dump_mode)
{
	printf("\nStage-3: \"panel_data_dump_mode\" enabled. one-to-mul keys...\n");

	printf("ON PROCEDURE\n");
	on_and_tv_to_pc_procedure(device_fd, new_e2prom_tab_p, 1);
	printf("TV_to_PC PROCEDURE\n");
	on_and_tv_to_pc_procedure(device_fd, new_e2prom_tab_p, 0);
	printf("PC_to_TV PROCEDURE\n");
	pc_to_tv_procedure(device_fd, new_e2prom_tab_p);
}

static void pc_to_tv_procedure(int device_fd, struct eeprom_table *new_e2prom_tab_p)
{
#define DIGIT_BUF_SIZE	10
	unsigned int digit;
	char digitbuf[DIGIT_BUF_SIZE];
	int idx = PROCEDURE_SIZE * 2;

	printf("TV transition? Digit[0] / TV[1] -> Use your keyboard.\n");
	if (get_value("%u", 0, 2)) {
		new_e2prom_tab_p->procedure[idx++] = PROCEDURE_COUNT_DEFAULT |
			(wait_key_and_grab(device_fd, new_e2prom_tab_p, "TV") + 1);
		printf("Should TV start at a specific channel number?(Optional) No[0] / Yes[1] -> Use your keyboard.\n");
		if (!get_value("%u", 0, 2))
			return;
	}
	printf("Which digit to use -> Use your keyboard.\n");
	digit = get_value("%u", 0, 10) + '0';
	//printf("digit is %c\n", digit);
	strcpy(digitbuf, (char *)&digit);
	new_e2prom_tab_p->procedure[idx++] = PROCEDURE_COUNT_DEFAULT |
			(wait_key_and_grab(device_fd, new_e2prom_tab_p, digitbuf) + 1);
}

static void on_and_tv_to_pc_procedure(int device_fd, struct eeprom_table *new_e2prom_tab_p, int onflag)
{
	unsigned int digit, count, src_or_mult_idx;
	char digitbuf[DIGIT_BUF_SIZE];
	int idx = PROCEDURE_SIZE;

	if (onflag) {
		idx = 0;
		new_e2prom_tab_p->procedure[idx++] = PROCEDURE_COUNT_DEFAULT |
			(wait_key_and_grab(device_fd, new_e2prom_tab_p, "ON") + 1);
	}

	printf("Is direct PC transition possible without entering a safe point? No[0] / Yes[1] -> Use your keyboard.\n");
	if (get_value("%u", 0, 2)) {
		new_e2prom_tab_p->procedure[idx++] = PROCEDURE_COUNT_DEFAULT |
			(magic_pc_transition_key(device_fd, new_e2prom_tab_p) + 1);
		goto out;
	}

	printf("Indirect PC transition => Safe point? Digit[0] / TV[1] -> Use your keyboard.\n");
	if (get_value("%u", 0, 2)) {
		new_e2prom_tab_p->procedure[idx++] = PROCEDURE_COUNT_DEFAULT |
			(wait_key_and_grab(device_fd, new_e2prom_tab_p, "TV") + 1);
	}
	else {
		printf("Which digit to use -> Use your keyboard.\n");
		digit = get_value("%u", 0, 10) + '0';
		//printf("digit is %c\n", digit);
		strcpy(digitbuf, (char *)&digit);
		new_e2prom_tab_p->procedure[idx++] = PROCEDURE_COUNT_DEFAULT |
			(wait_key_and_grab(device_fd, new_e2prom_tab_p, digitbuf) + 1);
	}
	printf("Is direct PC transition possible within the safe point? No[0] / Yes[1] -> Use your keyboard.\n");
	if (get_value("%u", 0, 2)) {
		new_e2prom_tab_p->procedure[idx++] = PROCEDURE_COUNT_DEFAULT |
			(magic_pc_transition_key(device_fd, new_e2prom_tab_p) + 1);
		goto out;
	}
	printf("Source[0] / Multimedia[1] -> Use your keyboard.\n");
	if (get_value("%u", 0, 2))
		src_or_mult_idx = wait_key_and_grab(device_fd, new_e2prom_tab_p, "MULTIMEDIA");
	else
		src_or_mult_idx = wait_key_and_grab(device_fd, new_e2prom_tab_p, "SOURCE");
	printf("Count number -> Use your keyboard.\n");
	count = get_value("%u", 1, 8);
	printf("count = %u\n", count);
	new_e2prom_tab_p->procedure[idx++] = count << PROCEDURE_COUNT_SHIFT |
		(src_or_mult_idx + 1);

out:
	idx = PROCEDURE_SIZE;
}

static int magic_pc_transition_key(int device_fd, struct eeprom_table *new_e2prom_tab_p)
{
	printf("Direct PC transition => Which key? TV_PC[0] / PC[1] -> Use your keyboard.\n");
	if (get_value("%u", 0, 2))
		return wait_key_and_grab(device_fd, new_e2prom_tab_p, "PC");
	else
		return wait_key_and_grab(device_fd, new_e2prom_tab_p, "TV_PC");
}

static void second_stage_dump(struct eeprom_table *e2prom_tab_p)
{
	int i;

		for (i = 0; i < PANEL_ENTRY_SIZE; i++) {
#if defined(CONFIG_EEPROM_TABLE_1) || defined(CONFIG_EEPROM_TABLE_2)
		printf("i2c: e2prom_tab_p->panel_tab[%s].panel_key = %x\n", idxtostr(keys, i), e2prom_tab_p->panel_tab[i].panel_key);
		printf("i2c: e2prom_tab_p->panel_tab[%s].wait_time = %x\n", idxtostr(keys, i), e2prom_tab_p->panel_tab[i].wait_time);
#endif
#ifdef CONFIG_EEPROM_TABLE_3
		printf("i2c: e2prom_tab_p->panel_irda_key[%s] = %x\n", idxtostr(keys, i), e2prom_tab_p->panel_irda_key[i]);
		printf("i2c: e2prom_tab_p->panel_wait_time[%s] = %x\n", idxtostr(keys, i), e2prom_tab_p->panel_wait_time[i]);
#endif
	}
}

static void second_stage(int device_fd, struct eeprom_table *new_e2prom_tab_p, int dump_mode)
{
	printf("\nStage-2: \"panel_data_dump_mode\" enabled. one-to-one keys...\n");

	wait_key_and_grab(device_fd, new_e2prom_tab_p, "OFF");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "0");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "1");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "2");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "3");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "4");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "5");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "6");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "7");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "8");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "9");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "VOL_UP");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "VOL_DOWN");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "MUTE");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "MENU");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "PROG_UP");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "PROG_DOWN");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "BACKWARD");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "FORWARD");
	wait_key_and_grab(device_fd, new_e2prom_tab_p, "OK");

	return;
}

static int wait_key_and_grab(int device_fd, struct eeprom_table *new_e2prom_tab_p, char *str)
{
	__u32 irda_data = 0;
	int result, idx;
	__u32 panel_wait_time;

	printf("panel_irda_key -> Use your panel remote controller.\n");
	printf("Press " "%s\n", str);
	result = read(device_fd, &irda_data, sizeof(irda_data));
	//printf("i2c: result = %d\n", result);
	if (result < 0) {
		fprintf(stderr, "i2c: cannot read the device\n");
		return -1; //ERR_READ_MCU
	}
	else if (result == 0) {
		fprintf(stderr, "i2c: there is no data\n");
		return -1;
	}
	else {
		printf("i2c: irda_data = %X\n", irda_data);
	}
	idx = strtoidx(keys, str, sizeof(keys) / sizeof(*keys));
	new_e2prom_tab_p->panel_irda_key[idx] = irda_data;
	printf("panel_wait_time as ms[0 - 0xFFFF] -> Use your keyboard.\n");
	panel_wait_time = get_value("%X", 0, 0x10000);
	printf("i2c: panel_wait_time = %X\n", panel_wait_time);
	new_e2prom_tab_p->panel_wait_time[idx] = panel_wait_time;

	return idx;
}

static int strtoidx(char **strp, const char *str, int len)
{
	int i;

	for (i = 0; i < len; i++)
		if (!strncmp(strp[i], str, strlen(strp[i]))) {
			printf("i2c: idx = %d\n", i);
			return i;
		}

	return -1;
}

static char *idxtostr(char **strp, int idx)
{
	return strp[idx];
}

static void first_stage_dump(struct eeprom_table *e2prom_tab_p)
{
	int i;

	for (i = 0; i < IRDA_CODING_SIZE; i++)
		printf("i2c: e2prom_tab_p->irda_coding[%s] = %x\n", idxtostr(irda_coding_str, i), e2prom_tab_p->irda_coding[i]);
	for (i = 0; i < PANEL_OPTIONS_SIZE; i++)
		printf("i2c: e2prom_tab_p->panel_options[%d] = %x\n", i, e2prom_tab_p->panel_options[i]);
}

static void first_stage(int device_fd, struct eeprom_table *new_e2prom_tab_p, int dump_mode)
{
#define LEARNING_FMT_SIZE	100
	unsigned int receive_or_send = 0, min = 0, size = 2;
	unsigned int pwm_or_manch = 0;
	unsigned int length;
	unsigned int sync_pulse_width;
	unsigned int repeat_on_off;
	unsigned int panel_on_state;
	unsigned int start_timeout;
	char fmt[LEARNING_FMT_SIZE] = "receive[0] / send[1]\n";
	int i;

	memset(new_e2prom_tab_p->irda_coding, 0, (IRDA_CODING_SIZE + PANEL_OPTIONS_SIZE));
	if (dump_mode) {
		printf("\nStage-1: in order to enter \"panel_data_dump_mode\", please "
				"give panel coding parameters...\n");
	}
	else {
		printf("\nStage-4: receive and send parameters...\n");
	}
	printf("Use your keyboard.\n");
	for (i = 0; i < 2; i++) {
			if (!dump_mode) {
				printf(fmt);
				receive_or_send = get_value("%u", min, size);
				printf("i2c: receve_or_send = %u\n", receive_or_send);
				if (receive_or_send) {
					strcpy(fmt, "receive[0]\n");
					size--;
				}
				else {
					strcpy(fmt, "send[1]\n");
					min++;
					size--;
				}
			}

			printf("pwm[0] / manchester[1]\n");
			pwm_or_manch = get_value("%u", 0, 2);
			printf("i2c: pwm_or_manch = %u\n", pwm_or_manch);
			((__u16 *)(new_e2prom_tab_p->irda_coding))[receive_or_send] |=
				pwm_or_manch << IRDA_CODING_PWM_OR_MANCH_SHIFT;

			printf("length[1 - 63]\n");
			length = get_value("%u", 1, 63);
			printf("i2c: length = %u\n", length);
			((__u16 *)(new_e2prom_tab_p->irda_coding))[receive_or_send] |=
				length << IRDA_CODING_LENGTH_SHIFT;

			/* pwm */
			if (!pwm_or_manch) {
				printf("sync pulse width: 4.5ms-Samsung-[0] / 9ms-LG-[1]\n");
				sync_pulse_width = get_value("%u", 0, 2);
				printf("i2c: sync_pulse_width = %u\n", sync_pulse_width);
				((__u16 *)(new_e2prom_tab_p->irda_coding))[receive_or_send] |=
					sync_pulse_width << IRDA_CODING_SYNC_PULSE_WIDTH;
			}

			if (!dump_mode) {
				/* send and pwm */
				if (receive_or_send && !pwm_or_manch) {
					printf("repeat on off: disable[0] / enable[1]\n");
					repeat_on_off = get_value("%u", 0, 2);
					printf("i2c: repeat_on_off = %u\n", repeat_on_off);
					((__u16 *)(new_e2prom_tab_p->irda_coding))[receive_or_send] |=
						repeat_on_off << IRDA_CODING_REPEAT_ON_OFF;
				}
			}

			if (dump_mode)
				break;
	}

	/* panel options */
	printf("Panel Options\n");
	printf("panel on state: [0] / [1]\n");
	panel_on_state = get_value("%u", 0, 2);
	printf("i2c: panel_on_state = %u\n", panel_on_state);
	(*((__u32 *)(new_e2prom_tab_p->panel_options))) |=
		panel_on_state << PANEL_OPTIONS_ON_STATE_SHIFT;
	if (!dump_mode) {
		printf("start timeout as minutes: [0] / [7]\n");
		start_timeout = get_value("%u", 0, 8);
		printf("i2c: start_timeout = %u\n", start_timeout);
		(*((__u32 *)(new_e2prom_tab_p->panel_options))) |=
			start_timeout << PANEL_OPTIONS_START_TIMEOUT_SHIFT;
	}

	return;
}

static unsigned int get_value(const char *conv_specifier, unsigned int min, unsigned int size)
{
	unsigned int value;
	char fmt[LEARNING_FMT_SIZE];

	sprintf(fmt, "Wrong value. Must be %s - %s\n", conv_specifier, conv_specifier);

	while (1) {
		if (!scanf(conv_specifier, &value)) {
			getchar();
			continue;
		}
		//printf("i2c: value = %u\n", value);
		if ((value >= min) && (value < (min + size)))
			break;
		else {
			printf(fmt, min, (min + size - 1));
		}
	}

	return value;
}

static int stage_wrapper(char *str,
			 void (*stage)(int device_fd, struct eeprom_table *new_e2prom_tab_p, int dump_mode),
			 void (*stage_dump)(struct eeprom_table *new_e2prom_tab_p),
			 int device_fd, struct eeprom_table *new_e2prom_tab_p, int dump_mode)
{
	unsigned int ret;

	if (dump_mode) {
		ret = epilogue(str);
		if (ret == 1)
			return 1;
	}
	while (1) {
		stage(device_fd, new_e2prom_tab_p, dump_mode);
summary:
		ret = prologue(str);
		if (!ret)
			break;
		else if (ret == 2) {
			stage_dump(new_e2prom_tab_p);
			goto summary;
		}
	}

	return 0;
}

static int epilogue(char *str)
{
	printf("\nYou are about to enter the %s...\n", str);
	printf("If you want to continue press [0], skip the stage press [1]\n");
	printf("Use your keyboard.\n");
	return get_value("%u", 0, 2);
}

static int prologue(char *str)
{
	printf("\nYou are about to exit the %s...\n", str);
	printf("If you want to pass one step ahead press [0], repeat the step press [1], ");
	printf("for summary press [2]\n");
	printf("Use your keyboard.\n");
	return get_value("%u", 0, 3);
}

static int test_eeprom_read_cmd(int device_fd)
{
	int result, i;
	int counter = 0;

	//printf("i2c: test_eeprom_read_cmd\n");

	if (param.io.addr + param.io.length >= EEPROM_SIZE) {
		fprintf(stderr, "i2c: addr + length >= EEPROM_SIZE\n");
		return ERR_INVALID_ARG;
	}

	param.io.buf = alloc_mem(param.io.length);
	if (!param.io.buf)
		return ERR_ALLOC_MEM;
	param.io.value = 0;
	memset(param.io.buf, param.io.value, param.io.length);
	//buf[len] = 'A';

	for (i = 0; i < 1; i++) {
		result = ioctl(device_fd, REMOTE_READ_EEPROM_CMD, &param.io);
		printf("i2c: ioctl result = %d\n", result);
		if (result < 0) {
			fprintf(stderr, "i2c: cannot read the eeprom\n");
			counter++;
			//return ERR_READ_EEPROM;
		}
		else
			dump_data_hex(param.io.buf, param.io.length);
	}
	printf("i2c: i = %d\n", i);
	printf("i2c: counter = %d\n", counter);

	free_mem(param.io.buf);

	return 0;
}

static int test_eeprom_write_cmd(int device_fd)
{
	int result, i;
	int counter = 0;

	//printf("i2c: test_eeprom_write_cmd\n");

	if (param.io.addr + param.io.length >= EEPROM_SIZE) {
		fprintf(stderr, "i2c: addr + length >= EEPROM_SIZE\n");
		return ERR_INVALID_ARG;
	}

	param.io.buf = alloc_mem(param.io.length);
	if (!param.io.buf)
		return ERR_ALLOC_MEM;
	memset(param.io.buf, param.io.value, param.io.length);
	//buf[len] = 'A';

	for (i = 0; i < 1; i++) {
		result = ioctl(device_fd, REMOTE_WRITE_EEPROM_CMD, &param.io);
		printf("i2c: ioctl result = %d\n", result);
		if (result < 0) {
			fprintf(stderr, "i2c: cannot write to the eeprom\n");
			counter++;
			//return ERR_WRITE_EEPROM;
		}
		//else
			//dump_data_hex(param.io.buf, param.io.length);
	}
	//printf("i2c: i = %d\n", i);
	printf("i2c: counter = %d\n", counter);

	free_mem(param.io.buf);

	return 0;
}

static int test_mcu_read_cmd(int device_fd)
{
	int result, i;
	int counter = 0;

	//printf("i2c: test_mcu_read_cmd\n");

	param.io.buf = alloc_mem(param.io.length);
	if (!param.io.buf)
		return ERR_ALLOC_MEM;
	param.io.value = 0;
	memset(param.io.buf, param.io.value, param.io.length);
	//buf[len] = 'A';

	for (i = 0; i < param.io.count; i++) {
		result = ioctl(device_fd, REMOTE_READ_MCU_CMD, &param.io);
		//printf("i2c: ioctl result = %d\n", result);
		if (result < 0) {
			fprintf(stderr, "i2c: cannot read the mcu\n");
			counter++;
			//return ERR_READ_MCU;
		}
		else
			dump_data_hex(param.io.buf, param.io.length);
	}
	//printf("i2c: i = %d\n", i);
	printf("i2c: counter = %d\n", counter);

	free_mem(param.io.buf);

	return 0;
}

static int test_mcu_write_cmd(int device_fd)
{
	int result, i;
	int counter = 0;

	//printf("i2c: test_mcu_write_cmd\n");

	param.io.buf = alloc_mem(param.io.length);
	if (!param.io.buf)
		return ERR_ALLOC_MEM;
	memset(param.io.buf, param.io.value, param.io.length);
	//buf[param.length] = 'A';

	for (i = 0; i < param.io.count; i++) {
		result = ioctl(device_fd, REMOTE_WRITE_MCU_CMD, &param.io);
		printf("i2c: ioctl result = %d\n", result);
		if (result < 0) {
			fprintf(stderr, "i2c: cannot write to the mcu\n");
			counter++;
			//return ERR_WRITE_MCU;
		}
		//else
			//dump_data_hex(param.io.buf, param.io.length);
	}
	printf("i2c: counter = %d\n", counter);

	free_mem(param.io.buf);

	return 0;
}

static char *alloc_mem(short length)
{
	char *buf = NULL;

	buf = (char *)malloc((size_t)length + 1);
	if (!buf) {
		fprintf(stderr, "i2c: cannot allocate memory\n");
		perror("i2c: parse_cmd");
	}

	return buf;
}

static void free_mem(char *buf)
{
	free(buf);
}

static void dump_config_data(struct eeprom_table *e2prom_tab_p)
{
	int i;

	for (i = 0; i < SIGNATURE_SIZE; i++)
		printf("i2c: e2prom_tab_p->reserved[%d] = %x\n", i, e2prom_tab_p->reserved[i]);
	for (i = 0; i < PANEL_ENTRY_SIZE; i++) {
#if defined(CONFIG_EEPROM_TABLE_1) || defined(CONFIG_EEPROM_TABLE_2)
		printf("i2c: e2prom_tab_p->panel_tab[%d].panel_key = %x\n", i, e2prom_tab_p->panel_tab[i].panel_key);
		printf("i2c: e2prom_tab_p->panel_tab[%d].wait_time = %x\n", i, e2prom_tab_p->panel_tab[i].wait_time);
#endif
#ifdef CONFIG_EEPROM_TABLE_3
		printf("i2c: e2prom_tab_p->panel_irda_key[%d] = %x\n", i, e2prom_tab_p->panel_irda_key[i]);
		printf("i2c: e2prom_tab_p->panel_wait_time[%d] = %x\n", i, e2prom_tab_p->panel_wait_time[i]);
#endif
	}
	for (i = 0; i < ONE_TO_MUL_MAPPING_SIZE; i++)
		printf("i2c: e2prom_tab_p->procedure[%d] = %x\n", i, e2prom_tab_p->procedure[i]);
	for (i = 0; i < IRDA_CODING_SIZE; i++)
		printf("i2c: e2prom_tab_p->irda_coding[%d] = %x\n", i, e2prom_tab_p->irda_coding[i]);
	for (i = 0; i < PANEL_OPTIONS_SIZE; i++)
		printf("i2c: e2prom_tab_p->panel_options[%d] = %x\n", i, e2prom_tab_p->panel_options[i]);
	for (i = 0; i < IRDA_KEY_SIZE; i++)
		printf("i2c: e2prom_tab_p->irda_tab[%d] = %x\n", i, e2prom_tab_p->irda_tab[i]);
}

static void dump_data_hex(unsigned char *buf, int size)
{
	int i;

	printf("i2c: ");
	for (i = 0; i < size; i++)
		printf("%x ", buf[i]);
	printf("\n");
}

static void dump_data_dec(unsigned char *buf, int size)
{
	int i;

	for (i = 0; i < size; i++)
		printf("%c", buf[i]);
	printf("\n");
}

static int parse_cmd(int device_fd)
{
	int result = 0;
	unsigned char *buf;

	switch (param.cmd) {
		case READ_DEVICE_CMD:
			result = read_device_cmd(device_fd); break;
		case READ_FILE_CMD:
			result = read_file_cmd(device_fd); break;
		case READ_POLL_CMD:
			result = read_poll_cmd(device_fd); break;
		case CONFIG_DATA_WRITE_CMD:
			result = config_data_write_cmd(device_fd); break;
		case CONFIG_DATA_READ_CMD:
			result = config_data_read_cmd(device_fd); break;
		/*
		case CONFIG_DATA_E2_READ_CMD:
			result = config_data_read_e2_cmd(device_fd); break;
		*/
		case TEST_MCU_READ_CMD:
			result = test_mcu_read_cmd(device_fd); break;
		case TEST_MCU_WRITE_CMD:
			result = test_mcu_write_cmd(device_fd); break;
		case TEST_EEPROM_READ_CMD:
			result = test_eeprom_read_cmd(device_fd); break;
		case TEST_EEPROM_WRITE_CMD:
			result = test_eeprom_write_cmd(device_fd); break;
		case WELCOME_CMD:
			result = welcome_cmd(device_fd); break;
		case START_CMD:
			result = start_cmd(device_fd); break;
		case STOP_CMD:
			result = stop_cmd(device_fd); break;
		case CONFIG_DATA_INVALID_CMD:
			result = cfg_invalid_cmd(device_fd); break;
		case LEARNING_CMD:
			result = learning_cmd(device_fd); break;
	}

	return result;
}

static int parse_args(int argc, char *argv[])
{
	int len;

	if (argc < 2) {
		show_help();
		return ERR_PARSE_ARGS;
	}

	len = strlen(argv[1]);
	if (!strncmp(argv[1], "readdev", len)) {
		if (argc != 2 || len != 7) {
			show_help();
			return ERR_PARSE_ARGS;
		}
		param.cmd = READ_DEVICE_CMD;
	}
	else if (!strncmp(argv[1], "readfile", len)) {
		if (argc != 3 || len != 8) {
			show_help();
			return ERR_PARSE_ARGS;
		}
		strncpy(param.irda_key_file, argv[2], FILE_NAME_SIZE);
		//printf("i2c: irda_key_file = %s\n", param.irda_key_file);
		param.cmd = READ_FILE_CMD;
	}
	else if (!strncmp(argv[1], "readpoll", len)) {
		if (argc != 2 || len != 8) {
			show_help();
			return ERR_PARSE_ARGS;
		}
		param.cmd = READ_POLL_CMD;
	}
	else if (!strncmp(argv[1], "mcuread", len)) {
		if (argc != 4 || len != 7) {
			show_help();
			return ERR_PARSE_ARGS;
		}
		param.io.count = (short)atoi(argv[2]);
		param.io.length = (short)atoi(argv[3]);
		//printf("i2c: length = %d\n", param.io.length);
		param.cmd = TEST_MCU_READ_CMD;
	}
	else if (!strncmp(argv[1], "mcuwrite", len)) {
		if (argc != 5 || len != 8) {
			show_help();
			return ERR_PARSE_ARGS;
		}
		param.io.count = (short)atoi(argv[2]);
		param.io.length = (short)atoi(argv[3]);
		param.io.value = (short)atoi(argv[4]);
		//printf("i2c: length = %d\n", param..io.length);
		//printf("i2c: value = %d\n", param.io.value);
		param.cmd = TEST_MCU_WRITE_CMD;
	}
	else if (!strncmp(argv[1], "configwrite", len)) {
		if (argc != 4 || len != 11) {
			show_help();
			return ERR_PARSE_ARGS;
		}
		strncpy(param.irda_key_file, argv[2], FILE_NAME_SIZE);
		strncpy(param.panel_file, argv[3], FILE_NAME_SIZE);
		//printf("i2c: irda_key_file = %s\n", param.irda_key_file);
		//printf("i2c: panel_file = %s\n", param.panel_file);
		param.cmd = CONFIG_DATA_WRITE_CMD;
	}
	else if (!strncmp(argv[1], "configread", len)) {
		if (argc != 2 || len != 10) {
			show_help();
			return ERR_PARSE_ARGS;
		}
		param.cmd = CONFIG_DATA_READ_CMD;
	}
	else if (!strncmp(argv[1], "e2read", len)) {
		if (argc != 4 || len != 6) {
			show_help();
			return ERR_PARSE_ARGS;
		}
		param.io.addr = (short)strtoul(argv[2], NULL, 16);
		param.io.length = (short)strtoul(argv[3], NULL, 10);
		//printf("i2c: addr = %x\n", param.io.addr);
		//printf("i2c: length = %d\n", param.io.length);
		param.cmd = TEST_EEPROM_READ_CMD;
	}
	else if (!strncmp(argv[1], "e2write", len)) {
		if (argc != 5 || len != 7) {
			show_help();
			return ERR_PARSE_ARGS;
		}
		param.io.addr = (short)strtoul(argv[2], NULL, 16);
		param.io.length = (short)strtoul(argv[3], NULL, 10);
		param.io.value = (short)strtoul(argv[4], NULL, 16);
		//printf("i2c: addr = %x\n", param.io.addr);
		//printf("i2c: length = %d\n", param.io.length);
		//printf("i2c: value = %d\n", param.io.value);
		param.cmd = TEST_EEPROM_WRITE_CMD;
	}
	else if (!strncmp(argv[1], "welcome", len)) {
		if (argc != 2 || len != 7) {
			show_help();
			return ERR_PARSE_ARGS;
		}
		param.cmd = WELCOME_CMD;
	}
	else if (!strncmp(argv[1], "start", len)) {
		if (argc != 2 || len != 5) {
			show_help();
			return ERR_PARSE_ARGS;
		}
		param.cmd = START_CMD;
	}
	else if (!strncmp(argv[1], "stop", len)) {
		if (argc != 2 || len != 4) {
			show_help();
			return ERR_PARSE_ARGS;
		}
		param.cmd = STOP_CMD;
	}
	else if (!strncmp(argv[1], "cfg_invalid", len)) {
		if (argc != 2 || len != 11) {
			show_help();
			return ERR_PARSE_ARGS;
		}
		param.cmd = CONFIG_DATA_INVALID_CMD;
	}
	else if (!strncmp(argv[1], "learning", len)) {
		if (argc != 3 || len != 8) {
			show_help();
			return ERR_PARSE_ARGS;
		}
		strncpy(param.irda_key_file, argv[2], FILE_NAME_SIZE);
		param.cmd = LEARNING_CMD;
	}
	else {
		show_help();
		return ERR_PARSE_ARGS;
	}

	return 0;
}

static void show_help(void)
{
	fprintf(stderr, "Wrong number of arguments!..\n");

	fprintf(stderr, "Usage: readdev\n");
	fprintf(stderr, "Usage: readfile irda_key_file\n");
	fprintf(stderr, "Usage: readpoll\n");

	fprintf(stderr, "Usage: mcuread count len\t");
	fprintf(stderr, "Example: mcuread 1 20\n");
	fprintf(stderr, "Usage: mcuwrite count len val\t");
	fprintf(stderr, "Example: mcuwrite 1 20 100\n");

	fprintf(stderr, "Usage: configwrite irda_key_file panel_file\n");
	fprintf(stderr, "Usage: configread\n");

	fprintf(stderr, "Usage: e2read addr len\n");
	fprintf(stderr, "Example: e2read 0x0 20 - ");
	fprintf(stderr, "Address range: 0x00 - 0x1FF\n");
	fprintf(stderr, "Usage: e2write addr len val\n");
	fprintf(stderr, "Example: e2write 0x0 16 0x10 - ");
	fprintf(stderr, "Address range: 0x00 - 0x1FF :: ");
	fprintf(stderr, "Value range: 0x00 - 0xFF\n");

	fprintf(stderr, "Usage: welcome\n");
	fprintf(stderr, "Usage: start\n");
	fprintf(stderr, "Usage: stop\n");
	fprintf(stderr, "Usage: cfg_invalid\n");

	fprintf(stderr, "Usage: learning irda_key_file\n");
}

