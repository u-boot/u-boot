/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#ifndef _MCEBU_CFG_EEPROM_H_
#define _MVEBU_CFG_EEPROM_H_
#include <common.h>
#include <i2c.h>
#include <errno.h>
#include <asm/io.h>

#define BOARD_DEV_TWSI_INIT_EEPROM		0x57
#define BOARD_HW_INFO_EEPROM_ADDR_LEN		2
#define BOARD_HW_INFO_EEPROM_DEV		0

enum mv_config_type_id {
	MV_CONFIG_CHECKSUM,
	MV_CONFIG_PATTERN,
	MV_CONFIG_LENGTH,
	MV_CONFIG_HW_INFO,
	MV_CONFIG_TYPE_MAX_OPTION,  /* limit for user read/write routines */
};

/* #pragma pack(1) */
#define MVEBU_HW_INFO_LEN			512
struct manufacturing_information_struct {
	u8 hw_info[MVEBU_HW_INFO_LEN];
};

/* #pragma pack(1) */
struct eeprom_struct {
	u32 checksum;
	u32 pattern;
	u16 length;
	struct manufacturing_information_struct man_info;
};

struct config_types_info {
	enum mv_config_type_id config_id;
	char name[30];
	u32 byte_num;
	u32 byte_cnt;
};

#define I2C_PAGE_WRITE_SIZE			32
#define EEPROM_STRUCT_SIZE			(sizeof(struct eeprom_struct))
#define HW_INFO_MAX_PARAM_NUM			32

struct hw_info_point_struct {
	char *name;
	char *value;
};

#define READ_SPECIFIC_FIELD			-1
#define HW_INFO_MAX_NAME_LEN			32
#define HW_INFO_MAX_VALUE_LEN			32
struct hw_info_data_struct {
	char name[HW_INFO_MAX_NAME_LEN];
	char value[HW_INFO_MAX_VALUE_LEN];
};

#define offset_in_eeprom(a)	((u32)(offsetof(struct eeprom_struct, a)))

/* MV_CONFIG_TYPE_ID */
/* {{configId,		name,		byte_num,	byte_cnt}} */
#define MV_EEPROM_CONFIG_INFO { \
{ MV_CONFIG_CHECKSUM,	"Checksum",	offset_in_eeprom(checksum),	   \
				sizeof(board_config_val.checksum)},	   \
{ MV_CONFIG_PATTERN,	"Pattern",	offset_in_eeprom(pattern),	   \
				sizeof(board_config_val.pattern)},	   \
{ MV_CONFIG_LENGTH,	"Data length",	offset_in_eeprom(length),	   \
				sizeof(board_config_val.length)},	   \
{ MV_CONFIG_HW_INFO,	"Box Info",	offset_in_eeprom(man_info.hw_info),\
				sizeof(board_config_val.man_info.hw_info)} \
}

#define CFG_DEFAULT_VALUE  {						     \
	0x00000000,				     /* checksum */	     \
	0xfecadefa,				     /* EEPROM pattern */    \
	EEPROM_STRUCT_SIZE,			     /* length = 0x10A B */  \
	{{[0 ... (MVEBU_HW_INFO_LEN - 1)] = 0x00} }   /* man info */	     \
}

int cfg_eeprom_init(void);
void cfg_eeprom_save(int length);
struct eeprom_struct *cfg_eeprom_get_board_config(void);
void cfg_eeprom_get_hw_info_str(uchar *hw_info_str);
void cfg_eeprom_set_hw_info_str(uchar *hw_info_str);
int cfg_eeprom_parse_hw_info(struct hw_info_data_struct *hw_info_data_array);
int cfg_eeprom_parse_env(struct hw_info_data_struct *data_array,
			 int size);
int cfg_eeprom_validate_name(char *name);

#endif /* _MVEBU_CFG_EEPROM_H_ */
