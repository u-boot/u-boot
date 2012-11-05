/*
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * (C) Copyright 2008
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
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

/* Required to obtain the getline prototype from stdio.h */
#define _GNU_SOURCE

#include "mkimage.h"
#include <image.h>
#include "imximage.h"

/*
 * Supported commands for configuration file
 */
static table_entry_t imximage_cmds[] = {
	{CMD_BOOT_FROM,         "BOOT_FROM",            "boot command",	  },
	{CMD_DATA,              "DATA",                 "Reg Write Data", },
	{CMD_IMAGE_VERSION,     "IMAGE_VERSION",        "image version",  },
	{-1,                    "",                     "",	          },
};

/*
 * Supported Boot options for configuration file
 * this is needed to set the correct flash offset
 */
static table_entry_t imximage_bootops[] = {
	{FLASH_OFFSET_ONENAND,	"onenand",	"OneNAND Flash",},
	{FLASH_OFFSET_NAND,	"nand",		"NAND Flash",	},
	{FLASH_OFFSET_NOR,	"nor",		"NOR Flash",	},
	{FLASH_OFFSET_SATA,	"sata",		"SATA Disk",	},
	{FLASH_OFFSET_SD,	"sd",		"SD Card",	},
	{FLASH_OFFSET_SPI,	"spi",		"SPI Flash",	},
	{-1,			"",		"Invalid",	},
};

/*
 * IMXIMAGE version definition for i.MX chips
 */
static table_entry_t imximage_versions[] = {
	{IMXIMAGE_V1,	"",	" (i.MX25/35/51 compatible)", },
	{IMXIMAGE_V2,	"",	" (i.MX53/6 compatible)",     },
	{-1,            "",     " (Invalid)",                 },
};

static struct imx_header imximage_header;
static uint32_t imximage_version;

static set_dcd_val_t set_dcd_val;
static set_dcd_rst_t set_dcd_rst;
static set_imx_hdr_t set_imx_hdr;
static uint32_t max_dcd_entries;
static uint32_t *header_size_ptr;

static uint32_t get_cfg_value(char *token, char *name,  int linenr)
{
	char *endptr;
	uint32_t value;

	errno = 0;
	value = strtoul(token, &endptr, 16);
	if (errno || (token == endptr)) {
		fprintf(stderr, "Error: %s[%d] - Invalid hex data(%s)\n",
			name,  linenr, token);
		exit(EXIT_FAILURE);
	}
	return value;
}

static uint32_t detect_imximage_version(struct imx_header *imx_hdr)
{
	imx_header_v1_t *hdr_v1 = &imx_hdr->header.hdr_v1;
	imx_header_v2_t *hdr_v2 = &imx_hdr->header.hdr_v2;
	flash_header_v1_t *fhdr_v1 = &hdr_v1->fhdr;
	flash_header_v2_t *fhdr_v2 = &hdr_v2->fhdr;

	/* Try to detect V1 */
	if ((fhdr_v1->app_code_barker == APP_CODE_BARKER) &&
		(hdr_v1->dcd_table.preamble.barker == DCD_BARKER))
		return IMXIMAGE_V1;

	/* Try to detect V2 */
	if ((fhdr_v2->header.tag == IVT_HEADER_TAG) &&
		(hdr_v2->dcd_table.header.tag == DCD_HEADER_TAG))
		return IMXIMAGE_V2;

	return IMXIMAGE_VER_INVALID;
}

static void err_imximage_version(int version)
{
	fprintf(stderr,
		"Error: Unsupported imximage version:%d\n", version);

	exit(EXIT_FAILURE);
}

