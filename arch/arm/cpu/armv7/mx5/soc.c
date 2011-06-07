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
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/errno.h>
#include <asm/io.h>

#ifdef CONFIG_FSL_ESDHC
#include <fsl_esdhc.h>
#endif

#if !(defined(CONFIG_MX51) || defined(CONFIG_MX53))
#error "CPU_TYPE not defined"
#endif

u32 get_cpu_rev(void)
{
#ifdef CONFIG_MX51
	int system_rev = 0x51000;
#else
	int system_rev = 0x53000;
#endif
	int reg = __raw_readl(ROM_SI_REV);

#if defined(CONFIG_MX51)
	switch (reg) {
	case 0x02:
		system_rev |= CHIP_REV_1_1;
		break;
	case 0x10:
		if ((__raw_readl(GPIO1_BASE_ADDR + 0x0) & (0x1 << 22)) == 0)
			system_rev |= CHIP_REV_2_5;
		else
			system_rev |= CHIP_REV_2_0;
		break;
	case 0x20:
		system_rev |= CHIP_REV_3_0;
		break;
	default:
		system_rev |= CHIP_REV_1_0;
		break;
	}
#else
	if (reg < 0x20)
		system_rev |= CHIP_REV_1_0;
	else
		system_rev |= reg;
#endif
	return system_rev;
}

static char *get_reset_cause(void)
{
	u32 cause;
	struct src *src_regs = (struct src *)SRC_BASE_ADDR;

	cause = readl(&src_regs->srsr);
	writel(cause, &src_regs->srsr);

	switch (cause) {
	case 0x00001:
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
int print_cpuinfo(void)
{
	u32 cpurev;

	cpurev = get_cpu_rev();
	printf("CPU:   Freescale i.MX%x family rev%d.%d at %d MHz\n",
		(cpurev & 0xFF000) >> 12,
		(cpurev & 0x000F0) >> 4,
		(cpurev & 0x0000F) >> 0,
		mxc_get_clock(MXC_ARM_CLK) / 1000000);
	printf("Reset cause: %s\n", get_reset_cause());
	return 0;
}
#endif

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
#if defined(CONFIG_FEC_MXC)
extern int fecmxc_initialize(bd_t *bis);
#endif

int cpu_eth_init(bd_t *bis)
{
	int rc = -ENODEV;

#if defined(CONFIG_FEC_MXC)
	rc = fecmxc_initialize(bis);
#endif

	return rc;
}

#if defined(CONFIG_FEC_MXC)
void imx_get_mac_from_fuse(unsigned char *mac)
{
	int i;
	struct iim_regs *iim = (struct iim_regs *)IMX_IIM_BASE;
	struct fuse_bank *bank = &iim->bank[1];
	struct fuse_bank1_regs *fuse =
			(struct fuse_bank1_regs *)bank->fuse_regs;

	for (i = 0; i < 6; i++)
		mac[i] = readl(&fuse->mac_addr[i]) & 0xff;
}
#endif

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

void set_chipselect_size(int const cs_size)
{
	unsigned int reg;
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	reg = readl(&iomuxc_regs->gpr1);

	switch (cs_size) {
	case CS0_128:
		reg &= ~0x7;	/* CS0=128MB, CS1=0, CS2=0, CS3=0 */
		reg |= 0x5;
		break;
	case CS0_64M_CS1_64M:
		reg &= ~0x3F;	/* CS0=64MB, CS1=64MB, CS2=0, CS3=0 */
		reg |= 0x1B;
		break;
	case CS0_64M_CS1_32M_CS2_32M:
		reg &= ~0x1FF;	/* CS0=64MB, CS1=32MB, CS2=32MB, CS3=0 */
		reg |= 0x4B;
		break;
	case CS0_32M_CS1_32M_CS2_32M_CS3_32M:
		reg &= ~0xFFF;  /* CS0=32MB, CS1=32MB, CS2=32MB, CS3=32MB */
		reg |= 0x249;
		break;
	default:
		printf("Unknown chip select size: %d\n", cs_size);
		break;
	}

	writel(reg, &iomuxc_regs->gpr1);
}

void reset_cpu(ulong addr)
{
	__raw_writew(4, WDOG1_BASE_ADDR);
}
