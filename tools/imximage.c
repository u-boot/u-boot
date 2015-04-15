/*
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * (C) Copyright 2008
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "imagetool.h"
#include <image.h>
#include "imximage.h"

#define UNDEFINED 0xFFFFFFFF

/*
 * Supported commands for configuration file
 */
static table_entry_t imximage_cmds[] = {
	{CMD_BOOT_FROM,         "BOOT_FROM",            "boot command",	  },
	{CMD_BOOT_OFFSET,       "BOOT_OFFSET",          "Boot offset",	  },
	{CMD_DATA,              "DATA",                 "Reg Write Data", },
	{CMD_CSF,               "CSF",           "Command Sequence File", },
	{CMD_IMAGE_VERSION,     "IMAGE_VERSION",        "image version",  },
	{-1,                    "",                     "",	          },
};

/*
 * Supported Boot options for configuration file
 * this is needed to set the correct flash offset
 */
static table_entry_t imximage_boot_offset[] = {
	{FLASH_OFFSET_ONENAND,	"onenand",	"OneNAND Flash",},
	{FLASH_OFFSET_NAND,	"nand",		"NAND Flash",	},
	{FLASH_OFFSET_NOR,	"nor",		"NOR Flash",	},
	{FLASH_OFFSET_SATA,	"sata",		"SATA Disk",	},
	{FLASH_OFFSET_SD,	"sd",		"SD Card",	},
	{FLASH_OFFSET_SPI,	"spi",		"SPI Flash",	},
	{FLASH_OFFSET_QSPI,	"qspi",		"QSPI NOR Flash",},
	{-1,			"",		"Invalid",	},
};

/*
 * Supported Boot options for configuration file
 * this is needed to determine the initial load size
 */
static table_entry_t imximage_boot_loadsize[] = {
	{FLASH_LOADSIZE_ONENAND,	"onenand",	"OneNAND Flash",},
	{FLASH_LOADSIZE_NAND,		"nand",		"NAND Flash",	},
	{FLASH_LOADSIZE_NOR,		"nor",		"NOR Flash",	},
	{FLASH_LOADSIZE_SATA,		"sata",		"SATA Disk",	},
	{FLASH_LOADSIZE_SD,		"sd",		"SD Card",	},
	{FLASH_LOADSIZE_SPI,		"spi",		"SPI Flash",	},
	{FLASH_LOADSIZE_QSPI,		"qspi",		"QSPI NOR Flash",},
	{-1,				"",		"Invalid",	},
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
/*
 * Image Vector Table Offset
 * Initialized to a wrong not 4-bytes aligned address to
 * check if it is was set by the cfg file.
 */
static uint32_t imximage_ivt_offset = UNDEFINED;
static uint32_t imximage_csf_size = UNDEFINED;
/* Initial Load Region Size */
static uint32_t imximage_init_loadsize;

static set_dcd_val_t set_dcd_val;
static set_dcd_rst_t set_dcd_rst;
static set_imx_hdr_t set_imx_hdr;
static uint32_t max_dcd_entries;
static uint32_t *header_size_ptr;
static uint32_t *csf_ptr;

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

	/* TODO: check i.MX image V1 handling, for now use 'old' style */
	hdr_base = entry_point - 4096;
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
	hdr_base = entry_point - imximage_init_loadsize +
		flash_offset;
	fhdr_v2->self = hdr_base;
	fhdr_v2->dcd_ptr = hdr_base + offsetof(imx_header_v2_t, dcd_table);
	fhdr_v2->boot_data_ptr = hdr_base
			+ offsetof(imx_header_v2_t, boot_data);
	hdr_v2->boot_data.start = entry_point - imximage_init_loadsize;

	fhdr_v2->csf = 0;

	header_size_ptr = &hdr_v2->boot_data.size;
	csf_ptr = &fhdr_v2->csf;
}

