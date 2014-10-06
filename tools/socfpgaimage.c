/*
 * Copyright (C) 2014 Charles Manning <cdhmanning@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Reference doc http://www.altera.com.cn/literature/hb/cyclone-v/cv_5400A.pdf
 * Note this doc is not entirely accurate. Of particular interest to us is the
 * "header" length field being in U32s and not bytes.
 *
 * "Header" is a structure of the following format.
 * this is positioned at 0x40.
 *
 * Endian is LSB.
 *
 * Offset   Length   Usage
 * -----------------------
 *   0x40        4   Validation word 0x31305341
 *   0x44        1   Version (whatever, zero is fine)
 *   0x45        1   Flags   (unused, zero is fine)
 *   0x46        2   Length  (in units of u32, including the end checksum).
 *   0x48        2   Zero
 *   0x4A        2   Checksum over the header. NB Not CRC32
 *
 * At the end of the code we have a 32-bit CRC checksum over whole binary
 * excluding the CRC.
 *
 * Note that the CRC used here is **not** the zlib/Adler crc32. It is the
 * CRC-32 used in bzip2, ethernet and elsewhere.
 *
 * The image is padded out to 64k, because that is what is
 * typically used to write the image to the boot medium.
 */

#include "pbl_crc32.h"
#include "imagetool.h"
#include <image.h>

#define HEADER_OFFSET	0x40
#define VALIDATION_WORD	0x31305341
#define PADDED_SIZE	0x10000

/* To allow for adding CRC, the max input size is a bit smaller. */
#define MAX_INPUT_SIZE	(PADDED_SIZE - sizeof(uint32_t))

static uint8_t buffer[PADDED_SIZE];

static struct socfpga_header {
	uint32_t validation;
	uint8_t  version;
	uint8_t  flags;
	uint16_t length_u32;
	uint16_t zero;
	uint16_t checksum;
} header;

/*
 * The header checksum is just a very simple checksum over
 * the header area.
 * There is still a crc32 over the whole lot.
 */
static uint16_t hdr_checksum(struct socfpga_header *header)
{
	int len = sizeof(*header) - sizeof(header->checksum);
	uint8_t *buf = (uint8_t *)header;
	uint16_t ret = 0;

	while (--len)
		ret += *buf++;

	return ret;
}


static void build_header(uint8_t *buf, uint8_t version, uint8_t flags,
			 uint16_t length_bytes)
{
	header.validation = htole32(VALIDATION_WORD);
	header.version = version;
	header.flags = flags;
	header.length_u32 = htole16(length_bytes/4);
	header.zero = 0;
	header.checksum = htole16(hdr_checksum(&header));

	memcpy(buf, &header, sizeof(header));
}

/*
 * Perform a rudimentary verification of header and return
 * size of image.
 */
static int verify_header(const uint8_t *buf)
{
	memcpy(&header, buf, sizeof(header));

	if (le32toh(header.validation) != VALIDATION_WORD)
		return -1;
	if (le16toh(header.checksum) != hdr_checksum(&header))
		return -1;

	return le16toh(header.length_u32) * 4;
}

/* Sign the buffer and return the signed buffer size */
static int sign_buffer(uint8_t *buf,
			uint8_t version, uint8_t flags,
			int len, int pad_64k)
{
	uint32_t calc_crc;

	/* Align the length up */
	len = (len + 3) & (~3);

	/* Build header, adding 4 bytes to length to hold the CRC32. */
	build_header(buf + HEADER_OFFSET,  version, flags, len + 4);

	/* Calculate and apply the CRC */
	calc_crc = ~pbl_crc32(0, (char *)buf, len);

	*((uint32_t *)(buf + len)) = htole32(calc_crc);

	if (!pad_64k)
		return len + 4;

	return PADDED_SIZE;
}

