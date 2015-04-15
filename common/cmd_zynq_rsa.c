/*
 * Copyright (C) 2013 Xilinx, Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <u-boot/md5.h>
#include <u-boot/rsa.h>
#include <u-boot/rsa-mod-exp.h>
#include <u-boot/sha256.h>
#include <spi_flash.h>
#include <zynqpl.h>

DECLARE_GLOBAL_DATA_PTR;

#define ZYNQ_IMAGE_PHDR_OFFSET		0x09C
#define ZYNQ_IMAGE_FSBL_LEN_OFFSET	0x040

#define ZYNQ_PART_HDR_CHKSUM_WORD_COUNT	0x0F
#define ZYNQ_PART_HDR_WORD_COUNT	0x10

#define ZYNQ_EFUSE_RSA_ENABLE_MASK	0x400

#define ZYNQ_ATTRIBUTE_PL_IMAGE_MASK		0x20
#define ZYNQ_ATTRIBUTE_CHECKSUM_TYPE_MASK	0x7000
#define ZYNQ_ATTRIBUTE_RSA_PRESENT_MASK		0x8000
#define ZYNQ_ATTRIBUTE_RSA_PART_OWNER_MASK	0x30000

#define ZYNQ_MAX_PARTITION_NUMBER	0xE

#define ZYNQ_RSA_MODULAR_SIZE			256
#define ZYNQ_RSA_MODULAR_EXT_SIZE		256
#define ZYNQ_RSA_EXPO_SIZE			64
#define ZYNQ_RSA_SPK_SIGNATURE_SIZE		256
#define ZYNQ_RSA_PARTITION_SIGNATURE_SIZE	256
#define ZYNQ_RSA_SIGNATURE_SIZE			0x6C0
#define ZYNQ_RSA_HEADER_SIZE			4
#define ZYNQ_RSA_MAGIC_WORD_SIZE		60
#define ZYNQ_RSA_PART_OWNER_UBOOT		1
#define ZYNQ_RSA_ALIGN_PPK_START		64

#define WORD_LENGTH_SHIFT	2
#define ZYNQ_MAXIMUM_IMAGE_WORD_LEN	0x40000000

#define MD5_CHECKSUM_SIZE	16

static u8 *ppkmodular;
static u8 *ppkmodularex;
static u32 ppkexp;

struct partition_hdr {
	u32 imagewordlen;	/* 0x0 */
	u32 datawordlen;	/* 0x4 */
	u32 partitionwordlen;	/* 0x8 */
	u32 loadaddr;		/* 0xC */
	u32 execaddr;		/* 0x10 */
	u32 partitionstart;	/* 0x14 */
	u32 partitionattr;	/* 0x18 */
	u32 sectioncount;	/* 0x1C */
	u32 checksumoffset;	/* 0x20 */
	u32 pads1[1];
	u32 acoffset;	/* 0x28 */
	u32 pads2[4];
	u32 checksum;		/* 0x3C */
};

struct zynq_rsa_public_key {
	uint len;		/* Length of modulus[] in number of uint32_t */
	uint32_t n0inv;		/* -1 / modulus[0] mod 2^32 */
	uint32_t *modulus;	/* modulus as little endian array */
	uint32_t *rr;		/* R^2 as little endian array */
};

struct partition_hdr part_hdr[ZYNQ_MAX_PARTITION_NUMBER];

struct headerarray {
	u32 fields[16];
};

struct zynq_rsa_public_key public_key;

static u32 fsbl_len;

/*
 * Check whether the given partition is last partition or not
 */
static int zynq_islastpartition(struct headerarray *head)
{
	int index;

	debug("zynq_islastpartition\n");
	if (head->fields[ZYNQ_PART_HDR_CHKSUM_WORD_COUNT] != 0xFFFFFFFF)
		return -1;

	for (index = 0; index < ZYNQ_PART_HDR_WORD_COUNT - 1; index++) {
		if (head->fields[index] != 0x0)
			return -1;
	}

	return 0;
}

