/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
#ifndef _COMMON_UTIL_H_
#define _COMMON_UTIL_H_

typedef struct {
	char signature[4];
	char serial_name[17];	/* "MIP405_1000xxxxx" */
	char eth_addr[21];	/* "00:60:C2:0a:00:00" */
} backup_t;

void get_backup_values(backup_t *buf);

#if defined(CONFIG_PIP405) || defined(CONFIG_MIP405)
#define BOOT_MPS	0x01
#define BOOT_PCI	0x02
#endif

void check_env(void);
#if defined(CONFIG_CMD_DOC)
void doc_init (void);
#endif

#endif /* _COMMON_UTIL_H_ */
