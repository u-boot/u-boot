/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 PHYTEC Messtechnik GmbH
 * Author: Teresa Remmet <t.remmet@phytec.de>
 */

#ifndef _PHYTEC_SOM_DETECTION_H
#define _PHYTEC_SOM_DETECTION_H

#include "phytec_som_detection_blocks.h"
#include <fdtdec.h>

#define PHYTEC_MAX_OPTIONS	17
#define PHYTEC_EEPROM_INVAL	0xff

#define PHYTEC_API2_DATA_LEN	32

#define PHYTEC_GET_OPTION(option) \
	(((option) > '9') ? (option) - 'A' + 10 : (option) - '0')

#define PHYTEC_PRODUCT_NAME_STD_LEN	7	// PCx-000
#define PHYTEC_PRODUCT_NAME_KSP_LEN	8	// KSP-0000
#define PHYTEC_PRODUCT_NAME_MAX_LEN	PHYTEC_PRODUCT_NAME_KSP_LEN
#define PHYTEC_PART_NUMBER_STD_LEN	11	// PCx-000-\w{1,17}.Ax
#define PHYTEC_PART_NUMBER_KSP_LEN	11	// KSP-0000.Ax
#define PHYTEC_PART_NUMBER_STD_KSP_LEN	16	// PCx-000-KSx00.Ax
#define PHYTEC_PART_NUMBER_MAX_LEN	PHYTEC_PRODUCT_NAME_MAX_LEN + 21

enum {
	PHYTEC_API_REV0 = 0,
	PHYTEC_API_REV1,
	PHYTEC_API_REV2,
	PHYTEC_API_REV3,
};

enum phytec_som_type_str {
	SOM_TYPE_PCM = 0,
	SOM_TYPE_PCL,
	SOM_TYPE_KSM,
	SOM_TYPE_KSP,
};

static const char * const phytec_som_type_str[] = {
	"PCM",
	"PCL",
	"KSM",
	"KSP",
};

struct phytec_api0_data {
	u8 pcb_rev;		/* PCB revision of SoM */
	u8 som_type;		/* SoM type */
	u8 ksp_no;		/* KSP no */
	char opt[16];		/* SoM options */
	u8 mac[6];		/* MAC address (optional) */
	u8 pad[5];		/* padding */
	u8 cksum;		/* checksum */
} __packed;

struct phytec_api2_data {
	u8 pcb_rev;		/* PCB revision of SoM */
	u8 pcb_sub_opt_rev;	/* PCB subrevision and opt revision */
	u8 som_type;		/* SoM type */
	u8 som_no;		/* SoM number */
	u8 ksp_no;		/* KSP information */
	char opt[PHYTEC_MAX_OPTIONS]; /* SoM options */
	char bom_rev[2];	/* BOM revision */
	u8 mac[6];		/* MAC address (optional) */
	u8 crc8;		/* checksum */
} __packed;

struct phytec_eeprom_payload {
	u8 api_rev;
	union {
		struct phytec_api0_data data_api0;
		struct phytec_api2_data data_api2;
	} data;
	struct phytec_api3_element *block_head;
} __packed;

struct phytec_eeprom_data {
	struct phytec_eeprom_payload payload;
	bool valid;
};

int phytec_eeprom_data_setup_fallback(struct phytec_eeprom_data *data,
				      int bus_num, int addr,
				      int addr_fallback);
int phytec_eeprom_data_setup(struct phytec_eeprom_data *data,
			     int bus_num, int addr);
int phytec_eeprom_data_init(struct phytec_eeprom_data *data, int bus_num,
			    int addr);
void __maybe_unused phytec_print_som_info(struct phytec_eeprom_data *data);

char * __maybe_unused phytec_get_opt(struct phytec_eeprom_data *data);
u8 __maybe_unused phytec_get_rev(struct phytec_eeprom_data *data);
u8 __maybe_unused phytec_get_som_type(struct phytec_eeprom_data *data);
#if IS_ENABLED(CONFIG_OF_LIBFDT)
int phytec_ft_board_fixup(struct phytec_eeprom_data *data, void *blob);
#endif /* IS_ENABLED(CONFIG_OF_LIBFDT) */

#if IS_ENABLED(CONFIG_CMD_EXTENSION)
struct extension *phytec_add_extension(const char *name, const char *overlay,
				       const char *other);
#endif /* IS_ENABLED(CONFIG_CMD_EXTENSION) */

struct phytec_api3_element *
	__maybe_unused phytec_get_block_head(struct phytec_eeprom_data *data);

#endif /* _PHYTEC_SOM_DETECTION_H */
