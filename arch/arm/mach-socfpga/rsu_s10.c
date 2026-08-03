// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (C) 2018-2023 Intel Corporation
 *
 */

#include <limits.h>
#include <linux/compiler.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#ifdef CONFIG_DM_SPI_FLASH
#include <dm/device.h>
#endif
#include <asm/arch/mailbox_s10.h>
#include <asm/arch/rsu.h>
#include <asm/arch/rsu_s10.h>
#include <command.h>
#include <rsu_console.h>
#include <vsprintf.h>
#include <spi.h>
#include <spi_flash.h>
#include <env.h>
#include <fdt_support.h>
#include <asm/arch/rsu_flash_if.h>

DECLARE_GLOBAL_DATA_PTR;

#define RSU_S10_SPT_SLOT_MAX 127

/* Linux DTB label identifying the RSU-managed boot partition. */
#define RSU_BOOT_PARTITION_LABEL "Boot and fpga data"

static unsigned int rsu_s10_spt_entry_count(const struct socfpga_rsu_s10_spt *spt)
{
	if (spt->magic_number != RSU_S10_SPT_MAGIC_NUMBER)
		return 0;
	if (spt->entries > RSU_S10_SPT_SLOT_MAX)
		return RSU_S10_SPT_SLOT_MAX;
	return spt->entries;
}

static int rsu_print_status(void)
{
	struct rsu_status_info status_info;

	if (mbox_rsu_status((u32 *)&status_info, sizeof(status_info) / 4)) {
		puts("RSU: Firmware or flash content not supporting RSU\n");
		return -ENOTSUPP;
	}
	puts("RSU: Remote System Update Status\n");
	printf("Current Image\t: 0x%08llx\n", status_info.current_image);
	printf("Last Fail Image\t: 0x%08llx\n", status_info.fail_image);
	printf("State\t\t: 0x%08x\n", status_info.state);
	printf("Version\t\t: 0x%08x\n", status_info.version);
	printf("Error location\t: 0x%08x\n", status_info.error_location);
	printf("Error details\t: 0x%08x\n", status_info.error_details);
	if (RSU_VERSION_ACMF_VERSION(status_info.version) &&
	    RSU_VERSION_DCMF_VERSION(status_info.version))
		printf("Retry counter\t: 0x%08x\n", status_info.retry_counter);

	return 0;
}

static void rsu_print_spt_slot(const struct socfpga_rsu_s10_spt *spt,
			       unsigned int nentries)
{
	unsigned int i;

	puts("RSU: Sub-partition table content\n");
	for (i = 0; i < nentries; i++) {
		printf("%16.16s\tOffset: 0x%08x%08x\tLength: 0x%08x\tFlag : 0x%08x\n",
		       spt->spt_slot[i].name,
		       spt->spt_slot[i].offset[1],
		       spt->spt_slot[i].offset[0],
		       spt->spt_slot[i].length,
		       spt->spt_slot[i].flag);
	}
}

static void rsu_print_cpb_slot(const struct socfpga_rsu_s10_cpb *cpb)
{
	int i, j = 1;
	unsigned int nslots = cpb->nslots;

	if (nslots > ARRAY_SIZE(cpb->pointer_slot))
		nslots = ARRAY_SIZE(cpb->pointer_slot);

	puts("RSU: CMF pointer block's image pointer list\n");
	if (!nslots)
		return;
	for (i = (int)nslots - 1; i >= 0; i--) {
		if (cpb->pointer_slot[i] != ~0ULL &&
		    cpb->pointer_slot[i] != 0) {
			printf("Priority %d Offset: 0x%016llx nslot: %d\n",
			       j, cpb->pointer_slot[i], i);
			j++;
		}
	}
}

static u32 rsu_spt_slot_find_cpb(const struct socfpga_rsu_s10_spt *spt,
				 unsigned int nentries)
{
	unsigned int i;

	for (i = 0; i < nentries; i++) {
		if (strstr(spt->spt_slot[i].name, "CPB0"))
			return spt->spt_slot[i].offset[0];
	}
	puts("RSU: Cannot find CPB0 entry in sub-partition table\n");
	return 0;
}

