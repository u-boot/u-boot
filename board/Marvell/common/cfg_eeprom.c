// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <common.h>
#include <mvebu/cfg_eeprom.h>

struct eeprom_struct board_config_val = CFG_DEFAULT_VALUE;
struct config_types_info config_types_info[] = MV_EEPROM_CONFIG_INFO;
int eeprom_initialized = -1;
int g_board_hw_info = -1;

static char hw_info_param_list[][HW_INFO_MAX_NAME_LEN] = {
	"pcb_slm",
	"pcb_rev",
	"eco_rev",
	"pcb_sn",
	"ethaddr",
	"eth1addr",
	"eth2addr",
	"eth3addr",
	"eth4addr",
	"eth5addr",
	"eth6addr",
	"eth7addr",
	"eth8addr",
	"eth9addr"
};

static int hw_info_param_num = (sizeof(hw_info_param_list) /
				sizeof(hw_info_param_list[0]));

static u32 cfg_eeprom_checksum8(u8 *start, u32 len)
{
	u32 sum = 0;
	u8 *startp = start;

	do {
		sum += *startp;
		startp++;
		len--;
	} while (len > 0);
	return sum;
}

/* cfg_eeprom_get_config_type
 * config_info input pointer receive the mapping of the
 * required field in the local struct
 */
static bool cfg_eeprom_get_config_type(enum mv_config_type_id id,
				       struct config_types_info *config_info)
{
	int i;

	/* verify existence of requested config type, pull its data */
	for (i = 0; i < MV_CONFIG_TYPE_MAX_OPTION ; i++)
		if (config_types_info[i].config_id == id) {
			*config_info = config_types_info[i];
			return true;
		}
	pr_err("requested MV_CONFIG_TYPE_ID was not found (%d)\n", id);

	return false;
}

/* read specific field from EEPROM
 * @data_length: if equal to -1 read number of bytes as the length of the field.
 */
static void read_field_from_eeprom(enum mv_config_type_id id,
				   u8 *data, int data_length)
{
	struct config_types_info config_info;
	struct udevice *dev;
	int err;

	err = i2c_get_chip_for_busnum(BOARD_HW_INFO_EEPROM_DEV,
				      BOARD_DEV_TWSI_INIT_EEPROM,
				      BOARD_HW_INFO_EEPROM_ADDR_LEN,
				      &dev);
	if (err) {
		debug("%s: Cannot find EEPROM I2C chip\n", __func__);
		return;
	}

	if (!cfg_eeprom_get_config_type(id, &config_info)) {
		pr_err("Could not find field %x in EEPROM struct\n", id);
		return;
	}

	if (data_length == READ_SPECIFIC_FIELD)
		data_length = config_info.byte_cnt;

	dm_i2c_read(dev, config_info.byte_num, data, data_length);
}

