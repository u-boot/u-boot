/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _FSL_VALIDATE_H_
#define _FSL_VALIDATE_H_

#include <fsl_sec.h>
#include <fsl_sec_mon.h>
#include <command.h>
#include <linux/types.h>

#define WORD_SIZE 4

/* Minimum and maximum size of RSA signature length in bits */
#define KEY_SIZE       4096
#define KEY_SIZE_BYTES (KEY_SIZE/8)
#define KEY_SIZE_WORDS (KEY_SIZE_BYTES/(WORD_SIZE))

extern struct jobring jr;

#ifdef CONFIG_KEY_REVOCATION
/* Srk table and key revocation check */
#define SRK_FLAG	0x01
#define UNREVOCABLE_KEY	4
#define ALIGN_REVOC_KEY 3
#define MAX_KEY_ENTRIES 4
#endif

/* Barker code size in bytes */
#define ESBC_BARKER_LEN	4	/* barker code length in ESBC uboot client */
				/* header */

/* No-error return values */
#define ESBC_VALID_HDR	0	/* header is valid */

/* Maximum number of SG entries allowed */
#define MAX_SG_ENTRIES	8

/*
 * ESBC uboot client header structure.
 * The struct contain the following fields
 * barker code
 * public key offset
 * pub key length
 * signature offset
 * length of the signature
 * ptr to SG table
 * no of entries in SG table
 * esbc ptr
 * size of esbc
 * esbc entry point
 * Scatter gather flag
 * UID flag
 * FSL UID
 * OEM UID
 * Here, pub key is modulus concatenated with exponent
 * of equal length
 */
struct fsl_secboot_img_hdr {
	u8 barker[ESBC_BARKER_LEN];	/* barker code */
	union {
		u32 pkey;		/* public key offset */
#ifdef CONFIG_KEY_REVOCATION
		u32 srk_tbl_off;
#endif
	};

	union {
		u32 key_len;		/* pub key length in bytes */
#ifdef CONFIG_KEY_REVOCATION
		struct {
			u32 srk_table_flag:8;
			u32 srk_sel:8;
			u32 num_srk:16;
		} len_kr;
#endif
	};

	u32 psign;		/* signature offset */
	u32 sign_len;		/* length of the signature in bytes */
	union {
		struct fsl_secboot_sg_table *psgtable;	/* ptr to SG table */
		u8 *pimg;	/* ptr to ESBC client image */
	};
	union {
		u32 sg_entries;	/* no of entries in SG table */
		u32 img_size;	/* ESBC client image size in bytes */
	};
	ulong img_start;		/* ESBC client entry point */
	u32 sg_flag;		/* Scatter gather flag */
	u32 uid_flag;
	u32 fsl_uid_0;
	u32 oem_uid_0;
	u32 reserved1[2];
	u32 fsl_uid_1;
	u32 oem_uid_1;
	u32 reserved2[2];
	u32 ie_flag;
	u32 ie_key_sel;
};

#if defined(CONFIG_FSL_ISBC_KEY_EXT)
struct ie_key_table {
	u32 key_len;
	u8 pkey[2 * KEY_SIZE_BYTES];
};

struct ie_key_info {
	uint32_t key_revok;
	uint32_t num_keys;
	struct ie_key_table ie_key_tbl[32];
};
#endif

#ifdef CONFIG_KEY_REVOCATION
struct srk_table {
	u32 key_len;
	u8 pkey[2 * KEY_SIZE_BYTES];
};
#endif

/*
 * SG table.
 */
#if defined(CONFIG_FSL_TRUST_ARCH_v1) && defined(CONFIG_FSL_CORENET)
/*
 * This struct contains the following fields
 * length of the segment
 * source address
 */
struct fsl_secboot_sg_table {
	u32 len;		/* length of the segment in bytes */
	ulong src_addr;		/* ptr to the data segment */
};
#else
/*
 * This struct contains the following fields
 * length of the segment
 * Destination Target ID
 * source address
 * destination address
 */
struct fsl_secboot_sg_table {
	u32 len;
	u32 trgt_id;
	ulong src_addr;
	ulong dst_addr;
};
#endif

/*
 * ESBC private structure.
 * Private structure used by ESBC to store following fields
 * ESBC client key
 * ESBC client key hash
 * ESBC client Signature
 * Encoded hash recovered from signature
 * Encoded hash of ESBC client header plus ESBC client image
 */
struct fsl_secboot_img_priv {
	uint32_t hdr_location;
	ulong ie_addr;
	u32 key_len;
	struct fsl_secboot_img_hdr hdr;

	u8 img_key[2 * KEY_SIZE_BYTES];	/* ESBC client key */
	u8 img_key_hash[32];	/* ESBC client key hash */

#ifdef CONFIG_KEY_REVOCATION
	struct srk_table srk_tbl[MAX_KEY_ENTRIES];
#endif
	u8 img_sign[KEY_SIZE_BYTES];		/* ESBC client signature */

	u8 img_encoded_hash[KEY_SIZE_BYTES];	/* EM wrt RSA PKCSv1.5  */
						/* Includes hash recovered after
						 * signature verification
						 */

	u8 img_encoded_hash_second[KEY_SIZE_BYTES];/* EM' wrt RSA PKCSv1.5 */
						/* Includes hash of
						 * ESBC client header plus
						 * ESBC client image
						 */

	struct fsl_secboot_sg_table sgtbl[MAX_SG_ENTRIES];	/* SG table */
	u32 ehdrloc;		/* ESBC client location */
};

int fsl_secboot_validate(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[]);
int fsl_secboot_blob_encap(cmd_tbl_t *cmdtp, int flag, int argc,
	char * const argv[]);
int fsl_secboot_blob_decap(cmd_tbl_t *cmdtp, int flag, int argc,
	char * const argv[]);

#endif
