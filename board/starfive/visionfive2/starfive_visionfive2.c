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
 * set_fdtfile() - set the $fdtfile variable based on product data in EEPROM
 */
static void set_fdtfile(void)
{
	const char *fdtfile;

	fdtfile = env_get("fdtfile");
	if (fdtfile)
		return;

	if (!get_product_id_from_eeprom()) {
		log_err("Can't read EEPROM\n");
		return;
	}

	if (!strncmp(get_product_id_from_eeprom(), "FML13V01", 8)) {
		fdtfile = "starfive/jh7110-deepcomputing-fml13v01.dtb";
	} else if (!strncmp(get_product_id_from_eeprom(), "MARS", 4)) {
		fdtfile = "starfive/jh7110-milkv-mars.dtb";
	} else if (!strncmp(get_product_id_from_eeprom(), "STAR64", 6)) {
		fdtfile = "starfive/jh7110-pine64-star64.dtb";
	} else if (!strncmp(get_product_id_from_eeprom(), "VF7110", 6)) {
		switch (get_pcb_revision_from_eeprom()) {
		case 'a':
		case 'A':
			fdtfile = "starfive/jh7110-starfive-visionfive-2-v1.2a.dtb";
			break;
		case 'b':
		case 'B':
			fdtfile = "starfive/jh7110-starfive-visionfive-2-v1.3b.dtb";
			break;
		default:
			log_err("Unknown revision\n");
			return;
		}
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

int ft_board_setup(void *blob, struct bd_info *bd)
{
	return fdt_fixup_memory(blob, 0x40000000, gd->ram_size);
}
