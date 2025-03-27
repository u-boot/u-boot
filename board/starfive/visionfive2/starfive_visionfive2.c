// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 * Author: Yanhong Wang<yanhong.wang@starfivetech.com>
 */

#include <cpu_func.h>
#include <dm.h>
#include <fdt_support.h>
#include <env.h>
#include <log.h>
#include <asm/arch/eeprom.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;
#define JH7110_L2_PREFETCHER_BASE_ADDR		0x2030000
#define JH7110_L2_PREFETCHER_HART_OFFSET	0x2000
#define FDTFILE_MILK_V_MARS \
	"starfive/jh7110-milkv-mars.dtb"
#define FDTFILE_VISIONFIVE2_1_2A \
	"starfive/jh7110-starfive-visionfive-2-v1.2a.dtb"
#define FDTFILE_VISIONFIVE2_1_3B \
	"starfive/jh7110-starfive-visionfive-2-v1.3b.dtb"
#define FDTFILE_PINE64_STAR64 \
	"starfive/jh7110-pine64-star64.dtb"

/* enable U74-mc hart1~hart4 prefetcher */
static void enable_prefetcher(void)
{
	u8 hart;
	u32 *reg;

	/* JH7110 use U74MC CORE IP, it include five cores(one S7 and four U7),
	 * but only U7 cores support prefetcher configuration
	 */
	for (hart = 1; hart < 5; hart++) {
		reg = (void *)(u64)(JH7110_L2_PREFETCHER_BASE_ADDR
					+ hart * JH7110_L2_PREFETCHER_HART_OFFSET);

		mb(); /* memory barrier */
		setbits_le32(reg, 0x1);
		mb(); /* memory barrier */
	}
}

/**
 * set_fdtfile() - set the $fdtfile variable based on the board revision
 */
static void set_fdtfile(void)
{
	u8 version;
	const char *fdtfile;
	const char *product_id;

	fdtfile = env_get("fdtfile");
	if (fdtfile)
		return;

	product_id = get_product_id_from_eeprom();
	if (!product_id) {
		log_err("Can't read EEPROM\n");
		return;
	}
	if (!strncmp(product_id, "MARS", 4)) {
		fdtfile = FDTFILE_MILK_V_MARS;
	} else if (!strncmp(product_id, "VF7110", 6)) {
		version = get_pcb_revision_from_eeprom();

		switch (version) {
		case 'a':
		case 'A':
			fdtfile = FDTFILE_VISIONFIVE2_1_2A;
			break;

		case 'b':
		case 'B':
		default:
			fdtfile = FDTFILE_VISIONFIVE2_1_3B;
			break;
		}
	} else if (!strncmp(product_id, "STAR64", 6)) {
		fdtfile = FDTFILE_PINE64_STAR64;
	} else {
		log_err("Unknown product\n");
		return;
	}

	env_set("fdtfile", fdtfile);
}

int board_init(void)
{
	enable_caches();
	enable_prefetcher();

	return 0;
}

int board_late_init(void)
{
	if (CONFIG_IS_ENABLED(ID_EEPROM))
		set_fdtfile();

	return 0;
}

int board_fdt_blob_setup(void **fdtp)
{
	if (gd->arch.firmware_fdt_addr) {
		*fdtp = (ulong *)(uintptr_t)gd->arch.firmware_fdt_addr;
		return 0;
	}

	return -EEXIST;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	return fdt_fixup_memory(blob, 0x40000000, gd->ram_size);
}
