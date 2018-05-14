// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marek Behun <marek.behun@nic.cz>
 */

#include <common.h>
#include <dm.h>
#include <clk.h>
#include <spi.h>
#include <linux/string.h>

#ifdef CONFIG_WDT_ARMADA_3720
#include <wdt.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_WDT_ARMADA_3720
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

#ifdef CONFIG_WDT_ARMADA_3720
	if (uclass_get_device(UCLASS_WDT, 0, &watchdog_dev)) {
		printf("Cannot find Armada 3720 watchdog!\n");
	} else {
		printf("Enabling Armada 3720 watchdog (3 minutes timeout).\n");
		wdt_start(watchdog_dev, 180000, 0);
	}
#endif

	return 0;
}

int last_stage_init(void)
{
	struct spi_slave *slave;
	struct udevice *dev;
	u8 din[10], dout[10];
	int ret, i;
	size_t len = 0;
	char module_topology[128];

	ret = spi_get_bus_and_cs(0, 1, 20000000, SPI_CPHA, "spi_generic_drv",
				 "mox-modules@1", &dev, &slave);
	if (ret)
		goto fail;

	ret = spi_claim_bus(slave);
	if (ret)
		goto fail_free;

	memset(din, 0, 10);
	memset(dout, 0, 10);

	ret = spi_xfer(slave, 80, dout, din, SPI_XFER_ONCE);
	if (ret)
		goto fail_release;

	if (din[0] != 0x00 && din[0] != 0xff)
		goto fail_release;

	printf("Module Topology:\n");
	for (i = 1; i < 10 && din[i] != 0xff; ++i) {
		u8 mid = din[i] & 0xf;
		size_t mlen;
		const char *mname = "";

		switch (mid) {
		case 0x1:
			mname = "sfp-";
			printf("% 4i: SFP Module\n", i);
			break;
		case 0x2:
			mname = "pci-";
			printf("% 4i: Mini-PCIe Module\n", i);
			break;
		case 0x3:
			mname = "topaz-";
			printf("% 4i: Topaz Switch Module\n", i);
			break;
		default:
			printf("% 4i: unknown (ID %i)\n", i, mid);
		}

		mlen = strlen(mname);
		if (len + mlen < sizeof(module_topology)) {
			strcpy(module_topology + len, mname);
			len += mlen;
		}
	}
	printf("\n");

	module_topology[len > 0 ? len - 1 : 0] = '\0';

	env_set("module_topology", module_topology);

fail_release:
	spi_release_bus(slave);
fail_free:
	spi_free_slave(slave);
fail:
	if (ret)
		printf("Cannot read module topology!\n");
	return ret;
}
