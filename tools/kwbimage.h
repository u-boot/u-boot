/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef _KWBIMAGE_H_
#define _KWBIMAGE_H_

#include <compiler.h>
#include <stdint.h>

#ifdef __GNUC__
#define __packed __attribute((packed))
#else
#define __packed
#endif

#define KWBIMAGE_MAX_CONFIG	((0x1dc - 0x20)/sizeof(struct reg_config))
#define MAX_TEMPBUF_LEN		32

/* NAND ECC Mode */
#define IBR_HDR_ECC_DEFAULT		0x00
#define IBR_HDR_ECC_FORCED_HAMMING	0x01
#define IBR_HDR_ECC_FORCED_RS		0x02
#define IBR_HDR_ECC_DISABLED		0x03

/* Boot Type - block ID */
#define IBR_HDR_I2C_ID			0x4D
#define IBR_HDR_SPI_ID			0x5A
#define IBR_HDR_NAND_ID			0x8B
#define IBR_HDR_SATA_ID			0x78
#define IBR_HDR_PEX_ID			0x9C
#define IBR_HDR_UART_ID			0x69
#define IBR_HDR_SDIO_ID			0xAE
#define IBR_DEF_ATTRIB			0x00

/* Structure of the main header, version 0 (Kirkwood, Dove) */
struct main_hdr_v0 {
	uint8_t  blockid;		/* 0x0       */
	uint8_t  nandeccmode;		/* 0x1       */
	uint16_t nandpagesize;		/* 0x2-0x3   */
	uint32_t blocksize;		/* 0x4-0x7   */
	uint8_t  version;		/* 0x8       */
	uint8_t  rsvd1[3];		/* 0x9-0xB   */
	uint32_t srcaddr;		/* 0xC-0xF   */
	uint32_t destaddr;		/* 0x10-0x13 */
	uint32_t execaddr;		/* 0x14-0x17 */
	uint8_t  satapiomode;		/* 0x18      */
	uint8_t  rsvd3;			/* 0x19      */
	uint16_t ddrinitdelay;		/* 0x1A-0x1B */
	uint16_t rsvd2;			/* 0x1C-0x1D */
	uint8_t  ext;			/* 0x1E      */
	uint8_t  checksum;		/* 0x1F      */
} __packed;

struct ext_hdr_v0_reg {
	uint32_t raddr;
	uint32_t rdata;
} __packed;

#define EXT_HDR_V0_REG_COUNT ((0x1dc - 0x20) / sizeof(struct ext_hdr_v0_reg))

struct ext_hdr_v0 {
	uint32_t              offset;
	uint8_t               reserved[0x20 - sizeof(uint32_t)];
	struct ext_hdr_v0_reg rcfg[EXT_HDR_V0_REG_COUNT];
	uint8_t               reserved2[7];
	uint8_t               checksum;
} __packed;

/* Structure of the main header, version 1 (Armada 370/XP/375/38x/39x) */
struct main_hdr_v1 {
	uint8_t  blockid;               /* 0x0       */
	uint8_t  flags;                 /* 0x1       */
	uint16_t nandpagesize;          /* 0x2-0x3   */
	uint32_t blocksize;             /* 0x4-0x7   */
	uint8_t  version;               /* 0x8       */
	uint8_t  headersz_msb;          /* 0x9       */
	uint16_t headersz_lsb;          /* 0xA-0xB   */
	uint32_t srcaddr;               /* 0xC-0xF   */
	uint32_t destaddr;              /* 0x10-0x13 */
	uint32_t execaddr;              /* 0x14-0x17 */
	uint8_t  options;               /* 0x18      */
	uint8_t  nandblocksize;         /* 0x19      */
	uint8_t  nandbadblklocation;    /* 0x1A      */
	uint8_t  reserved4;             /* 0x1B      */
	uint16_t reserved5;             /* 0x1C-0x1D */
	uint8_t  ext;                   /* 0x1E      */
	uint8_t  checksum;              /* 0x1F      */
} __packed;

/*
 * Main header options
 */
#define MAIN_HDR_V1_OPT_BAUD_DEFAULT	0
#define MAIN_HDR_V1_OPT_BAUD_2400	0x1
#define MAIN_HDR_V1_OPT_BAUD_4800	0x2
#define MAIN_HDR_V1_OPT_BAUD_9600	0x3
#define MAIN_HDR_V1_OPT_BAUD_19200	0x4
#define MAIN_HDR_V1_OPT_BAUD_38400	0x5
#define MAIN_HDR_V1_OPT_BAUD_57600	0x6
#define MAIN_HDR_V1_OPT_BAUD_115200	0x7

/*
 * Header for the optional headers, version 1 (Armada 370/XP/375/38x/39x)
 */
struct opt_hdr_v1 {
	uint8_t  headertype;
	uint8_t  headersz_msb;
	uint16_t headersz_lsb;
	char     data[0];
} __packed;

/*
 * Public Key data in DER format
 */
struct pubkey_der_v1 {
	uint8_t key[524];
} __packed;

/*
 * Signature (RSA 2048)
 */
struct sig_v1 {
	uint8_t sig[256];
} __packed;

/*
 * Structure of secure header (Armada XP/375/38x/39x)
 */
