// SPDX-License-Identifier: GPL-2.0+
/*
 * Common initialisation for Qualcomm Snapdragon boards.
 * U-Boot proper only, see mem_map.c and dram.c for parts shared with SPL
 *
 * Copyright (c) 2024 Linaro Ltd.
 * Author: Casey Connolly <casey.connolly@linaro.org>
 */

#define LOG_CATEGORY LOGC_BOARD
#define pr_fmt(fmt) "QCOM: " fmt

#include <asm/psci.h>
#include <dm/ofnode.h>
#include <env.h>
#include <fdt_support.h>
#include <init.h>
#include <linux/arm-smccc.h>
#include <linux/errno.h>
#include <linux/psci.h>
#include <linux/sizes.h>
#include <lmb.h>
#include <malloc.h>
#include <soc/qcom/smem.h>
#include <sort.h>
#include <time.h>

#include "qcom-priv.h"

enum qcom_boot_source qcom_boot_source __section(".data") = 0;
enum qcom_memmap_source qcom_memmap_source __section(".data") = 0;

#if CONFIG_IS_ENABLED(SYSRESET_PSCI)
static void show_psci_version(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(ARM_PSCI_0_2_FN_PSCI_VERSION, 0, 0, 0, 0, 0, 0, 0, &res);

	/* Some older SoCs like MSM8916 don't always support PSCI */
	if ((int)res.a0 == PSCI_RET_NOT_SUPPORTED)
		return;

	debug("PSCI:  v%ld.%ld\n",
	      PSCI_VERSION_MAJOR(res.a0),
	      PSCI_VERSION_MINOR(res.a0));
}

/**
 * Most MSM8916 devices in the wild shipped without PSCI support, but the
 * upstream DTs pretend that PSCI exists. If that situation is detected here,
 * the /psci node is deleted. This is done very early to ensure the PSCI
 * firmware driver doesn't bind (which then binds a sysreset driver that won't
 * work).
 */
static void qcom_psci_fixup(void *fdt)
{
	int offset, ret;
	struct arm_smccc_res res;

	arm_smccc_smc(ARM_PSCI_0_2_FN_PSCI_VERSION, 0, 0, 0, 0, 0, 0, 0, &res);

	if ((int)res.a0 != PSCI_RET_NOT_SUPPORTED)
		return;

	offset = fdt_path_offset(fdt, "/psci");
	if (offset < 0)
		return;

	debug("Found /psci DT node on device with no PSCI. Deleting.\n");
	ret = fdt_del_node(fdt, offset);
	if (ret)
		log_err("Failed to delete /psci node: %d\n", ret);
}
#endif

/* We support booting U-Boot with an internal DT when running as a first-stage bootloader
 * or for supporting quirky devices where it's easier to leave the downstream DT in place
 * to improve ABL compatibility. Otherwise, we use the DT provided by ABL.
 */
int board_fdt_blob_setup(void **fdtp)
{
	struct fdt_header *external_fdt, *internal_fdt;
	bool internal_valid, external_valid;
	int ret = -ENODATA;

	internal_fdt = (struct fdt_header *)*fdtp;
	external_fdt = (struct fdt_header *)get_prev_bl_fdt_addr();
	external_valid = external_fdt && !fdt_check_header(external_fdt);
	internal_valid = !fdt_check_header(internal_fdt);

	/*
	 * There is no point returning an error here, U-Boot can't do anything useful in this situation.
	 * Bail out while we can still print a useful error message.
	 */
	if (!internal_valid && !external_valid)
		panic("Internal FDT is invalid and no external FDT was provided! (fdt=%#llx)\n",
		      (phys_addr_t)external_fdt);

	/* Prefer memory information from internal DT if it's present */
	if (internal_valid)
		ret = qcom_parse_memory(internal_fdt, true);

	if (ret < 0 && external_valid) {
		/* No internal FDT or it lacks a proper /memory node.
		 * The previous bootloader handed us something, let's try that.
		 */
		if (internal_valid)
			debug("No memory info in internal FDT, falling back to external\n");

		ret = qcom_parse_memory(external_fdt, false);
	}

	if (ret < 0)
		panic("No valid memory ranges found!\n");

	/* If we have an external FDT, it can only have come from the Android bootloader. */
	if (external_valid)
		qcom_boot_source = QCOM_BOOT_SOURCE_ANDROID;
	else
		qcom_boot_source = QCOM_BOOT_SOURCE_XBL;

	debug("ram_base = %#011lx, ram_size = %#011llx\n",
	      gd->ram_base, gd->ram_size);

	if (internal_valid) {
		debug("Using built in FDT\n");
		ret = -EEXIST;
	} else {
		debug("Using external FDT\n");
		*fdtp = external_fdt;
		ret = 0;
	}

#if CONFIG_IS_ENABLED(SYSRESET_PSCI)
	qcom_psci_fixup(*fdtp);
#endif

	return ret;
}

