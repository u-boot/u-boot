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

u32 get_cpu_rev(void)
{
	int system_rev = 0x61000 | CHIP_REV_1_0;

	return system_rev;
}

#ifdef CONFIG_ARCH_CPU_INIT
void init_aips(void)
{
	u32 reg = AIPS1_BASE_ADDR;

	/*
	 * Set all MPROTx to be non-bufferable, trusted for R/W,
	 * not forced to user-mode.
	 */
	writel(0x77777777, reg + 0x00);
	writel(0x77777777, reg + 0x04);

	reg = AIPS2_BASE_ADDR;
	writel(0x77777777, reg + 0x00);
	writel(0x77777777, reg + 0x04);
}

int arch_cpu_init(void)
{
	init_aips();

	return 0;
}
#endif

#if defined(CONFIG_FEC_MXC)
void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	struct iim_regs *iim = (struct iim_regs *)IMX_IIM_BASE;
	struct fuse_bank *bank = &iim->bank[4];
	struct fuse_bank4_regs *fuse =
			(struct fuse_bank4_regs *)bank->fuse_regs;

	u32 mac_lo = readl(&fuse->mac_addr_low);
	u32 mac_hi = readl(&fuse->mac_addr_high);

	*(u32 *)mac = mac_lo;

	mac[4] = mac_hi & 0xff;
	mac[5] = (mac_hi >> 8) & 0xff;

}
#endif
