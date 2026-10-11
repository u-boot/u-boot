// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Dennis Gilmore <dgilmore@redhat.com>
 *  based on board/solidrun/clearfog/clearfog.c
 */

#include <config.h>
#include <env.h>
#include <i2c.h>
#include <init.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <spi-mem.h>
#include <spi_flash.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <dm/device.h>
#include <linux/string.h>
#include <u-boot/sha256.h>

#include "../drivers/ddr/marvell/a38x/ddr3_init.h"
#include <../serdes/a38x/high_speed_env_spec.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Those values and defines are taken from the Marvell U-Boot version
 * "u-boot-2013.01-15t1-helios4" as well as the upstream config for clearfog
 */
#define BOARD_GPP_OUT_ENA_LOW	0xffffffff
#define BOARD_GPP_OUT_ENA_MID	0xffffffff

#define BOARD_GPP_OUT_VAL_LOW	0x0
#define BOARD_GPP_OUT_VAL_MID	0x0
#define BOARD_GPP_POL_LOW	0x0
#define BOARD_GPP_POL_MID	0x0

static struct serdes_map board_serdes_map[] = {
	{SATA0, SERDES_SPEED_6_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SATA1, SERDES_SPEED_6_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SATA3, SERDES_SPEED_6_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SATA2, SERDES_SPEED_6_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
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
static struct mv_ddr_topology_map board_topology_map = {
	DEBUG_LEVEL_ERROR,
	0x1, /* active interfaces */
	/* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
	{ { { {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0} },
	    SPEED_BIN_DDR_1600K,	/* speed_bin */
	    MV_DDR_DEV_WIDTH_16BIT,	/* memory_width */
	    MV_DDR_DIE_CAP_8GBIT,	/* mem_size */
	    MV_DDR_FREQ_800,		/* frequency */
	    0, 0,			/* cas_wl cas_l */
	    MV_DDR_TEMP_LOW,		/* temperature */
	    MV_DDR_TIM_DEFAULT} },	/* timing */
	BUS_MASK_32BIT_ECC,		/* Busses mask */
	MV_DDR_CFG_DEFAULT,		/* ddr configuration data source */
	NOT_COMBINED,			/* ddr twin-die combined */
	{ {0} },			/* raw spd data */
	{0},				/* timing parameters */
	{ {0} },			/* electrical configuration */
	{0,},				/* electrical parameters */
	0x30000,			/* ODT configuration */
	0x3,				/* clock enable mask */
};

struct mv_ddr_topology_map *mv_ddr_topology_map_get(void)
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
	/* Address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

int checkboard(void)
{
	puts("Board: Helios4\n");

	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	cpu_eth_init(bis); /* Built in controller(s) come first */
	return pci_eth_init(bis);
}

#define SPINOR_OP_RDUID		0x4b
#define SPINOR_UID_LEN		8

static int helios4_read_flash_uid(u8 *uid)
{
	struct spi_mem_op op =
		SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_RDUID, 1),
			   SPI_MEM_OP_NO_ADDR,
			   SPI_MEM_OP_DUMMY(4, 1),
			   SPI_MEM_OP_DATA_IN(SPINOR_UID_LEN, uid, 1));
	struct spi_flash *flash;
	struct udevice *dev;
	int ret;

	ret = spi_flash_probe_bus_cs(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS, &dev);
	if (ret)
		return ret;

	flash = dev_get_uclass_priv(dev);
	ret = spi_mem_exec_op(flash->spi, &op);
	if (ret)
		return ret;

	if (!memchr_inv(uid, 0x00, SPINOR_UID_LEN) ||
	    !memchr_inv(uid, 0xff, SPINOR_UID_LEN))
		return -ENODATA;

	return 0;
}

/*
 * The board has no factory MAC address, so derive a stable one from the
 * unique ID of the SPI NOR on the SoM.
 */
int misc_init_r(void)
{
	u8 uid[SPINOR_UID_LEN];
	u8 mac[SHA256_SUM_LEN];

	/* ethernet@70000 is aliased as ethernet1 */
	if (env_get("eth1addr") || helios4_read_flash_uid(uid))
		return 0;

	sha256_csum_wd(uid, sizeof(uid), mac, CHUNKSZ_SHA256);
	mac[0] &= 0xfe;	/* unicast */
	mac[0] |= 0x02;	/* locally administered */
	eth_env_set_enetaddr("eth1addr", mac);

	return 0;
}
