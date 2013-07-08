/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _COMMON_UTIL_H_
#define _COMMON_UTIL_H_

typedef struct {
	char signature[4];
	char serial_name[17];	/* "MIP405_1000xxxxx" */
	char eth_addr[21];	/* "00:60:C2:0a:00:00" */
} backup_t;

extern flash_info_t flash_info[];	/* info for FLASH chips */

void get_backup_values(backup_t *buf);

#if defined(CONFIG_PIP405) || defined(CONFIG_MIP405)
#define BOOT_MPS	0x01
#define BOOT_PCI	0x02
int get_boot_mode(void);
void setup_cs_reloc(void);
#endif

void check_env(void);
#if defined(CONFIG_CMD_DOC)
void doc_init (void);
#endif

#endif /* _COMMON_UTIL_H_ */
