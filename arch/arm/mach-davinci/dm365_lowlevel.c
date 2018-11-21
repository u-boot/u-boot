// SPDX-License-Identifier: GPL-2.0+
/*
 * SoC-specific lowlevel code for tms320dm365 and similar chips
 * Actually used for booting from NAND with nand_spl.
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */
#include <common.h>
#include <nand.h>
#include <ns16550.h>
#include <post.h>
#include <asm/ti-common/davinci_nand.h>
#include <asm/arch/dm365_lowlevel.h>
#include <asm/arch/hardware.h>

void dm365_waitloop(unsigned long loopcnt)
{
	unsigned long	i;

	for (i = 0; i < loopcnt; i++)
		asm("   NOP");
}

int dm365_pll1_init(unsigned long pllmult, unsigned long prediv)
{
	unsigned int clksrc = 0x0;

	/* Power up the PLL */
	clrbits_le32(&dv_pll0_regs->pllctl, PLLCTL_PLLPWRDN);

	clrbits_le32(&dv_pll0_regs->pllctl, PLLCTL_RES_9);
	setbits_le32(&dv_pll0_regs->pllctl,
		clksrc << PLLCTL_CLOCK_MODE_SHIFT);

	/*
	 * Set PLLENSRC '0', PLL Enable(PLLEN) selection is controlled
	 * through MMR
	 */
	clrbits_le32(&dv_pll0_regs->pllctl, PLLCTL_PLLENSRC);

	/* Set PLLEN=0 => PLL BYPASS MODE */
	clrbits_le32(&dv_pll0_regs->pllctl, PLLCTL_PLLEN);

	dm365_waitloop(150);

	 /* PLLRST=1(reset assert) */
	setbits_le32(&dv_pll0_regs->pllctl, PLLCTL_PLLRST);

	dm365_waitloop(300);

	/*Bring PLL out of Reset*/
	clrbits_le32(&dv_pll0_regs->pllctl, PLLCTL_PLLRST);

	/* Program the Multiper and Pre-Divider for PLL1 */
	writel(pllmult, &dv_pll0_regs->pllm);
	writel(prediv, &dv_pll0_regs->prediv);

	/* Assert TENABLE = 1, TENABLEDIV = 1, TINITZ = 1 */
	writel(PLLSECCTL_STOPMODE | PLLSECCTL_TENABLEDIV | PLLSECCTL_TENABLE |
		PLLSECCTL_TINITZ, &dv_pll0_regs->secctl);
	/* Assert TENABLE = 1, TENABLEDIV = 1, TINITZ = 0 */
	writel(PLLSECCTL_STOPMODE | PLLSECCTL_TENABLEDIV | PLLSECCTL_TENABLE,
		&dv_pll0_regs->secctl);
	/* Assert TENABLE = 0, TENABLEDIV = 0, TINITZ = 0 */
	writel(PLLSECCTL_STOPMODE, &dv_pll0_regs->secctl);
	/* Assert TENABLE = 0, TENABLEDIV = 0, TINITZ = 1 */
	writel(PLLSECCTL_STOPMODE | PLLSECCTL_TINITZ, &dv_pll0_regs->secctl);

	/* Program the PostDiv for PLL1 */
	writel(PLL_POSTDEN, &dv_pll0_regs->postdiv);

	/* Post divider setting for PLL1 */
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV1, &dv_pll0_regs->plldiv1);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV2, &dv_pll0_regs->plldiv2);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV3, &dv_pll0_regs->plldiv3);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV4, &dv_pll0_regs->plldiv4);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV5, &dv_pll0_regs->plldiv5);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV6, &dv_pll0_regs->plldiv6);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV7, &dv_pll0_regs->plldiv7);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV8, &dv_pll0_regs->plldiv8);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV9, &dv_pll0_regs->plldiv9);

	dm365_waitloop(300);

	/* Set the GOSET bit */
	writel(PLLCMD_GOSET, &dv_pll0_regs->pllcmd); /* Go */

	dm365_waitloop(300);

	/* Wait for PLL to LOCK */
	while (!((readl(&dv_sys_module_regs->pll0_config) & PLL0_LOCK)
		== PLL0_LOCK))
		;

	/* Enable the PLL Bit of PLLCTL*/
	setbits_le32(&dv_pll0_regs->pllctl, PLLCTL_PLLEN);

	return 0;
}

