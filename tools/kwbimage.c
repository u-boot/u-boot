/*
 * Image manipulator for Marvell SoCs
 *  supports Kirkwood, Dove, Armada 370, and Armada XP
 *
 * (C) Copyright 2013 Thomas Petazzoni
 * <thomas.petazzoni@free-electrons.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Not implemented: support for the register headers and secure
 * headers in v1 images
 */

#include "imagetool.h"
#include <limits.h>
#include <image.h>
#include <stdint.h>
#include "kwbimage.h"

static struct image_cfg_element *image_cfg;
static int cfgn;

struct boot_mode {
	unsigned int id;
	const char *name;
};

struct boot_mode boot_modes[] = {
	{ 0x4D, "i2c"  },
	{ 0x5A, "spi"  },
	{ 0x8B, "nand" },
	{ 0x78, "sata" },
	{ 0x9C, "pex"  },
	{ 0x69, "uart" },
	{ 0xAE, "sdio" },
	{},
};

struct nand_ecc_mode {
	unsigned int id;
	const char *name;
};

struct nand_ecc_mode nand_ecc_modes[] = {
	{ 0x00, "default" },
	{ 0x01, "hamming" },
	{ 0x02, "rs" },
	{ 0x03, "disabled" },
	{},
};

/* Used to identify an undefined execution or destination address */
#define ADDR_INVALID ((uint32_t)-1)

#define BINARY_MAX_ARGS 8

/* In-memory representation of a line of the configuration file */
struct image_cfg_element {
	enum {
		IMAGE_CFG_VERSION = 0x1,
		IMAGE_CFG_BOOT_FROM,
		IMAGE_CFG_DEST_ADDR,
		IMAGE_CFG_EXEC_ADDR,
		IMAGE_CFG_NAND_BLKSZ,
		IMAGE_CFG_NAND_BADBLK_LOCATION,
		IMAGE_CFG_NAND_ECC_MODE,
		IMAGE_CFG_NAND_PAGESZ,
		IMAGE_CFG_BINARY,
		IMAGE_CFG_PAYLOAD,
		IMAGE_CFG_DATA,
	} type;
	union {
		unsigned int version;
		unsigned int bootfrom;
		struct {
			const char *file;
			unsigned int args[BINARY_MAX_ARGS];
			unsigned int nargs;
		} binary;
		const char *payload;
		unsigned int dstaddr;
		unsigned int execaddr;
		unsigned int nandblksz;
		unsigned int nandbadblklocation;
		unsigned int nandeccmode;
		unsigned int nandpagesz;
		struct ext_hdr_v0_reg regdata;
	};
};

#define IMAGE_CFG_ELEMENT_MAX 256

/*
 * Utility functions to manipulate boot mode and ecc modes (convert
 * them back and forth between description strings and the
 * corresponding numerical identifiers).
 */

static const char *image_boot_mode_name(unsigned int id)
{
	int i;
	for (i = 0; boot_modes[i].name; i++)
		if (boot_modes[i].id == id)
			return boot_modes[i].name;
	return NULL;
}

int image_boot_mode_id(const char *boot_mode_name)
{
	int i;
	for (i = 0; boot_modes[i].name; i++)
		if (!strcmp(boot_modes[i].name, boot_mode_name))
			return boot_modes[i].id;

	return -1;
}

int image_nand_ecc_mode_id(const char *nand_ecc_mode_name)
{
	int i;
	for (i = 0; nand_ecc_modes[i].name; i++)
		if (!strcmp(nand_ecc_modes[i].name, nand_ecc_mode_name))
			return nand_ecc_modes[i].id;
	return -1;
}

static struct image_cfg_element *
image_find_option(unsigned int optiontype)
{
	int i;

	for (i = 0; i < cfgn; i++) {
		if (image_cfg[i].type == optiontype)
			return &image_cfg[i];
	}

	return NULL;
}

static unsigned int
image_count_options(unsigned int optiontype)
{
	int i;
	unsigned int count = 0;

	for (i = 0; i < cfgn; i++)
		if (image_cfg[i].type == optiontype)
			count++;

	return count;
}

/*
 * Compute a 8-bit checksum of a memory area. This algorithm follows
 * the requirements of the Marvell SoC BootROM specifications.
 */
