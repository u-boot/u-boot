/*
 * (C) Copyright 2004
 * Pierre AUBERT, Staubli Faverges, <p.aubert@staubli.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Init is derived from Linux code.
 */
#include <common.h>

#ifdef CFG_CMD_IDE
#include <mpc5xxx.h>

#define CALC_TIMING(t) (t + period - 1) / period

#define GPIO_PSC1_4	0x01000000ul

int ide_preinit (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	long period, t0, t1, t2_8, t2_16, t4, ta;
	vu_long reg;
	struct mpc5xxx_sdma *psdma = (struct mpc5xxx_sdma *) MPC5XXX_SDMA;

	reg = *(vu_long *) MPC5XXX_GPS_PORT_CONFIG;
	reg = (reg & ~0x03000000ul) | 0x01000000ul;
	*(vu_long *) MPC5XXX_GPS_PORT_CONFIG = reg;

	/* All sample codes do that... */
	*(vu_long *) MPC5XXX_ATA_SHARE_COUNT = 0;

	/* Configure and reset host */
	*(vu_long *) MPC5XXX_ATA_HOST_CONFIG = MPC5xxx_ATA_HOSTCONF_IORDY |
		MPC5xxx_ATA_HOSTCONF_SMR | MPC5xxx_ATA_HOSTCONF_FR;
	udelay (10);
	*(vu_long *) MPC5XXX_ATA_HOST_CONFIG = MPC5xxx_ATA_HOSTCONF_IORDY;

	/* Disable prefetch on Commbus */
	psdma->PtdCntrl |= 1;

	/* Init timings : we use PIO mode 0 timings */
	period = 1000000000 / gd->ipb_clk;	/* period in ns */

	t0 = CALC_TIMING (600);
	t2_8 = CALC_TIMING (290);
	t2_16 = CALC_TIMING (165);
	reg = (t0 << 24) | (t2_8 << 16) | (t2_16 << 8);
	*(vu_long *) MPC5XXX_ATA_PIO1 = reg;

	t4 = CALC_TIMING (30);
	t1 = CALC_TIMING (70);
	ta = CALC_TIMING (35);
	reg = (t4 << 24) | (t1 << 16) | (ta << 8);

	*(vu_long *) MPC5XXX_ATA_PIO2 = reg;

#if defined (CONFIG_ICECUBE) && defined (CONFIG_IDE_RESET)
	/* Configure PSC1_4 as GPIO output for ATA reset */
	*(vu_long *) MPC5XXX_WU_GPIO_DATA |= GPIO_PSC1_4;
	*(vu_long *) MPC5XXX_WU_GPIO_ENABLE |= GPIO_PSC1_4;
	*(vu_long *) MPC5XXX_WU_GPIO_DIR |= GPIO_PSC1_4;
#endif /* defined (CONFIG_ICECUBE) && defined (CONFIG_IDE_RESET) */

	return (0);
}

#if defined (CONFIG_ICECUBE) && defined (CONFIG_IDE_RESET)
void ide_set_reset (int idereset)
{
	if (idereset) {
		*(vu_long *) MPC5XXX_WU_GPIO_DATA &= ~GPIO_PSC1_4;
	} else {
		*(vu_long *) MPC5XXX_WU_GPIO_DATA |= GPIO_PSC1_4;
	}
}
#endif /* defined (CONFIG_ICECUBE) && defined (CONFIG_IDE_RESET) */
#endif /* CFG_CMD_IDE */