int dm365_pll2_init(unsigned long pllm, unsigned long prediv)
{
	unsigned int clksrc = 0x0;

	/* Power up the PLL*/
	clrbits_le32(&dv_pll1_regs->pllctl, PLLCTL_PLLPWRDN);

	/*
	 * Select the Clock Mode as Onchip Oscilator or External Clock on
	 * MXI pin
	 * VDB has input on MXI pin
	 */
	clrbits_le32(&dv_pll1_regs->pllctl, PLLCTL_RES_9);
	setbits_le32(&dv_pll1_regs->pllctl,
		clksrc << PLLCTL_CLOCK_MODE_SHIFT);

	/*
	 * Set PLLENSRC '0', PLL Enable(PLLEN) selection is controlled
	 * through MMR
	 */
	clrbits_le32(&dv_pll1_regs->pllctl, PLLCTL_PLLENSRC);

	/* Set PLLEN=0 => PLL BYPASS MODE */
	clrbits_le32(&dv_pll1_regs->pllctl, PLLCTL_PLLEN);

	dm365_waitloop(50);

	 /* PLLRST=1(reset assert) */
	setbits_le32(&dv_pll1_regs->pllctl, PLLCTL_PLLRST);

	dm365_waitloop(300);

	/* Bring PLL out of Reset */
	clrbits_le32(&dv_pll1_regs->pllctl, PLLCTL_PLLRST);

	/* Program the Multiper and Pre-Divider for PLL2 */
	writel(pllm, &dv_pll1_regs->pllm);
	writel(prediv, &dv_pll1_regs->prediv);

	writel(PLL_POSTDEN, &dv_pll1_regs->postdiv);

	/* Assert TENABLE = 1, TENABLEDIV = 1, TINITZ = 1 */
	writel(PLLSECCTL_STOPMODE | PLLSECCTL_TENABLEDIV | PLLSECCTL_TENABLE |
		PLLSECCTL_TINITZ, &dv_pll1_regs->secctl);
	/* Assert TENABLE = 1, TENABLEDIV = 1, TINITZ = 0 */
	writel(PLLSECCTL_STOPMODE | PLLSECCTL_TENABLEDIV | PLLSECCTL_TENABLE,
		&dv_pll1_regs->secctl);
	/* Assert TENABLE = 0, TENABLEDIV = 0, TINITZ = 0 */
	writel(PLLSECCTL_STOPMODE, &dv_pll1_regs->secctl);
	/* Assert TENABLE = 0, TENABLEDIV = 0, TINITZ = 1 */
	writel(PLLSECCTL_STOPMODE | PLLSECCTL_TINITZ, &dv_pll1_regs->secctl);

	/* Post divider setting for PLL2 */
	writel(CONFIG_SYS_DM36x_PLL2_PLLDIV1, &dv_pll1_regs->plldiv1);
	writel(CONFIG_SYS_DM36x_PLL2_PLLDIV2, &dv_pll1_regs->plldiv2);
	writel(CONFIG_SYS_DM36x_PLL2_PLLDIV3, &dv_pll1_regs->plldiv3);
	writel(CONFIG_SYS_DM36x_PLL2_PLLDIV4, &dv_pll1_regs->plldiv4);
	writel(CONFIG_SYS_DM36x_PLL2_PLLDIV5, &dv_pll1_regs->plldiv5);

	/* GoCmd for PostDivider to take effect */
	writel(PLLCMD_GOSET, &dv_pll1_regs->pllcmd);

	dm365_waitloop(150);

	/* Wait for PLL to LOCK */
	while (!((readl(&dv_sys_module_regs->pll1_config) & PLL1_LOCK)
		== PLL1_LOCK))
		;

	dm365_waitloop(4100);

	/* Enable the PLL2 */
	setbits_le32(&dv_pll1_regs->pllctl, PLLCTL_PLLEN);

	/* do this after PLL's have been set up */
	writel(CONFIG_SYS_DM36x_PERI_CLK_CTRL,
		&dv_sys_module_regs->peri_clkctl);

	return 0;
}