/*
 * Get the partition count from the partition header
 */
static int zynq_get_part_count(struct partition_hdr *part_hdr_info)
{
	u32 count = 0;
	struct headerarray *hap;

	debug("zynq_get_part_count\n");

	for (count = 0; count < ZYNQ_MAX_PARTITION_NUMBER; count++) {
		hap = (struct headerarray *)&part_hdr_info[count];
		if (zynq_islastpartition(hap) != -1)
			break;
	}

	return count;
}

/*
 * Get the partition info of all the partitions available.
 */
static int zynq_get_partition_info(u32 image_base_addr)
{
	u32 parthdroffset;

	fsbl_len = *((u32 *)(image_base_addr + ZYNQ_IMAGE_FSBL_LEN_OFFSET));

	parthdroffset = *((u32 *)(image_base_addr + ZYNQ_IMAGE_PHDR_OFFSET));

	parthdroffset  += image_base_addr;

	memcpy(&part_hdr[0], (u32 *)parthdroffset,
	       (sizeof(struct partition_hdr) * ZYNQ_MAX_PARTITION_NUMBER));

	return 0;
}

/*
 * Check whether the partition header is valid or not
 */
static int zynq_validate_hdr(struct partition_hdr *header)
{
	struct headerarray *hap;
	u32 index;
	u32 checksum;

	debug("zynq_validate_hdr\n");

	hap = (struct headerarray *)header;

	for (index = 0; index < ZYNQ_PART_HDR_WORD_COUNT; index++) {
		if (hap->fields[index] != 0x0)
			break;
	}
	if (index  == ZYNQ_PART_HDR_WORD_COUNT)
		return -1;

	checksum = 0;
	for (index = 0; index < ZYNQ_PART_HDR_CHKSUM_WORD_COUNT; index++)
		checksum += hap->fields[index];

	checksum ^= 0xFFFFFFFF;

	if (hap->fields[ZYNQ_PART_HDR_CHKSUM_WORD_COUNT] != checksum) {
		printf("Error: Checksum 0x%8.8x != 0x%8.8x\r\n",
		       checksum, hap->fields[ZYNQ_PART_HDR_CHKSUM_WORD_COUNT]);
		return -1;
	}

	if (header->imagewordlen > ZYNQ_MAXIMUM_IMAGE_WORD_LEN) {
		printf("INVALID_PARTITION_LENGTH\r\n");
		return -1;
	}

	return 0;
}

/*
 * Validate the partition by calculationg the md5 checksum for the
 * partition and compare with checksum present in checksum offset of
 * partition
 */
static int zynq_validate_partition(u32 start_addr, u32 len, u32 chksum_off)
{
	u8 checksum[MD5_CHECKSUM_SIZE];
	u8 calchecksum[MD5_CHECKSUM_SIZE];

	memcpy(&checksum[0], (u32 *)chksum_off, MD5_CHECKSUM_SIZE);

	md5_wd((u8 *)start_addr, len, &calchecksum[0], 0x10000);

	if ((memcmp(checksum, calchecksum, MD5_CHECKSUM_SIZE)) != 0) {
		printf("Error: Partition DataChecksum \r\n");
		return -1;
	}
	return 0;
}

/*
 * Extract the primary public key components from already autheticated FSBL
 */
static void zynq_extract_ppk(void)
{
	u32 padsize;
	u8 *ppkptr;

	debug("zynq_extract_ppk\n");

	ppkptr = (u8 *)(fsbl_len + 0xFFFC0000);
	padsize = ((u32)ppkptr % ZYNQ_RSA_ALIGN_PPK_START);
	if (padsize != 0)
		ppkptr += (ZYNQ_RSA_ALIGN_PPK_START - padsize);

	ppkptr += ZYNQ_RSA_HEADER_SIZE;

	ppkptr += ZYNQ_RSA_MAGIC_WORD_SIZE;

	ppkmodular = (u8 *)ppkptr;
	ppkptr += ZYNQ_RSA_MODULAR_SIZE;
	ppkmodularex = (u8 *)ppkptr;
	ppkptr += ZYNQ_RSA_MODULAR_EXT_SIZE;
	ppkexp = *(u32 *)ppkptr;
}

