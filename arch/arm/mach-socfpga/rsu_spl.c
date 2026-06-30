// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023 Intel Corporation <www.intel.com>
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 */

#include <env_internal.h>
#include <log.h>
#include <spi.h>
#include <spi_flash.h>
#include <stdio.h>
#include <dm/device.h>
#include <asm/arch/mailbox_s10.h>
#include <asm/arch/rsu.h>
#include <asm/arch/rsu_flash_if.h>
#include <asm/arch/rsu_s10.h>
#include <asm/arch/rsu_spl.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <linux/string.h>

#define SSBL_PART_PREFIX	"SSBL."
#define RSU_ADDR_MASK		0xFFFFFFFF
#define RSU_ADDR_SHIFT		32
#define UBOOT_ENV_EXT		".env"
#define UBOOT_IMG_EXT		".img"
#define UBOOT_ITB_EXT		".itb"
#define UBOOT_ENV_PREFIX	"u-boot_"
#define UBOOT_ENV_REDUND_PREFIX	"u-boot-redund_"
#define UBOOT_PREFIX		"u-boot_"
#define FACTORY_IMG_NAME	"FACTORY_IM"

#define RSU_SPL_SPT_SLOT_MAX	127

/* Bytes; pairs with CONFIG_SYS_SPI_U_BOOT_OFFS so env-offset math cannot underflow. */
#define RSU_SPL_SSBL_FALLBACK_BYTES \
	(CONFIG_SPL_SOCFPGA_RSU_SSBL_FALLBACK * SZ_1M)

static unsigned int rsu_spl_spt_nentries(const struct socfpga_rsu_s10_spt *spt)
{
	if (spt->magic_number != RSU_S10_SPT_MAGIC_NUMBER)
		return 0;
	if (spt->entries > RSU_SPL_SPT_SLOT_MAX)
		return RSU_SPL_SPT_SLOT_MAX;
	return spt->entries;
}

static int get_spl_slot(struct socfpga_rsu_s10_spt *rsu_spt,
			size_t rsu_spt_size, int *crt_spt_index)
{
	u32 rsu_spt0_offset = 0, rsu_spt1_offset = 0;
	u32 spt_offset[4] = {0};
	struct rsu_status_info rsu_status = {0};
#ifdef CONFIG_DM_SPI_FLASH
	struct udevice *flash;
#else
	struct spi_flash *flash;
#endif
	unsigned int nentries;
	int i;
	int pret;
	int ret = -EINVAL;

	/* get rsu status */
	if (mbox_rsu_status((u32 *)&rsu_status, sizeof(rsu_status) / 4)) {
		puts("RSU: Error - mbox_rsu_status failed!\n");
		return -EOPNOTSUPP;
	}

	/* get spt offsets */
	if (mbox_rsu_get_spt_offset(spt_offset, 4)) {
		puts("RSU: Error - mbox_rsu_get_spt_offset failed!\n");
		return -EINVAL;
	}

	rsu_spt0_offset = spt_offset[SPT0_INDEX];
	rsu_spt1_offset = spt_offset[SPT1_INDEX];

	pret = rsu_mtd_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SOCFPGA_RSU_SF_CS, &flash);
	if (pret) {
		printf("RSU: Error - rsu_mtd_probe failed (%d)!\n", pret);
		return pret;
	}

	/* read spt0 */
	pret = rsu_mtd_read(flash, rsu_spt0_offset, rsu_spt_size, rsu_spt);
	if (pret) {
		printf("RSU: Error - rsu_mtd_read spt0 failed (%d)!\n", pret);
		ret = pret;
		goto out;
	}

	/* if spt0 does not have the correct magic number */
	if (rsu_spt->magic_number != RSU_S10_SPT_MAGIC_NUMBER) {
		/* read spt1 */
		pret = rsu_mtd_read(flash, rsu_spt1_offset, rsu_spt_size, rsu_spt);
		if (pret) {
			printf("RSU: Error - rsu_mtd_read spt1 failed (%d)!\n",
			       pret);
			ret = pret;
			goto out;
		}

		/* bail out if spt1 does not have the correct magic number */
		if (rsu_spt->magic_number != RSU_S10_SPT_MAGIC_NUMBER) {
			printf("RSU: Error: spt table magic number not match 0x%08x!\n",
			       rsu_spt->magic_number);
			goto out;
		}
	}

	nentries = rsu_spl_spt_nentries(rsu_spt);
	if (!nentries)
		goto out;

	/* Flash may omit NUL in 16-byte names; cap strings for strstr/printf safety */
	for (i = 0; i < (int)nentries; i++)
		rsu_spt->spt_slot[i].name[MAX_PART_NAME_LENGTH - 1] = '\0';

	/* display status */
	debug("RSU current image:  0x%08x\n", (u32)rsu_status.current_image);
	debug("RSU state:          0x%08x\n", rsu_status.state);
	debug("RSU error location: 0x%08x\n", rsu_status.error_location);
	debug("RSU error details:  0x%08x\n", rsu_status.error_details);

	/* display partitions */
	for (i = 0; i < (int)nentries; i++) {
		debug("RSU: Partition '%s' start=0x%08x length=0x%08x\n",
		      rsu_spt->spt_slot[i].name, rsu_spt->spt_slot[i].offset[0],
		      rsu_spt->spt_slot[i].length);
	}

	/* locate the SPT entry for currently loaded image */
	for (i = 0; i < (int)nentries; i++) {
		if (((rsu_status.current_image & RSU_ADDR_MASK) ==
			rsu_spt->spt_slot[i].offset[0]) &&
		   ((rsu_status.current_image >> RSU_ADDR_SHIFT) ==
			rsu_spt->spt_slot[i].offset[1])) {
			*crt_spt_index = i;
			ret = 0;
			goto out;
		}
	}

	puts("RSU: Error - could not locate SPL partition in the SPT table!\n");