static uint8_t image_checksum8(void *start, uint32_t len)
{
	uint8_t csum = 0;
	uint8_t *p = start;

	/* check len and return zero checksum if invalid */
	if (!len)
		return 0;

	do {
		csum += *p;
		p++;
	} while (--len);

	return csum;
}

static uint32_t image_checksum32(void *start, uint32_t len)
{
	uint32_t csum = 0;
	uint32_t *p = start;

	/* check len and return zero checksum if invalid */
	if (!len)
		return 0;

	if (len % sizeof(uint32_t)) {
		fprintf(stderr, "Length %d is not in multiple of %zu\n",
			len, sizeof(uint32_t));
		return 0;
	}

	do {
		csum += *p;
		p++;
		len -= sizeof(uint32_t);
	} while (len > 0);

	return csum;
}

static void *image_create_v0(size_t *imagesz, struct image_tool_params *params,
			     int payloadsz)
{
	struct image_cfg_element *e;
	size_t headersz;
	struct main_hdr_v0 *main_hdr;
	struct ext_hdr_v0 *ext_hdr;
	void *image;
	int has_ext = 0;

	/*
	 * Calculate the size of the header and the size of the
	 * payload
	 */
	headersz  = sizeof(struct main_hdr_v0);

	if (image_count_options(IMAGE_CFG_DATA) > 0) {
		has_ext = 1;
		headersz += sizeof(struct ext_hdr_v0);
	}

	if (image_count_options(IMAGE_CFG_PAYLOAD) > 1) {
		fprintf(stderr, "More than one payload, not possible\n");
		return NULL;
	}

	image = malloc(headersz);
	if (!image) {
		fprintf(stderr, "Cannot allocate memory for image\n");
		return NULL;
	}

	memset(image, 0, headersz);

	main_hdr = image;

	/* Fill in the main header */
	main_hdr->blocksize =
		cpu_to_le32(payloadsz + sizeof(uint32_t) - headersz);
	main_hdr->srcaddr   = cpu_to_le32(headersz);
	main_hdr->ext       = has_ext;
	main_hdr->destaddr  = cpu_to_le32(params->addr);
	main_hdr->execaddr  = cpu_to_le32(params->ep);

	e = image_find_option(IMAGE_CFG_BOOT_FROM);
	if (e)
		main_hdr->blockid = e->bootfrom;
	e = image_find_option(IMAGE_CFG_NAND_ECC_MODE);
	if (e)
		main_hdr->nandeccmode = e->nandeccmode;
	e = image_find_option(IMAGE_CFG_NAND_PAGESZ);
	if (e)
		main_hdr->nandpagesize = cpu_to_le16(e->nandpagesz);
	main_hdr->checksum = image_checksum8(image,
					     sizeof(struct main_hdr_v0));

	/* Generate the ext header */
	if (has_ext) {
		int cfgi, datai;

		ext_hdr = image + sizeof(struct main_hdr_v0);
		ext_hdr->offset = cpu_to_le32(0x40);

		for (cfgi = 0, datai = 0; cfgi < cfgn; cfgi++) {
			e = &image_cfg[cfgi];
			if (e->type != IMAGE_CFG_DATA)
				continue;

			ext_hdr->rcfg[datai].raddr =
				cpu_to_le32(e->regdata.raddr);
			ext_hdr->rcfg[datai].rdata =
				cpu_to_le32(e->regdata.rdata);
			datai++;
		}

		ext_hdr->checksum = image_checksum8(ext_hdr,
						    sizeof(struct ext_hdr_v0));
	}

	*imagesz = headersz;
	return image;
}

