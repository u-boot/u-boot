/*
 * (C) Copyright 2011 HALE electronic <helmut.raiger@hale.at>
 * (C) Copyright 2009 Magnus Lilja <lilja.magnus@gmail.com>
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <netdev.h>
#include <command.h>
#include <pmic.h>
#include <fsl_pmic.h>
#include <mc13783.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define BOARD_STRING	"Board: HALE TT-01"

/* Clock configuration */
#define CCM_CCMR_SETUP		0x074B0BF5

static void board_setup_clocks(void)
{
	struct clock_control_regs *ccm = (struct clock_control_regs *) CCM_BASE;
	volatile int wait = 0x10000;

	writel(CCM_CCMR_SETUP, &ccm->ccmr);
	while (wait--)
		;

	writel(CCM_CCMR_SETUP | CCMR_MPE, &ccm->ccmr);
	writel((CCM_CCMR_SETUP | CCMR_MPE) & ~CCMR_MDS, &ccm->ccmr);

	/* Set up clock to 532MHz */
	writel(PDR0_CSI_PODF(0x1ff) | PDR0_PER_PODF(7) |
			PDR0_HSP_PODF(3) | PDR0_NFC_PODF(5) |
			PDR0_IPG_PODF(1) | PDR0_MAX_PODF(3) |
			PDR0_MCU_PODF(0), &ccm->pdr0);
	writel(PLL_PD(0) | PLL_MFD(51) | PLL_MFI(10) | PLL_MFN(12),
			&ccm->mpctl);
	writel(PLL_PD(1) | PLL_MFD(4) | PLL_MFI(12) | PLL_MFN(1),
			&ccm->spctl);
}

/* DRAM configuration */

#define ESDMISC_MDDR_SETUP	0x00000004
#define ESDMISC_MDDR_RESET_DL	0x0000000c
/*
 * decoding magic 0x6ac73a = 0b 0110 1010   1100 0111   0011 1010 below:
 *   tXP = 11, tWTR = 0, tRP = 10, tMRD = 10
 *   tWR = 1, tRAS = 100, tRRD = 01, tCAS = 11
 *   tRCD = 011, tRC = 010
 *  note: all but tWTR (1), tRC (111) are reset defaults,
 *     the same values work in the jtag configuration
 *
 *  Bluetechnix setup has 0x75e73a (for 128MB) =
 *			0b 0111 0101   1110 0111   0011 1010
 *   tXP = 11, tWTR = 1, tRP = 01, tMRD = 01
 *   tWR = 1, tRAS = 110, tRRD = 01, tCAS = 11
 *   tRCD = 011, tRC = 010
 */
#define ESDCFG0_MDDR_SETUP	0x006ac73a
#define ESDCTL_ROW_COL		(ESDCTL_SDE | ESDCTL_ROW(2) | ESDCTL_COL(2))
#define ESDCTL_SETTINGS		(ESDCTL_ROW_COL | ESDCTL_SREFR(3) | \
				 ESDCTL_DSIZ(2) | ESDCTL_BL(1))
#define ESDCTL_PRECHARGE	(ESDCTL_ROW_COL | ESDCTL_CMD_PRECHARGE)
#define ESDCTL_AUTOREFRESH	(ESDCTL_ROW_COL | ESDCTL_CMD_AUTOREFRESH)
#define ESDCTL_LOADMODEREG	(ESDCTL_ROW_COL | ESDCTL_CMD_LOADMODEREG)
#define ESDCTL_RW		ESDCTL_SETTINGS

static void board_setup_sdram(void)
{
	u32 *pad;
	struct esdc_regs *esdc = (struct esdc_regs *)ESDCTL_BASE_ADDR;

	/*
	 * setup pad control for the controller pins
	 * no loopback, no pull, no keeper, no open drain,
	 * standard input, standard drive, slow slew rate
	 */
	for (pad = (u32 *) IOMUXC_SW_PAD_CTL_SDCKE1_SDCLK_SDCLK_B;
			pad <= (u32 *) IOMUXC_SW_PAD_CTL_VPG0_VPG1_A0; pad++)
		*pad = 0;

	/* set up MX31 DDR Memory Controller */
	writel(ESDMISC_MDDR_SETUP, &esdc->misc);
	writel(ESDCFG0_MDDR_SETUP, &esdc->cfg0);

	/* perform DDR init sequence for CSD0 */
	writel(ESDCTL_PRECHARGE, &esdc->ctl0);
	writel(0x12344321, CSD0_BASE+0x0f00);
	writel(ESDCTL_AUTOREFRESH, &esdc->ctl0);
	writel(0x12344321, CSD0_BASE);
	writel(0x12344321, CSD0_BASE);
	writel(ESDCTL_LOADMODEREG, &esdc->ctl0);
	writeb(0xda, CSD0_BASE+0x33);
	writeb(0xff, CSD0_BASE+0x1000000);
	writel(ESDCTL_RW, &esdc->ctl0);
	writel(0xDEADBEEF, CSD0_BASE);
	writel(ESDMISC_MDDR_RESET_DL, &esdc->misc);
}