/*
 * Calculate the inverse(-1 / modulus[0] mod 2^32 ) for the PPK
 */
static u32 zynq_calc_inv(void)
{
	u32 modulus = public_key.modulus[0];
	u32 tmp = 2;
	u32 inverse;

	inverse = modulus & 0x1;

	while (tmp) {
		inverse *= 2-modulus*inverse;
		tmp *= tmp;
	}

	return -inverse;
}

/*
 * Recreate the signature by padding the bytes and verify with hash value
 */
static int zynq_pad_and_check(u8 *signature, u8 *hash)
{
	u8 padding[] = {0x30, 0x31, 0x30, 0x0D, 0x06, 0x09, 0x60, 0x86, 0x48,
			0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04,
			0x20 };
	u8 *pad_ptr = signature + 256;
	u32 pad = 256 - 3 - 19 - 32;
	u32 ii;

	/* Re-Create PKCS#1v1.5 Padding */
	if (*--pad_ptr != 0x00 || *--pad_ptr != 0x01)
		return -1;

	for (ii = 0; ii < pad; ii++) {
		if (*--pad_ptr != 0xFF)
			return -1;
	}

	if (*--pad_ptr != 0x00)
		return -1;

	for (ii = 0; ii < sizeof(padding); ii++) {
		if (*--pad_ptr != padding[ii])
			return -1;
	}

	for (ii = 0; ii < 32; ii++) {
		if (*--pad_ptr != hash[ii])
			return -1;
	}
	return 0;
}

/*
 * Verify and extract the hash value from signature using the public key
 * and compare it with calculated hash value.
 */
static int zynq_rsa_verify_key(const struct zynq_rsa_public_key *key,
			       const u8 *sig, const u32 sig_len, const u8 *hash)
{
	int status;

	if ((!key) || (!sig) || (!hash))
		return -1;

	if (sig_len != (key->len * sizeof(uint32_t))) {
		printf("Signature is of incorrect length %d\n", sig_len);
		return -1;
	}

	/* Sanity check for stack size */
	if (sig_len > ZYNQ_RSA_SPK_SIGNATURE_SIZE) {
		printf("Signature length %u exceeds maximum %d\n", sig_len,
		       ZYNQ_RSA_SPK_SIGNATURE_SIZE);
		return -1;
	}

	u32 buf[sig_len / sizeof(uint32_t)];

	memcpy(buf, sig, sig_len);

	status = zynq_pow_mod((u32 *)key, buf);
	if (status == -1)
		return status;

	status = zynq_pad_and_check((u8 *)buf, (u8 *)hash);
	if (status == -1)
		return status;
	return 0;
}

/*
 * Authenticate the partition
 */
