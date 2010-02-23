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
	{CMD_BOOT_FROM,		"BOOT_FROM",		"boot comand",	},
	{CMD_DATA,		"DATA",			"Reg Write Data", },
	{-1,		"",			"",	},
};

/*
 * Supported Boot options for configuration file
 * this is needed to set the correct flash offset
 */
static table_entry_t imximage_bootops[] = {
	{FLASH_OFFSET_SPI,	"spi",		"SPI Flash",	},
	{FLASH_OFFSET_NAND,	"nand",		"NAND Flash",	},
	{FLASH_OFFSET_SD,	"sd",		"SD Card",	},
	{FLASH_OFFSET_ONENAND,	"onenand",	"OneNAND Flash",},
	{-1,			"",		"Invalid",	},
};


static struct imx_header imximage_header;

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
	flash_header_t *hdr = &imx_hdr->fhdr;

	/* Only a few checks can be done: search for magic numbers */
	if (hdr->app_code_barker != APP_CODE_BARKER)
		return -FDT_ERR_BADSTRUCTURE;

	if (imx_hdr->dcd_table.preamble.barker != DCD_BARKER)
		return -FDT_ERR_BADSTRUCTURE;

	return 0;
}

static void imximage_print_header(const void *ptr)
{
	struct imx_header *imx_hdr = (struct imx_header *) ptr;
	flash_header_t *hdr = &imx_hdr->fhdr;
	uint32_t size;
	uint32_t length;
	dcd_t *dcd = &imx_hdr->dcd_table;

	size = imx_hdr->dcd_table.preamble.length;
	if (size > (MAX_HW_CFG_SIZE * sizeof(dcd_type_addr_data_t))) {
		fprintf(stderr,
			"Error: Image corrupt DCD size %d exceed maximum %d\n",
			(uint32_t)(size / sizeof(dcd_type_addr_data_t)),
			MAX_HW_CFG_SIZE);
		exit(EXIT_FAILURE);
	}

	length =  dcd->preamble.length / sizeof(dcd_type_addr_data_t);

	printf("Image Type:   Freescale IMX Boot Image\n");
	printf("Data Size:    ");
	genimg_print_size(dcd->addr_data[length].type);
	printf("Load Address: %08x\n", (unsigned int)hdr->app_dest_ptr);
	printf("Entry Point:  %08x\n", (unsigned int)hdr->app_code_jump_vector);
}

static uint32_t imximage_parse_cfg_file(struct imx_header *imxhdr, char *name)
{
	FILE *fd = NULL;
	char *line = NULL;
	char *token, *saveptr1, *saveptr2;
	int lineno = 0;
	int fld, value;
	size_t len;
	int dcd_len = 0;
	dcd_t *dcd = &imxhdr->dcd_table;
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

			/* parse all fields in a single line */
			switch (fld) {
			case CFG_COMMAND:
				cmd = get_table_entry_id(imximage_cmds,
					"imximage commands", token);
				if (cmd < 0) {
					fprintf(stderr,
						"Error: %s[%d] - "
						"Invalid command (%s)\n",
						name, lineno, token);
					exit(EXIT_FAILURE);
				}
				break;
			case CFG_REG_SIZE:
				switch (cmd) {
				case CMD_BOOT_FROM:
					/* Get flash header offset */
					imxhdr->flash_offset =
						get_table_entry_id(
							imximage_bootops,
							"imximage boot option",
							token);
					if (imxhdr->flash_offset == -1) {
						fprintf(stderr,
							"Error: %s[%d] -"
							"Invalid boot device"
							"(%s)\n",
							name, lineno, token);
						exit(EXIT_FAILURE);
					}
					break;
				case CMD_DATA:
					value = get_cfg_value(token,
							name, lineno);

					/* Byte, halfword, word */
					if ((value != 1) &&
						(value != 2) && (value != 4)) {
						fprintf(stderr,
							"Error: %s[%d] - "
							"Invalid register size "
							"(%d)\n",
							name, lineno, value);
						exit(EXIT_FAILURE);
					}
					dcd->addr_data[dcd_len].type = value;
					break;
				}

			case CFG_REG_ADDRESS:
				if (cmd == CMD_DATA)
					dcd->addr_data[dcd_len].addr =
						get_cfg_value(token,
							name, lineno);
				break;
			case CFG_REG_VALUE:
				if (cmd == CMD_DATA) {
					dcd->addr_data[dcd_len].value =
						get_cfg_value(token,
							name, lineno);
					dcd_len++;
				}
				break;
			}
		}

		if (dcd_len > MAX_HW_CFG_SIZE) {
			fprintf(stderr,
				"Error: %s[%d] -"
				"DCD table exceeds maximum size(%d)\n",
				name, lineno, MAX_HW_CFG_SIZE);
		}
	}
	dcd->preamble.barker = DCD_BARKER;
	dcd->preamble.length = dcd_len * sizeof(dcd_type_addr_data_t);
	fclose(fd);

	return dcd_len;
}

static void imximage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct mkimage_params *params)
{
	struct imx_header *hdr = (struct imx_header *)ptr;
	flash_header_t *fhdr = &hdr->fhdr;
	int dcd_len;
	dcd_t *dcd = &hdr->dcd_table;
	uint32_t base_offset;

	/* Set default offset */
	hdr->flash_offset = FLASH_OFFSET_STANDARD;

	/* Set magic number */
	fhdr->app_code_barker = APP_CODE_BARKER;

	/* Parse dcd configuration file */
	dcd_len = imximage_parse_cfg_file(hdr, params->imagename);

	fhdr->app_dest_ptr = params->addr;
	fhdr->app_dest_ptr = params->ep - hdr->flash_offset -
		sizeof(struct imx_header);
	fhdr->app_code_jump_vector = params->ep;

	base_offset = fhdr->app_dest_ptr + hdr->flash_offset ;
	fhdr->dcd_ptr_ptr = (uint32_t) (offsetof(flash_header_t, dcd_ptr) -
		offsetof(flash_header_t, app_code_jump_vector) +
		base_offset);

	fhdr->dcd_ptr = base_offset +
			offsetof(struct imx_header, dcd_table);

	/* The external flash header must be at the end of the DCD table */
	dcd->addr_data[dcd_len].type = sbuf->st_size +
				hdr->flash_offset +
				sizeof(struct imx_header);

	/* Security feature are not supported */
	fhdr->app_code_csf = 0;
	fhdr->super_root_key = 0;

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
	.name		= "Freescale i.MX 51 Boot Image support",
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