static void tt01_spi3_hw_init(void)
{
	/* CSPI3 */
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_CSPI3_MISO, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_CSPI3_MOSI, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_CSPI3_SCLK, MUX_CTL_FUNC));
	/* CSPI3, SS0 = Atlas */
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_CSPI2_SS0, MUX_CTL_ALT1));

	/* start CSPI3 clock (3 = always on except if PLL off) */
	setbits_le32(CCM_CGR0, 3 << 16);
}

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((long *) CONFIG_SYS_SDRAM_BASE,
			PHYS_SDRAM_1_SIZE);
	return 0;
}

int board_early_init_f(void)
{
	/* CS4: FPGA incl. network controller */
	struct mxc_weimcs cs4 = {
		/*    sp wp bcd bcs psz pme sync dol cnc wsc ew wws edc */
		CSCR_U(0, 0,  0,  0,  0,  0,   0,  0,  3, 28, 1,  7,  6),
		/*   oea oen ebwa ebwn csa ebc dsz csn psr cre wrap csen */
		CSCR_L(4,  4,   4,  10,  4,  0,  5,  4,  0,  0,   0,   1),
		/*  ebra ebrn rwa rwn mum lah lbn lba dww dct wwu age cnc2 fce*/
		CSCR_A(4,   4,  4,  4,  0,  1,  4,  3,  0,  0,  0,  0,  1,   0)
	};

	/* this seems essential, won't start without, but why? */
	writel(IPU_CONF_DI_EN, (u32 *) IPU_CONF);

	board_setup_clocks();
	board_setup_sdram();
	mxc_setup_weimcs(4, &cs4);

	/* Setup UART2 and SPI3 pins */
	mx31_uart2_hw_init();
	tt01_spi3_hw_init();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;
	return 0;
}

int board_late_init(void)
{
#ifdef CONFIG_HW_WATCHDOG
	mxc_hw_watchdog_enable();
#endif

	return 0;
}

int checkboard(void)
{
	puts(BOARD_STRING "\n");
	return 0;
}

#ifdef CONFIG_MXC_MMC
int board_mmc_init(bd_t *bis)
{
	u32 val;
	struct pmic *p;

	/*
	* this is the first driver to use the pmic, so call
	* pmic_init() here. board_late_init() is too late for
	* the MMC driver.
	*/
	pmic_init();
	p = get_pmic();

	/* configure pins for SDHC1 only */
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_SD1_CLK, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_SD1_CMD, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_SD1_DATA0, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_SD1_DATA1, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_SD1_DATA2, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE(MUX_CTL_SD1_DATA3, MUX_CTL_FUNC));

	/* turn on power V_MMC1 */
	if (pmic_reg_read(p, REG_MODE_1, &val) < 0)
		pmic_reg_write(p, REG_MODE_1, val | VMMC1EN);

	return mxc_mmc_init(bis);
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}

#ifdef CONFIG_CONSOLE_EXTRA_INFO
void video_get_info_str(int line_number, char *info)
{
	u32 srev = get_cpu_rev();

	switch (line_number) {
	case 2:
		sprintf(info, " CPU  : Freescale i.MX31 rev %d.%d%s at %d MHz",
			(srev & 0xF0) >> 4, (srev & 0x0F),
			((srev & 0x8000) ? " unknown" : ""),
		mxc_get_clock(MXC_ARM_CLK) / 1000000);
		break;
	case 3:
		strcpy(info, " " BOARD_STRING);
		break;
	default:
		info[0] = 0;
	}
}
#endif
