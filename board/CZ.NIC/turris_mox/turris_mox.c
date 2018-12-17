// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marek Behun <marek.behun@nic.cz>
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <clk.h>
#include <spi.h>
#include <mvebu/comphy.h>
#include <linux/string.h>
#include <linux/libfdt.h>
#include <fdt_support.h>

#ifdef CONFIG_WDT_ARMADA_37XX
#include <wdt.h>
#endif

#define MAX_MOX_MODULES		10

#define MOX_MODULE_SFP		0x1
#define MOX_MODULE_PCI		0x2
#define MOX_MODULE_TOPAZ	0x3
#define MOX_MODULE_PERIDOT	0x4
#define MOX_MODULE_USB3		0x5
#define MOX_MODULE_PASSPCI	0x6

#define ARMADA_37XX_NB_GPIO_SEL	0xd0013830
#define ARMADA_37XX_SPI_CTRL	0xd0010600
#define ARMADA_37XX_SPI_CFG	0xd0010604
#define ARMADA_37XX_SPI_DOUT	0xd0010608
#define ARMADA_37XX_SPI_DIN	0xd001060c

#define PCIE_PATH	"/soc/pcie@d0070000"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_OF_BOARD_FIXUP)
int board_fix_fdt(void *blob)
{
	u8 topology[MAX_MOX_MODULES];
	int i, size, node;
	bool enable;

	/*
	 * SPI driver is not loaded in driver model yet, but we have to find out
	 * if pcie should be enabled in U-Boot's device tree. Therefore we have
	 * to read SPI by reading/writing SPI registers directly
	 */

	writel(0x563fa, ARMADA_37XX_NB_GPIO_SEL);
	writel(0x10df, ARMADA_37XX_SPI_CFG);
	writel(0x2005b, ARMADA_37XX_SPI_CTRL);

	while (!(readl(ARMADA_37XX_SPI_CTRL) & 0x2))
		udelay(1);

	for (i = 0; i < MAX_MOX_MODULES; ++i) {
		writel(0x0, ARMADA_37XX_SPI_DOUT);

		while (!(readl(ARMADA_37XX_SPI_CTRL) & 0x2))
			udelay(1);

		topology[i] = readl(ARMADA_37XX_SPI_DIN) & 0xff;
		if (topology[i] == 0xff)
			break;

		topology[i] &= 0xf;
	}

	size = i;

	writel(0x5b, ARMADA_37XX_SPI_CTRL);

	if (size > 1 && (topology[1] == MOX_MODULE_PCI ||
			 topology[1] == MOX_MODULE_USB3 ||
			 topology[1] == MOX_MODULE_PASSPCI))
		enable = true;
	else
		enable = false;

	node = fdt_path_offset(blob, PCIE_PATH);

	if (node < 0) {
		printf("Cannot find PCIe node in U-Boot's device tree!\n");
		return 0;
	}

	if (fdt_setprop_string(blob, node, "status",
			       enable ? "okay" : "disabled") < 0) {
		printf("Cannot %s PCIe in U-Boot's device tree!\n",
		       enable ? "enable" : "disable");
		return 0;
	}

	return 0;
}
#endif

#ifdef CONFIG_WDT_ARMADA_37XX
static struct udevice *watchdog_dev;

void watchdog_reset(void)
{
	static ulong next_reset;
	ulong now;

	if (!watchdog_dev)
		return;

	now = timer_get_us();

	/* Do not reset the watchdog too often */
	if (now > next_reset) {
		wdt_reset(watchdog_dev);
		next_reset = now + 100000;
	}
}
#endif

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_WDT_ARMADA_37XX
	if (uclass_get_device(UCLASS_WDT, 0, &watchdog_dev)) {
		printf("Cannot find Armada 3720 watchdog!\n");
	} else {
		printf("Enabling Armada 3720 watchdog (3 minutes timeout).\n");
		wdt_start(watchdog_dev, 180000, 0);
	}
#endif

	return 0;
}