out:
	/* Release the probed SPI flash; no-op under DM_SPI_FLASH. */
	rsu_mtd_unclaim(flash);
	return ret;
}

static int get_ssbl_slot(struct socfpga_rsu_s10_spt_slot *rsu_ssbl_slot)
{
	struct socfpga_rsu_s10_spt rsu_spt = {0};
	int crt_spt_index = -EINVAL;
	char *result;
	unsigned int nentries;
	int i, ret;

	rsu_ssbl_slot->offset[0] = -EINVAL;

	ret = get_spl_slot(&rsu_spt, sizeof(rsu_spt), &crt_spt_index);
	if (ret) {
		puts("RSU: Error - could not locate partition in the SPT table!\n");
		return ret;
	}

	nentries = rsu_spl_spt_nentries(&rsu_spt);

	/* locate the u-boot proper(SSBL) partition and return its address */
	for (i = 0; i < (int)nentries; i++) {
		/* get the substring ptr to the first occurrence of SSBL. prefix */
		result = strstr(rsu_spt.spt_slot[i].name, SSBL_PART_PREFIX);

		/* skip if not found the SSBL prefix */
		if (!result)
			continue;

		/* check if the prefix is located at the first */
		if (result == rsu_spt.spt_slot[i].name) {
			/* move to the substring after SSBL. prefix */
			result += strlen(SSBL_PART_PREFIX);

			/* compare SPL's spt name after the prefix */
			if (!strncmp(result, rsu_spt.spt_slot[crt_spt_index].name,
				     MAX_PART_NAME_LENGTH - strlen(SSBL_PART_PREFIX)) ||
			    !strncmp(result, rsu_spt.spt_slot[crt_spt_index].name,
				     strlen(FACTORY_IMG_NAME))) {
				printf("RSU: found SSBL partition %s at address 0x%08x.\n",
				       result, (int)rsu_spt.spt_slot[i].offset[0]);
				memcpy(rsu_ssbl_slot, &rsu_spt.spt_slot[i],
				       sizeof(struct socfpga_rsu_s10_spt_slot));

				return 0;
			}
		}
	}

	/* fail to find u-boot proper(SSBL) */
	if (crt_spt_index >= 0 && crt_spt_index < (int)nentries)
		printf("RSU: Error - could not find u-boot proper partition SSBL.%s!\n",
		       rsu_spt.spt_slot[crt_spt_index].name);
	else
		puts("RSU: Error - could not find u-boot proper (SSBL) partition!\n");

	return -EINVAL;
}

int rsu_spl_mmc_filename(char *filename, int max_size)
{
	struct socfpga_rsu_s10_spt rsu_spt = {0};
	int crt_spt_index = -EINVAL;
	int ret;

	if (!filename) {
		printf("RSU: filename is NULL!\n");
		return -ENOENT;
	}

	/*
	 * max_size is `int` for ABI reasons but used as size_t below;
	 * reject non-positive up front so it cannot sign-extend.
	 */
	if (max_size <= 0)
		return -EINVAL;

	if ((strlen(UBOOT_PREFIX) + MAX_PART_NAME_LENGTH + strlen(UBOOT_ITB_EXT))
	     > (size_t)max_size)
		return -ENAMETOOLONG;

	ret = get_spl_slot(&rsu_spt, sizeof(rsu_spt), &crt_spt_index);
	if (ret) {
		if (ret == -EOPNOTSUPP) {
			puts("RSU: Error - mbox_rsu_status failed! Check for RSU image.\n");
			return -EOPNOTSUPP;
		}

		panic("ERROR: could not find u-boot proper (SSBL) for MMC load");
	}

	if (IS_ENABLED(CONFIG_SPL_LOAD_FIT))
		ret = snprintf(filename, (size_t)max_size, "%s%s%s", UBOOT_PREFIX,
			       rsu_spt.spt_slot[crt_spt_index].name, UBOOT_ITB_EXT);
	else
		ret = snprintf(filename, (size_t)max_size, "%s%s%s", UBOOT_PREFIX,
			       rsu_spt.spt_slot[crt_spt_index].name, UBOOT_IMG_EXT);

	if (ret < 0 || ret >= max_size)
		return -ENAMETOOLONG;

	printf("%s, filename: %s\n", __func__, filename);
	return 0;
}

