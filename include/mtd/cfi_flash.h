/*
 * (C) Copyright 2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#ifndef __CFI_FLASH_H__
#define __CFI_FLASH_H__

#define FLASH_CMD_CFI			0x98
#define FLASH_CMD_READ_ID		0x90
#define FLASH_CMD_RESET			0xff
#define FLASH_CMD_BLOCK_ERASE		0x20
#define FLASH_CMD_ERASE_CONFIRM		0xD0
#define FLASH_CMD_WRITE			0x40
#define FLASH_CMD_PROTECT		0x60
#define FLASH_CMD_PROTECT_SET		0x01
#define FLASH_CMD_PROTECT_CLEAR		0xD0
#define FLASH_CMD_CLEAR_STATUS		0x50
#define FLASH_CMD_READ_STATUS		0x70
#define FLASH_CMD_WRITE_TO_BUFFER	0xE8
#define FLASH_CMD_WRITE_BUFFER_PROG	0xE9
#define FLASH_CMD_WRITE_BUFFER_CONFIRM	0xD0

#define FLASH_STATUS_DONE		0x80
#define FLASH_STATUS_ESS		0x40
#define FLASH_STATUS_ECLBS		0x20
#define FLASH_STATUS_PSLBS		0x10
#define FLASH_STATUS_VPENS		0x08
#define FLASH_STATUS_PSS		0x04
#define FLASH_STATUS_DPS		0x02
#define FLASH_STATUS_R			0x01
#define FLASH_STATUS_PROTECT		0x01

#define AMD_CMD_RESET			0xF0
#define AMD_CMD_WRITE			0xA0
#define AMD_CMD_ERASE_START		0x80
#define AMD_CMD_ERASE_SECTOR		0x30
#define AMD_CMD_UNLOCK_START		0xAA
#define AMD_CMD_UNLOCK_ACK		0x55
#define AMD_CMD_WRITE_TO_BUFFER		0x25
#define AMD_CMD_WRITE_BUFFER_CONFIRM	0x29

#define AMD_STATUS_TOGGLE		0x40
#define AMD_STATUS_ERROR		0x20

#define ATM_CMD_UNLOCK_SECT		0x70
#define ATM_CMD_SOFTLOCK_START		0x80
#define ATM_CMD_LOCK_SECT		0x40

#define FLASH_CONTINUATION_CODE		0x7F

#define FLASH_OFFSET_MANUFACTURER_ID	0x00
#define FLASH_OFFSET_DEVICE_ID		0x01
#define FLASH_OFFSET_DEVICE_ID2		0x0E
#define FLASH_OFFSET_DEVICE_ID3		0x0F
#define FLASH_OFFSET_CFI		0x55
#define FLASH_OFFSET_CFI_ALT		0x555
#define FLASH_OFFSET_CFI_RESP		0x10
#define FLASH_OFFSET_PRIMARY_VENDOR	0x13
/* extended query table primary address */
#define FLASH_OFFSET_EXT_QUERY_T_P_ADDR	0x15
#define FLASH_OFFSET_WTOUT		0x1F
#define FLASH_OFFSET_WBTOUT		0x20
#define FLASH_OFFSET_ETOUT		0x21
#define FLASH_OFFSET_CETOUT		0x22
#define FLASH_OFFSET_WMAX_TOUT		0x23
#define FLASH_OFFSET_WBMAX_TOUT		0x24
#define FLASH_OFFSET_EMAX_TOUT		0x25
#define FLASH_OFFSET_CEMAX_TOUT		0x26
#define FLASH_OFFSET_SIZE		0x27
#define FLASH_OFFSET_INTERFACE		0x28
#define FLASH_OFFSET_BUFFER_SIZE	0x2A
#define FLASH_OFFSET_NUM_ERASE_REGIONS	0x2C
#define FLASH_OFFSET_ERASE_REGIONS	0x2D
#define FLASH_OFFSET_PROTECT		0x02
#define FLASH_OFFSET_USER_PROTECTION	0x85
#define FLASH_OFFSET_INTEL_PROTECTION	0x81

#define CFI_CMDSET_NONE			0
#define CFI_CMDSET_INTEL_EXTENDED	1
#define CFI_CMDSET_AMD_STANDARD		2
#define CFI_CMDSET_INTEL_STANDARD	3
#define CFI_CMDSET_AMD_EXTENDED		4
#define CFI_CMDSET_MITSU_STANDARD	256
#define CFI_CMDSET_MITSU_EXTENDED	257
#define CFI_CMDSET_SST			258
#define CFI_CMDSET_INTEL_PROG_REGIONS	512

#ifdef CONFIG_SYS_FLASH_CFI_AMD_RESET /* needed for STM_ID_29W320DB on UC100 */
# undef  FLASH_CMD_RESET
# define FLASH_CMD_RESET	AMD_CMD_RESET /* use AMD-Reset instead */
#endif

#define NUM_ERASE_REGIONS	4 /* max. number of erase regions */

typedef union {
	unsigned char c;
	unsigned short w;
	unsigned long l;
	unsigned long long ll;
} cfiword_t;

/* CFI standard query structure */
struct cfi_qry {
	u8	qry[3];
	u16	p_id;
	u16	p_adr;
	u16	a_id;
	u16	a_adr;
	u8	vcc_min;
	u8	vcc_max;
	u8	vpp_min;
	u8	vpp_max;
	u8	word_write_timeout_typ;
	u8	buf_write_timeout_typ;
	u8	block_erase_timeout_typ;
	u8	chip_erase_timeout_typ;
	u8	word_write_timeout_max;
	u8	buf_write_timeout_max;
	u8	block_erase_timeout_max;
	u8	chip_erase_timeout_max;
	u8	dev_size;
	u16	interface_desc;
	u16	max_buf_write_size;
	u8	num_erase_regions;
	u32	erase_region_info[NUM_ERASE_REGIONS];
} __attribute__((packed));

struct cfi_pri_hdr {
	u8	pri[3];
	u8	major_version;
	u8	minor_version;
} __attribute__((packed));

void flash_write_cmd(flash_info_t * info, flash_sect_t sect,
		     uint offset, u32 cmd);

#endif /* __CFI_FLASH_H__ */
