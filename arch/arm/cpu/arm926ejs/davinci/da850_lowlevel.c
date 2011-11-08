/*
 * SoC-specific lowlevel code for DA850
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <common.h>
#include <nand.h>
#include <ns16550.h>
#include <post.h>
#include <asm/arch/da850_lowlevel.h>
#include <asm/arch/hardware.h>
#include <asm/arch/ddr2_defs.h>
#include <asm/arch/emif_defs.h>

void da850_waitloop(unsigned long loopcnt)
{
	unsigned long	i;

	for (i = 0; i < loopcnt; i++)
		asm("   NOP");
}

int da850_pll_init(struct davinci_pllc_regs *reg, unsigned long pllmult)
{
	if (reg == davinci_pllc0_regs)
		/* Unlock PLL registers. */
		clrbits_le32(&davinci_syscfg_regs->cfgchip0, 0x00000010);

	/*
	 * Set PLLENSRC '0',bit 5, PLL Enable(PLLEN) selection is controlled
	 * through MMR
	 */
	clrbits_le32(&reg->pllctl, 0x00000020);
	/* PLLCTL.EXTCLKSRC bit 9 should be left at 0 for Freon */
	clrbits_le32(&reg->pllctl, 0x00000200);

	/* Set PLLEN=0 => PLL BYPASS MODE */
	clrbits_le32(&reg->pllctl, 0x00000001);

	da850_waitloop(150);

	if (reg == davinci_pllc0_regs) {
		/*
		 * Select the Clock Mode bit 8 as External Clock or On Chip
		 * Oscilator
		 */
		dv_maskbits(&reg->pllctl, 0xFFFFFEFF);
		setbits_le32(&reg->pllctl, (CONFIG_SYS_DV_CLKMODE << 8));
	}

	/* Clear PLLRST bit to reset the PLL */
	clrbits_le32(&reg->pllctl, 0x00000008);

	/* Disable the PLL output */
	setbits_le32(&reg->pllctl, 0x00000010);

	/* PLL initialization sequence */
	/*
	 * Power up the PLL- PWRDN bit set to 0 to bring the PLL out of
	 * power down bit
	 */
	clrbits_le32(&reg->pllctl, 0x00000002);

	/* Enable the PLL from Disable Mode PLLDIS bit to 0 */
	clrbits_le32(&reg->pllctl, 0x00000010);

	/* Program the required multiplier value in PLLM */
	writel(pllmult, &reg->pllm);

	/* program the postdiv */
	if (reg == davinci_pllc0_regs)
		writel((0x8000 | CONFIG_SYS_DA850_PLL0_POSTDIV),
			&reg->postdiv);
	else
		writel((0x8000 | CONFIG_SYS_DA850_PLL1_POSTDIV),
			&reg->postdiv);

	/*
	 * Check for the GOSTAT bit in PLLSTAT to clear to 0 to indicate that
	 * no GO operation is currently in progress
	 */
	while ((readl(&reg->pllstat) & 0x1) == 1)
		;

	if (reg == davinci_pllc0_regs) {
		writel(CONFIG_SYS_DA850_PLL0_PLLDIV1, &reg->plldiv1);
		writel(CONFIG_SYS_DA850_PLL0_PLLDIV2, &reg->plldiv2);
		writel(CONFIG_SYS_DA850_PLL0_PLLDIV3, &reg->plldiv3);
		writel(CONFIG_SYS_DA850_PLL0_PLLDIV4, &reg->plldiv4);
		writel(CONFIG_SYS_DA850_PLL0_PLLDIV5, &reg->plldiv5);
		writel(CONFIG_SYS_DA850_PLL0_PLLDIV6, &reg->plldiv6);
		writel(CONFIG_SYS_DA850_PLL0_PLLDIV7, &reg->plldiv7);
	} else {
		writel(CONFIG_SYS_DA850_PLL1_PLLDIV1, &reg->plldiv1);
		writel(CONFIG_SYS_DA850_PLL1_PLLDIV2, &reg->plldiv2);
		writel(CONFIG_SYS_DA850_PLL1_PLLDIV3, &reg->plldiv3);
	}

	/*
	 * Set the GOSET bit in PLLCMD to 1 to initiate a new divider
	 * transition.
	 */
	setbits_le32(&reg->pllcmd, 0x01);

	/*
	 * Wait for the GOSTAT bit in PLLSTAT to clear to 0
	 * (completion of phase alignment).
	 */
	while ((readl(&reg->pllstat) & 0x1) == 1)
		;

	/* Wait for PLL to reset properly. See PLL spec for PLL reset time */
	da850_waitloop(200);

	/* Set the PLLRST bit in PLLCTL to 1 to bring the PLL out of reset */
	setbits_le32(&reg->pllctl, 0x00000008);

	/* Wait for PLL to lock. See PLL spec for PLL lock time */
	da850_waitloop(2400);

	/*
	 * Set the PLLEN bit in PLLCTL to 1 to remove the PLL from bypass
	 * mode
	 */
	setbits_le32(&reg->pllctl, 0x00000001);


	/*
	 * clear EMIFA and EMIFB clock source settings, let them
	 * run off SYSCLK
	 */
	if (reg == davinci_pllc0_regs)
		dv_maskbits(&davinci_syscfg_regs->cfgchip3, 0xFFFFFFF8);

	return 0;
}