static size_t image_headersz_v1(struct image_tool_params *params,
				int *hasext)
{
	struct image_cfg_element *binarye;
	size_t headersz;
	int ret;

	/*
	 * Calculate the size of the header and the size of the
	 * payload
	 */
	headersz = sizeof(struct main_hdr_v1);

	if (image_count_options(IMAGE_CFG_BINARY) > 1) {
		fprintf(stderr, "More than one binary blob, not supported\n");
		return 0;
	}

	if (image_count_options(IMAGE_CFG_PAYLOAD) > 1) {
		fprintf(stderr, "More than one payload, not possible\n");
		return 0;
	}

	binarye = image_find_option(IMAGE_CFG_BINARY);
	if (binarye) {
		struct stat s;

		ret = stat(binarye->binary.file, &s);
		if (ret < 0) {
			char cwd[PATH_MAX];
			char *dir = cwd;

			memset(cwd, 0, sizeof(cwd));
			if (!getcwd(cwd, sizeof(cwd))) {
				dir = "current working directory";
				perror("getcwd() failed");
			}

			fprintf(stderr,
				"Didn't find the file '%s' in '%s' which is mandatory to generate the image\n"
				"This file generally contains the DDR3 training code, and should be extracted from an existing bootable\n"
				"image for your board. See 'kwbimage -x' to extract it from an existing image.\n",
				binarye->binary.file, dir);
			return 0;
		}

		headersz += sizeof(struct opt_hdr_v1) +
			s.st_size +
			(binarye->binary.nargs + 2) * sizeof(uint32_t);
		if (hasext)
			*hasext = 1;
	}

#if defined(CONFIG_SYS_U_BOOT_OFFS)
	if (headersz > CONFIG_SYS_U_BOOT_OFFS) {
		fprintf(stderr, "Error: Image header (incl. SPL image) too big!\n");
		fprintf(stderr, "header=0x%x CONFIG_SYS_U_BOOT_OFFS=0x%x!\n",
			(int)headersz, CONFIG_SYS_U_BOOT_OFFS);
		fprintf(stderr, "Increase CONFIG_SYS_U_BOOT_OFFS!\n");
		return 0;
	} else {
		headersz = CONFIG_SYS_U_BOOT_OFFS;
	}
#endif

	/*
	 * The payload should be aligned on some reasonable
	 * boundary
	 */
	return ALIGN_SUP(headersz, 4096);
}

static void *image_create_v1(size_t *imagesz, struct image_tool_params *params,
			     int payloadsz)
{
	struct image_cfg_element *e, *binarye;
	struct main_hdr_v1 *main_hdr;
	size_t headersz;
	void *image, *cur;
	int hasext = 0;
	int ret;

	/*
	 * Calculate the size of the header and the size of the
	 * payload
	 */
	headersz = image_headersz_v1(params, &hasext);
	if (headersz == 0)
		return NULL;

	image = malloc(headersz);
	if (!image) {
		fprintf(stderr, "Cannot allocate memory for image\n");
		return NULL;
	}

	memset(image, 0, headersz);

	cur = main_hdr = image;
	cur += sizeof(struct main_hdr_v1);

	/* Fill the main header */
	main_hdr->blocksize    =
		cpu_to_le32(payloadsz - headersz + sizeof(uint32_t));
	main_hdr->headersz_lsb = cpu_to_le16(headersz & 0xFFFF);
	main_hdr->headersz_msb = (headersz & 0xFFFF0000) >> 16;
	main_hdr->destaddr     = cpu_to_le32(params->addr);
	main_hdr->execaddr     = cpu_to_le32(params->ep);
	main_hdr->srcaddr      = cpu_to_le32(headersz);
	main_hdr->ext          = hasext;
	main_hdr->version      = 1;
	e = image_find_option(IMAGE_CFG_BOOT_FROM);
	if (e)
		main_hdr->blockid = e->bootfrom;
	e = image_find_option(IMAGE_CFG_NAND_BLKSZ);
	if (e)
		main_hdr->nandblocksize = e->nandblksz / (64 * 1024);
	e = image_find_option(IMAGE_CFG_NAND_BADBLK_LOCATION);
	if (e)
		main_hdr->nandbadblklocation = e->nandbadblklocation;

	binarye = image_find_option(IMAGE_CFG_BINARY);
	if (binarye) {
		struct opt_hdr_v1 *hdr = cur;
		uint32_t *args;
		size_t binhdrsz;
		struct stat s;
		int argi;
		FILE *bin;

		hdr->headertype = OPT_HDR_V1_BINARY_TYPE;

		bin = fopen(binarye->binary.file, "r");
		if (!bin) {
			fprintf(stderr, "Cannot open binary file %s\n",
				binarye->binary.file);
			return NULL;
		}

		fstat(fileno(bin), &s);

		binhdrsz = sizeof(struct opt_hdr_v1) +
			(binarye->binary.nargs + 2) * sizeof(uint32_t) +
			s.st_size;

		/*
		 * The size includes the binary image size, rounded
		 * up to a 4-byte boundary. Plus 4 bytes for the
		 * next-header byte and 3-byte alignment at the end.
		 */
		binhdrsz = ALIGN_SUP(binhdrsz, 4) + 4;
		hdr->headersz_lsb = cpu_to_le16(binhdrsz & 0xFFFF);
		hdr->headersz_msb = (binhdrsz & 0xFFFF0000) >> 16;

		cur += sizeof(struct opt_hdr_v1);

		args = cur;
		*args = cpu_to_le32(binarye->binary.nargs);
		args++;
		for (argi = 0; argi < binarye->binary.nargs; argi++)
			args[argi] = cpu_to_le32(binarye->binary.args[argi]);

		cur += (binarye->binary.nargs + 1) * sizeof(uint32_t);

		ret = fread(cur, s.st_size, 1, bin);
		if (ret != 1) {
			fprintf(stderr,
				"Could not read binary image %s\n",
				binarye->binary.file);
			return NULL;
		}

		fclose(bin);

		cur += ALIGN_SUP(s.st_size, 4);

		/*
		 * For now, we don't support more than one binary
		 * header, and no other header types are
		 * supported. So, the binary header is necessarily the
		 * last one
		 */
		*((uint32_t *)cur) = 0x00000000;

		cur += sizeof(uint32_t);
	}

	/* Calculate and set the header checksum */
	main_hdr->checksum = image_checksum8(main_hdr, headersz);

	*imagesz = headersz;
	return image;
}