int dm365_ddr_setup(void)
{
	lpsc_on(DAVINCI_LPSC_DDR_EMIF);
	clrbits_le32(&dv_sys_module_regs->vtpiocr,
		VPTIO_IOPWRDN | VPTIO_CLRZ | VPTIO_LOCK | VPTIO_PWRDN);

	/* Set bit CLRZ (bit 13) */
	setbits_le32(&dv_sys_module_regs->vtpiocr, VPTIO_CLRZ);

	/* Check VTP READY Status */
	while (!(readl(&dv_sys_module_regs->vtpiocr) & VPTIO_RDY))
		;

	/* Set bit VTP_IOPWRDWN bit 14 for DDR input buffers) */
	setbits_le32(&dv_sys_module_regs->vtpiocr, VPTIO_IOPWRDN);

	/* Set bit LOCK(bit7) */
	setbits_le32(&dv_sys_module_regs->vtpiocr, VPTIO_LOCK);

	/*
	 * Powerdown VTP as it is locked (bit 6)
	 * Set bit VTP_IOPWRDWN bit 14 for DDR input buffers)
	 */
	setbits_le32(&dv_sys_module_regs->vtpiocr,
		VPTIO_IOPWRDN | VPTIO_PWRDN);

	/* Wait for calibration to complete */
	dm365_waitloop(150);

	/* Set the DDR2 to synreset, then enable it again */
	lpsc_syncreset(DAVINCI_LPSC_DDR_EMIF);
	lpsc_on(DAVINCI_LPSC_DDR_EMIF);

	writel(CONFIG_SYS_DM36x_DDR2_DDRPHYCR, &dv_ddr2_regs_ctrl->ddrphycr);

	/* Program SDRAM Bank Config Register */
	writel((CONFIG_SYS_DM36x_DDR2_SDBCR | DV_DDR_BOOTUNLOCK),
		&dv_ddr2_regs_ctrl->sdbcr);
	writel((CONFIG_SYS_DM36x_DDR2_SDBCR | DV_DDR_TIMUNLOCK),
		&dv_ddr2_regs_ctrl->sdbcr);

	/* Program SDRAM Timing Control Register1 */
	writel(CONFIG_SYS_DM36x_DDR2_SDTIMR, &dv_ddr2_regs_ctrl->sdtimr);
	/* Program SDRAM Timing Control Register2 */
	writel(CONFIG_SYS_DM36x_DDR2_SDTIMR2, &dv_ddr2_regs_ctrl->sdtimr2);

	writel(CONFIG_SYS_DM36x_DDR2_PBBPR, &dv_ddr2_regs_ctrl->pbbpr);

	writel(CONFIG_SYS_DM36x_DDR2_SDBCR, &dv_ddr2_regs_ctrl->sdbcr);

	/* Program SDRAM Refresh Control Register */
	writel(CONFIG_SYS_DM36x_DDR2_SDRCR, &dv_ddr2_regs_ctrl->sdrcr);

	lpsc_syncreset(DAVINCI_LPSC_DDR_EMIF);
	lpsc_on(DAVINCI_LPSC_DDR_EMIF);

	return 0;
}

static void dm365_vpss_sync_reset(void)
{
	unsigned int PdNum = 0;

	/* VPSS_CLKMD 1:1 */
	setbits_le32(&dv_sys_module_regs->vpss_clkctl,
		VPSS_CLK_CTL_VPSS_CLKMD);

	/* LPSC SyncReset DDR Clock Enable */
	writel(((readl(&dv_psc_regs->mdctl[DAVINCI_LPSC_VPSSMASTER]) &
		~PSC_MD_STATE_MSK) | PSC_SYNCRESET),
		&dv_psc_regs->mdctl[DAVINCI_LPSC_VPSSMASTER]);

	writel((1 << PdNum), &dv_psc_regs->ptcmd);

	while (!(((readl(&dv_psc_regs->ptstat) >> PdNum) & PSC_GOSTAT) == 0))
		;
	while (!((readl(&dv_psc_regs->mdstat[DAVINCI_LPSC_VPSSMASTER]) &
		PSC_MD_STATE_MSK) == PSC_SYNCRESET))
		;
}