static int zynq_authenticate_part(u8 *buffer, u32 size)
{
	u8 hash_signature[32];
	u8 *spk_modular;
	u8 *spk_modular_ex;
	u8 *signature_ptr;
	u32 status;

	debug("zynq_authenticate_part\n");

	signature_ptr = (u8 *)(buffer + size - ZYNQ_RSA_SIGNATURE_SIZE);

	signature_ptr += ZYNQ_RSA_HEADER_SIZE;

	signature_ptr += ZYNQ_RSA_MAGIC_WORD_SIZE;

	ppkmodular = (u8 *)signature_ptr;
	signature_ptr += ZYNQ_RSA_MODULAR_SIZE;
	ppkmodularex = signature_ptr;
	signature_ptr += ZYNQ_RSA_MODULAR_EXT_SIZE;
	signature_ptr += ZYNQ_RSA_EXPO_SIZE;

	sha256_csum_wd((const unsigned char *)signature_ptr,
		       (ZYNQ_RSA_MODULAR_EXT_SIZE + ZYNQ_RSA_EXPO_SIZE +
		       ZYNQ_RSA_MODULAR_SIZE),
		       (unsigned char *)hash_signature, 0x1000);

	spk_modular = (u8 *)signature_ptr;
	signature_ptr += ZYNQ_RSA_MODULAR_SIZE;
	spk_modular_ex = (u8 *)signature_ptr;
	signature_ptr += ZYNQ_RSA_MODULAR_EXT_SIZE;
	signature_ptr += ZYNQ_RSA_EXPO_SIZE;

	public_key.len = ZYNQ_RSA_MODULAR_SIZE/sizeof(u32);
	public_key.modulus = (u32 *)ppkmodular;
	public_key.rr = (u32 *)ppkmodularex;
	public_key.n0inv = zynq_calc_inv();

	status = zynq_rsa_verify_key(&public_key, signature_ptr,
				ZYNQ_RSA_SPK_SIGNATURE_SIZE, hash_signature);

	if (status)
		return status;

	signature_ptr += ZYNQ_RSA_SPK_SIGNATURE_SIZE;

	sha256_csum_wd((const unsigned char *)buffer,
		       (size - ZYNQ_RSA_PARTITION_SIGNATURE_SIZE),
		       (unsigned char *)hash_signature, 0x1000);

	public_key.len = ZYNQ_RSA_MODULAR_SIZE/sizeof(u32);
	public_key.modulus = (u32 *)spk_modular;
	public_key.rr = (u32 *)spk_modular_ex;
	public_key.n0inv = zynq_calc_inv();

	status = zynq_rsa_verify_key(&public_key, (u8 *)signature_ptr,
				     ZYNQ_RSA_PARTITION_SIGNATURE_SIZE,
				     (u8 *)hash_signature);

	if (status)
		return status;

	return 0;
}

/*
 * Parses the partition header and verfies the authenticated and
 * encrypted image.
 */