static int image_create_config_parse_oneline(char *line,
					     struct image_cfg_element *el)
{
	char *keyword, *saveptr;
	char deliminiters[] = " \t";

	keyword = strtok_r(line, deliminiters, &saveptr);
	if (!strcmp(keyword, "VERSION")) {
		char *value = strtok_r(NULL, deliminiters, &saveptr);
		el->type = IMAGE_CFG_VERSION;
		el->version = atoi(value);
	} else if (!strcmp(keyword, "BOOT_FROM")) {
		char *value = strtok_r(NULL, deliminiters, &saveptr);
		int ret = image_boot_mode_id(value);
		if (ret < 0) {
			fprintf(stderr,
				"Invalid boot media '%s'\n", value);
			return -1;
		}
		el->type = IMAGE_CFG_BOOT_FROM;
		el->bootfrom = ret;
	} else if (!strcmp(keyword, "NAND_BLKSZ")) {
		char *value = strtok_r(NULL, deliminiters, &saveptr);
		el->type = IMAGE_CFG_NAND_BLKSZ;
		el->nandblksz = strtoul(value, NULL, 16);
	} else if (!strcmp(keyword, "NAND_BADBLK_LOCATION")) {
		char *value = strtok_r(NULL, deliminiters, &saveptr);
		el->type = IMAGE_CFG_NAND_BADBLK_LOCATION;
		el->nandbadblklocation =
			strtoul(value, NULL, 16);
	} else if (!strcmp(keyword, "NAND_ECC_MODE")) {
		char *value = strtok_r(NULL, deliminiters, &saveptr);
		int ret = image_nand_ecc_mode_id(value);
		if (ret < 0) {
			fprintf(stderr,
				"Invalid NAND ECC mode '%s'\n", value);
			return -1;
		}
		el->type = IMAGE_CFG_NAND_ECC_MODE;
		el->nandeccmode = ret;
	} else if (!strcmp(keyword, "NAND_PAGE_SIZE")) {
		char *value = strtok_r(NULL, deliminiters, &saveptr);
		el->type = IMAGE_CFG_NAND_PAGESZ;
		el->nandpagesz = strtoul(value, NULL, 16);
	} else if (!strcmp(keyword, "BINARY")) {
		char *value = strtok_r(NULL, deliminiters, &saveptr);
		int argi = 0;

		el->type = IMAGE_CFG_BINARY;
		el->binary.file = strdup(value);
		while (1) {
			value = strtok_r(NULL, deliminiters, &saveptr);
			if (!value)
				break;
			el->binary.args[argi] = strtoul(value, NULL, 16);
			argi++;
			if (argi >= BINARY_MAX_ARGS) {
				fprintf(stderr,
					"Too many argument for binary\n");
				return -1;
			}
		}
		el->binary.nargs = argi;
	} else if (!strcmp(keyword, "DATA")) {
		char *value1 = strtok_r(NULL, deliminiters, &saveptr);
		char *value2 = strtok_r(NULL, deliminiters, &saveptr);

		if (!value1 || !value2) {
			fprintf(stderr,
				"Invalid number of arguments for DATA\n");
			return -1;
		}

		el->type = IMAGE_CFG_DATA;
		el->regdata.raddr = strtoul(value1, NULL, 16);
		el->regdata.rdata = strtoul(value2, NULL, 16);
	} else {
		fprintf(stderr, "Ignoring unknown line '%s'\n", line);
	}