int da850_ddr_setup(unsigned int freq)
{
	unsigned long	tmp;

	/* Enable the Clock to DDR2/mDDR */
	lpsc_on(DAVINCI_LPSC_DDR_EMIF);

	tmp = readl(&davinci_syscfg1_regs->vtpio_ctl);
	if ((tmp & VTP_POWERDWN) == VTP_POWERDWN) {
		/* Begin VTP Calibration */
		clrbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_POWERDWN);
		clrbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_LOCK);
		setbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_CLKRZ);
		clrbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_CLKRZ);
		setbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_CLKRZ);

		/* Polling READY bit to see when VTP calibration is done */
		tmp = readl(&davinci_syscfg1_regs->vtpio_ctl);
		while ((tmp & VTP_READY) != VTP_READY)
			tmp = readl(&davinci_syscfg1_regs->vtpio_ctl);

		setbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_LOCK);
		setbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_POWERDWN);

		setbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_IOPWRDWN);
	}

	writel(CONFIG_SYS_DA850_DDR2_DDRPHYCR, &dv_ddr2_regs_ctrl->ddrphycr);
	clrbits_le32(&davinci_syscfg1_regs->ddr_slew,
		(1 << DDR_SLEW_CMOSEN_BIT));

	/*
	 * SDRAM Configuration Register (SDCR):
	 * First set the BOOTUNLOCK bit to make configuration bits
	 * writeable.
	 */
	setbits_le32(&dv_ddr2_regs_ctrl->sdbcr, DV_DDR_BOOTUNLOCK);

	/*
	 * Write the new value of these bits and clear BOOTUNLOCK.
	 * At the same time, set the TIMUNLOCK bit to allow changing
	 * the timing registers
	 */
	tmp = CONFIG_SYS_DA850_DDR2_SDBCR;
	tmp &= ~(0x1 << DV_DDR_SDCR_BOOTUNLOCK_SHIFT);
	tmp |= (0x1 << DV_DDR_SDCR_TIMUNLOCK_SHIFT);
	writel(tmp, &dv_ddr2_regs_ctrl->sdbcr);

	/* write memory configuration and timing */
	writel(CONFIG_SYS_DA850_DDR2_SDBCR2, &dv_ddr2_regs_ctrl->sdbcr2);
	writel(CONFIG_SYS_DA850_DDR2_SDTIMR, &dv_ddr2_regs_ctrl->sdtimr);
	writel(CONFIG_SYS_DA850_DDR2_SDTIMR2, &dv_ddr2_regs_ctrl->sdtimr2);

	/* clear the TIMUNLOCK bit and write the value of the CL field */
	tmp &= ~(0x1 << DV_DDR_SDCR_TIMUNLOCK_SHIFT);
	writel(tmp, &dv_ddr2_regs_ctrl->sdbcr);

	/*
	 * LPMODEN and MCLKSTOPEN must be set!
	 * Without this bits set, PSC don;t switch states !!
	 */
	writel(CONFIG_SYS_DA850_DDR2_SDRCR |
		(1 << DV_DDR_SRCR_LPMODEN_SHIFT) |
		(1 << DV_DDR_SRCR_MCLKSTOPEN_SHIFT),
		&dv_ddr2_regs_ctrl->sdrcr);

	/* SyncReset the Clock to EMIF3A SDRAM */
	lpsc_syncreset(DAVINCI_LPSC_DDR_EMIF);
	/* Enable the Clock to EMIF3A SDRAM */
	lpsc_on(DAVINCI_LPSC_DDR_EMIF);

	/* disable self refresh */
	clrbits_le32(&dv_ddr2_regs_ctrl->sdrcr, 0xc0000000);
	writel(0x30, &dv_ddr2_regs_ctrl->pbbpr);

	return 0;
}

void da850_pinmux_ctl(unsigned long offset, unsigned long mask,
	unsigned long value)
{
	clrbits_le32(&davinci_syscfg_regs->pinmux[offset], mask);
	setbits_le32(&davinci_syscfg_regs->pinmux[offset], (mask & value));
}

