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
#include <asm/arch/sys_proto.h>

#include <asm/errno.h>
#include <asm/io.h>
#include <asm/imx-common/boot_mode.h>

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

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif

#if defined(CONFIG_FEC_MXC)
void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
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

#ifdef CONFIG_MX53
void boot_mode_apply(unsigned cfg_val)
{
	writel(cfg_val, &((struct srtc_regs *)SRTC_BASE_ADDR)->lpgr);
}
/*
 * cfg_val will be used for
 * Boot_cfg3[7:0]:Boot_cfg2[7:0]:Boot_cfg1[7:0]
 *
 * If bit 28 of LPGR is set upon watchdog reset,
 * bits[25:0] of LPGR will move to SBMR.
 */
const struct boot_mode soc_boot_modes[] = {
	{"normal",	MAKE_CFGVAL(0x00, 0x00, 0x00, 0x00)},
	/* usb or serial download */
	{"usb",		MAKE_CFGVAL(0x00, 0x00, 0x00, 0x13)},
	{"sata",	MAKE_CFGVAL(0x28, 0x00, 0x00, 0x12)},
	{"escpi1:0",	MAKE_CFGVAL(0x38, 0x20, 0x00, 0x12)},
	{"escpi1:1",	MAKE_CFGVAL(0x38, 0x20, 0x04, 0x12)},
	{"escpi1:2",	MAKE_CFGVAL(0x38, 0x20, 0x08, 0x12)},
	{"escpi1:3",	MAKE_CFGVAL(0x38, 0x20, 0x0c, 0x12)},
	/* 4 bit bus width */
	{"esdhc1",	MAKE_CFGVAL(0x40, 0x20, 0x00, 0x12)},
	{"esdhc2",	MAKE_CFGVAL(0x40, 0x20, 0x08, 0x12)},
	{"esdhc3",	MAKE_CFGVAL(0x40, 0x20, 0x10, 0x12)},
	{"esdhc4",	MAKE_CFGVAL(0x40, 0x20, 0x18, 0x12)},
	{NULL,		0},
};
#endif