static void set_dcd_val_v1(struct imx_header *imxhdr, char *name, int lineno,
					int fld, uint32_t value, uint32_t off)
{
	dcd_v1_t *dcd_v1 = &imxhdr->header.hdr_v1.dcd_table;

	switch (fld) {
	case CFG_REG_SIZE:
		/* Byte, halfword, word */
		if ((value != 1) && (value != 2) && (value != 4)) {
			fprintf(stderr, "Error: %s[%d] - "
				"Invalid register size " "(%d)\n",
				name, lineno, value);
			exit(EXIT_FAILURE);
		}
		dcd_v1->addr_data[off].type = value;
		break;
	case CFG_REG_ADDRESS:
		dcd_v1->addr_data[off].addr = value;
		break;
	case CFG_REG_VALUE:
		dcd_v1->addr_data[off].value = value;
		break;
	default:
		break;

	}
}

static void set_dcd_val_v2(struct imx_header *imxhdr, char *name, int lineno,
					int fld, uint32_t value, uint32_t off)
{
	dcd_v2_t *dcd_v2 = &imxhdr->header.hdr_v2.dcd_table;

	switch (fld) {
	case CFG_REG_ADDRESS:
		dcd_v2->addr_data[off].addr = cpu_to_be32(value);
		break;
	case CFG_REG_VALUE:
		dcd_v2->addr_data[off].value = cpu_to_be32(value);
		break;
	default:
		break;

	}
}

/*
 * Complete setting up the rest field of DCD of V1
 * such as barker code and DCD data length.
 */
static void set_dcd_rst_v1(struct imx_header *imxhdr, uint32_t dcd_len,
						char *name, int lineno)
{
	dcd_v1_t *dcd_v1 = &imxhdr->header.hdr_v1.dcd_table;

	dcd_v1->preamble.barker = DCD_BARKER;
	dcd_v1->preamble.length = dcd_len * sizeof(dcd_type_addr_data_t);
}

/*
 * Complete setting up the reset field of DCD of V2
 * such as DCD tag, version, length, etc.
 */
static void set_dcd_rst_v2(struct imx_header *imxhdr, uint32_t dcd_len,
						char *name, int lineno)
{
	dcd_v2_t *dcd_v2 = &imxhdr->header.hdr_v2.dcd_table;

	dcd_v2->header.tag = DCD_HEADER_TAG;
	dcd_v2->header.length = cpu_to_be16(
			dcd_len * sizeof(dcd_addr_data_t) + 8);
	dcd_v2->header.version = DCD_VERSION;
	dcd_v2->write_dcd_command.tag = DCD_COMMAND_TAG;
	dcd_v2->write_dcd_command.length = cpu_to_be16(
			dcd_len * sizeof(dcd_addr_data_t) + 4);
	dcd_v2->write_dcd_command.param = DCD_COMMAND_PARAM;
}

static void set_imx_hdr_v1(struct imx_header *imxhdr, uint32_t dcd_len,
		uint32_t entry_point, uint32_t flash_offset)
{
	imx_header_v1_t *hdr_v1 = &imxhdr->header.hdr_v1;
	flash_header_v1_t *fhdr_v1 = &hdr_v1->fhdr;
	dcd_v1_t *dcd_v1 = &hdr_v1->dcd_table;
	uint32_t hdr_base;
	uint32_t header_length = (((char *)&dcd_v1->addr_data[dcd_len].addr)
			- ((char *)imxhdr));

	/* Set magic number */
	fhdr_v1->app_code_barker = APP_CODE_BARKER;

	hdr_base = entry_point - sizeof(struct imx_header);
	fhdr_v1->app_dest_ptr = hdr_base - flash_offset;
	fhdr_v1->app_code_jump_vector = entry_point;

	fhdr_v1->dcd_ptr_ptr = hdr_base + offsetof(flash_header_v1_t, dcd_ptr);
	fhdr_v1->dcd_ptr = hdr_base + offsetof(imx_header_v1_t, dcd_table);

	/* Security feature are not supported */
	fhdr_v1->app_code_csf = 0;
	fhdr_v1->super_root_key = 0;
	header_size_ptr = (uint32_t *)(((char *)imxhdr) + header_length - 4);
}