static void rsu_s10_sanitize_spt_names(struct socfpga_rsu_s10_spt *spt,
				       unsigned int nentries)
{
	unsigned int i;

	for (i = 0; i < nentries; i++)
		spt->spt_slot[i].name[MAX_PART_NAME_LENGTH - 1] = '\0';
}

/**
 * rsu_spt_cpb_list_inner() - read mailbox SPT offsets, flash SPT/CPB, print
 * @spt0_out: if non-NULL, set after successful mailbox read (for rsu dtb)
 * @spt1_out: if non-NULL, set after successful mailbox read
 */
static int rsu_spt_cpb_list_inner(int argc, char * const argv[],
				  u32 *spt0_out, u32 *spt1_out)
{
	u32 spt_offset[4];
	u32 cpb_offset;
	u32 spt0_off, spt1_off;
	int err;
#ifdef CONFIG_DM_SPI_FLASH
	struct udevice *flash;
#else
	struct spi_flash *flash;
#endif
	struct socfpga_rsu_s10_spt spt = { 0 };
	struct socfpga_rsu_s10_cpb cpb = { 0 };
	unsigned int nentries;

	if (argc != 1)
		return CMD_RET_USAGE;

	err = rsu_print_status();
	if (err)
		return err;

	if (mbox_rsu_get_spt_offset(spt_offset, 4)) {
		puts("RSU: Error from mbox_rsu_get_spt_offset\n");
		return -ECOMM;
	}
	spt0_off = spt_offset[SPT0_INDEX];
	spt1_off = spt_offset[SPT1_INDEX];

	if (spt0_out)
		*spt0_out = spt0_off;
	if (spt1_out)
		*spt1_out = spt1_off;

	env_set_hex("rsu_sbt0", spt0_off);
	env_set_hex("rsu_sbt1", spt1_off);
	printf("RSU: Sub-partition table 0 offset 0x%08x\n", spt0_off);
	printf("RSU: Sub-partition table 1 offset 0x%08x\n", spt1_off);

	err = rsu_mtd_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SOCFPGA_RSU_SF_CS, &flash);
	if (err) {
		puts("RSU: SPI probe failed.\n");
		return -ENODEV;
	}

	if (rsu_mtd_read(flash, spt0_off, sizeof(spt), &spt)) {
		puts("RSU: rsu_mtd_read failed\n");
		err = -EIO;
		goto out;
	}

	if (spt.magic_number != RSU_S10_SPT_MAGIC_NUMBER) {
		printf("RSU: Sub-partition table magic number not match 0x%08x\n",
		       spt.magic_number);
		err = -EFAULT;
		goto out;
	}

	nentries = rsu_s10_spt_entry_count(&spt);
	rsu_s10_sanitize_spt_names(&spt, nentries);
	rsu_print_spt_slot(&spt, nentries);

	cpb_offset = rsu_spt_slot_find_cpb(&spt, nentries);
	if (!cpb_offset) {
		err = -ENXIO;
		goto out;
	}
	printf("RSU: CMF pointer block offset 0x%08x\n", cpb_offset);

	if (rsu_mtd_read(flash, cpb_offset, sizeof(cpb), &cpb)) {
		puts("RSU: rsu_mtd_read failed\n");
		err = -EIO;
		goto out;
	}

	if (cpb.magic_number != RSU_S10_CPB_MAGIC_NUMBER) {
		printf("RSU: CMF pointer block magic number not match 0x%08x\n",
		       cpb.magic_number);
		err = -EFAULT;
		goto out;
	}

	rsu_print_cpb_slot(&cpb);
	err = 0;
out:
	/* Release the probed SPI flash; no-op under DM_SPI_FLASH. */
	rsu_mtd_unclaim(flash);
	return err;
}