static int do_zynq_verify_image(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	u32 silicon_ver;
	u32 image_base_addr;
	u32 status;
	u32 partition_num;
	u32 efuseval;
	u32 srcaddr;
	u32 size;
	struct partition_hdr *hdr_ptr;
	u32 part_data_len;
	u32 part_img_len;
	u32 part_attr;
	u32 part_load_addr;
	u32 part_chksum_offset;
	u32 part_start_addr;
	u32 part_total_size;
	u32 partitioncount;
	u8 encrypt_part_flag;
	u8 part_chksum_flag;
	u8 signed_part_flag;
	char *endp;

	if (argc < 2)
		goto usage;

	image_base_addr = simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		return -1;

	silicon_ver = zynq_get_silicon_version();

	/* RSA not supported in silicon versions 1.0 and 2.0 */
	if (silicon_ver == 0 || silicon_ver == 1)
		return -1;

	status = zynq_get_partition_info(image_base_addr);
	if (status == -1) {
		printf("Get Partition Info Failed\n");
		return status;
	}

	/* Extract ppk if efuse was blown Otherwise return error */
	efuseval = readl(&efuse_base->status);
	if (efuseval & ZYNQ_EFUSE_RSA_ENABLE_MASK)
		zynq_extract_ppk();
	else
		return -1;

	partitioncount = zynq_get_part_count(&part_hdr[0]);

	if ((partitioncount <= 2) ||
	    (partitioncount > ZYNQ_MAX_PARTITION_NUMBER))
		return -1;

	/* Skip the first two partitions FSBL and u-boot */
	partition_num = 2;

	while (partition_num < partitioncount) {
		if (((part_hdr[partition_num].partitionattr &
		   ZYNQ_ATTRIBUTE_RSA_PART_OWNER_MASK) >> 16) !=
		   ZYNQ_RSA_PART_OWNER_UBOOT) {
			printf("UBOOT is not Owner for partition %d\r\n",
			       partition_num);
		} else {
			hdr_ptr = &part_hdr[partition_num];
			status = zynq_validate_hdr(hdr_ptr);
			if (status == -1)
				return status;

			part_data_len = hdr_ptr->datawordlen;
			part_img_len = hdr_ptr->imagewordlen;
			part_attr = hdr_ptr->partitionattr;
			part_load_addr = hdr_ptr->loadaddr;
			part_chksum_offset = hdr_ptr->checksumoffset;
			part_start_addr = hdr_ptr->partitionstart;
			part_total_size = hdr_ptr->partitionwordlen;

			if (part_attr & ZYNQ_ATTRIBUTE_PL_IMAGE_MASK) {
				printf("Bitstream\r\n");
				return -1;
			}

			if (part_data_len != part_img_len) {
				debug("Encrypted\r\n");
				encrypt_part_flag = 1;
			} else {
				encrypt_part_flag = 0;
			}

			if (part_attr & ZYNQ_ATTRIBUTE_CHECKSUM_TYPE_MASK)
				part_chksum_flag = 1;
			else
				part_chksum_flag = 0;

			if (part_attr & ZYNQ_ATTRIBUTE_RSA_PRESENT_MASK) {
				debug("RSA Signed\r\n");
				signed_part_flag = 1;
			} else {
				signed_part_flag = 0;
			}

			srcaddr = image_base_addr +
				  (part_start_addr << WORD_LENGTH_SHIFT);

			if (part_attr & ZYNQ_ATTRIBUTE_RSA_PRESENT_MASK) {
				signed_part_flag = 1;
				size = part_total_size << WORD_LENGTH_SHIFT;
			} else {
				signed_part_flag = 0;
				size = part_img_len;
			}

			if ((part_load_addr < CONFIG_SYS_SDRAM_BASE) &&
			    ((part_load_addr + part_data_len) >
			    (CONFIG_SYS_SDRAM_BASE + gd->ram_size))) {
				printf("INVALID_LOAD_ADDRESS_FAIL\r\n");
				return -1;
			}

			memcpy((u32 *)part_load_addr, (u32 *)srcaddr, size);

			if (!signed_part_flag && !part_chksum_flag) {
				printf("Partition not signed & no chksum\n");
				continue;
			}

			if (part_chksum_flag) {
				part_chksum_offset = image_base_addr +
						     (part_chksum_offset <<
						     WORD_LENGTH_SHIFT);
				status = zynq_validate_partition(part_load_addr,
							(part_total_size <<
							WORD_LENGTH_SHIFT),
							part_chksum_offset);
				if (status != 0) {
					printf("PART_CHKSUM_FAIL\r\n");
					return -1;
				}
				debug("Partition Validation Done\r\n");
			}

			if (signed_part_flag == 1) {
				status = zynq_authenticate_part(
							   (u8 *)part_load_addr,
							   size);
				if (status != 0) {
					printf("AUTHENTICATION_FAIL\r\n");
					return -1;
				}
				debug("Authentication Done\r\n");
			}

			if (encrypt_part_flag) {
				debug("DECRYPTION \r\n");
				status = zynq_decrypt_load(part_load_addr,
							   part_img_len,
							   part_load_addr,
							   part_data_len);
				if (status != 0) {
					printf("DECRYPTION_FAIL\r\n");
					return -1;
				}
			}
		}
		partition_num++;
	}

	return 0;

usage:
	return CMD_RET_USAGE;
}

#ifdef CONFIG_SYS_LONGHELP
static char zynqrsa_help_text[] =
"zynqrsa <baseaddr>  - Verifies the authenticated and encrypted zynq images\n";
#endif

U_BOOT_CMD(
	zynqrsa,	2,	0,	do_zynq_verify_image,
	"Zynq RSA verfication ", zynqrsa_help_text
);
