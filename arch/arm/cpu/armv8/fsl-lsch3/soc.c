/*
 * Copyright 2015 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fsl_ifc.h>
#include <nand.h>
#include <spl.h>
#include <asm/arch-fsl-lsch3/soc.h>
#include <asm/io.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static void erratum_a008751(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A008751
	u32 __iomem *scfg = (u32 __iomem *)SCFG_BASE;

	writel(0x27672b2a, scfg + SCFG_USB3PRM1CR / 4);
#endif
}

static void erratum_rcw_src(void)
{
#if defined(CONFIG_SPL)
	u32 __iomem *dcfg_ccsr = (u32 __iomem *)DCFG_BASE;
	u32 __iomem *dcfg_dcsr = (u32 __iomem *)DCFG_DCSR_BASE;
	u32 val;

	val = in_le32(dcfg_ccsr + DCFG_PORSR1 / 4);
	val &= ~DCFG_PORSR1_RCW_SRC;
	val |= DCFG_PORSR1_RCW_SRC_NOR;
	out_le32(dcfg_dcsr + DCFG_DCSR_PORCR1 / 4, val);
#endif
}

#define I2C_DEBUG_REG 0x6
#define I2C_GLITCH_EN 0x8
/*
 * This erratum requires setting glitch_en bit to enable
 * digital glitch filter to improve clock stability.
 */
static void erratum_a009203(void)
{
	u8 __iomem *ptr;
#ifdef CONFIG_SYS_I2C
#ifdef I2C1_BASE_ADDR
	ptr = (u8 __iomem *)(I2C1_BASE_ADDR + I2C_DEBUG_REG);

	writeb(I2C_GLITCH_EN, ptr);
#endif
#ifdef I2C2_BASE_ADDR
	ptr = (u8 __iomem *)(I2C2_BASE_ADDR + I2C_DEBUG_REG);

	writeb(I2C_GLITCH_EN, ptr);
#endif
#ifdef I2C3_BASE_ADDR
	ptr = (u8 __iomem *)(I2C3_BASE_ADDR + I2C_DEBUG_REG);

	writeb(I2C_GLITCH_EN, ptr);
#endif
#ifdef I2C4_BASE_ADDR
	ptr = (u8 __iomem *)(I2C4_BASE_ADDR + I2C_DEBUG_REG);

	writeb(I2C_GLITCH_EN, ptr);
#endif
#endif
}

void fsl_lsch3_early_init_f(void)
{
	erratum_a008751();
	erratum_rcw_src();
	init_early_memctl_regs();	/* tighten IFC timing */
	erratum_a009203();
}

#ifdef CONFIG_SPL_BUILD
void board_init_f(ulong dummy)
{
	/* Clear global data */
	memset((void *)gd, 0, sizeof(gd_t));

	arch_cpu_init();
	board_early_init_f();
	timer_init();
	env_init();
	gd->baudrate = getenv_ulong("baudrate", 10, CONFIG_BAUDRATE);

	serial_init();
	console_init_f();
	dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	board_init_r(NULL, 0);
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NAND;
}
#endif