static void dm365_por_reset(void)
{
	struct davinci_timer *wdog =
		(struct davinci_timer *)DAVINCI_WDOG_BASE;

	if (readl(&dv_pll0_regs->rstype) &
		(PLL_RSTYPE_POR | PLL_RSTYPE_XWRST)) {
		dm365_vpss_sync_reset();

		writel(DV_TMPBUF_VAL, TMPBUF);
		setbits_le32(TMPSTATUS, FLAG_PORRST);
		writel(DV_WDT_ENABLE_SYS_RESET, &wdog->na1);
		writel(DV_WDT_TRIGGER_SYS_RESET, &wdog->na2);

		while (1);
	}
}

static void dm365_wdt_reset(void)
{
	struct davinci_timer *wdog =
		(struct davinci_timer *)DAVINCI_WDOG_BASE;

	if (readl(TMPBUF) != DV_TMPBUF_VAL) {
		writel(DV_TMPBUF_VAL, TMPBUF);
		setbits_le32(TMPSTATUS, FLAG_PORRST);
		setbits_le32(TMPSTATUS, FLAG_FLGOFF);

		dm365_waitloop(100);

		dm365_vpss_sync_reset();

		writel(DV_WDT_ENABLE_SYS_RESET, &wdog->na1);
		writel(DV_WDT_TRIGGER_SYS_RESET, &wdog->na2);

		while (1);
	}
}

static void dm365_wdt_flag_on(void)
{
	/* VPSS_CLKMD 1:2 */
	clrbits_le32(&dv_sys_module_regs->vpss_clkctl,
		VPSS_CLK_CTL_VPSS_CLKMD);
	writel(0, TMPBUF);
	setbits_le32(TMPSTATUS, FLAG_FLGON);
}

void dm365_psc_init(void)
{
	unsigned char i = 0;
	unsigned char lpsc_start;
	unsigned char lpsc_end, lpscgroup, lpscmin, lpscmax;
	unsigned int  PdNum = 0;

	lpscmin = 0;
	lpscmax = 2;

	for (lpscgroup = lpscmin; lpscgroup <= lpscmax; lpscgroup++) {
		if (lpscgroup == 0) {
			/* Enabling LPSC 3 to 28 SCR first */
			lpsc_start = DAVINCI_LPSC_VPSSMSTR;
			lpsc_end   = DAVINCI_LPSC_TIMER1;
		} else if (lpscgroup == 1) { /* Skip locked LPSCs [29-37] */
			lpsc_start = DAVINCI_LPSC_CFG5;
			lpsc_end   = DAVINCI_LPSC_VPSSMASTER;
		} else {
			lpsc_start = DAVINCI_LPSC_MJCP;
			lpsc_end   = DAVINCI_LPSC_HDVICP;
		}

		/* NEXT=0x3, Enable LPSC's */
		for (i = lpsc_start; i <= lpsc_end; i++)
			setbits_le32(&dv_psc_regs->mdctl[i], PSC_ENABLE);

		/*
		 * Program goctl to start transition sequence for LPSCs
		 * CSL_PSC_0_REGS->PTCMD = (1<<PdNum); Kick off Power
		 * Domain 0 Modules
		 */
		writel((1 << PdNum), &dv_psc_regs->ptcmd);

		/*
		 * Wait for GOSTAT = NO TRANSITION from PSC for Powerdomain 0
		 */
		while (!(((readl(&dv_psc_regs->ptstat) >> PdNum) & PSC_GOSTAT)
			== 0))
			;

		/* Wait for MODSTAT = ENABLE from LPSC's */
		for (i = lpsc_start; i <= lpsc_end; i++)
			while (!((readl(&dv_psc_regs->mdstat[i]) &
				PSC_MD_STATE_MSK) == PSC_ENABLE))
				;
	}
}

