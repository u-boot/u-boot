/*
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
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
 */

#ifndef _IMXIMAGE_H_
#define _IMXIMAGE_H_

#define MAX_HW_CFG_SIZE_V2 121 /* Max number of registers imx can set for v2 */
#define MAX_HW_CFG_SIZE_V1 60  /* Max number of registers imx can set for v1 */
#define APP_CODE_BARKER	0xB1
#define DCD_BARKER	0xB17219E9

#define HEADER_OFFSET	0x400

#define CMD_DATA_STR	"DATA"
#define FLASH_OFFSET_STANDARD	0x400
#define FLASH_OFFSET_NAND	FLASH_OFFSET_STANDARD
#define FLASH_OFFSET_SD		FLASH_OFFSET_STANDARD
#define FLASH_OFFSET_SPI	FLASH_OFFSET_STANDARD
#define FLASH_OFFSET_ONENAND	0x100

#define IVT_HEADER_TAG 0xD1
#define IVT_VERSION 0x40
#define DCD_HEADER_TAG 0xD2
#define DCD_COMMAND_TAG 0xCC
#define DCD_VERSION 0x40
#define DCD_COMMAND_PARAM 0x4

enum imximage_cmd {
	CMD_INVALID,
	CMD_IMAGE_VERSION,
	CMD_BOOT_FROM,
	CMD_DATA
};

enum imximage_fld_types {
	CFG_INVALID = -1,
	CFG_COMMAND,
	CFG_REG_SIZE,
	CFG_REG_ADDRESS,
	CFG_REG_VALUE
};

enum imximage_version {
	IMXIMAGE_VER_INVALID = -1,
	IMXIMAGE_V1 = 1,
	IMXIMAGE_V2
};

typedef struct {
	uint32_t type; /* Type of pointer (byte, halfword, word, wait/read) */
	uint32_t addr; /* Address to write to */
	uint32_t value; /* Data to write */
} dcd_type_addr_data_t;

typedef struct {
	uint32_t barker; /* Barker for sanity check */
	uint32_t length; /* Device configuration length (without preamble) */
} dcd_preamble_t;

typedef struct {
	dcd_preamble_t preamble;
	dcd_type_addr_data_t addr_data[MAX_HW_CFG_SIZE_V1];
} dcd_v1_t;

typedef struct {
	uint32_t app_code_jump_vector;
	uint32_t app_code_barker;
	uint32_t app_code_csf;
	uint32_t dcd_ptr_ptr;
	uint32_t super_root_key;
	uint32_t dcd_ptr;
	uint32_t app_dest_ptr;
} flash_header_v1_t;

typedef struct {
	uint32_t length; 	/* Length of data to be read from flash */
} flash_cfg_parms_t;

typedef struct {
	flash_header_v1_t fhdr;
	dcd_v1_t dcd_table;
	flash_cfg_parms_t ext_header;
} imx_header_v1_t;

typedef struct {
	uint32_t addr;
	uint32_t value;
} dcd_addr_data_t;

typedef struct {
	uint8_t tag;
	uint16_t length;
	uint8_t version;
} __attribute__((packed)) ivt_header_t;

typedef struct {
	uint8_t tag;
	uint16_t length;
	uint8_t param;
} __attribute__((packed)) write_dcd_command_t;

typedef struct {
	ivt_header_t header;
	write_dcd_command_t write_dcd_command;
	dcd_addr_data_t addr_data[MAX_HW_CFG_SIZE_V2];
} dcd_v2_t;

typedef struct {
	uint32_t start;
	uint32_t size;
	uint32_t plugin;
} boot_data_t;

typedef struct {
	ivt_header_t header;
	uint32_t entry;
	uint32_t reserved1;
	uint32_t dcd_ptr;
	uint32_t boot_data_ptr;
	uint32_t self;
	uint32_t csf;
	uint32_t reserved2;
} flash_header_v2_t;

typedef struct {
	flash_header_v2_t fhdr;
	boot_data_t boot_data;
	dcd_v2_t dcd_table;
} imx_header_v2_t;

struct imx_header {
	union {
		imx_header_v1_t hdr_v1;
		imx_header_v2_t hdr_v2;
	} header;
	uint32_t flash_offset;
};

typedef void (*set_dcd_val_t)(struct imx_header *imxhdr,
					char *name, int lineno,
					int fld, uint32_t value,
					uint32_t off);

typedef void (*set_dcd_rst_t)(struct imx_header *imxhdr,
					uint32_t dcd_len,
					char *name, int lineno);

typedef void (*set_imx_hdr_t)(struct imx_header *imxhdr,
					uint32_t dcd_len,
					struct stat *sbuf,
					struct mkimage_params *params);

#endif /* _IMXIMAGE_H_ */
