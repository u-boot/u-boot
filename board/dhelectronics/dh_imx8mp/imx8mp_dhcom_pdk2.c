// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <i2c_eeprom.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>

#include "lpddr4_timing.h"

DECLARE_GLOBAL_DATA_PTR;

int mach_cpu_init(void)
{
	icache_enable();
	return 0;
}

int board_phys_sdram_size(phys_size_t *size)
{
	const u16 memsz[] = { 512, 1024, 1536, 2048, 3072, 4096, 6144, 8192 };
	u8 memcfg = dh_get_memcfg();

	*size = (u64)memsz[memcfg] << 20ULL;

	return 0;
}

/* IMX8M SNVS registers needed for the bootcount functionality */
#define SNVS_BASE_ADDR			0x30370000
#define SNVS_LPSR			0x4c
#define SNVS_LPLVDR			0x64
#define SNVS_LPPGDR_INIT		0x41736166

static void setup_snvs(void)
{
	/* Enable SNVS clock */
	clock_enable(CCGR_SNVS, 1);
	/* Initialize glitch detect */
	writel(SNVS_LPPGDR_INIT, SNVS_BASE_ADDR + SNVS_LPLVDR);
	/* Clear interrupt status */
	writel(0xffffffff, SNVS_BASE_ADDR + SNVS_LPSR);
}

static void setup_eqos(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Set INTF as RGMII, enable RGMII TXC clock. */
	clrsetbits_le32(&gpr->gpr[1],
			IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL_MASK, BIT(16));
	setbits_le32(&gpr->gpr[1], BIT(19) | BIT(21));

	set_clk_eqos(ENET_125MHZ);
}

static void setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Enable RGMII TX clk output. */
	setbits_le32(&gpr->gpr[1], BIT(22));

	set_clk_enet(ENET_125MHZ);
}

static int setup_mac_address_from_eeprom(char *alias, char *env, bool odd)
{
	unsigned char enetaddr[6];
	struct udevice *dev;
	int ret, offset;

	offset = fdt_path_offset(gd->fdt_blob, alias);
	if (offset < 0) {
		printf("%s: No eeprom0 path offset\n", __func__);
		return offset;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_I2C_EEPROM, offset, &dev);
	if (ret) {
		printf("Cannot find EEPROM!\n");
		return ret;
	}

	ret = i2c_eeprom_read(dev, 0xfa, enetaddr, 0x6);
	if (ret) {
		printf("Error reading configuration EEPROM!\n");
		return ret;
	}

	/*
	 * Populate second ethernet MAC from first ethernet EEPROM with MAC
	 * address LSByte incremented by 1. This is only used on SoMs without
	 * second ethernet EEPROM, i.e. early prototypes.
	 */
	if (odd)
		enetaddr[5]++;

	eth_env_set_enetaddr(env, enetaddr);

	return 0;
}

static void setup_mac_address(void)
{
	unsigned char enetaddr[6];
	bool skip_eth0 = false;
	bool skip_eth1 = false;
	int ret;

	ret = eth_env_get_enetaddr("ethaddr", enetaddr);
	if (ret)	/* ethaddr is already set */
		skip_eth0 = true;

	ret = eth_env_get_enetaddr("eth1addr", enetaddr);
	if (ret)	/* eth1addr is already set */
		skip_eth1 = true;

	/* Both MAC addresses are already set in U-Boot environment. */
	if (skip_eth0 && skip_eth1)
		return;

	/*
	 * If IIM fuses contain valid MAC address, use it.
	 * The IIM MAC address fuses are NOT programmed by default.
	 */
	imx_get_mac_from_fuse(0, enetaddr);
	if (is_valid_ethaddr(enetaddr)) {
		if (!skip_eth0)
			eth_env_set_enetaddr("ethaddr", enetaddr);
		/*
		 * The LSbit of MAC address in fuses is always 0, use the
		 * next consecutive MAC address for the second ethernet.
		 */
		enetaddr[5]++;
		if (!skip_eth1)
			eth_env_set_enetaddr("eth1addr", enetaddr);
		return;
	}

	/* Use on-SoM EEPROMs with pre-programmed MAC address. */
	if (!skip_eth0) {
		/* We cannot do much more if this returns -ve . */
		setup_mac_address_from_eeprom("eeprom0", "ethaddr", false);
	}

	if (!skip_eth1) {
		ret = setup_mac_address_from_eeprom("eeprom1", "eth1addr",
						    false);
		if (ret) {	/* Second EEPROM might not be populated. */
			/* We cannot do much more if this returns -ve . */
			setup_mac_address_from_eeprom("eeprom0", "eth1addr",
						      true);
		}
	}
}

int board_init(void)
{
	setup_eqos();
	setup_fec();
	setup_snvs();
	return 0;
}

int board_late_init(void)
{
	setup_mac_address();
	return 0;
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	return prio ? ENVL_UNKNOWN : ENVL_SPI_FLASH;
}
