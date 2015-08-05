/*
 * Copyright (C) 2014-2015 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <ahci.h>
#include <linux/mbus.h>
#include <asm/io.h>
#include <asm/pl310.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <sdhci.h>

#define DDR_BASE_CS_OFF(n)	(0x0000 + ((n) << 3))
#define DDR_SIZE_CS_OFF(n)	(0x0004 + ((n) << 3))

static struct mbus_win windows[] = {
	/* PCIE MEM address space */
	{ DEFADR_PCI_MEM, 256 << 20, CPU_TARGET_PCIE13, CPU_ATTR_PCIE_MEM },

	/* PCIE IO address space */
	{ DEFADR_PCI_IO, 64 << 10, CPU_TARGET_PCIE13, CPU_ATTR_PCIE_IO },

	/* SPI */
	{ DEFADR_SPIF, 8 << 20, CPU_TARGET_DEVICEBUS_BOOTROM_SPI,
	  CPU_ATTR_SPIFLASH },

	/* NOR */
	{ DEFADR_BOOTROM, 8 << 20, CPU_TARGET_DEVICEBUS_BOOTROM_SPI,
	  CPU_ATTR_BOOTROM },
};

void reset_cpu(unsigned long ignored)
{
	struct mvebu_system_registers *reg =
		(struct mvebu_system_registers *)MVEBU_SYSTEM_REG_BASE;

	writel(readl(&reg->rstoutn_mask) | 1, &reg->rstoutn_mask);
	writel(readl(&reg->sys_soft_rst) | 1, &reg->sys_soft_rst);
	while (1)
		;
}