__attribute__((weak))
void board_gpio_init(void)
{
	return;
}

#if defined(CONFIG_NAND_SPL)
void nand_boot(void)
{
	__attribute__((noreturn)) void (*uboot)(void);

	/* copy image from NOR to RAM */
	memcpy((void *)CONFIG_SYS_NAND_U_BOOT_DST,
		(void *)CONFIG_SYS_NAND_U_BOOT_OFFS,
		CONFIG_SYS_NAND_U_BOOT_SIZE);

	/* and jump to it ... */
	uboot = (void *)CONFIG_SYS_NAND_U_BOOT_START;
	(*uboot)();
}
#endif

#if defined(CONFIG_NAND_SPL)
void board_init_f(ulong bootflag)
#else
int arch_cpu_init(void)
#endif
{
	/*
	 * copied from arch/arm/cpu/arm926ejs/start.S
	 *
	 * flush v4 I/D caches
	 */
	asm("mov	r0, #0");
	asm("mcr	p15, 0, r0, c7, c7, 0");	/* flush v3/v4 cache */
	asm("mcr	p15, 0, r0, c8, c7, 0");	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	asm("mrc	p15, 0, r0, c1, c0, 0");
	/* clear bits 13, 9:8 (--V- --RS) */
	asm("bic	r0, r0, #0x00002300");
	/* clear bits 7, 2:0 (B--- -CAM) */
	asm("bic	r0, r0, #0x00000087");
	/* set bit 2 (A) Align */
	asm("orr	r0, r0, #0x00000002");
	/* set bit 12 (I) I-Cache */
	asm("orr	r0, r0, #0x00001000");
	asm("mcr	p15, 0, r0, c1, c0, 0");

	/* Unlock kick registers */
	writel(0x83e70b13, &davinci_syscfg_regs->kick0);
	writel(0x95a4f1e0, &davinci_syscfg_regs->kick1);

	dv_maskbits(&davinci_syscfg_regs->suspsrc,
		((1 << 27) | (1 << 22) | (1 << 20) | (1 << 5) |	(1 << 16)));

	/* Setup Pinmux */
	da850_pinmux_ctl(0, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX0);
	da850_pinmux_ctl(1, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX1);
	da850_pinmux_ctl(2, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX2);
	da850_pinmux_ctl(3, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX3);
	da850_pinmux_ctl(4, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX4);
	da850_pinmux_ctl(5, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX5);
	da850_pinmux_ctl(6, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX6);
	da850_pinmux_ctl(7, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX7);
	da850_pinmux_ctl(8, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX8);
	da850_pinmux_ctl(9, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX9);
	da850_pinmux_ctl(10, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX10);
	da850_pinmux_ctl(11, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX11);
	da850_pinmux_ctl(12, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX12);
	da850_pinmux_ctl(13, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX13);
	da850_pinmux_ctl(14, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX14);
	da850_pinmux_ctl(15, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX15);
	da850_pinmux_ctl(16, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX16);
	da850_pinmux_ctl(17, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX17);
	da850_pinmux_ctl(18, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX18);
	da850_pinmux_ctl(19, 0xFFFFFFFF, CONFIG_SYS_DA850_PINMUX19);

	/* PLL setup */
	da850_pll_init(davinci_pllc0_regs, CONFIG_SYS_DA850_PLL0_PLLM);
	da850_pll_init(davinci_pllc1_regs, CONFIG_SYS_DA850_PLL1_PLLM);

	/* GPIO setup */
	board_gpio_init();

	/* setup CSn config */
	writel(CONFIG_SYS_DA850_CS2CFG, &davinci_emif_regs->ab1cr);
	writel(CONFIG_SYS_DA850_CS3CFG, &davinci_emif_regs->ab2cr);

	lpsc_on(DAVINCI_LPSC_UART2);
	NS16550_init((NS16550_t)(CONFIG_SYS_NS16550_COM1),
			CONFIG_SYS_NS16550_CLK / 16 / CONFIG_BAUDRATE);

	/*
	 * Fix Power and Emulation Management Register
	 * see sprufw3a.pdf page 37 Table 24
	 */
	writel(readl((CONFIG_SYS_NS16550_COM1 + 0x30)) | 0x00006001,
		(CONFIG_SYS_NS16550_COM1 + 0x30));
#if defined(CONFIG_NAND_SPL)
	puts("ddr init\n");
	da850_ddr_setup(132);

	puts("boot u-boot ...\n");

	nand_boot();
#else
	da850_ddr_setup(132);
	return 0;
#endif
}
