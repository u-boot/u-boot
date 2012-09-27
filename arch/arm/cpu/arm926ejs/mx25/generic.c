/*
 * (C) Copyright 2009 DENX Software Engineering
 * Author: John Rigby <jrigby@gmail.com>
 *
 * Based on mx27/generic.c:
 *  Copyright (c) 2008 Eric Jarrige <eric.jarrige@armadeus.org>
 *  Copyright (c) 2009 Ilya Yanok <yanok@emcraft.com>
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
#include <div64.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/imx25-pinmux.h>
#include <asm/arch/clock.h>

#ifdef CONFIG_FSL_ESDHC
#include <fsl_esdhc.h>

DECLARE_GLOBAL_DATA_PTR;
#endif

/*
 *  get the system pll clock in Hz
 *
 *                  mfi + mfn / (mfd +1)
 *  f = 2 * f_ref * --------------------
 *                        pd + 1
 */
static unsigned int imx_decode_pll(unsigned int pll, unsigned int f_ref)
{
	unsigned int mfi = (pll >> CCM_PLL_MFI_SHIFT)
	    & CCM_PLL_MFI_MASK;
	int mfn = (pll >> CCM_PLL_MFN_SHIFT)
	    & CCM_PLL_MFN_MASK;
	unsigned int mfd = (pll >> CCM_PLL_MFD_SHIFT)
	    & CCM_PLL_MFD_MASK;
	unsigned int pd = (pll >> CCM_PLL_PD_SHIFT)
	    & CCM_PLL_PD_MASK;

	mfi = mfi <= 5 ? 5 : mfi;
	mfn = mfn >= 512 ? mfn - 1024 : mfn;
	mfd += 1;
	pd += 1;

	return lldiv(2 * (u64) f_ref * (mfi * mfd + mfn),
		     mfd * pd);
}

static ulong imx_get_mpllclk(void)
{
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;
	ulong fref = MXC_HCLK;

	return imx_decode_pll(readl(&ccm->mpctl), fref);
}

static ulong imx_get_armclk(void)
{
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;
	ulong cctl = readl(&ccm->cctl);
	ulong fref = imx_get_mpllclk();
	ulong div;

	if (cctl & CCM_CCTL_ARM_SRC)
		fref = lldiv((u64) fref * 3, 4);

	div = ((cctl >> CCM_CCTL_ARM_DIV_SHIFT)
	       & CCM_CCTL_ARM_DIV_MASK) + 1;

	return fref / div;
}

static ulong imx_get_ahbclk(void)
{
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;
	ulong cctl = readl(&ccm->cctl);
	ulong fref = imx_get_armclk();
	ulong div;

	div = ((cctl >> CCM_CCTL_AHB_DIV_SHIFT)
	       & CCM_CCTL_AHB_DIV_MASK) + 1;

	return fref / div;
}

static ulong imx_get_ipgclk(void)
{
	return imx_get_ahbclk() / 2;
}

static ulong imx_get_perclk(int clk)
{
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;
	ulong fref = imx_get_ahbclk();
	ulong div;

	div = readl(&ccm->pcdr[CCM_PERCLK_REG(clk)]);
	div = ((div >> CCM_PERCLK_SHIFT(clk)) & CCM_PERCLK_MASK) + 1;

	return fref / div;
}

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	if (clk >= MXC_CLK_NUM)
		return -1;
	switch (clk) {
	case MXC_ARM_CLK:
		return imx_get_armclk();
	case MXC_AHB_CLK:
		return imx_get_ahbclk();
	case MXC_IPG_CLK:
	case MXC_CSPI_CLK:
	case MXC_FEC_CLK:
		return imx_get_ipgclk();
	default:
		return imx_get_perclk(clk);
	}
}