static void set_imx_hdr_v2(struct imx_header *imxhdr, uint32_t dcd_len,
		uint32_t entry_point, uint32_t flash_offset)
{
	imx_header_v2_t *hdr_v2 = &imxhdr->header.hdr_v2;
	flash_header_v2_t *fhdr_v2 = &hdr_v2->fhdr;
	uint32_t hdr_base;

	/* Set magic number */
	fhdr_v2->header.tag = IVT_HEADER_TAG; /* 0xD1 */
	fhdr_v2->header.length = cpu_to_be16(sizeof(flash_header_v2_t));
	fhdr_v2->header.version = IVT_VERSION; /* 0x40 */

	fhdr_v2->entry = entry_point;
	fhdr_v2->reserved1 = fhdr_v2->reserved2 = 0;
	fhdr_v2->self = hdr_base = entry_point - sizeof(struct imx_header);

	fhdr_v2->dcd_ptr = hdr_base + offsetof(imx_header_v2_t, dcd_table);
	fhdr_v2->boot_data_ptr = hdr_base
			+ offsetof(imx_header_v2_t, boot_data);
	hdr_v2->boot_data.start = hdr_base - flash_offset;

	/* Security feature are not supported */
	fhdr_v2->csf = 0;
	header_size_ptr = &hdr_v2->boot_data.size;
}

static void set_hdr_func(struct imx_header *imxhdr)
{
	switch (imximage_version) {
	case IMXIMAGE_V1:
		set_dcd_val = set_dcd_val_v1;
		set_dcd_rst = set_dcd_rst_v1;
		set_imx_hdr = set_imx_hdr_v1;
		max_dcd_entries = MAX_HW_CFG_SIZE_V1;
		break;
	case IMXIMAGE_V2:
		set_dcd_val = set_dcd_val_v2;
		set_dcd_rst = set_dcd_rst_v2;
		set_imx_hdr = set_imx_hdr_v2;
		max_dcd_entries = MAX_HW_CFG_SIZE_V2;
		break;
	default:
		err_imximage_version(imximage_version);
		break;
	}
}

static void print_hdr_v1(struct imx_header *imx_hdr)
{
	imx_header_v1_t *hdr_v1 = &imx_hdr->header.hdr_v1;
	flash_header_v1_t *fhdr_v1 = &hdr_v1->fhdr;
	dcd_v1_t *dcd_v1 = &hdr_v1->dcd_table;
	uint32_t size, length, ver;

	size = dcd_v1->preamble.length;
	if (size > (MAX_HW_CFG_SIZE_V1 * sizeof(dcd_type_addr_data_t))) {
		fprintf(stderr,
			"Error: Image corrupt DCD size %d exceed maximum %d\n",
			(uint32_t)(size / sizeof(dcd_type_addr_data_t)),
			MAX_HW_CFG_SIZE_V1);
		exit(EXIT_FAILURE);
	}

	length = dcd_v1->preamble.length / sizeof(dcd_type_addr_data_t);
	ver = detect_imximage_version(imx_hdr);

	printf("Image Type:   Freescale IMX Boot Image\n");
	printf("Image Ver:    %x", ver);
	printf("%s\n", get_table_entry_name(imximage_versions, NULL, ver));
	printf("Data Size:    ");
	genimg_print_size(dcd_v1->addr_data[length].type);
	printf("Load Address: %08x\n", (uint32_t)fhdr_v1->app_dest_ptr);
	printf("Entry Point:  %08x\n", (uint32_t)fhdr_v1->app_code_jump_vector);
}

static void print_hdr_v2(struct imx_header *imx_hdr)
{
	imx_header_v2_t *hdr_v2 = &imx_hdr->header.hdr_v2;
	flash_header_v2_t *fhdr_v2 = &hdr_v2->fhdr;
	dcd_v2_t *dcd_v2 = &hdr_v2->dcd_table;
	uint32_t size, version;

	size = be16_to_cpu(dcd_v2->header.length) - 8;
	if (size > (MAX_HW_CFG_SIZE_V2 * sizeof(dcd_addr_data_t))) {
		fprintf(stderr,
			"Error: Image corrupt DCD size %d exceed maximum %d\n",
			(uint32_t)(size / sizeof(dcd_addr_data_t)),
			MAX_HW_CFG_SIZE_V2);
		exit(EXIT_FAILURE);
	}

	version = detect_imximage_version(imx_hdr);

	printf("Image Type:   Freescale IMX Boot Image\n");
	printf("Image Ver:    %x", version);
	printf("%s\n", get_table_entry_name(imximage_versions, NULL, version));
	printf("Data Size:    ");
	genimg_print_size(hdr_v2->boot_data.size);
	printf("Load Address: %08x\n", (uint32_t)fhdr_v2->boot_data_ptr);
	printf("Entry Point:  %08x\n", (uint32_t)fhdr_v2->entry);
}

