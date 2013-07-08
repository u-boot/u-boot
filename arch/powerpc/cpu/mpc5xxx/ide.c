/*
 * (C) Copyright 2004
 * Pierre AUBERT, Staubli Faverges, <p.aubert@staubli.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Init is derived from Linux code.
 */
#include <common.h>

#if defined(CONFIG_CMD_IDE)
#include <mpc5xxx.h>

DECLARE_GLOBAL_DATA_PTR;

#define CALC_TIMING(t) (t + period - 1) / period

#ifdef CONFIG_IDE_RESET
extern void init_ide_reset (void);
#endif

int ide_preinit (void)
{
	long period, t0, t1, t2_8, t2_16, t4, ta;
	vu_long reg;
	struct mpc5xxx_sdma *psdma = (struct mpc5xxx_sdma *) MPC5XXX_SDMA;

	reg = *(vu_long *) MPC5XXX_GPS_PORT_CONFIG;
#if defined(CONFIG_SYS_ATA_CS_ON_I2C2)
	/* ATA cs0/1 on i2c2 clk/io */
	reg = (reg & ~0x03000000ul) | 0x02000000ul;
#elif defined(CONFIG_SYS_ATA_CS_ON_TIMER01)
	/* ATA cs0/1 on Timer 0/1 */
	reg = (reg & ~0x03000000ul) | 0x03000000ul;
#else
	/* ATA cs0/1 on Local Plus cs4/5 */
	reg = (reg & ~0x03000000ul) | 0x01000000ul;
#endif	/* CONFIG_TOTAL5200 */
	*(vu_long *) MPC5XXX_GPS_PORT_CONFIG = reg;

	/* All sample codes do that... */
	*(vu_long *) MPC5XXX_ATA_SHARE_COUNT = 0;

#if defined(CONFIG_UC101)
	/* Configure and reset host */
	*(vu_long *) MPC5XXX_ATA_HOST_CONFIG =
		MPC5xxx_ATA_HOSTCONF_SMR | MPC5xxx_ATA_HOSTCONF_FR;
	udelay (10);
	*(vu_long *) MPC5XXX_ATA_HOST_CONFIG = 0;
#else
	/* Configure and reset host */
	*(vu_long *) MPC5XXX_ATA_HOST_CONFIG = MPC5xxx_ATA_HOSTCONF_IORDY |
		MPC5xxx_ATA_HOSTCONF_SMR | MPC5xxx_ATA_HOSTCONF_FR;
	udelay (10);
	*(vu_long *) MPC5XXX_ATA_HOST_CONFIG = MPC5xxx_ATA_HOSTCONF_IORDY;
#endif

	/* Disable prefetch on Commbus */
	psdma->PtdCntrl |= 1;

	/* Init timings : we use PIO mode 0 timings */
	period = 1000000000 / gd->arch.ipb_clk;	/* period in ns */

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

#ifdef CONFIG_IDE_RESET
	init_ide_reset ();
#endif /* CONFIG_IDE_RESET */

	return (0);
}
#endif