static void set_hdr_func(void)
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
	if (fhdr_v2->csf && (imximage_ivt_offset != UNDEFINED) &&
	    (imximage_csf_size != UNDEFINED)) {
		printf("HAB Blocks:   %08x %08x %08x\n",
		       (uint32_t)fhdr_v2->self, 0,
		       hdr_v2->boot_data.size - imximage_ivt_offset -
		       imximage_csf_size);
	}
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
		set_hdr_func();
		break;
	case CMD_BOOT_FROM:
		imximage_ivt_offset = get_table_entry_id(imximage_boot_offset,
					"imximage boot option", token);
		if (imximage_ivt_offset == -1) {
			fprintf(stderr, "Error: %s[%d] -Invalid boot device"
				"(%s)\n", name, lineno, token);
			exit(EXIT_FAILURE);
		}

		imximage_init_loadsize =
			get_table_entry_id(imximage_boot_loadsize,
					   "imximage boot option", token);

		if (imximage_init_loadsize == -1) {
			fprintf(stderr,
				"Error: %s[%d] -Invalid boot device(%s)\n",
				name, lineno, token);
			exit(EXIT_FAILURE);
		}

		/*
		 * The SOC loads from the storage starting at address 0
		 * then ensures that the load size contains the offset
		 */
		if (imximage_init_loadsize < imximage_ivt_offset)
			imximage_init_loadsize = imximage_ivt_offset;
		if (unlikely(cmd_ver_first != 1))
			cmd_ver_first = 0;
		break;
	case CMD_BOOT_OFFSET:
		imximage_ivt_offset = get_cfg_value(token, name, lineno);
		if (unlikely(cmd_ver_first != 1))
			cmd_ver_first = 0;
		break;
	case CMD_DATA:
		value = get_cfg_value(token, name, lineno);
		(*set_dcd_val)(imxhdr, name, lineno, fld, value, dcd_len);
		if (unlikely(cmd_ver_first != 1))
			cmd_ver_first = 0;
		break;
	case CMD_CSF:
		if (imximage_version != 2) {
			fprintf(stderr,
				"Error: %s[%d] - CSF only supported for VERSION 2(%s)\n",
				name, lineno, token);
			exit(EXIT_FAILURE);
		}
		imximage_csf_size = get_cfg_value(token, name, lineno);
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

	/*
	 * Very simple parsing, line starting with # are comments
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
	if (imximage_ivt_offset == FLASH_OFFSET_UNDEFINED) {
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
			struct image_tool_params *params)
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
				struct image_tool_params *params)
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
	imximage_ivt_offset = FLASH_OFFSET_UNDEFINED;
	imximage_csf_size = 0;
	set_hdr_func();

	/* Parse dcd configuration file */
	dcd_len = parse_cfg_file(imxhdr, params->imagename);

	if (imximage_version == IMXIMAGE_V2) {
		if (imximage_init_loadsize < imximage_ivt_offset +
			sizeof(imx_header_v2_t))
				imximage_init_loadsize = imximage_ivt_offset +
					sizeof(imx_header_v2_t);
	}

	/* Set the imx header */
	(*set_imx_hdr)(imxhdr, dcd_len, params->ep, imximage_ivt_offset);

	/*
	 * ROM bug alert
	 *
	 * MX53 only loads 512 byte multiples in case of SD boot.
	 * MX53 only loads NAND page multiples in case of NAND boot and
	 * supports up to 4096 byte large pages, thus align to 4096.
	 *
	 * The remaining fraction of a block bytes would not be loaded!
	 */
	*header_size_ptr = ROUND((sbuf->st_size + imximage_ivt_offset), 4096);

	if (csf_ptr && imximage_csf_size) {
		*csf_ptr = params->ep - imximage_init_loadsize +
			*header_size_ptr;
		*header_size_ptr += imximage_csf_size;
	}
}

int imximage_check_params(struct image_tool_params *params)
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

static int imximage_generate(struct image_tool_params *params,
	struct image_type_params *tparams)
{
	struct imx_header *imxhdr;
	size_t alloc_len;
	struct stat sbuf;
	char *datafile = params->datafile;
	uint32_t pad_len;

	memset(&imximage_header, 0, sizeof(imximage_header));

	/*
	 * In order to not change the old imx cfg file
	 * by adding VERSION command into it, here need
	 * set up function ptr group to V1 by default.
	 */
	imximage_version = IMXIMAGE_V1;
	/* Be able to detect if the cfg file has no BOOT_FROM tag */
	imximage_ivt_offset = FLASH_OFFSET_UNDEFINED;
	imximage_csf_size = 0;
	set_hdr_func();

	/* Parse dcd configuration file */
	parse_cfg_file(&imximage_header, params->imagename);

	/* TODO: check i.MX image V1 handling, for now use 'old' style */
	if (imximage_version == IMXIMAGE_V1) {
		alloc_len = 4096;
	} else {
		if (imximage_init_loadsize < imximage_ivt_offset +
			sizeof(imx_header_v2_t))
				imximage_init_loadsize = imximage_ivt_offset +
					sizeof(imx_header_v2_t);
		alloc_len = imximage_init_loadsize - imximage_ivt_offset;
	}

	if (alloc_len < sizeof(struct imx_header)) {
		fprintf(stderr, "%s: header error\n",
			params->cmdname);
		exit(EXIT_FAILURE);
	}

	imxhdr = malloc(alloc_len);

	if (!imxhdr) {
		fprintf(stderr, "%s: malloc return failure: %s\n",
			params->cmdname, strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(imxhdr, 0, alloc_len);

	tparams->header_size = alloc_len;
	tparams->hdr         = imxhdr;

	/* determine data image file length */

	if (stat(datafile, &sbuf) < 0) {
		fprintf(stderr, "%s: Can't stat %s: %s\n",
			params->cmdname, datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	pad_len = ROUND(sbuf.st_size, 4096) - sbuf.st_size;

	/* TODO: check i.MX image V1 handling, for now use 'old' style */
	if (imximage_version == IMXIMAGE_V1)
		return 0;
	else
		return pad_len;
}


/*
 * imximage parameters
 */
U_BOOT_IMAGE_TYPE(
	imximage,
	"Freescale i.MX Boot Image support",
	0,
	NULL,
	imximage_check_params,
	imximage_verify_header,
	imximage_print_header,
	imximage_set_header,
	NULL,
	imximage_check_image_types,
	NULL,
	imximage_generate
);
