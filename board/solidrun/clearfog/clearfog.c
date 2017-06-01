/*
 * Copyright (C) 2015 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "../drivers/ddr/marvell/a38x/ddr3_a38x_topology.h"
#include <../serdes/a38x/high_speed_env_spec.h>

DECLARE_GLOBAL_DATA_PTR;

#define ETH_PHY_CTRL_REG		0
#define ETH_PHY_CTRL_POWER_DOWN_BIT	11
#define ETH_PHY_CTRL_POWER_DOWN_MASK	(1 << ETH_PHY_CTRL_POWER_DOWN_BIT)

/*
 * Those values and defines are taken from the Marvell U-Boot version
 * "u-boot-2013.01-15t1-clearfog"
 */
#define BOARD_GPP_OUT_ENA_LOW	0xffffffff
#define BOARD_GPP_OUT_ENA_MID	0xffffffff

#define BOARD_GPP_OUT_VAL_LOW	0x0
#define BOARD_GPP_OUT_VAL_MID	0x0
#define BOARD_GPP_POL_LOW	0x0
#define BOARD_GPP_POL_MID	0x0

/* IO expander on Marvell GP board includes e.g. fan enabling */
struct marvell_io_exp {
	u8 chip;
	u8 addr;
	u8 val;
};

static struct marvell_io_exp io_exp[] = {
	{ 0x20, 2, 0x40 },	/* Deassert both mini pcie reset signals */
	{ 0x20, 6, 0xf9 },
	{ 0x20, 2, 0x46 },	/* rst signals and ena USB3 current limiter */
	{ 0x20, 6, 0xb9 },
	{ 0x20, 3, 0x00 },	/* Set SFP_TX_DIS to zero */
	{ 0x20, 7, 0xbf },	/* Drive SFP_TX_DIS to zero */
};

static struct serdes_map board_serdes_map[] = {
	{SATA0, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SGMII1, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{SGMII2, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0},
};

int hws_board_topology_load(struct serdes_map **serdes_map_array, u8 *count)
{
	*serdes_map_array = board_serdes_map;
	*count = ARRAY_SIZE(board_serdes_map);
	return 0;
}

/*
 * Define the DDR layout / topology here in the board file. This will
 * be used by the DDR3 init code in the SPL U-Boot version to configure
 * the DDR3 controller.
 */
static struct hws_topology_map board_topology_map = {
	0x1, /* active interfaces */
	/* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
	{ { { {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0} },
	    SPEED_BIN_DDR_1600K,	/* speed_bin */
	    BUS_WIDTH_16,		/* memory_width */
	    MEM_4G,			/* mem_size */
	    DDR_FREQ_800,		/* frequency */
	    0, 0,			/* cas_l cas_wl */
	    HWS_TEMP_LOW} },		/* temperature */
	5,				/* Num Of Bus Per Interface*/
	BUS_MASK_32BIT			/* Busses mask */
};

struct hws_topology_map *ddr3_get_topology_map(void)
{
	/* Return the board topology as defined in the board code */
	return &board_topology_map;
}

int board_early_init_f(void)
{
	/* Configure MPP */
	writel(0x11111111, MVEBU_MPP_BASE + 0x00);
	writel(0x11111111, MVEBU_MPP_BASE + 0x04);
	writel(0x10400011, MVEBU_MPP_BASE + 0x08);
	writel(0x22043333, MVEBU_MPP_BASE + 0x0c);
	writel(0x44400002, MVEBU_MPP_BASE + 0x10);
	writel(0x41144004, MVEBU_MPP_BASE + 0x14);
	writel(0x40333333, MVEBU_MPP_BASE + 0x18);
	writel(0x00004444, MVEBU_MPP_BASE + 0x1c);

	/* Set GPP Out value */
	writel(BOARD_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(BOARD_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);

	/* Set GPP Polarity */
	writel(BOARD_GPP_POL_LOW, MVEBU_GPIO0_BASE + 0x0c);
	writel(BOARD_GPP_POL_MID, MVEBU_GPIO1_BASE + 0x0c);

	/* Set GPP Out Enable */
	writel(BOARD_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(BOARD_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);

	return 0;
}

int board_init(void)
{
	int i;

	/* Address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	/* Toggle GPIO41 to reset onboard switch and phy */
	clrbits_le32(MVEBU_GPIO1_BASE + 0x0, BIT(9));
	clrbits_le32(MVEBU_GPIO1_BASE + 0x4, BIT(9));
	/* GPIO 19 on ClearFog rev 2.1 controls the uSOM onboard phy reset */
	clrbits_le32(MVEBU_GPIO0_BASE + 0x0, BIT(19));
	clrbits_le32(MVEBU_GPIO0_BASE + 0x4, BIT(19));
	mdelay(1);
	setbits_le32(MVEBU_GPIO1_BASE + 0x0, BIT(9));
	setbits_le32(MVEBU_GPIO0_BASE + 0x0, BIT(19));
	mdelay(10);

	/* Init I2C IO expanders */
	for (i = 0; i < ARRAY_SIZE(io_exp); i++)
		i2c_write(io_exp[i].chip, io_exp[i].addr, 1, &io_exp[i].val, 1);

	return 0;
}

int checkboard(void)
{
	puts("Board: SolidRun ClearFog\n");

	return 0;
}

int board_eth_init(bd_t *bis)
{
	cpu_eth_init(bis); /* Built in controller(s) come first */
	return pci_eth_init(bis);
}