/* cfg_eeprom_write_to_eeprom - write the global struct to EEPROM. */
int cfg_eeprom_write_to_eeprom(int length)
{
	int reserve_length, size_of_loop, i;
	struct udevice *dev;
	u8 *pattern = (u8 *)&board_config_val.pattern;
	int err;

	err = i2c_get_chip_for_busnum(BOARD_HW_INFO_EEPROM_DEV,
				      BOARD_DEV_TWSI_INIT_EEPROM,
				      BOARD_HW_INFO_EEPROM_ADDR_LEN,
				      &dev);
	if (err) {
		debug("%s: Cannot find EEPROM I2C chip\n", __func__);
		return err;
	}

	/* calculate checksum and save it in struct */
	board_config_val.checksum = cfg_eeprom_checksum8(pattern,
							 EEPROM_STRUCT_SIZE -
							 4);

	/* write fdt struct to EEPROM */
	size_of_loop = length / I2C_PAGE_WRITE_SIZE;
	reserve_length = length % I2C_PAGE_WRITE_SIZE;

	/* i2c support on page write with size 32-byets */
	for (i = 0; i < size_of_loop; i++) {
		u8 *buffer = (u8 *)&(board_config_val) +
						i * I2C_PAGE_WRITE_SIZE;
		dm_i2c_write(dev, i * I2C_PAGE_WRITE_SIZE,
			     buffer,
			     I2C_PAGE_WRITE_SIZE);
#ifdef CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS
		/* EEPROM write need delay, or cause write operation fail */
		udelay(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
#endif
	}

	/* write the reserve data from 32-bytes */
	if (reserve_length) {
		u8 *buffer = (u8 *)&(board_config_val) +
						i * I2C_PAGE_WRITE_SIZE;
		dm_i2c_write(dev, i * I2C_PAGE_WRITE_SIZE,
			     buffer,
			     reserve_length);
#ifdef CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS
		/* EEPROM write need delay */
		udelay(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
#endif
	}

	return 0;
}

/* cfg_eeprom_save - write the local struct to EEPROM */
void cfg_eeprom_save(int length)
{
	/* write local struct with fdt blob to EEPROM */
	cfg_eeprom_write_to_eeprom(length);
	/* reset g_board_id so it will get board ID from EEPROM again */
	g_board_hw_info = -1;
}

/* cfg_eeprom_get_board_config - return the whole board config
 * It is assumed the cfg_eeprom_init must be called prior to this routine,
 * otherwise static default configuration will be used.
 */
struct eeprom_struct *cfg_eeprom_get_board_config(void)
{
	return &board_config_val;
}

/* cfg_eeprom_get_hw_info_str - copy hw_info from cfg_eeprom to destination */
void cfg_eeprom_get_hw_info_str(uchar *hw_info_str)
{
	int len;

	/* hw_info isn't initialized, need to read hw_info from EEPROM */
	if (g_board_hw_info == -1) {
		u8 *hw_info = (u8 *)board_config_val.man_info.hw_info;
		/* read hw_info config from EEPROM */
		read_field_from_eeprom(MV_CONFIG_HW_INFO,
				       hw_info,
				       READ_SPECIFIC_FIELD);
	}
	len = strlen((const char *)board_config_val.man_info.hw_info);
	if (len >= MVEBU_HW_INFO_LEN)
		len = MVEBU_HW_INFO_LEN - 1;

	memcpy(hw_info_str, board_config_val.man_info.hw_info, len);
}

/* cfg_eeprom_set_hw_info_str - copy hw_info sting to cfg_eeprom module
 * It is assumed the cfg_eeprom_init must be called prior to this routine,
 * otherwise static default configuration will be used.
 */
void cfg_eeprom_set_hw_info_str(uchar *hw_info_str)
{
	int len;
	struct config_types_info config_info;

	/* read hw_info config from EEPROM */
	if (!cfg_eeprom_get_config_type(MV_CONFIG_HW_INFO, &config_info)) {
		pr_err("Could not find MV_CONFIG_hw_info\n");
		return;
	}

	len = strlen((const char *)hw_info_str);
	if (len >= config_info.byte_cnt)
		len = config_info.byte_cnt - 1;

	/* need to set all value to 0 at first for later string operation */
	memset(board_config_val.man_info.hw_info, 0, config_info.byte_cnt);
	memcpy(board_config_val.man_info.hw_info, hw_info_str, len);
}

/* cfg_eeprom_skip_space - skip the space character */
static char *cfg_eeprom_skip_space(char *buf)
{
	while ((buf[0] == ' ' || buf[0] == '\t'))
		++buf;
	return buf;
}

/*
 * cfg_eeprom_parse_hw_info
 * - parse the hw_info from string to name/value pairs
 */
int cfg_eeprom_parse_hw_info(struct hw_info_data_struct *hw_info_data_array)
{
	int count;
	char *name;
	char *value;
	int len;
	uchar hw_info_str[MVEBU_HW_INFO_LEN];

	/* need to set all to 0 for later string operation */
	memset(hw_info_str, 0, sizeof(hw_info_str));

	cfg_eeprom_get_hw_info_str(hw_info_str);
	name = (char *)hw_info_str;
	name = cfg_eeprom_skip_space(name);
	/* return 0 in case the string is empty */
	if (!name)
		return 0;

	for (count = 0; name; count++) {
		value = strchr(name, '=');

		if (!value)
			return count;

		*value = '\0';
		len = strlen(name);
		memcpy(hw_info_data_array[count].name, name, len);
		hw_info_data_array[count].name[len] = '\0';
		value++;

		name = strchr(value, ' ');
		if (!name)
			return ++count;

		*name = '\0';
		len = strlen(value);
		memcpy(hw_info_data_array[count].value, value, len);
		hw_info_data_array[count].value[len] = '\0';
		name = cfg_eeprom_skip_space(name + 1);
	}
	count++;

	return count;
}

/* cfg_eeprom_validate_name - check parameter's name is valid or not
 * valid - return 0
 * invalid - return -1
 */
int cfg_eeprom_validate_name(char *name)
{
	int idx;

	for (idx = 0; idx < hw_info_param_num; idx++) {
		if (strcmp(name, hw_info_param_list[idx]) == 0)
			return 0;
	}

	return -1;
}

/* cfg_eeprom_parse_env - parse the env from env to name/value pairs */
int cfg_eeprom_parse_env(struct hw_info_data_struct *data_array,
			 int size)
{
	int param_num = 0;
	int idx;
	int len;
	char *name;
	char *value;

	/* need to memset to 0 for later string operation */
	memset(data_array, 0, size);
	for (idx = 0; idx < hw_info_param_num; idx++) {
		name = hw_info_param_list[idx];
		value = env_get(name);

		if (!value) {
			printf("miss %s in env, please set it at first\n",
			       hw_info_param_list[idx]);
			continue;
		}

		len = strlen(name);
		if (len > HW_INFO_MAX_NAME_LEN)
			len  = HW_INFO_MAX_NAME_LEN;
		memcpy(data_array[param_num].name, name, len);
		len = strlen(value);
		if (len > HW_INFO_MAX_NAME_LEN)
			len  = HW_INFO_MAX_NAME_LEN;
		memcpy(data_array[param_num].value, value, len);

		param_num++;
	}

	return param_num;
}

/*
 * cfg_eeprom_init - initialize FDT configuration struct
 * The EEPROM FDT is used if 1) the checksum is valid, 2) the system
 * is not in recovery mode, 3) validation_counter < AUTO_RECOVERY_RETRY_TIMES
 * Otherwise the default FDT is used.
 */
int cfg_eeprom_init(void)
{
	struct eeprom_struct eeprom_buffer;
	u32 calculate_checksum;
	struct udevice *dev;
	u8 *pattern = (u8 *)&eeprom_buffer.pattern;
	int err;

	err = i2c_get_chip_for_busnum(BOARD_HW_INFO_EEPROM_DEV,
				      BOARD_DEV_TWSI_INIT_EEPROM,
				      BOARD_HW_INFO_EEPROM_ADDR_LEN,
				      &dev);
	if (err) {
		debug("%s: Cannot find EEPROM I2C chip\n", __func__);
		return err;
	}

	/* It is possible that this init will be called by several
	 * modules during init, however only need to initialize it
	 * for one time
	 */
	if (eeprom_initialized == 1)
		return 0;

	/* read pattern from EEPROM */
	read_field_from_eeprom(MV_CONFIG_PATTERN,
			       pattern,
			       READ_SPECIFIC_FIELD);

	/* check if pattern in EEPROM is invalid */
	if (eeprom_buffer.pattern != board_config_val.pattern) {
		printf("EEPROM configuration pattern not detected.\n");
		goto init_done;
	}

	/* read struct from EEPROM */
	err = dm_i2c_read(dev, 0,
			  (u8 *)&eeprom_buffer,
			  EEPROM_STRUCT_SIZE);
	if (err) {
		pr_err("read error from device: %p", dev);
		return err;
	}

	/* calculate checksum */
	calculate_checksum = cfg_eeprom_checksum8(pattern,
						  EEPROM_STRUCT_SIZE - 4);
	if (calculate_checksum == eeprom_buffer.checksum) {
		/* update board_config_val struct with read from EEPROM */
		board_config_val = eeprom_buffer;
	}

init_done:
	eeprom_initialized = 1;
	return 0;
}

