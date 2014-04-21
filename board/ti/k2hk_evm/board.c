/*
 * K2HK EVM : Board initialization
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <exports.h>
#include <fdt_support.h>
#include <libfdt.h>

#include <asm/arch/hardware.h>
#include <asm/arch/clock.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/arch/nand_defs.h>
#include <asm/arch/emac_defs.h>
#include <asm/arch/psc_defs.h>

DECLARE_GLOBAL_DATA_PTR;

u32 device_big_endian;

unsigned int external_clk[ext_clk_count] = {
	[sys_clk]	=	122880000,
	[alt_core_clk]	=	125000000,
	[pa_clk]	=	122880000,
	[tetris_clk]	=	125000000,
	[ddr3a_clk]	=	100000000,
	[ddr3b_clk]	=	100000000,
	[mcm_clk]	=	312500000,
	[pcie_clk]	=	100000000,
	[sgmii_srio_clk] =	156250000,
	[xgmii_clk]	=	156250000,
	[usb_clk]	=	100000000,
	[rp1_clk]	=	123456789    /* TODO: cannot find
						what is that */
};

static struct async_emif_config async_emif_config[ASYNC_EMIF_NUM_CS] = {
	{			/* CS0 */
		.mode		= ASYNC_EMIF_MODE_NAND,
		.wr_setup	= 0xf,
		.wr_strobe	= 0x3f,
		.wr_hold	= 7,
		.rd_setup	= 0xf,
		.rd_strobe	= 0x3f,
		.rd_hold	= 7,
		.turn_around	= 3,
		.width		= ASYNC_EMIF_8,
	},

};

static struct pll_init_data pll_config[] = {
	CORE_PLL_1228,
	PASS_PLL_983,
	TETRIS_PLL_1200,
};

int dram_init(void)
{
	init_ddr3();

	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_MAX_RAM_BANK_SIZE);
	init_async_emif(ARRAY_SIZE(async_emif_config), async_emif_config);
	return 0;
}

#ifdef CONFIG_DRIVER_TI_KEYSTONE_NET
struct eth_priv_t eth_priv_cfg[] = {
	{
		.int_name	= "K2HK_EMAC",
		.rx_flow	= 22,
		.phy_addr	= 0,
		.slave_port	= 1,
		.sgmii_link_type = SGMII_LINK_MAC_PHY,
	},
	{
		.int_name	= "K2HK_EMAC1",
		.rx_flow	= 23,
		.phy_addr	= 1,
		.slave_port	= 2,
		.sgmii_link_type = SGMII_LINK_MAC_PHY,
	},
	{
		.int_name	= "K2HK_EMAC2",
		.rx_flow	= 24,
		.phy_addr	= 2,
		.slave_port	= 3,
		.sgmii_link_type = SGMII_LINK_MAC_MAC_FORCED,
	},
	{
		.int_name	= "K2HK_EMAC3",
		.rx_flow	= 25,
		.phy_addr	= 3,
		.slave_port	= 4,
		.sgmii_link_type = SGMII_LINK_MAC_MAC_FORCED,
	},
};

int get_eth_env_param(char *env_name)
{
	char *env;
	int  res = -1;

	env = getenv(env_name);
	if (env)
		res = simple_strtol(env, NULL, 0);

	return res;
}

int board_eth_init(bd_t *bis)
{
	int	j;
	int	res;
	char	link_type_name[32];

	for (j = 0; j < (sizeof(eth_priv_cfg) / sizeof(struct eth_priv_t));
	     j++) {
		sprintf(link_type_name, "sgmii%d_link_type", j);
		res = get_eth_env_param(link_type_name);
		if (res >= 0)
			eth_priv_cfg[j].sgmii_link_type = res;

		keystone2_emac_initialize(&eth_priv_cfg[j]);
	}

	return 0;
}
#endif

/* Byte swap the 32-bit data if the device is BE */
int cpu_to_bus(u32 *ptr, u32 length)
{
	u32 i;

	if (device_big_endian)
		for (i = 0; i < length; i++, ptr++)
			*ptr = __swab32(*ptr);

	return 0;
}

#if defined(CONFIG_BOARD_EARLY_INIT_F)
int board_early_init_f(void)
{
	init_plls(ARRAY_SIZE(pll_config), pll_config);
	return 0;
}
#endif

int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
#define K2_DDR3_START_ADDR 0x80000000
void ft_board_setup(void *blob, bd_t *bd)
{
	u64 start[2];
	u64 size[2];
	char name[32], *env, *endp;
	int lpae, nodeoffset;
	u32 ddr3a_size;
	int nbanks;

	env = getenv("mem_lpae");
	lpae = env && simple_strtol(env, NULL, 0);

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
		start[0] -= K2_DDR3_START_ADDR;
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
	if (lpae) {
		u64 initrd_start, initrd_end;
		u32 *prop1, *prop2;
		int err;
		nodeoffset = fdt_path_offset(blob, "/chosen");
		if (nodeoffset >= 0) {
			prop1 = (u32 *)fdt_getprop(blob, nodeoffset,
					    "linux,initrd-start", NULL);
			prop2 = (u32 *)fdt_getprop(blob, nodeoffset,
					    "linux,initrd-end", NULL);
			if (prop1 && prop2) {
				initrd_start = __be32_to_cpu(*prop1);
				initrd_start -= K2_DDR3_START_ADDR;
				initrd_start += CONFIG_SYS_LPAE_SDRAM_BASE;
				initrd_start = __cpu_to_be64(initrd_start);
				initrd_end = __be32_to_cpu(*prop2);
				initrd_end -= K2_DDR3_START_ADDR;
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
	int	lpae;
	char	*env;
	u64	*reserve_start, size;

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
				*reserve_start -= K2_DDR3_START_ADDR;
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