	return 0;
}

/*
 * Parse the configuration file 'fcfg' into the array of configuration
 * elements 'image_cfg', and return the number of configuration
 * elements in 'cfgn'.
 */
static int image_create_config_parse(FILE *fcfg)
{
	int ret;
	int cfgi = 0;

	/* Parse the configuration file */
	while (!feof(fcfg)) {
		char *line;
		char buf[256];

		/* Read the current line */
		memset(buf, 0, sizeof(buf));
		line = fgets(buf, sizeof(buf), fcfg);
		if (!line)
			break;

		/* Ignore useless lines */
		if (line[0] == '\n' || line[0] == '#')
			continue;

		/* Strip final newline */
		if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = 0;

		/* Parse the current line */
		ret = image_create_config_parse_oneline(line,
							&image_cfg[cfgi]);
		if (ret)
			return ret;

		cfgi++;

		if (cfgi >= IMAGE_CFG_ELEMENT_MAX) {
			fprintf(stderr,
				"Too many configuration elements in .cfg file\n");
			return -1;
		}
	}

	cfgn = cfgi;
	return 0;
}

static int image_get_version(void)
{
	struct image_cfg_element *e;

	e = image_find_option(IMAGE_CFG_VERSION);
	if (!e)
		return -1;

	return e->version;
}

static int image_version_file(const char *input)
{
	FILE *fcfg;
	int version;
	int ret;

	fcfg = fopen(input, "r");
	if (!fcfg) {
		fprintf(stderr, "Could not open input file %s\n", input);
		return -1;
	}

	image_cfg = malloc(IMAGE_CFG_ELEMENT_MAX *
			   sizeof(struct image_cfg_element));
	if (!image_cfg) {
		fprintf(stderr, "Cannot allocate memory\n");
		fclose(fcfg);
		return -1;
	}

	memset(image_cfg, 0,
	       IMAGE_CFG_ELEMENT_MAX * sizeof(struct image_cfg_element));
	rewind(fcfg);

	ret = image_create_config_parse(fcfg);
	fclose(fcfg);
	if (ret) {
		free(image_cfg);
		return -1;
	}

	version = image_get_version();
	/* Fallback to version 0 is no version is provided in the cfg file */
	if (version == -1)
		version = 0;

	free(image_cfg);

	return version;
}

static void kwbimage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
	FILE *fcfg;
	void *image = NULL;
	int version;
	size_t headersz = 0;
	uint32_t checksum;
	int ret;
	int size;

	fcfg = fopen(params->imagename, "r");
	if (!fcfg) {
		fprintf(stderr, "Could not open input file %s\n",
			params->imagename);
		exit(EXIT_FAILURE);
	}

	image_cfg = malloc(IMAGE_CFG_ELEMENT_MAX *
			   sizeof(struct image_cfg_element));
	if (!image_cfg) {
		fprintf(stderr, "Cannot allocate memory\n");
		fclose(fcfg);
		exit(EXIT_FAILURE);
	}

	memset(image_cfg, 0,
	       IMAGE_CFG_ELEMENT_MAX * sizeof(struct image_cfg_element));
	rewind(fcfg);

	ret = image_create_config_parse(fcfg);
	fclose(fcfg);
	if (ret) {
		free(image_cfg);
		exit(EXIT_FAILURE);
	}

	/* The MVEBU BootROM does not allow non word aligned payloads */
	sbuf->st_size = ALIGN_SUP(sbuf->st_size, 4);

	version = image_get_version();
	switch (version) {
		/*
		 * Fallback to version 0 if no version is provided in the
		 * cfg file
		 */
	case -1:
	case 0:
		image = image_create_v0(&headersz, params, sbuf->st_size);
		break;

	case 1:
		image = image_create_v1(&headersz, params, sbuf->st_size);
		break;

	default:
		fprintf(stderr, "Unsupported version %d\n", version);
		free(image_cfg);
		exit(EXIT_FAILURE);
	}

	if (!image) {
		fprintf(stderr, "Could not create image\n");
		free(image_cfg);
		exit(EXIT_FAILURE);
	}

	free(image_cfg);

	/* Build and add image checksum header */
	checksum =
		cpu_to_le32(image_checksum32((uint32_t *)ptr, sbuf->st_size));
	size = write(ifd, &checksum, sizeof(uint32_t));
	if (size != sizeof(uint32_t)) {
		fprintf(stderr, "Error:%s - Checksum write %d bytes %s\n",
			params->cmdname, size, params->imagefile);
		exit(EXIT_FAILURE);
	}

	sbuf->st_size += sizeof(uint32_t);

	/* Finally copy the header into the image area */
	memcpy(ptr, image, headersz);

	free(image);
}

