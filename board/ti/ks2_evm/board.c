/*
 * Keystone : Board initialization
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include "board.h"
#include <common.h>
#include <exports.h>
#include <fdt_support.h>
#include <asm/arch/ddr3.h>
#include <asm/arch/emac_defs.h>
#include <asm/ti-common/ti-aemif.h>

DECLARE_GLOBAL_DATA_PTR;

static struct aemif_config aemif_configs[] = {
	{			/* CS0 */
		.mode		= AEMIF_MODE_NAND,
		.wr_setup	= 0xf,
		.wr_strobe	= 0x3f,
		.wr_hold	= 7,
		.rd_setup	= 0xf,
		.rd_strobe	= 0x3f,
		.rd_hold	= 7,
		.turn_around	= 3,
		.width		= AEMIF_WIDTH_8,
	},
};

int dram_init(void)
{
	ddr3_init();

	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_MAX_RAM_BANK_SIZE);
	aemif_init(ARRAY_SIZE(aemif_configs), aemif_configs);
	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_LINUX_BOOT_PARAM_ADDR;

	return 0;
}

#ifdef CONFIG_DRIVER_TI_KEYSTONE_NET
int get_eth_env_param(char *env_name)
{
	char *env;
	int res = -1;

	env = getenv(env_name);
	if (env)
		res = simple_strtol(env, NULL, 0);

	return res;
}

int board_eth_init(bd_t *bis)
{
	int j;
	int res;
	int port_num;
	char link_type_name[32];

	port_num = get_num_eth_ports();

	for (j = 0; j < port_num; j++) {
		sprintf(link_type_name, "sgmii%d_link_type", j);
		res = get_eth_env_param(link_type_name);
		if (res >= 0)
			eth_priv_cfg[j].sgmii_link_type = res;

		keystone2_emac_initialize(&eth_priv_cfg[j]);
	}

	return 0;
}
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	int lpae;
	char *env;
	char *endp;
	int nbanks;
	u64 size[2];
	u64 start[2];
	char name[32];
	int nodeoffset;
	u32 ddr3a_size;
	int unitrd_fixup = 0;

	env = getenv("mem_lpae");
	lpae = env && simple_strtol(env, NULL, 0);
	env = getenv("uinitrd_fixup");
	unitrd_fixup = env && simple_strtol(env, NULL, 0);

	ddr3a_size = 0;
	if (lpae) {
		env = getenv("ddr3a_size");
		if (env)
			ddr3a_size = simple_strtol(env, NULL, 10);
		if ((ddr3a_size != 8) && (ddr3a_size != 4))
			ddr3a_size = 0;
	}

	nbanks = 1;
	start[0] = bd->bi_dram[0].start;
	size[0]  = bd->bi_dram[0].size;

	/* adjust memory start address for LPAE */
	if (lpae) {
		start[0] -= CONFIG_SYS_SDRAM_BASE;
		start[0] += CONFIG_SYS_LPAE_SDRAM_BASE;
	}

	if ((size[0] == 0x80000000) && (ddr3a_size != 0)) {
		size[1] = ((u64)ddr3a_size - 2) << 30;
		start[1] = 0x880000000;
		nbanks++;
	}

	/* reserve memory at start of bank */
	sprintf(name, "mem_reserve_head");
	env = getenv(name);
	if (env) {
		start[0] += ustrtoul(env, &endp, 0);
		size[0] -= ustrtoul(env, &endp, 0);
	}

	sprintf(name, "mem_reserve");
	env = getenv(name);
	if (env)
		size[0] -= ustrtoul(env, &endp, 0);

	fdt_fixup_memory_banks(blob, start, size, nbanks);

	/* Fix up the initrd */
	if (lpae && unitrd_fixup) {
		int err;
		u32 *prop1, *prop2;
		u64 initrd_start, initrd_end;

		nodeoffset = fdt_path_offset(blob, "/chosen");
		if (nodeoffset >= 0) {
			prop1 = (u32 *)fdt_getprop(blob, nodeoffset,
					    "linux,initrd-start", NULL);
			prop2 = (u32 *)fdt_getprop(blob, nodeoffset,
					    "linux,initrd-end", NULL);
			if (prop1 && prop2) {
				initrd_start = __be32_to_cpu(*prop1);
				initrd_start -= CONFIG_SYS_SDRAM_BASE;
				initrd_start += CONFIG_SYS_LPAE_SDRAM_BASE;
				initrd_start = __cpu_to_be64(initrd_start);
				initrd_end = __be32_to_cpu(*prop2);
				initrd_end -= CONFIG_SYS_SDRAM_BASE;
				initrd_end += CONFIG_SYS_LPAE_SDRAM_BASE;
				initrd_end = __cpu_to_be64(initrd_end);

				err = fdt_delprop(blob, nodeoffset,
						  "linux,initrd-start");
				if (err < 0)
					puts("error deleting initrd-start\n");

				err = fdt_delprop(blob, nodeoffset,
						  "linux,initrd-end");
				if (err < 0)
					puts("error deleting initrd-end\n");

				err = fdt_setprop(blob, nodeoffset,
						  "linux,initrd-start",
						  &initrd_start,
						  sizeof(initrd_start));
				if (err < 0)
					puts("error adding initrd-start\n");

				err = fdt_setprop(blob, nodeoffset,
						  "linux,initrd-end",
						  &initrd_end,
						  sizeof(initrd_end));
				if (err < 0)
					puts("error adding linux,initrd-end\n");
			}
		}
	}
}

void ft_board_setup_ex(void *blob, bd_t *bd)
{
	int lpae;
	u64 size;
	char *env;
	u64 *reserve_start;

	env = getenv("mem_lpae");
	lpae = env && simple_strtol(env, NULL, 0);

	if (lpae) {
		/*
		 * the initrd and other reserved memory areas are
		 * embedded in in the DTB itslef. fix up these addresses
		 * to 36 bit format
		 */
		reserve_start = (u64 *)((char *)blob +
				       fdt_off_mem_rsvmap(blob));
		while (1) {
			*reserve_start = __cpu_to_be64(*reserve_start);
			size = __cpu_to_be64(*(reserve_start + 1));
			if (size) {
				*reserve_start -= CONFIG_SYS_SDRAM_BASE;
				*reserve_start +=
					CONFIG_SYS_LPAE_SDRAM_BASE;
				*reserve_start =
					__cpu_to_be64(*reserve_start);
			} else {
				break;
			}
			reserve_start += 2;
		}
	}
}
#endif