struct secure_hdr_v1 {
	uint8_t  headertype;		/* 0x0 */
	uint8_t  headersz_msb;		/* 0x1 */
	uint16_t headersz_lsb;		/* 0x2 - 0x3 */
	uint32_t reserved1;		/* 0x4 - 0x7 */
	struct pubkey_der_v1 kak;	/* 0x8 - 0x213 */
	uint8_t  jtag_delay;		/* 0x214 */
	uint8_t  reserved2;		/* 0x215 */
	uint16_t reserved3;		/* 0x216 - 0x217 */
	uint32_t boxid;			/* 0x218 - 0x21B */
	uint32_t flashid;		/* 0x21C - 0x21F */
	struct sig_v1 hdrsig;		/* 0x220 - 0x31F */
	struct sig_v1 imgsig;		/* 0x320 - 0x41F */
	struct pubkey_der_v1 csk[16];	/* 0x420 - 0x24DF */
	struct sig_v1 csksig;		/* 0x24E0 - 0x25DF */
	uint8_t  next;			/* 0x25E0 */
	uint8_t  reserved4;		/* 0x25E1 */
	uint16_t reserved5;		/* 0x25E2 - 0x25E3 */
} __packed;

/*
 * Structure of register set
 */
struct register_set_hdr_v1 {
	uint8_t  headertype;		/* 0x0 */
	uint8_t  headersz_msb;		/* 0x1 */
	uint16_t headersz_lsb;		/* 0x2 - 0x3 */
	union {
		struct {
			uint32_t address;	/* 0x4+8*N - 0x7+8*N */
			uint32_t value;		/* 0x8+8*N - 0xB+8*N */
		} __packed entry;
		struct {
			uint8_t  next;		/* 0xC+8*N */
			uint8_t  delay;		/* 0xD+8*N */
			uint16_t reserved;	/* 0xE+8*N - 0xF+8*N */
		} __packed last_entry;
	} data[];
} __packed;

/*
 * Value 0 in register_set_hdr_v1 delay field is special.
 * Instead of delay it setup SDRAM Controller.
 */
#define REGISTER_SET_HDR_OPT_DELAY_SDRAM_SETUP 0
#define REGISTER_SET_HDR_OPT_DELAY_MS(val) ((val) ?: 1)

/*
 * Various values for the opt_hdr_v1->headertype field, describing the
 * different types of optional headers. The "secure" header contains
 * informations related to secure boot (encryption keys, etc.). The
 * "binary" header contains ARM binary code to be executed prior to
 * executing the main payload (usually the bootloader). This is
 * typically used to execute DDR3 training code. The "register" header
 * allows to describe a set of (address, value) tuples that are
 * generally used to configure the DRAM controller.
 */
#define OPT_HDR_V1_SECURE_TYPE   0x1
#define OPT_HDR_V1_BINARY_TYPE   0x2
#define OPT_HDR_V1_REGISTER_TYPE 0x3

/*
 * Byte 8 of the image header contains the version number. In the v0
 * header, byte 8 was reserved, and always set to 0. In the v1 header,
 * byte 8 has been changed to a proper field, set to 1.
 */
static inline unsigned int kwbimage_version(const void *header)
{
	const unsigned char *ptr = header;
	return ptr[8];
}

static inline size_t kwbheader_size(const void *header)
{
	if (kwbimage_version(header) == 0) {
		const struct main_hdr_v0 *hdr = header;

		return sizeof(*hdr) +
		       (hdr->ext & 0x1) ? sizeof(struct ext_hdr_v0) : 0;
	} else {
		const struct main_hdr_v1 *hdr = header;

		return (hdr->headersz_msb << 16) |
		       le16_to_cpu(hdr->headersz_lsb);
	}
}

static inline size_t kwbheader_size_for_csum(const void *header)
{
	if (kwbimage_version(header) == 0)
		return sizeof(struct main_hdr_v0);
	else
		return kwbheader_size(header);
}

static inline uint32_t opt_hdr_v1_size(const struct opt_hdr_v1 *ohdr)
{
	return (ohdr->headersz_msb << 16) | le16_to_cpu(ohdr->headersz_lsb);
}

static inline int opt_hdr_v1_valid_size(const struct opt_hdr_v1 *ohdr,
					const void *mhdr_end)
{
	uint32_t ohdr_size;

	if ((void *)(ohdr + 1) > mhdr_end)
		return 0;

	ohdr_size = opt_hdr_v1_size(ohdr);
	if (ohdr_size < 8 || (void *)((uint8_t *)ohdr + ohdr_size) > mhdr_end)
		return 0;

	return 1;
}

static inline struct opt_hdr_v1 *opt_hdr_v1_first(void *img) {
	struct main_hdr_v1 *mhdr;

	if (kwbimage_version(img) != 1)
		return NULL;

	mhdr = img;
	if (mhdr->ext & 0x1)
		return (struct opt_hdr_v1 *)(mhdr + 1);
	else
		return NULL;
}

static inline uint8_t *opt_hdr_v1_ext(struct opt_hdr_v1 *cur)
{
	uint32_t size = opt_hdr_v1_size(cur);

	return (uint8_t *)cur + size - 4;
}

static inline struct opt_hdr_v1 *_opt_hdr_v1_next(struct opt_hdr_v1 *cur)
{
	return (struct opt_hdr_v1 *)((uint8_t *)cur + opt_hdr_v1_size(cur));
}

static inline struct opt_hdr_v1 *opt_hdr_v1_next(struct opt_hdr_v1 *cur)
{
	if (*opt_hdr_v1_ext(cur) & 0x1)
		return _opt_hdr_v1_next(cur);
	else
		return NULL;
}

#define for_each_opt_hdr_v1(ohdr, img)		\
	for ((ohdr) = opt_hdr_v1_first((img));	\
	     (ohdr) != NULL;			\
	     (ohdr) = opt_hdr_v1_next((ohdr)))

#endif /* _KWBIMAGE_H_ */
