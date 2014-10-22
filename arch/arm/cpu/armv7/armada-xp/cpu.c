/*
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

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
	default:
		puts("Unknown-");
		break;
	}

	switch (revid) {
	case 1:
		puts("A0\n");
		break;
	case 2:
		puts("B0\n");
		break;
	default:
		puts("??\n");
		break;
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
int arch_cpu_init(void)
{
	/* Linux expects the internal registers to be at 0xf1000000 */
	writel(SOC_REGS_PHY_BASE, INTREG_BASE_ADDR_REG);

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

	/*
	 * Now the SDRAM access windows can be reconfigured using
	 * the information in the SDRAM scratch pad registers
	 */
	update_sdram_window_sizes();

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
	mvneta_initialize(bis, MVEBU_EGIGA0_BASE, 0, CONFIG_PHY_BASE_ADDR + 0);
	mvneta_initialize(bis, MVEBU_EGIGA1_BASE, 1, CONFIG_PHY_BASE_ADDR + 1);
	mvneta_initialize(bis, MVEBU_EGIGA2_BASE, 2, CONFIG_PHY_BASE_ADDR + 2);
	mvneta_initialize(bis, MVEBU_EGIGA3_BASE, 3, CONFIG_PHY_BASE_ADDR + 3);

	return 0;
}
#endif

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