/* Verify that the buffer looks sane */
static int verify_buffer(const uint8_t *buf)
{
	int len; /* Including 32bit CRC */
	uint32_t calc_crc;
	uint32_t buf_crc;

	len = verify_header(buf + HEADER_OFFSET);
	if (len < 0) {
		fprintf(stderr, "Invalid header\n");
		return -1;
	}

	if (len < HEADER_OFFSET || len > PADDED_SIZE) {
		fprintf(stderr, "Invalid header length (%i)\n", len);
		return -1;
	}

	/*
	 * Adjust length to the base of the CRC.
	 * Check the CRC.
	*/
	len -= 4;

	calc_crc = ~pbl_crc32(0, (const char *)buf, len);

	buf_crc = le32toh(*((uint32_t *)(buf + len)));

	if (buf_crc != calc_crc) {
		fprintf(stderr, "CRC32 does not match (%08x != %08x)\n",
			buf_crc, calc_crc);
		return -1;
	}

	return 0;
}

/* mkimage glue functions */
static int socfpgaimage_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params)
{
	if (image_size != PADDED_SIZE)
		return -1;

	return verify_buffer(ptr);
}

static void socfpgaimage_print_header(const void *ptr)
{
	if (verify_buffer(ptr) == 0)
		printf("Looks like a sane SOCFPGA preloader\n");
	else
		printf("Not a sane SOCFPGA preloader\n");
}

static int socfpgaimage_check_params(struct image_tool_params *params)
{
	/* Not sure if we should be accepting fflags */
	return	(params->dflag && (params->fflag || params->lflag)) ||
		(params->fflag && (params->dflag || params->lflag)) ||
		(params->lflag && (params->dflag || params->fflag));
}

static int socfpgaimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_SOCFPGAIMAGE)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

/*
 * To work in with the mkimage framework, we do some ugly stuff...
 *
 * First, socfpgaimage_vrec_header() is called.
 * We prepend a fake header big enough to make the file PADDED_SIZE.
 * This gives us enough space to do what we want later.
 *
 * Next, socfpgaimage_set_header() is called.
 * We fix up the buffer by moving the image to the start of the buffer.
 * We now have some room to do what we need (add CRC and padding).
 */

static int data_size;
#define FAKE_HEADER_SIZE (PADDED_SIZE - data_size)

static int socfpgaimage_vrec_header(struct image_tool_params *params,
				struct image_type_params *tparams)
{
	struct stat sbuf;

	if (params->datafile &&
	    stat(params->datafile, &sbuf) == 0 &&
	    sbuf.st_size <= MAX_INPUT_SIZE) {
		data_size = sbuf.st_size;
		tparams->header_size = FAKE_HEADER_SIZE;
	}
	return 0;
}

static void socfpgaimage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
	uint8_t *buf = (uint8_t *)ptr;

	/*
	 * This function is called after vrec_header() has been called.
	 * At this stage we have the FAKE_HEADER_SIZE dummy bytes followed by
	 * data_size image bytes. Total = PADDED_SIZE.
	 * We need to fix the buffer by moving the image bytes back to
	 * the beginning of the buffer, then actually do the signing stuff...
	 */
	memmove(buf, buf + FAKE_HEADER_SIZE, data_size);
	memset(buf + data_size, 0, FAKE_HEADER_SIZE);

	sign_buffer(buf, 0, 0, data_size, 0);
}

static struct image_type_params socfpgaimage_params = {
	.name		= "Altera SOCFPGA preloader support",
	.vrec_header	= socfpgaimage_vrec_header,
	.header_size	= 0, /* This will be modified by vrec_header() */
	.hdr		= (void *)buffer,
	.check_image_type = socfpgaimage_check_image_types,
	.verify_header	= socfpgaimage_verify_header,
	.print_header	= socfpgaimage_print_header,
	.set_header	= socfpgaimage_set_header,
	.check_params	= socfpgaimage_check_params,
};

void init_socfpga_image_type(void)
{
	register_image_type(&socfpgaimage_params);
}
