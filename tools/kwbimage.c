/*
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
#include "kwbimage.h"

/*
 * Supported commands for configuration file
 */
static table_entry_t kwbimage_cmds[] = {
	{CMD_BOOT_FROM,		"BOOT_FROM",		"boot comand",	},
	{CMD_NAND_ECC_MODE,	"NAND_ECC_MODE",	"NAND mode",	},
	{CMD_NAND_PAGE_SIZE,	"NAND_PAGE_SIZE",	"NAND size",	},
	{CMD_SATA_PIO_MODE,	"SATA_PIO_MODE",	"SATA mode",	},
	{CMD_DDR_INIT_DELAY,	"DDR_INIT_DELAY",	"DDR init dly",	},
	{CMD_DATA,		"DATA",			"Reg Write Data", },
	{CMD_INVALID,		"",			"",	},
};

/*
 * Supported Boot options for configuration file
 */
static table_entry_t kwbimage_bootops[] = {
	{IBR_HDR_SPI_ID,	"spi",		"SPI Flash",	},
	{IBR_HDR_NAND_ID,	"nand",		"NAND Flash",	},
	{IBR_HDR_SATA_ID,	"sata",		"Sata port",	},
	{IBR_HDR_PEX_ID,	"pex",		"PCIe port",	},
	{IBR_HDR_UART_ID,	"uart",		"Serial port",	},
	{-1,			"",		"Invalid",	},
};

/*
 * Supported NAND ecc options configuration file
 */
static table_entry_t kwbimage_eccmodes[] = {
	{IBR_HDR_ECC_DEFAULT,		"default",	"Default mode",	},
	{IBR_HDR_ECC_FORCED_HAMMING,	"hamming",	"Hamming mode",	},
	{IBR_HDR_ECC_FORCED_RS,		"rs",		"RS mode",	},
	{IBR_HDR_ECC_DISABLED,		"disabled",	"ECC Disabled",	},
	{-1,				"",		"",	},
};

static struct kwb_header kwbimage_header;
static int datacmd_cnt = 0;
static char * fname = "Unknown";
static int lineno = -1;

/*
 * Report Error if xflag is set in addition to default
 */
static int kwbimage_check_params (struct mkimage_params *params)
{
	if (!strlen (params->imagename)) {
		printf ("Error:%s - Configuration file not specified, "
			"it is needed for kwbimage generation\n",
			params->cmdname);
		return CFG_INVALID;
	}
	return	((params->dflag && (params->fflag || params->lflag)) ||
		(params->fflag && (params->dflag || params->lflag)) ||
		(params->lflag && (params->dflag || params->fflag)) ||
		(params->xflag) || !(strlen (params->imagename)));
}

static uint32_t check_get_hexval (char *token)
{
	uint32_t hexval;

	if (!sscanf (token, "%x", &hexval)) {
		printf ("Error:%s[%d] - Invalid hex data(%s)\n", fname,
			lineno, token);
		exit (EXIT_FAILURE);
	}
	return hexval;
}

/*
 * Generates 8 bit checksum
 */
static uint8_t kwbimage_checksum8 (void *start, uint32_t len, uint8_t csum)
{
	register uint8_t sum = csum;
	volatile uint8_t *p = (volatile uint8_t *)start;

	/* check len and return zero checksum if invalid */
	if (!len)
		return 0;

	do {
		sum += *p;
		p++;
	} while (--len);
	return (sum);
}

/*
 * Generates 32 bit checksum
 */
static uint32_t kwbimage_checksum32 (uint32_t *start, uint32_t len, uint32_t csum)
{
	register uint32_t sum = csum;
	volatile uint32_t *p = start;

	/* check len and return zero checksum if invalid */
	if (!len)
		return 0;

	if (len % sizeof(uint32_t)) {
		printf ("Error:%s[%d] - length is not in multiple of %zu\n",
			__FUNCTION__, len, sizeof(uint32_t));
		return 0;
	}

	do {
		sum += *p;
		p++;
		len -= sizeof(uint32_t);
	} while (len > 0);
	return (sum);
}

static void kwbimage_check_cfgdata (char *token, enum kwbimage_cmd cmdsw,
					struct kwb_header *kwbhdr)
{
	bhr_t *mhdr = &kwbhdr->kwb_hdr;
	extbhr_t *exthdr = &kwbhdr->kwb_exthdr;
	int i;

