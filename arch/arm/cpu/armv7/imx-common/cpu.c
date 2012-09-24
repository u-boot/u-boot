/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
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
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>

#ifdef CONFIG_FSL_ESDHC
#include <fsl_esdhc.h>
#endif

char *get_reset_cause(void)
{
	u32 cause;
	struct src *src_regs = (struct src *)SRC_BASE_ADDR;

	cause = readl(&src_regs->srsr);
	writel(cause, &src_regs->srsr);

	switch (cause) {
	case 0x00001:
	case 0x00011:
		return "POR";
	case 0x00004:
		return "CSU";
	case 0x00008:
		return "IPP USER";
	case 0x00010:
		return "WDOG";
	case 0x00020:
		return "JTAG HIGH-Z";
	case 0x00040:
		return "JTAG SW";
	case 0x10000:
		return "WARM BOOT";
	default:
		return "unknown reset";
	}
}

#if defined(CONFIG_DISPLAY_CPUINFO)

static char *get_imx_type(u32 imxtype)
{
	switch (imxtype) {
	case 0x63:
		return "6Q";	/* Quad-core version of the mx6 */
	case 0x61:
		return "6DS";	/* Dual/Solo version of the mx6 */
	case 0x60:
		return "6SL";	/* Solo-Lite version of the mx6 */
	case 0x51:
		return "51";
	case 0x53:
		return "53";
	default:
		return "unknown";
	}
}

int print_cpuinfo(void)
{
	u32 cpurev;

	cpurev = get_cpu_rev();

	printf("CPU:   Freescale i.MX%s rev%d.%d at %d MHz\n",
		get_imx_type((cpurev & 0xFF000) >> 12),
		(cpurev & 0x000F0) >> 4,
		(cpurev & 0x0000F) >> 0,
		mxc_get_clock(MXC_ARM_CLK) / 1000000);
	printf("Reset cause: %s\n", get_reset_cause());
	return 0;
}
#endif

int cpu_eth_init(bd_t *bis)
{
	int rc = -ENODEV;

#if defined(CONFIG_FEC_MXC)
	rc = fecmxc_initialize(bis);
#endif

	return rc;
}

/*
 * Initializes on-chip MMC controllers.
 * to override, implement board_mmc_init()
 */
int cpu_mmc_init(bd_t *bis)
{
#ifdef CONFIG_FSL_ESDHC
	return fsl_esdhc_mmc_init(bis);
#else
	return 0;
#endif
}

void reset_cpu(ulong addr)
{
	__raw_writew(4, WDOG1_BASE_ADDR);
}

u32 get_ahb_clk(void)
{
	struct mxc_ccm_reg *imx_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	u32 reg, ahb_podf;

	reg = __raw_readl(&imx_ccm->cbcdr);
	reg &= MXC_CCM_CBCDR_AHB_PODF_MASK;
	ahb_podf = reg >> MXC_CCM_CBCDR_AHB_PODF_OFFSET;

	return get_periph_clk() / (ahb_podf + 1);
}