static int mox_do_spi(u8 *in, u8 *out, size_t size)
{
	struct spi_slave *slave;
	struct udevice *dev;
	int ret;

	ret = spi_get_bus_and_cs(0, 1, 1000000, SPI_CPHA | SPI_CPOL,
				 "spi_generic_drv", "moxtet@1", &dev,
				 &slave);
	if (ret)
		goto fail;

	ret = spi_claim_bus(slave);
	if (ret)
		goto fail_free;

	ret = spi_xfer(slave, size * 8, out, in, SPI_XFER_ONCE);

	spi_release_bus(slave);
fail_free:
	spi_free_slave(slave);
fail:
	return ret;
}

static int mox_get_topology(const u8 **ptopology, int *psize, int *pis_sd)
{
	static int is_sd;
	static u8 topology[MAX_MOX_MODULES - 1];
	static int size;
	u8 din[MAX_MOX_MODULES], dout[MAX_MOX_MODULES];
	int ret, i;

	if (size) {
		if (ptopology)
			*ptopology = topology;
		if (psize)
			*psize = size;
		if (pis_sd)
			*pis_sd = is_sd;
		return 0;
	}

	memset(din, 0, MAX_MOX_MODULES);
	memset(dout, 0, MAX_MOX_MODULES);

	ret = mox_do_spi(din, dout, MAX_MOX_MODULES);
	if (ret)
		return ret;

	if (din[0] == 0x10)
		is_sd = 1;
	else if (din[0] == 0x00)
		is_sd = 0;
	else
		return -ENODEV;

	for (i = 1; i < MAX_MOX_MODULES && din[i] != 0xff; ++i)
		topology[i - 1] = din[i] & 0xf;
	size = i - 1;

	if (ptopology)
		*ptopology = topology;
	if (psize)
		*psize = size;
	if (pis_sd)
		*pis_sd = is_sd;

	return 0;
}

int comphy_update_map(struct comphy_map *serdes_map, int count)
{
	int ret, i, size, sfpindex = -1, swindex = -1;
	const u8 *topology;

	ret = mox_get_topology(&topology, &size, NULL);
	if (ret)
		return ret;

	for (i = 0; i < size; ++i) {
		if (topology[i] == MOX_MODULE_SFP && sfpindex == -1)
			sfpindex = i;
		else if ((topology[i] == MOX_MODULE_TOPAZ ||
			  topology[i] == MOX_MODULE_PERIDOT) &&
			 swindex == -1)
			swindex = i;
	}

	if (sfpindex >= 0 && swindex >= 0) {
		if (sfpindex < swindex)
			serdes_map[0].speed = PHY_SPEED_1_25G;
		else
			serdes_map[0].speed = PHY_SPEED_3_125G;
	} else if (sfpindex >= 0) {
		serdes_map[0].speed = PHY_SPEED_1_25G;
	} else if (swindex >= 0) {
		serdes_map[0].speed = PHY_SPEED_3_125G;
	}

	return 0;
}

int last_stage_init(void)
{
	int ret, i;
	const u8 *topology;
	int module_count, is_sd;

	ret = mox_get_topology(&topology, &module_count, &is_sd);
	if (ret) {
		printf("Cannot read module topology!\n");
		return 0;
	}

	printf("Found Turris Mox %s version\n", is_sd ? "SD" : "eMMC");
	printf("Module Topology:\n");
	for (i = 0; i < module_count; ++i) {
		switch (topology[i]) {
		case MOX_MODULE_SFP:
			printf("% 4i: SFP Module\n", i + 1);
			break;
		case MOX_MODULE_PCI:
			printf("% 4i: Mini-PCIe Module\n", i + 1);
			break;
		case MOX_MODULE_TOPAZ:
			printf("% 4i: Topaz Switch Module (4-port)\n", i + 1);
			break;
		case MOX_MODULE_PERIDOT:
			printf("% 4i: Peridot Switch Module (8-port)\n", i + 1);
			break;
		case MOX_MODULE_USB3:
			printf("% 4i: USB 3.0 Module (4 ports)\n", i + 1);
			break;
		case MOX_MODULE_PASSPCI:
			printf("% 4i: Passthrough Mini-PCIe Module\n", i + 1);
			break;
		default:
			printf("% 4i: unknown (ID %i)\n", i + 1, topology[i]);
		}
	}

	printf("\n");

	return 0;
}