int mvebu_soc_family(void)
{
	u16 devid = (readl(MVEBU_REG_PCIE_DEVID) >> 16) & 0xffff;

	if (devid == SOC_MV78460_ID)
		return MVEBU_SOC_AXP;

	if (devid == SOC_88F6810_ID || devid == SOC_88F6820_ID ||
	    devid == SOC_88F6828_ID)
		return MVEBU_SOC_A38X;

	return MVEBU_SOC_UNKNOWN;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	u16 devid = (readl(MVEBU_REG_PCIE_DEVID) >> 16) & 0xffff;
	u8 revid = readl(MVEBU_REG_PCIE_REVID) & 0xff;

	puts("SoC:   ");

	switch (devid) {
	case SOC_MV78460_ID:
		puts("MV78460-");
		break;
	case SOC_88F6810_ID:
		puts("MV88F6810-");
		break;
	case SOC_88F6820_ID:
		puts("MV88F6820-");
		break;
	case SOC_88F6828_ID:
		puts("MV88F6828-");
		break;
	default:
		puts("Unknown-");
		break;
	}

	if (mvebu_soc_family() == MVEBU_SOC_AXP) {
		switch (revid) {
		case 1:
			puts("A0\n");
			break;
		case 2:
			puts("B0\n");
			break;
		default:
			printf("?? (%x)\n", revid);
			break;
		}
	}

	if (mvebu_soc_family() == MVEBU_SOC_A38X) {
		switch (revid) {
		case MV_88F68XX_Z1_ID:
			puts("Z1\n");
			break;
		case MV_88F68XX_A0_ID:
			puts("A0\n");
			break;
		default:
			printf("?? (%x)\n", revid);
			break;
		}
	}

	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */

/*
 * This function initialize Controller DRAM Fastpath windows.
 * It takes the CS size information from the 0x1500 scratch registers
 * and sets the correct windows sizes and base addresses accordingly.
 *
 * These values are set in the scratch registers by the Marvell
 * DDR3 training code, which is executed by the BootROM before the
 * main payload (U-Boot) is executed. This training code is currently
 * only available in the Marvell U-Boot version. It needs to be
 * ported to mainline U-Boot SPL at some point.
 */
static void update_sdram_window_sizes(void)
{
	u64 base = 0;
	u32 size, temp;
	int i;

	for (i = 0; i < SDRAM_MAX_CS; i++) {
		size = readl((MVEBU_SDRAM_SCRATCH + (i * 8))) & SDRAM_ADDR_MASK;
		if (size != 0) {
			size |= ~(SDRAM_ADDR_MASK);

			/* Set Base Address */
			temp = (base & 0xFF000000ll) | ((base >> 32) & 0xF);
			writel(temp, MVEBU_SDRAM_BASE + DDR_BASE_CS_OFF(i));

			/*
			 * Check if out of max window size and resize
			 * the window
			 */
			temp = (readl(MVEBU_SDRAM_BASE + DDR_SIZE_CS_OFF(i)) &
				~(SDRAM_ADDR_MASK)) | 1;
			temp |= (size & SDRAM_ADDR_MASK);
			writel(temp, MVEBU_SDRAM_BASE + DDR_SIZE_CS_OFF(i));

			base += ((u64)size + 1);
		} else {
			/*
			 * Disable window if not used, otherwise this
			 * leads to overlapping enabled windows with
			 * pretty strange results
			 */
			clrbits_le32(MVEBU_SDRAM_BASE + DDR_SIZE_CS_OFF(i), 1);
		}
	}
}

#ifdef CONFIG_ARCH_CPU_INIT
static void set_cbar(u32 addr)
{
	asm("mcr p15, 4, %0, c15, c0" : : "r" (addr));
}


int arch_cpu_init(void)
{
	/* Linux expects the internal registers to be at 0xf1000000 */
	writel(SOC_REGS_PHY_BASE, INTREG_BASE_ADDR_REG);
	set_cbar(SOC_REGS_PHY_BASE + 0xC000);

	/*
	 * We need to call mvebu_mbus_probe() before calling
	 * update_sdram_window_sizes() as it disables all previously
	 * configured mbus windows and then configures them as
	 * required for U-Boot. Calling update_sdram_window_sizes()
	 * without this configuration will not work, as the internal
	 * registers can't be accessed reliably because of potenial
	 * double mapping.
	 * After updating the SDRAM access windows we need to call
	 * mvebu_mbus_probe() again, as this now correctly configures
	 * the SDRAM areas that are later used by the MVEBU drivers
	 * (e.g. USB, NETA).
	 */

	/*
	 * First disable all windows
	 */
	mvebu_mbus_probe(NULL, 0);

	if (mvebu_soc_family() == MVEBU_SOC_AXP) {
		/*
		 * Now the SDRAM access windows can be reconfigured using
		 * the information in the SDRAM scratch pad registers
		 */
		update_sdram_window_sizes();
	}

	/*
	 * Finally the mbus windows can be configured with the
	 * updated SDRAM sizes
	 */
	mvebu_mbus_probe(windows, ARRAY_SIZE(windows));

	return 0;
}
#endif /* CONFIG_ARCH_CPU_INIT */

/*
 * SOC specific misc init
 */
#if defined(CONFIG_ARCH_MISC_INIT)
int arch_misc_init(void)
{
	/* Nothing yet, perhaps we need something here later */
	return 0;
}
#endif /* CONFIG_ARCH_MISC_INIT */

#ifdef CONFIG_MVNETA
int cpu_eth_init(bd_t *bis)
{
	u32 enet_base[] = { MVEBU_EGIGA0_BASE, MVEBU_EGIGA1_BASE,
			    MVEBU_EGIGA2_BASE, MVEBU_EGIGA3_BASE };
	u8 phy_addr[] = CONFIG_PHY_ADDR;
	int i;

	/*
	 * Only Armada XP supports all 4 ethernet interfaces. A38x has
	 * slightly different base addresses for its 2-3 interfaces.
	 */
	if (mvebu_soc_family() != MVEBU_SOC_AXP) {
		enet_base[1] = MVEBU_EGIGA2_BASE;
		enet_base[2] = MVEBU_EGIGA3_BASE;
	}

	for (i = 0; i < ARRAY_SIZE(phy_addr); i++)
		mvneta_initialize(bis, enet_base[i], i, phy_addr[i]);

	return 0;
}
#endif

#ifdef CONFIG_MV_SDHCI
int board_mmc_init(bd_t *bis)
{
	mv_sdh_init(MVEBU_SDIO_BASE, 0, 0,
		    SDHCI_QUIRK_32BIT_DMA_ADDR | SDHCI_QUIRK_WAIT_SEND_CMD);

	return 0;
}
#endif

#ifdef CONFIG_SCSI_AHCI_PLAT
#define AHCI_VENDOR_SPECIFIC_0_ADDR	0xa0
#define AHCI_VENDOR_SPECIFIC_0_DATA	0xa4

#define AHCI_WINDOW_CTRL(win)		(0x60 + ((win) << 4))
#define AHCI_WINDOW_BASE(win)		(0x64 + ((win) << 4))
#define AHCI_WINDOW_SIZE(win)		(0x68 + ((win) << 4))

static void ahci_mvebu_mbus_config(void __iomem *base)
{
	const struct mbus_dram_target_info *dram;
	int i;

	dram = mvebu_mbus_dram_info();

	for (i = 0; i < 4; i++) {
		writel(0, base + AHCI_WINDOW_CTRL(i));
		writel(0, base + AHCI_WINDOW_BASE(i));
		writel(0, base + AHCI_WINDOW_SIZE(i));
	}

	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;

		writel((cs->mbus_attr << 8) |
		       (dram->mbus_dram_target_id << 4) | 1,
		       base + AHCI_WINDOW_CTRL(i));
		writel(cs->base >> 16, base + AHCI_WINDOW_BASE(i));
		writel(((cs->size - 1) & 0xffff0000),
		       base + AHCI_WINDOW_SIZE(i));
	}
}

static void ahci_mvebu_regret_option(void __iomem *base)
{
	/*
	 * Enable the regret bit to allow the SATA unit to regret a
	 * request that didn't receive an acknowlegde and avoid a
	 * deadlock
	 */
	writel(0x4, base + AHCI_VENDOR_SPECIFIC_0_ADDR);
	writel(0x80, base + AHCI_VENDOR_SPECIFIC_0_DATA);
}

void scsi_init(void)
{
	printf("MVEBU SATA INIT\n");
	ahci_mvebu_mbus_config((void __iomem *)MVEBU_SATA0_BASE);
	ahci_mvebu_regret_option((void __iomem *)MVEBU_SATA0_BASE);
	ahci_init((void __iomem *)MVEBU_SATA0_BASE);
}
#endif

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	struct pl310_regs *const pl310 =
		(struct pl310_regs *)CONFIG_SYS_PL310_BASE;

	/* First disable L2 cache - may still be enable from BootROM */
	if (mvebu_soc_family() == MVEBU_SOC_A38X)
		clrbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);

	/* Avoid problem with e.g. neta ethernet driver */
	invalidate_dcache_all();

	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