	switch (cmdsw) {
	case CMD_BOOT_FROM:
		i = get_table_entry_id (kwbimage_bootops,
				"Kwbimage boot option", token);

		if (i < 0)
			goto INVL_DATA;

		mhdr->blockid = i;
		printf ("Preparing kirkwood boot image to boot "
			"from %s\n", token);
		break;
	case CMD_NAND_ECC_MODE:
		i = get_table_entry_id (kwbimage_eccmodes,
			"NAND ecc mode", token);

		if (i < 0)
			goto INVL_DATA;

		mhdr->nandeccmode = i;
		printf ("Nand ECC mode = %s\n", token);
		break;
	case CMD_NAND_PAGE_SIZE:
		mhdr->nandpagesize =
			(uint16_t) check_get_hexval (token);
		printf ("Nand page size = 0x%x\n", mhdr->nandpagesize);
		break;
	case CMD_SATA_PIO_MODE:
		mhdr->satapiomode =
			(uint8_t) check_get_hexval (token);
		printf ("Sata PIO mode = 0x%x\n",
				mhdr->satapiomode);
		break;
	case CMD_DDR_INIT_DELAY:
		mhdr->ddrinitdelay =
			(uint16_t) check_get_hexval (token);
		printf ("DDR init delay = %d msec\n", mhdr->ddrinitdelay);
		break;
	case CMD_DATA:
		exthdr->rcfg[datacmd_cnt].raddr =
			check_get_hexval (token);

		break;
	case CMD_INVALID:
		goto INVL_DATA;
	default:
		goto INVL_DATA;
	}
	return;

INVL_DATA:
	printf ("Error:%s[%d] - Invalid data\n", fname, lineno);
	exit (EXIT_FAILURE);
}

/*
 * this function sets the kwbimage header by-
 * 	1. Abstracting input command line arguments data
 *	2. parses the kwbimage configuration file and update extebded header data
 *	3. calculates header, extended header and image checksums
 */
static void kwdimage_set_ext_header (struct kwb_header *kwbhdr, char* name) {
	bhr_t *mhdr = &kwbhdr->kwb_hdr;
	extbhr_t *exthdr = &kwbhdr->kwb_exthdr;
	FILE *fd = NULL;
	int j;
	char *line = NULL;
	char * token, *saveptr1, *saveptr2;
	size_t len = 0;
	enum kwbimage_cmd cmd;

	fname = name;
	/* set dram register offset */
	exthdr->dramregsoffs = (intptr_t)&exthdr->rcfg - (intptr_t)mhdr;

	if ((fd = fopen (name, "r")) == 0) {
		printf ("Error:%s - Can't open\n", fname);
		exit (EXIT_FAILURE);
	}

	/* Simple kwimage.cfg file parser */
	lineno=0;
	while ((getline (&line, &len, fd)) > 0) {
		lineno++;
		token = strtok_r (line, "\r\n", &saveptr1);
		/* drop all lines with zero tokens (= empty lines) */
		if (token == NULL)
			continue;

		for (j = 0, cmd = CMD_INVALID, line = token; ; line = NULL) {
			token = strtok_r (line, " \t", &saveptr2);
			if (token == NULL)
			break;
			/* Drop all text starting with '#' as comments */
			if (token[0] == '#')
				break;

			/* Process rest as valid config command line */
			switch (j) {
			case CFG_COMMAND:
				cmd = get_table_entry_id (kwbimage_cmds,
						"Kwbimage command", token);

				if (cmd == CMD_INVALID)
					goto INVL_CMD;
				break;

			case CFG_DATA0:
				kwbimage_check_cfgdata (token, cmd, kwbhdr);
				break;

			case CFG_DATA1:
				if (cmd != CMD_DATA)
					goto INVL_CMD;

				exthdr->rcfg[datacmd_cnt].rdata =
						check_get_hexval (token);

				if (datacmd_cnt > KWBIMAGE_MAX_CONFIG ) {
					printf ("Error:%s[%d] - Found more "
						"than max(%zd) allowed "
						"data configurations\n",
						fname, lineno,
						KWBIMAGE_MAX_CONFIG);
				exit (EXIT_FAILURE);
				} else
					datacmd_cnt++;
				break;

			default:
				goto INVL_CMD;
			}
			j++;
		}
	}
	if (line)
		free (line);

	fclose (fd);
	return;

/*
 * Invalid Command error reporring
 *
 * command CMD_DATA needs three strings on a line
 * whereas other commands need only two.
 *
 * if more than two/three (as per command type) are observed,
 * then error will be reported
 */
INVL_CMD:
	printf ("Error:%s[%d] - Invalid command\n", fname, lineno);
	exit (EXIT_FAILURE);
}