static void parse_cfg_cmd(struct imx_header *imxhdr, int32_t cmd, char *token,
				char *name, int lineno, int fld, int dcd_len)
{
	int value;
	static int cmd_ver_first = ~0;

	switch (cmd) {
	case CMD_IMAGE_VERSION:
		imximage_version = get_cfg_value(token, name, lineno);
		if (cmd_ver_first == 0) {
			fprintf(stderr, "Error: %s[%d] - IMAGE_VERSION "
				"command need be the first before other "
				"valid command in the file\n", name, lineno);
			exit(EXIT_FAILURE);
		}
		cmd_ver_first = 1;
		set_hdr_func(imxhdr);
		break;
	case CMD_BOOT_FROM:
		imxhdr->flash_offset = get_table_entry_id(imximage_bootops,
					"imximage boot option", token);
		if (imxhdr->flash_offset == -1) {
			fprintf(stderr, "Error: %s[%d] -Invalid boot device"
				"(%s)\n", name, lineno, token);
			exit(EXIT_FAILURE);
		}
		if (unlikely(cmd_ver_first != 1))
			cmd_ver_first = 0;
		break;
	case CMD_DATA:
		value = get_cfg_value(token, name, lineno);
		(*set_dcd_val)(imxhdr, name, lineno, fld, value, dcd_len);
		if (unlikely(cmd_ver_first != 1))
			cmd_ver_first = 0;
		break;
	}
}

static void parse_cfg_fld(struct imx_header *imxhdr, int32_t *cmd,
		char *token, char *name, int lineno, int fld, int *dcd_len)
{
	int value;

	switch (fld) {
	case CFG_COMMAND:
		*cmd = get_table_entry_id(imximage_cmds,
			"imximage commands", token);
		if (*cmd < 0) {
			fprintf(stderr, "Error: %s[%d] - Invalid command"
			"(%s)\n", name, lineno, token);
			exit(EXIT_FAILURE);
		}
		break;
	case CFG_REG_SIZE:
		parse_cfg_cmd(imxhdr, *cmd, token, name, lineno, fld, *dcd_len);
		break;
	case CFG_REG_ADDRESS:
	case CFG_REG_VALUE:
		if (*cmd != CMD_DATA)
			return;

		value = get_cfg_value(token, name, lineno);
		(*set_dcd_val)(imxhdr, name, lineno, fld, value, *dcd_len);

		if (fld == CFG_REG_VALUE) {
			(*dcd_len)++;
			if (*dcd_len > max_dcd_entries) {
				fprintf(stderr, "Error: %s[%d] -"
					"DCD table exceeds maximum size(%d)\n",
					name, lineno, max_dcd_entries);
				exit(EXIT_FAILURE);
			}
		}
		break;
	default:
		break;
	}
}
static uint32_t parse_cfg_file(struct imx_header *imxhdr, char *name)
{
	FILE *fd = NULL;
	char *line = NULL;
	char *token, *saveptr1, *saveptr2;
	int lineno = 0;
	int fld;
	size_t len;
	int dcd_len = 0;
	int32_t cmd;

	fd = fopen(name, "r");
	if (fd == 0) {
		fprintf(stderr, "Error: %s - Can't open DCD file\n", name);
		exit(EXIT_FAILURE);
	}

	/* Very simple parsing, line starting with # are comments
	 * and are dropped
	 */
	while ((getline(&line, &len, fd)) > 0) {
		lineno++;

		token = strtok_r(line, "\r\n", &saveptr1);
		if (token == NULL)
			continue;

		/* Check inside the single line */
		for (fld = CFG_COMMAND, cmd = CMD_INVALID,
				line = token; ; line = NULL, fld++) {
			token = strtok_r(line, " \t", &saveptr2);
			if (token == NULL)
				break;

			/* Drop all text starting with '#' as comments */
			if (token[0] == '#')
				break;

			parse_cfg_fld(imxhdr, &cmd, token, name,
					lineno, fld, &dcd_len);
		}

	}

	(*set_dcd_rst)(imxhdr, dcd_len, name, lineno);
	fclose(fd);

	/* Exit if there is no BOOT_FROM field specifying the flash_offset */
	if (imxhdr->flash_offset == FLASH_OFFSET_UNDEFINED) {
		fprintf(stderr, "Error: No BOOT_FROM tag in %s\n", name);
		exit(EXIT_FAILURE);
	}
	return dcd_len;
}


