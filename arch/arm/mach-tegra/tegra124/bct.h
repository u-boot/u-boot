/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _BCT_H_
#define _BCT_H_

/*
 * Defines the BCT parametres for T124
 */
#define UBCT_LENGTH		0x6b0  /* bytes */
#define SBCT_LENGTH		0x1950 /* bytes */

#define BCT_HASH		0x10
#define EBT_ALIGNMENT		0x10

/*
 * Defines the CMAC-AES-128 hash length in 32 bit words. (128 bits = 4 words)
 */
#define NVBOOT_CMAC_AES_HASH_LENGTH		4

/*
 * Defines the RSA modulus length in 32 bit words used for PKC secure boot.
 */
#define NVBOOT_SE_RSA_MODULUS_LENGTH		64

/*
 * Defines the maximum number of bootloader descriptions in the BCT.
 */
#define NVBOOT_MAX_BOOTLOADERS			4

struct nv_bootloader_info {
	u32 version;
	u32 start_blk;
	u32 start_page;
	u32 length;
	u32 load_addr;
	u32 entry_point;
	u32 attribute;

	/* Specifies the AES-CMAC MAC or RSASSA-PSS signature of the BL. */
	u32 crypto_hash[NVBOOT_CMAC_AES_HASH_LENGTH];
	u32 bl_rsa_sig[NVBOOT_SE_RSA_MODULUS_LENGTH];
};

struct nvboot_config_table {
	u32 ubct_unused1[196];
	u32 crypto_hash[NVBOOT_CMAC_AES_HASH_LENGTH];
	u32 ubct_unused2[228];

	u32 sbct_unused1[1318];
	u32 bootloader_used;
	struct nv_bootloader_info bootloader[NVBOOT_MAX_BOOTLOADERS];
	u32 sbct_unused2;
};

#endif /* _BCT_H_ */