/*
 * Some boards still need board specific init code, they can implement that by
 * overriding this function.
 *
 * FIXME: get rid of board specific init code
 */
void __weak qcom_board_init(void)
{
}

int board_init(void)
{
#if CONFIG_IS_ENABLED(SYSRESET_PSCI)
	show_psci_version();
#endif
	qcom_board_init();
	return 0;
}

/**
 * out_len includes the trailing null space
 */
static int get_cmdline_option(const char *cmdline, const char *key, char *out, int out_len)
{
	const char *p, *p_end;
	int len;

	p = strstr(cmdline, key);
	if (!p)
		return -ENOENT;

	p += strlen(key);
	p_end = strstr(p, " ");
	if (!p_end)
		return -ENOENT;

	len = p_end - p;
	if (len > out_len)
		len = out_len;

	strncpy(out, p, len);
	out[len] = '\0';

	return 0;
}

/* The bootargs are populated by the previous stage bootloader */
static const char *get_cmdline(void)
{
	ofnode node;
	static const char *cmdline = NULL;

	if (cmdline)
		return cmdline;

	node = ofnode_path("/chosen");
	if (!ofnode_valid(node))
		return NULL;

	cmdline = ofnode_read_string(node, "bootargs");

	return cmdline;
}

void qcom_set_serialno(void)
{
	const char *cmdline;
	char serial[32];

	if (!qcom_socinfo_init())
		return;

	cmdline = get_cmdline();

	if (!cmdline) {
		log_debug("Failed to get bootargs\n");
		return;
	}

	get_cmdline_option(cmdline, "androidboot.serialno=", serial, sizeof(serial));
	if (serial[0] != '\0')
		env_set("serial#", serial);
}

/* Sets up the "board", and "soc" environment variables as well as constructing the devicetree
 * path, with a few quirks to handle non-standard dtb filenames. This is not meant to be a
 * comprehensive solution to automatically picking the DTB, but aims to be correct for the
 * majority case. For most devices it should be possible to make this algorithm work by
 * adjusting the root compatible property in the U-Boot DTS. Handling devices with multiple
 * variants that are all supported by a single U-Boot image will require implementing device-
 * specific detection.
 */
static void configure_env(void)
{
	const char *first_compat, *last_compat;
	char *tmp;
	char buf[32] = { 0 };
	/*
	 * Most DTB filenames follow the scheme: qcom/<soc>-[vendor]-<board>.dtb
	 * The vendor is skipped when it's a Qualcomm reference board, or the
	 * db845c.
	 */
	char dt_path[64] = { 0 };
	int compat_count, ret;
	ofnode root;

	root = ofnode_root();
	/* This is almost always 2, but be explicit that we want the first and last compatibles
	 * not the first and second.
	 */
	compat_count = ofnode_read_string_count(root, "compatible");
	if (compat_count < 2) {
		log_warning("%s: only one root compatible bailing!\n", __func__);
		return;
	}

	/* The most specific device compatible (e.g. "thundercomm,db845c") */
	ret = ofnode_read_string_index(root, "compatible", 0, &first_compat);
	if (ret < 0) {
		log_warning("Can't read first compatible\n");
		return;
	}

	strlcpy(buf, first_compat, sizeof(buf) - 1);
	tmp = buf;

	/* The Qualcomm reference boards (RBx, HDK, etc)  */
	if (!strncmp("qcom", buf, strlen("qcom"))) {
		char *soc;

		/*
		 * They all have the first compatible as "qcom,<soc>-<board>"
		 * (e.g. "qcom,qrb5165-rb5"). We extract just the part after
		 * the dash.
		 */
		if (!strsep(&tmp, ",")) {
			log_warning("compatible '%s' has no ','\n", buf);
			return;
		}
		soc = strsep(&tmp, "-");
		if (!soc) {
			log_warning("compatible '%s' has no '-'\n", buf);
			return;
		}

		env_set("soc", soc);
		env_set("board", tmp);
	} else {
		if (!strsep(&tmp, ",")) {
			log_warning("compatible '%s' has no ','\n", buf);
			return;
		}
		/*
		 * For thundercomm we just want the bit after the comma
		 * (e.g. "db845c"), for all other boards we replace the comma
		 * with a '-' and take both (e.g. "oneplus-enchilada")
		 */
		if (!strncmp("thundercomm", buf, strlen("thundercomm"))) {
			env_set("board", tmp);
		} else {
			*(tmp - 1) = '-';
			env_set("board", buf);
		}

		/* The last compatible is always the SoC compatible */
		ret = ofnode_read_string_index(root, "compatible",
					       compat_count - 1, &last_compat);
		if (ret < 0) {
			log_warning("Can't read second compatible\n");
			return;
		}

		/* Copy the last compat (e.g. "qcom,sdm845") into buf */
		memset(buf, 0, sizeof(buf));
		strlcpy(buf, last_compat, sizeof(buf) - 1);
		tmp = buf;

		/* strsep() is destructive, it replaces the comma with a \0 */
		if (!strsep(&tmp, ",")) {
			log_warning("second compatible '%s' has no ','\n", buf);
			return;
		}

		/* tmp now points to just the "sdm845" part of the string */
		env_set("soc", tmp);
	}

	/* Now build the full path name */
	snprintf(dt_path, sizeof(dt_path), "qcom/%s-%s.dtb",
		 env_get("soc"), env_get("board"));
	env_set("fdtfile", dt_path);

	qcom_set_serialno();
}