static void dm365_emif_init(void)
{
	writel(CONFIG_SYS_DM36x_AWCCR, &davinci_emif_regs->awccr);
	writel(CONFIG_SYS_DM36x_AB1CR, &davinci_emif_regs->ab1cr);

	setbits_le32(&davinci_emif_regs->nandfcr, DAVINCI_NANDFCR_CS2NAND);

	writel(CONFIG_SYS_DM36x_AB2CR, &davinci_emif_regs->ab2cr);

	return;
}

void dm365_pinmux_ctl(unsigned long offset, unsigned long mask,
	unsigned long value)
{
	clrbits_le32(&dv_sys_module_regs->pinmux[offset], mask);
	setbits_le32(&dv_sys_module_regs->pinmux[offset], (mask & value));
}

__attribute__((weak))
void board_gpio_init(void)
{
	return;
}

#if defined(CONFIG_POST)
int post_log(char *format, ...)
{
	return 0;
}
#endif

void dm36x_lowlevel_init(ulong bootflag)
{
	struct davinci_uart_ctrl_regs *davinci_uart_ctrl_regs =
		(struct davinci_uart_ctrl_regs *)(CONFIG_SYS_NS16550_COM1 +
		DAVINCI_UART_CTRL_BASE);

	/* Mask all interrupts */
	writel(DV_AINTC_INTCTL_IDMODE, &dv_aintc_regs->intctl);
	writel(0x0, &dv_aintc_regs->eabase);
	writel(0x0, &dv_aintc_regs->eint0);
	writel(0x0, &dv_aintc_regs->eint1);

	/* Clear all interrupts */
	writel(0xffffffff, &dv_aintc_regs->fiq0);
	writel(0xffffffff, &dv_aintc_regs->fiq1);
	writel(0xffffffff, &dv_aintc_regs->irq0);
	writel(0xffffffff, &dv_aintc_regs->irq1);

	dm365_por_reset();
	dm365_wdt_reset();

	/* System PSC setup - enable all */
	dm365_psc_init();

	/* Setup Pinmux */
	dm365_pinmux_ctl(0, 0xFFFFFFFF, CONFIG_SYS_DM36x_PINMUX0);
	dm365_pinmux_ctl(1, 0xFFFFFFFF, CONFIG_SYS_DM36x_PINMUX1);
	dm365_pinmux_ctl(2, 0xFFFFFFFF, CONFIG_SYS_DM36x_PINMUX2);
	dm365_pinmux_ctl(3, 0xFFFFFFFF, CONFIG_SYS_DM36x_PINMUX3);
	dm365_pinmux_ctl(4, 0xFFFFFFFF, CONFIG_SYS_DM36x_PINMUX4);

	/* PLL setup */
	dm365_pll1_init(CONFIG_SYS_DM36x_PLL1_PLLM,
		CONFIG_SYS_DM36x_PLL1_PREDIV);
	dm365_pll2_init(CONFIG_SYS_DM36x_PLL2_PLLM,
		CONFIG_SYS_DM36x_PLL2_PREDIV);

	/* GPIO setup */
	board_gpio_init();

	NS16550_init((NS16550_t)(CONFIG_SYS_NS16550_COM1),
			CONFIG_SYS_NS16550_CLK / 16 / CONFIG_BAUDRATE);

	/*
	 * Fix Power and Emulation Management Register
	 * see sprufh2.pdf page 38 Table 22
	 */
	writel((DAVINCI_UART_PWREMU_MGMT_FREE | DAVINCI_UART_PWREMU_MGMT_URRST |
		DAVINCI_UART_PWREMU_MGMT_UTRST),
	       &davinci_uart_ctrl_regs->pwremu_mgmt);

	puts("ddr init\n");
	dm365_ddr_setup();

	puts("emif init\n");
	dm365_emif_init();

	dm365_wdt_flag_on();

#if defined(CONFIG_POST)
	/*
	 * Do memory tests, calls arch_memory_failure_handle()
	 * if error detected.
	 */
	memory_post_test(0);
#endif
}