int rsu_spt_cpb_list(int argc, char * const argv[])
{
	return rsu_spt_cpb_list_inner(argc, argv, NULL, NULL);
}

/*
 * Strictly parse a hex u64 from argv[].
 *
 * U-Boot's simple_strtoull() silently accepts partial input ("12xyz")
 * and wraps on overflow; both would let an unintended flash offset
 * reach the SDM. Parse digit-by-digit so we can reject either case.
 *
 * Allows a single trailing '\n' for pasted-command compatibility.
 */
static int rsu_parse_hex_u64(const char *s, u64 *out)
{
	const char *start;
	u64 result = 0;

	if (!s || !*s || !out)
		return -EINVAL;

	if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
		s += 2;

	start = s;
	while (*s) {
		unsigned int digit;
		char c = *s;

		if (c >= '0' && c <= '9')
			digit = c - '0';
		else if (c >= 'a' && c <= 'f')
			digit = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			digit = c - 'A' + 10;
		else
			break;

		if (result > (ULLONG_MAX - digit) / 16)
			return -ERANGE;

		result = result * 16 + digit;
		s++;
	}

	if (s == start)
		return -EINVAL;
	if (*s != '\0' && !(*s == '\n' && s[1] == '\0'))
		return -EINVAL;

	*out = result;
	return 0;
}