static void kwbimage_print_header(const void *ptr)
{
	struct main_hdr_v0 *mhdr = (struct main_hdr_v0 *)ptr;

	printf("Image Type:   MVEBU Boot from %s Image\n",
	       image_boot_mode_name(mhdr->blockid));
	printf("Image version:%d\n", image_version((void *)ptr));
	printf("Data Size:    ");
	genimg_print_size(mhdr->blocksize - sizeof(uint32_t));
	printf("Load Address: %08x\n", mhdr->destaddr);
	printf("Entry Point:  %08x\n", mhdr->execaddr);
}

static int kwbimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_KWBIMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static int kwbimage_verify_header(unsigned char *ptr, int image_size,
				  struct image_tool_params *params)
{
	struct main_hdr_v0 *main_hdr;
	struct ext_hdr_v0 *ext_hdr;
	uint8_t checksum;

	main_hdr = (void *)ptr;
	checksum = image_checksum8(ptr,
				   sizeof(struct main_hdr_v0)
				   - sizeof(uint8_t));
	if (checksum != main_hdr->checksum)
		return -FDT_ERR_BADSTRUCTURE;

	/* Only version 0 extended header has checksum */
	if (image_version((void *)ptr) == 0) {
		ext_hdr = (void *)ptr + sizeof(struct main_hdr_v0);
		checksum = image_checksum8(ext_hdr,
					   sizeof(struct ext_hdr_v0)
					   - sizeof(uint8_t));
		if (checksum != ext_hdr->checksum)
			return -FDT_ERR_BADSTRUCTURE;
	}

	return 0;
}

static int kwbimage_generate(struct image_tool_params *params,
			     struct image_type_params *tparams)
{
	int alloc_len;
	void *hdr;
	int version = 0;

	version = image_version_file(params->imagename);
	if (version == 0) {
		alloc_len = sizeof(struct main_hdr_v0) +
			sizeof(struct ext_hdr_v0);
	} else {
		alloc_len = image_headersz_v1(params, NULL);
	}

	hdr = malloc(alloc_len);
	if (!hdr) {
		fprintf(stderr, "%s: malloc return failure: %s\n",
			params->cmdname, strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(hdr, 0, alloc_len);
	tparams->header_size = alloc_len;
	tparams->hdr = hdr;

	/*
	 * The resulting image needs to be 4-byte aligned. At least
	 * the Marvell hdrparser tool complains if its unaligned.
	 * By returning 1 here in this function, called via
	 * tparams->vrec_header() in mkimage.c, mkimage will
	 * automatically pad the the resulting image to a 4-byte
	 * size if necessary.
	 */
	return 1;
}

/*
 * Report Error if xflag is set in addition to default
 */
static int kwbimage_check_params(struct image_tool_params *params)
{
	if (!strlen(params->imagename)) {
		fprintf(stderr, "Error:%s - Configuration file not specified, "
			"it is needed for kwbimage generation\n",
			params->cmdname);
		return CFG_INVALID;
	}

	return (params->dflag && (params->fflag || params->lflag)) ||
		(params->fflag && (params->dflag || params->lflag)) ||
		(params->lflag && (params->dflag || params->fflag)) ||
		(params->xflag) || !(strlen(params->imagename));
}

/*
 * kwbimage type parameters definition
 */
U_BOOT_IMAGE_TYPE(
	kwbimage,
	"Marvell MVEBU Boot Image support",
	0,
	NULL,
	kwbimage_check_params,
	kwbimage_verify_header,
	kwbimage_print_header,
	kwbimage_set_header,
	NULL,
	kwbimage_check_image_types,
	NULL,
	kwbimage_generate
);