u32 get_cpu_rev(void)
{
	u32 srev;
	u32 system_rev = 0x25000;

	/* read SREV register from IIM module */
	struct iim_regs *iim = (struct iim_regs *)IMX_IIM_BASE;
	srev = readl(&iim->iim_srev);

	switch (srev) {
	case 0x00:
		system_rev |= CHIP_REV_1_0;
		break;
	case 0x01:
		system_rev |= CHIP_REV_1_1;
		break;
	case 0x02:
		system_rev |= CHIP_REV_1_2;
		break;
	default:
		system_rev |= 0x8000;
		break;
	}

	return system_rev;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
static char *get_reset_cause(void)
{
	/* read RCSR register from CCM module */
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;

	u32 cause = readl(&ccm->rcsr) & 0x0f;

	if (cause == 0)
		return "POR";
	else if (cause == 1)
		return "RST";
	else if ((cause & 2) == 2)
		return "WDOG";
	else if ((cause & 4) == 4)
		return "SW RESET";
	else if ((cause & 8) == 8)
		return "JTAG";
	else
		return "unknown reset";

}

int print_cpuinfo(void)
{
	char buf[32];
	u32 cpurev = get_cpu_rev();

	printf("CPU:   Freescale i.MX25 rev%d.%d%s at %s MHz\n",
		(cpurev & 0xF0) >> 4, (cpurev & 0x0F),
		((cpurev & 0x8000) ? " unknown" : ""),
		strmhz(buf, imx_get_armclk()));
	printf("Reset cause: %s\n\n", get_reset_cause());
	return 0;
}
#endif

void enable_caches(void)
{
#ifndef CONFIG_SYS_DCACHE_OFF
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
#endif
}

#if defined(CONFIG_FEC_MXC)
/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;
	ulong val;

	val = readl(&ccm->cgr0);
	val |= (1 << 23);
	writel(val, &ccm->cgr0);
	return fecmxc_initialize(bis);
}
#endif

int get_clocks(void)
{
#ifdef CONFIG_FSL_ESDHC
#if CONFIG_SYS_FSL_ESDHC_ADDR == IMX_MMC_SDHC2_BASE
	gd->sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
#else
	gd->sdhc_clk = mxc_get_clock(MXC_ESDHC1_CLK);
#endif
#endif
	return 0;
}

#ifdef CONFIG_FSL_ESDHC
/*
 * Initializes on-chip MMC controllers.
 * to override, implement board_mmc_init()
 */
int cpu_mmc_init(bd_t *bis)
{
	return fsl_esdhc_mmc_init(bis);
}
#endif

#ifdef CONFIG_MXC_UART
void mx25_uart1_init_pins(void)
{
	struct iomuxc_mux_ctl *muxctl;
	struct iomuxc_pad_ctl *padctl;
	u32 inpadctl;
	u32 outpadctl;
	u32 muxmode0;

	muxctl = (struct iomuxc_mux_ctl *)IMX_IOPADMUX_BASE;
	padctl = (struct iomuxc_pad_ctl *)IMX_IOPADCTL_BASE;
	muxmode0 = MX25_PIN_MUX_MODE(0);
	/*
	 * set up input pins with hysteresis and 100K pull-ups
	 */
	inpadctl = MX25_PIN_PAD_CTL_HYS
	    | MX25_PIN_PAD_CTL_PKE
	    | MX25_PIN_PAD_CTL_PUE | MX25_PIN_PAD_CTL_100K_PU;

	/*
	 * set up output pins with 100K pull-downs
	 * FIXME: need to revisit this
	 *      PUE is ignored if PKE is not set
	 *      so the right value here is likely
	 *        0x0 for no pull up/down
	 *      or
	 *        0xc0 for 100k pull down
	 */
	outpadctl = MX25_PIN_PAD_CTL_PUE | MX25_PIN_PAD_CTL_100K_PD;

	/* UART1 */
	/* rxd */
	writel(muxmode0, &muxctl->pad_uart1_rxd);
	writel(inpadctl, &padctl->pad_uart1_rxd);

	/* txd */
	writel(muxmode0, &muxctl->pad_uart1_txd);
	writel(outpadctl, &padctl->pad_uart1_txd);

	/* rts */
	writel(muxmode0, &muxctl->pad_uart1_rts);
	writel(outpadctl, &padctl->pad_uart1_rts);

	/* cts */
	writel(muxmode0, &muxctl->pad_uart1_cts);
	writel(inpadctl, &padctl->pad_uart1_cts);
}
#endif /* CONFIG_MXC_UART */