static void qcom_show_boot_context(void)
{
	const char *boot_source = "UNKNOWN";
	const char *memmap_source = "UNKNOWN";

	switch (qcom_boot_source) {
	case QCOM_BOOT_SOURCE_ANDROID:
		boot_source = "ABL";
		break;
	case QCOM_BOOT_SOURCE_XBL:
		boot_source = "XBL";
		break;
	}

	log_info("U-Boot loaded from %s\n", boot_source);
	env_set("boot_source", boot_source);

	switch (qcom_memmap_source) {
	case QCOM_MEMMAP_SOURCE_INTERNAL_FDT:
		memmap_source = "INTERNAL_FDT";
		break;
	case QCOM_MEMMAP_SOURCE_EXTERNAL_FDT:
		memmap_source = "EXTERNAL_FDT";
		break;
	case QCOM_MEMMAP_SOURCE_SMEM:
		memmap_source = "SMEM";
		break;
	}

	log_info("Memory map loaded from %s\n", memmap_source);
	env_set("memmap_source", memmap_source);
}

void __weak qcom_late_init(void)
{
}

#define KERNEL_COMP_SIZE	SZ_64M
#ifdef CONFIG_FASTBOOT_BUF_SIZE
#define FASTBOOT_BUF_SIZE CONFIG_FASTBOOT_BUF_SIZE
#else
#define FASTBOOT_BUF_SIZE 0
#endif

#define lmb_alloc(size, addr) lmb_alloc_mem(LMB_MEM_ALLOC_ANY, SZ_2M, addr, size, LMB_NONE)

/* Stolen from arch/arm/mach-apple/board.c */
int board_late_init(void)
{
	u32 status = 0, fdt_status = 0;
	phys_addr_t addr;
	struct fdt_header *fdt_blob = (struct fdt_header *)gd->fdt_blob;

	/* We need to be fairly conservative here as we support boards with just 1G of TOTAL RAM */
	status |= !lmb_alloc(SZ_128M, &addr) ?
		env_set_hex("loadaddr", addr) : 1;
	status |= env_set_hex("kernel_addr_r", addr);
	status |= !lmb_alloc(SZ_128M, &addr) ?
		env_set_hex("ramdisk_addr_r", addr) : 1;
	status |= !lmb_alloc(KERNEL_COMP_SIZE, &addr) ?
		env_set_hex("kernel_comp_addr_r", addr) : 1;
	status |= env_set_hex("kernel_comp_size", KERNEL_COMP_SIZE);
	status |= !lmb_alloc(SZ_4M, &addr) ?
		env_set_hex("scriptaddr", addr) : 1;
	status |= !lmb_alloc(SZ_4M, &addr) ?
		env_set_hex("pxefile_addr_r", addr) : 1;

	if (IS_ENABLED(CONFIG_FASTBOOT)) {
		status |= !lmb_alloc(FASTBOOT_BUF_SIZE, &addr) ?
			env_set_hex("fastboot_addr_r", addr) : 1;
		/*
		 * Override loadaddr for memory rich soc since ${loadaddr} and
		 * ${kernel_addr_r} need to be different for the Android boot image
		 * flow. It's typically safe for ${loadaddr} to be the same address
		 * as the fastboot buffer.
		 */
		status |= env_set_hex("loadaddr", addr);
	}

	fdt_status |= !lmb_alloc(SZ_2M, &addr) ?
		env_set_hex("fdt_addr_r", addr) : 1;

	if (IS_ENABLED(CONFIG_OF_LIBFDT_OVERLAY)) {
		status |= !lmb_alloc(SZ_1M, &addr) ?
			env_set_hex("fdtoverlay_addr_r", addr) : 1;
	}

	if (status || fdt_status)
		log_warning("%s: Failed to set run time variables\n", __func__);

	/* By default copy U-Boots FDT, it will be used as a fallback */
	if (fdt_status)
		log_warning("%s: Failed to reserve memory for copying FDT\n",
			    __func__);
	else
		memcpy((void *)addr, (void *)gd->fdt_blob,
		       fdt32_to_cpu(fdt_blob->totalsize));

	/* Initialise SMEM if it wasn't done already and ensure it's memory is mapped */
	qcom_smem_init();

	configure_env();
	qcom_late_init();

	qcom_show_boot_context();
	/* Configure the dfu_string for capsule updates */
	qcom_configure_capsule_updates();

	return 0;
}
