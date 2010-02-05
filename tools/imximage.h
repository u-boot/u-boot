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

#define MAX_HW_CFG_SIZE 60	/* Max number of registers imx can set */
#define MAX_EXP_SIZE	4
#define APP_CODE_BARKER	0xB1
#define DCD_BARKER	0xB17219E9
#define HEADER_OFFSET	0x400


#define CMD_DATA_STR	"DATA"
#define FLASH_OFFSET_STANDARD	0x400
#define FLASH_OFFSET_NAND	FLASH_OFFSET_STANDARD
#define FLASH_OFFSET_SD		FLASH_OFFSET_STANDARD
#define FLASH_OFFSET_SPI	FLASH_OFFSET_STANDARD
#define FLASH_OFFSET_ONENAND	0x100

enum imximage_cmd {
	CMD_INVALID,
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

typedef struct {
	uint8_t rsa_exponent[MAX_EXP_SIZE];	 /* RSA public exponent */
	uint8_t *rsa_modulus;			 /* RSA modulus pointer */
	uint16_t exponent_size;			 /* Exponent size (bytes) */
	uint16_t modulus_size;			 /* Modulus size (bytes) */
	uint8_t init_flag;			 /* key initialized */
} hab_rsa_public_key;

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
	dcd_type_addr_data_t addr_data[MAX_HW_CFG_SIZE];
} dcd_t;

typedef struct {
	uint32_t app_code_jump_vector;
	uint32_t app_code_barker;
	uint32_t app_code_csf;
	uint32_t dcd_ptr_ptr;
	uint32_t super_root_key;
	uint32_t dcd_ptr;
	uint32_t app_dest_ptr;
} flash_header_t;

typedef struct {
	uint32_t length; 	/* Length of data to be read from flash */
} flash_cfg_parms_t;

struct imx_header {
	flash_header_t fhdr;
	dcd_t dcd_table;
	flash_cfg_parms_t ext_header;
	uint32_t flash_offset;
};

struct reg_config {
	uint32_t raddr;
	uint32_t rdata;
};

#endif /* _IMXIMAGE_H_ */