#ifdef CONFIG_FEC_MXC
void mx25_fec_init_pins(void)
{
	struct iomuxc_mux_ctl *muxctl;
	struct iomuxc_pad_ctl *padctl;
	u32 inpadctl_100kpd;
	u32 inpadctl_22kpu;
	u32 outpadctl;
	u32 muxmode0;

	muxctl = (struct iomuxc_mux_ctl *)IMX_IOPADMUX_BASE;
	padctl = (struct iomuxc_pad_ctl *)IMX_IOPADCTL_BASE;
	muxmode0 = MX25_PIN_MUX_MODE(0);
	inpadctl_100kpd = MX25_PIN_PAD_CTL_HYS
	    | MX25_PIN_PAD_CTL_PKE
	    | MX25_PIN_PAD_CTL_PUE | MX25_PIN_PAD_CTL_100K_PD;
	inpadctl_22kpu = MX25_PIN_PAD_CTL_HYS
	    | MX25_PIN_PAD_CTL_PKE
	    | MX25_PIN_PAD_CTL_PUE | MX25_PIN_PAD_CTL_22K_PU;
	/*
	 * set up output pins with 100K pull-downs
	 * FIXME: need to revisit this
	 *      PUE is ignored if PKE is not set
	 *      so the right value here is likely
	 *        0x0 for no pull
	 *      or
	 *        0xc0 for 100k pull down
	 */
	outpadctl = MX25_PIN_PAD_CTL_PUE | MX25_PIN_PAD_CTL_100K_PD;

	/* FEC_TX_CLK */
	writel(muxmode0, &muxctl->pad_fec_tx_clk);
	writel(inpadctl_100kpd, &padctl->pad_fec_tx_clk);

	/* FEC_RX_DV */
	writel(muxmode0, &muxctl->pad_fec_rx_dv);
	writel(inpadctl_100kpd, &padctl->pad_fec_rx_dv);

	/* FEC_RDATA0 */
	writel(muxmode0, &muxctl->pad_fec_rdata0);
	writel(inpadctl_100kpd, &padctl->pad_fec_rdata0);

	/* FEC_TDATA0 */
	writel(muxmode0, &muxctl->pad_fec_tdata0);
	writel(outpadctl, &padctl->pad_fec_tdata0);

	/* FEC_TX_EN */
	writel(muxmode0, &muxctl->pad_fec_tx_en);
	writel(outpadctl, &padctl->pad_fec_tx_en);

	/* FEC_MDC */
	writel(muxmode0, &muxctl->pad_fec_mdc);
	writel(outpadctl, &padctl->pad_fec_mdc);

	/* FEC_MDIO */
	writel(muxmode0, &muxctl->pad_fec_mdio);
	writel(inpadctl_22kpu, &padctl->pad_fec_mdio);

	/* FEC_RDATA1 */
	writel(muxmode0, &muxctl->pad_fec_rdata1);
	writel(inpadctl_100kpd, &padctl->pad_fec_rdata1);

	/* FEC_TDATA1 */
	writel(muxmode0, &muxctl->pad_fec_tdata1);
	writel(outpadctl, &padctl->pad_fec_tdata1);

}

void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	int i;
	struct iim_regs *iim = (struct iim_regs *)IMX_IIM_BASE;
	struct fuse_bank *bank = &iim->bank[0];
	struct fuse_bank0_regs *fuse =
			(struct fuse_bank0_regs *)bank->fuse_regs;

	for (i = 0; i < 6; i++)
		mac[i] = readl(&fuse->mac_addr[i]) & 0xff;
}
#endif /* CONFIG_FEC_MXC */