int rsu_update(int argc, char * const argv[])
{
	u32 flash_offset[2];
	u64 addr;
	int ret;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (rsu_parse_hex_u64(argv[1], &addr))
		return CMD_RET_USAGE;

	flash_offset[0] = lower_32_bits(addr);
	flash_offset[1] = upper_32_bits(addr);

	printf("RSU: RSU update to 0x%08x%08x\n",
	       flash_offset[1], flash_offset[0]);
	ret = mbox_rsu_update(flash_offset);
	if (ret) {
		printf("RSU: mbox_rsu_update failed (%d)\n", ret);
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

int rsu_dtb(int argc, char * const argv[])
{
	char flash0_string[100];
	int nodeoffset, parentoffset, fdt_flash0_offset, len;
	int child, fp_off;
	u32 end;
	const fdt32_t *val;
	u32 reg[2];
	u32 spt0_off = 0;
	u32 spt1_off __always_unused = 0;
	int err;
	bool found = false;

	/* Extracting RSU info from bitstream */
	err = rsu_spt_cpb_list_inner(argc, argv, &spt0_off, &spt1_off);
	/*
	 * The shared inner helper returns CMD_RET_USAGE (positive) when
	 * argv has extra tokens. Surface that to the command framework
	 * directly instead of treating it as SPT/CPB corruption and
	 * stomping on the live DTB.
	 */
	if (err == CMD_RET_USAGE)
		return CMD_RET_USAGE;
	if (err == -ENOTSUPP) {
		return 0;
	} else if ((err == -ECOMM) || (err == -ENODEV) || (err == -EIO)) {
		return err;
	} else if (err) {
		/*
		 * There was corruption occurred in SPT or CPB, doesn't
		 * return error & let load process continue. So that Linux
		 * can recovery the corrupted SPT or CPB.
		 */
		puts("Corrupted SPT or CPB, Linux will recovery them\n");
	}

	/* Retrieve the soc partition node from Linux DTB as start offset */
	parentoffset = fdt_path_offset(working_fdt, "/soc");
	if (parentoffset < 0) {
		printf("DTB: /soc node not found. Check the dtb and fdt addr.\n");
		return -ENODEV;
	}

	/*
	 * A board may carry more than one fixed-partitions node (e.g.
	 * one for QSPI and one for NAND). Scan every fixed-partitions
	 * node to find the RSU-managed boot partition. Primary pass
	 * matches by label; fallback pass honours the legacy rsu-handle
	 * phandle on the fixed-partitions parent (older Linux DTBs).
	 */
	fp_off = parentoffset;
	while ((fp_off = fdt_node_offset_by_compatible(working_fdt, fp_off,
						       "fixed-partitions")) >= 0) {
		fdt_for_each_subnode(child, working_fdt, fp_off) {
			const char *lbl;

			lbl = fdt_getprop(working_fdt, child, "label", NULL);
			if (lbl && !strcmp(lbl, RSU_BOOT_PARTITION_LABEL)) {
				nodeoffset = child;
				found = true;
				break;
			}
		}
		if (found)
			break;
	}

	if (!found) {
		fp_off = parentoffset;
		while ((fp_off = fdt_node_offset_by_compatible(working_fdt,
							       fp_off,
							       "fixed-partitions")) >= 0) {
			const __be32 *rsu_handle;
			u32 alt_phandle = 0;

			rsu_handle = fdt_getprop(working_fdt, fp_off,
						 "rsu-handle", NULL);
			if (rsu_handle)
				alt_phandle = be32_to_cpup(rsu_handle);
			if (!alt_phandle)
				continue;

			nodeoffset = fdt_node_offset_by_phandle(working_fdt,
								alt_phandle);
			if (nodeoffset < 0)
				continue;

			printf("DTB: boot partition found via rsu-handle (legacy DTB).\n");
			found = true;
			break;
		}
	}

	if (!found) {
		printf("DTB: boot partition not found by label \"%s\" or rsu-handle.\n",
		       RSU_BOOT_PARTITION_LABEL);
		return -ENODEV;
	}

	/* Extract the flash0's reg from Linux DTB */
	fdt_flash0_offset = fdt_get_path(working_fdt, nodeoffset, flash0_string,
					 sizeof(flash0_string));
	if (fdt_flash0_offset < 0) {
		puts("DTB: qspi_boot alias node not found. Check your dts\n");
		return -ENODEV;
	}
	printf("DTB: qspi_boot node at %s\n", flash0_string);

	/* locate the boot partition */
	nodeoffset = fdt_path_offset(working_fdt, flash0_string);
	if (nodeoffset < 0) {
		printf("DTB: %s node not found\n", flash0_string);
		return -ENODEV;
	}

	/* determine initial end address of boot partition */
	val = fdt_getprop(working_fdt, nodeoffset, "reg", &len);
	if (!val) {
		printf("DTB: %s.reg was not found\n", flash0_string);
		return -ENODEV;
	}
	if (len != 2 * sizeof(fdt32_t)) {
		printf("DTB: %s.reg has incorrect length\n", flash0_string);
		return -ENODEV;
	}
	reg[0] = fdt32_to_cpu(val[0]);
	reg[1] = fdt32_to_cpu(val[1]);
	/*
	 * Reject a DTB-supplied reg window whose start+size wraps u32.
	 * Without this check, `end` underflows into a tiny value that
	 * silently passes the spt0_off > end guard below, and the
	 * subsequent (end - spt0_off) length poisons the boot partition.
	 */
	if (reg[0] > U32_MAX - reg[1]) {
		printf("DTB: %s.reg start+size overflows u32 (0x%x + 0x%x)\n",
		       flash0_string, reg[0], reg[1]);
		return -ERANGE;
	}
	end = reg[0] + reg[1];

	/* align to 64Kb flash sector size */
	end = roundup(end, 64 * 1024);

	/*
	 * Guard reg[1]: spt0_off must lie within the boot partition, else
	 * the u32 subtract below underflows into a multi-GiB length and
	 * corrupts the DTB.
	 */
	if (spt0_off > end) {
		printf("DTB: SPT0 offset 0x%x exceeds boot partition end 0x%x\n",
		       spt0_off, end);
		return -EINVAL;
	}

	/* assemble new reg value for boot partition */
	reg[0] = cpu_to_fdt32(spt0_off);
	reg[1] = cpu_to_fdt32(end  - spt0_off);

	/* update back to Linux DTB */
	return fdt_setprop(working_fdt, nodeoffset, "reg", reg, sizeof(reg));
}