static int imximage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_IMXIMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static int imximage_verify_header(unsigned char *ptr, int image_size,
			struct mkimage_params *params)
{
	struct imx_header *imx_hdr = (struct imx_header *) ptr;

	if (detect_imximage_version(imx_hdr) == IMXIMAGE_VER_INVALID)
		return -FDT_ERR_BADSTRUCTURE;

	return 0;
}

static void imximage_print_header(const void *ptr)
{
	struct imx_header *imx_hdr = (struct imx_header *) ptr;
	uint32_t version = detect_imximage_version(imx_hdr);

	switch (version) {
	case IMXIMAGE_V1:
		print_hdr_v1(imx_hdr);
		break;
	case IMXIMAGE_V2:
		print_hdr_v2(imx_hdr);
		break;
	default:
		err_imximage_version(version);
		break;
	}
}

static void imximage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct mkimage_params *params)
{
	struct imx_header *imxhdr = (struct imx_header *)ptr;
	uint32_t dcd_len;

	/*
	 * In order to not change the old imx cfg file
	 * by adding VERSION command into it, here need
	 * set up function ptr group to V1 by default.
	 */
	imximage_version = IMXIMAGE_V1;
	/* Be able to detect if the cfg file has no BOOT_FROM tag */
	imxhdr->flash_offset = FLASH_OFFSET_UNDEFINED;
	set_hdr_func(imxhdr);

	/* Parse dcd configuration file */
	dcd_len = parse_cfg_file(imxhdr, params->imagename);

	/* Set the imx header */
	(*set_imx_hdr)(imxhdr, dcd_len, params->ep, imxhdr->flash_offset);
	*header_size_ptr = sbuf->st_size + imxhdr->flash_offset;
}

int imximage_check_params(struct mkimage_params *params)
{
	if (!params)
		return CFG_INVALID;
	if (!strlen(params->imagename)) {
		fprintf(stderr, "Error: %s - Configuration file not specified, "
			"it is needed for imximage generation\n",
			params->cmdname);
		return CFG_INVALID;
	}
	/*
	 * Check parameters:
	 * XIP is not allowed and verify that incompatible
	 * parameters are not sent at the same time
	 * For example, if list is required a data image must not be provided
	 */
	return	(params->dflag && (params->fflag || params->lflag)) ||
		(params->fflag && (params->dflag || params->lflag)) ||
		(params->lflag && (params->dflag || params->fflag)) ||
		(params->xflag) || !(strlen(params->imagename));
}

/*
 * imximage parameters
 */
static struct image_type_params imximage_params = {
	.name		= "Freescale i.MX 5x Boot Image support",
	.header_size	= sizeof(struct imx_header),
	.hdr		= (void *)&imximage_header,
	.check_image_type = imximage_check_image_types,
	.verify_header	= imximage_verify_header,
	.print_header	= imximage_print_header,
	.set_header	= imximage_set_header,
	.check_params	= imximage_check_params,
};

void init_imx_image_type(void)
{
	mkimage_register(&imximage_params);
}