int rsu_spl_mmc_env_name(char *filename, int max_size, bool redund)
{
	struct socfpga_rsu_s10_spt rsu_spt = {0};
	int crt_spt_index = -EINVAL;
	int ret;

	if (!filename) {
		printf("RSU: filename is NULL!\n");
		return -ENOENT;
	}

	/* Same signed/unsigned guard as rsu_spl_mmc_filename(). */
	if (max_size <= 0)
		return -EINVAL;

	/*
	 * Only one of UBOOT_ENV_REDUND_PREFIX / UBOOT_ENV_PREFIX is used
	 * per call; size against the chosen one so a buffer that actually
	 * fits is not falsely rejected.
	 */
	if ((strlen(redund ? UBOOT_ENV_REDUND_PREFIX : UBOOT_ENV_PREFIX) +
	     MAX_PART_NAME_LENGTH + strlen(UBOOT_ENV_EXT)) > (size_t)max_size)
		return -ENAMETOOLONG;

	ret = get_spl_slot(&rsu_spt, sizeof(rsu_spt), &crt_spt_index);
	if (ret) {
		if (ret == -EOPNOTSUPP) {
			puts("RSU: Error - mbox_rsu_status failed! Check for RSU image.\n");
			return -EOPNOTSUPP;
		}

		/* should throw error if cannot find u-boot proper(SSBL) in MMC */
		printf("ERROR: could not find u-boot.env!");
		return ret;
	}

	if (redund)
		ret = snprintf(filename, (size_t)max_size, "%s%s%s",
			       UBOOT_ENV_REDUND_PREFIX,
			       rsu_spt.spt_slot[crt_spt_index].name,
			       UBOOT_ENV_EXT);
	else
		ret = snprintf(filename, (size_t)max_size, "%s%s%s",
			       UBOOT_ENV_PREFIX,
			       rsu_spt.spt_slot[crt_spt_index].name,
			       UBOOT_ENV_EXT);

	if (ret < 0 || ret >= max_size)
		return -ENAMETOOLONG;

	printf("%s, filename: %s\n", __func__, filename);
	return 0;
}

u32 rsu_spl_ssbl_address(bool is_qspi_imge_check)
{
	int ret;
	struct socfpga_rsu_s10_spt_slot rsu_ssbl_slot = {0};

	ret = get_ssbl_slot(&rsu_ssbl_slot);
	if (ret) {
		if (ret == -EOPNOTSUPP) {
			puts("RSU: Error - mbox_rsu_status failed! Check for RSU image.\n");
			return CONFIG_SYS_SPI_U_BOOT_OFFS;
		}

		/* should throw error if cannot find u-boot proper(SSBL) address */
		if (is_qspi_imge_check) {
			panic("ERROR: could not find u-boot proper(SSBL) address!");
		} else {
			printf("ERROR: could not find u-boot env address!");
			return CONFIG_SYS_SPI_U_BOOT_OFFS;
		}
	}

	if (!rsu_ssbl_slot.length) {
		if (is_qspi_imge_check)
			panic("ERROR: could not find u-boot proper(SSBL) size!");
		/* No log here; the size() variant reports it. */
		return CONFIG_SYS_SPI_U_BOOT_OFFS;
	}

	printf("RSU: Success found SSBL at offset: %08x.\n",
	       rsu_ssbl_slot.offset[0]);
	return rsu_ssbl_slot.offset[0];
}

u32 rsu_spl_ssbl_size(bool is_qspi_imge_check)
{
	int ret;
	struct socfpga_rsu_s10_spt_slot rsu_ssbl_slot = {0};

	/* check for valid u-boot proper(SSBL) address for the size */
	ret = get_ssbl_slot(&rsu_ssbl_slot);
	if (ret) {
		if (ret == -EOPNOTSUPP) {
			printf("ERROR: Invalid address, could not retrieve SSBL size!");
			return RSU_SPL_SSBL_FALLBACK_BYTES;
		}

		/* should throw error if cannot find u-boot proper(SSBL) address */
		if (is_qspi_imge_check) {
			panic("ERROR: could not find u-boot proper(SSBL) address!");
		} else {
			printf("ERROR: could not find u-boot env address!");
			return RSU_SPL_SSBL_FALLBACK_BYTES;
		}
	}

	if (!rsu_ssbl_slot.length) {
		/* throw error if cannot find u-boot proper(SSBL) size */
		printf("ERROR: could not retrieve u-boot proper(SSBL) size!");
		if (is_qspi_imge_check)
			panic("ERROR: could not find u-boot proper(SSBL) size!");
		return RSU_SPL_SSBL_FALLBACK_BYTES;
	}

	printf("RSU: Success found SSBL with length: %08x.\n",
	       rsu_ssbl_slot.length);
	return rsu_ssbl_slot.length;
}

unsigned int spl_spi_get_uboot_offs(struct spi_flash *flash)
{
	return rsu_spl_ssbl_address(true);
}

u32 env_sf_get_env_offset(void)
{
	/* Place the environment at the tail of the selected SSBL slot. */
	return rsu_spl_ssbl_address(false) + rsu_spl_ssbl_size(false) -
	       CONFIG_ENV_SIZE;
}