static void kwbimage_set_header (void *ptr, struct stat *sbuf, int ifd,
				struct mkimage_params *params)
{
	struct kwb_header *hdr = (struct kwb_header *)ptr;
	bhr_t *mhdr = &hdr->kwb_hdr;
	extbhr_t *exthdr = &hdr->kwb_exthdr;
	uint32_t checksum;
	int size;

	/* Build and add image checksum header */
	checksum = kwbimage_checksum32 ((uint32_t *)ptr, sbuf->st_size, 0);

	size = write (ifd, &checksum, sizeof(uint32_t));
	if (size != sizeof(uint32_t)) {
		printf ("Error:%s - Checksum write %d bytes %s\n",
			params->cmdname, size, params->imagefile);
		exit (EXIT_FAILURE);
	}

	sbuf->st_size += sizeof(uint32_t);

	mhdr->blocksize = sbuf->st_size - sizeof(struct kwb_header);
	mhdr->srcaddr = sizeof(struct kwb_header);
	mhdr->destaddr= params->addr;
	mhdr->execaddr =params->ep;
	mhdr->ext = 0x1; /* header extension appended */

	kwdimage_set_ext_header (hdr, params->imagename);
	/* calculate checksums */
	mhdr->checkSum = kwbimage_checksum8 ((void *)mhdr, sizeof(bhr_t), 0);
	exthdr->checkSum = kwbimage_checksum8 ((void *)exthdr,
						sizeof(extbhr_t), 0);
}

static int kwbimage_verify_header (unsigned char *ptr, int image_size,
			struct mkimage_params *params)
{
	struct kwb_header *hdr = (struct kwb_header *)ptr;
	bhr_t *mhdr = &hdr->kwb_hdr;
	extbhr_t *exthdr = &hdr->kwb_exthdr;
	uint8_t calc_hdrcsum;
	uint8_t calc_exthdrcsum;

	calc_hdrcsum = kwbimage_checksum8 ((void *)mhdr,
			sizeof(bhr_t) - sizeof(uint8_t), 0);
	if (calc_hdrcsum != mhdr->checkSum)
		return -FDT_ERR_BADSTRUCTURE;	/* mhdr csum not matched */

	calc_exthdrcsum = kwbimage_checksum8 ((void *)exthdr,
			sizeof(extbhr_t) - sizeof(uint8_t), 0);
	if (calc_hdrcsum != mhdr->checkSum)
		return -FDT_ERR_BADSTRUCTURE; /* exthdr csum not matched */

	return 0;
}

static void kwbimage_print_header (const void *ptr)
{
	struct kwb_header *hdr = (struct kwb_header *) ptr;
	bhr_t *mhdr = &hdr->kwb_hdr;
	char *name = get_table_entry_name (kwbimage_bootops,
				"Kwbimage boot option",
				(int) mhdr->blockid);

	printf ("Image Type:   Kirkwood Boot from %s Image\n", name);
	printf ("Data Size:    ");
	genimg_print_size (mhdr->blocksize - sizeof(uint32_t));
	printf ("Load Address: %08x\n", mhdr->destaddr);
	printf ("Entry Point:  %08x\n", mhdr->execaddr);
}

static int kwbimage_check_image_types (uint8_t type)
{
	if (type == IH_TYPE_KWBIMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

/*
 * kwbimage type parameters definition
 */
static struct image_type_params kwbimage_params = {
	.name = "Kirkwood Boot Image support",
	.header_size = sizeof(struct kwb_header),
	.hdr = (void*)&kwbimage_header,
	.check_image_type = kwbimage_check_image_types,
	.verify_header = kwbimage_verify_header,
	.print_header = kwbimage_print_header,
	.set_header = kwbimage_set_header,
	.check_params = kwbimage_check_params,
};

void init_kwb_image_type (void)
{
	mkimage_register (&kwbimage_params);
}
