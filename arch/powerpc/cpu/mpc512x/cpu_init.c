/*
 * Copyright (C) 2004-2006 Freescale Semiconductor, Inc.
 * Copyright (C) 2007-2009 DENX Software Engineering
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
 *
 * Derived from the MPC83xx code.
 *
 */

#include <common.h>
#include <asm/io.h>
#include <asm/mpc512x.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Set up the memory map, initialize registers,
 */
void cpu_init_f (volatile immap_t * im)
{
	u32 ips_div;

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET);

	/* Clear initial global data */
	memset ((void *) gd, 0, sizeof (gd_t));

	/* Local Window and chip select configuration */
#if defined(CONFIG_SYS_CS0_START) && defined(CONFIG_SYS_CS0_SIZE)
	out_be32(&im->sysconf.lpcs0aw,
		CSAW_START(CONFIG_SYS_CS0_START) |
		CSAW_STOP(CONFIG_SYS_CS0_START, CONFIG_SYS_CS0_SIZE));
	sync_law(&im->sysconf.lpcs0aw);
#endif
#if defined(CONFIG_SYS_CS0_CFG)
	out_be32(&im->lpc.cs_cfg[0], CONFIG_SYS_CS0_CFG);
#endif

#if defined(CONFIG_SYS_CS1_START) && defined(CONFIG_SYS_CS1_SIZE)
	out_be32(&im->sysconf.lpcs1aw,
		CSAW_START(CONFIG_SYS_CS1_START) |
		CSAW_STOP(CONFIG_SYS_CS1_START, CONFIG_SYS_CS1_SIZE));
	sync_law(&im->sysconf.lpcs1aw);
#endif
#if defined(CONFIG_SYS_CS1_CFG)
	out_be32(&im->lpc.cs_cfg[1], CONFIG_SYS_CS1_CFG);
#endif

#if defined(CONFIG_SYS_CS2_START) && (defined CONFIG_SYS_CS2_SIZE)
	out_be32(&im->sysconf.lpcs2aw,
		CSAW_START(CONFIG_SYS_CS2_START) |
		CSAW_STOP(CONFIG_SYS_CS2_START, CONFIG_SYS_CS2_SIZE));
	sync_law(&im->sysconf.lpcs2aw);
#endif
#if defined(CONFIG_SYS_CS2_CFG)
	out_be32(&im->lpc.cs_cfg[2], CONFIG_SYS_CS2_CFG);
#endif

#if defined(CONFIG_SYS_CS3_START) && defined(CONFIG_SYS_CS3_SIZE)
	out_be32(&im->sysconf.lpcs3aw,
		CSAW_START(CONFIG_SYS_CS3_START) |
		CSAW_STOP(CONFIG_SYS_CS3_START, CONFIG_SYS_CS3_SIZE));
	sync_law(&im->sysconf.lpcs3aw);
#endif
#if defined(CONFIG_SYS_CS3_CFG)
	out_be32(&im->lpc.cs_cfg[3], CONFIG_SYS_CS3_CFG);
#endif

#if defined(CONFIG_SYS_CS4_START) && defined(CONFIG_SYS_CS4_SIZE)
	out_be32(&im->sysconf.lpcs4aw,
		CSAW_START(CONFIG_SYS_CS4_START) |
		CSAW_STOP(CONFIG_SYS_CS4_START, CONFIG_SYS_CS4_SIZE));
	sync_law(&im->sysconf.lpcs4aw);
#endif
#if defined(CONFIG_SYS_CS4_CFG)
	out_be32(&im->lpc.cs_cfg[4], CONFIG_SYS_CS4_CFG);
#endif

#if defined(CONFIG_SYS_CS5_START) && defined(CONFIG_SYS_CS5_SIZE)
	out_be32(&im->sysconf.lpcs5aw,
		CSAW_START(CONFIG_SYS_CS5_START) |
		CSAW_STOP(CONFIG_SYS_CS5_START, CONFIG_SYS_CS5_SIZE));
	sync_law(&im->sysconf.lpcs5aw);
#endif
#if defined(CONFIG_SYS_CS5_CFG)
	out_be32(&im->lpc.cs_cfg[5], CONFIG_SYS_CS5_CFG);
#endif

#if defined(CONFIG_SYS_CS6_START) && defined(CONFIG_SYS_CS6_SIZE)
	out_be32(&im->sysconf.lpcs6aw,
		CSAW_START(CONFIG_SYS_CS6_START) |
		CSAW_STOP(CONFIG_SYS_CS6_START, CONFIG_SYS_CS6_SIZE));
	sync_law(&im->sysconf.lpcs6aw);
#endif
#if defined(CONFIG_SYS_CS6_CFG)
	out_be32(&im->lpc.cs_cfg[6], CONFIG_SYS_CS6_CFG);
#endif

#if defined(CONFIG_SYS_CS7_START) && defined(CONFIG_SYS_CS7_SIZE)
	out_be32(&im->sysconf.lpcs7aw,
		CSAW_START(CONFIG_SYS_CS7_START) |
		CSAW_STOP(CONFIG_SYS_CS7_START, CONFIG_SYS_CS7_SIZE));
	sync_law(&im->sysconf.lpcs7aw);
#endif
#if defined(CONFIG_SYS_CS7_CFG)
	out_be32(&im->lpc.cs_cfg[7], CONFIG_SYS_CS7_CFG);
#endif

#if defined CONFIG_SYS_CS_ALETIMING
	if (SVR_MJREV(in_be32(&im->sysconf.spridr)) >= 2)
		out_be32(&im->lpc.altr, CONFIG_SYS_CS_ALETIMING);
#endif
#if defined CONFIG_SYS_CS_BURST
	out_be32(&im->lpc.cs_bcr, CONFIG_SYS_CS_BURST);
#endif
#if defined CONFIG_SYS_CS_DEADCYCLE
	out_be32(&im->lpc.cs_dccr, CONFIG_SYS_CS_DEADCYCLE);
#endif
#if defined CONFIG_SYS_CS_HOLDCYCLE
	out_be32(&im->lpc.cs_hccr, CONFIG_SYS_CS_HOLDCYCLE);
#endif

	/* system performance tweaking */

#ifdef CONFIG_SYS_ACR_PIPE_DEP
	/* Arbiter pipeline depth */
	out_be32(&im->arbiter.acr,
		(im->arbiter.acr & ~ACR_PIPE_DEP) |
		(CONFIG_SYS_ACR_PIPE_DEP << ACR_PIPE_DEP_SHIFT)
	);
#endif

#ifdef CONFIG_SYS_ACR_RPTCNT
	/* Arbiter repeat count */
	out_be32(im->arbiter.acr,
		(im->arbiter.acr & ~(ACR_RPTCNT)) |
		(CONFIG_SYS_ACR_RPTCNT << ACR_RPTCNT_SHIFT)
	);
#endif

	/* RSR - Reset Status Register - clear all status */
	gd->arch.reset_status = im->reset.rsr;
	out_be32(&im->reset.rsr, ~RSR_RES);

	/*
	 * RMR - Reset Mode Register - enable checkstop reset
	 */
	out_be32(&im->reset.rmr, RMR_CSRE & (1 << RMR_CSRE_SHIFT));

	/* Set IPS-CSB divider: IPS = 1/2 CSB */
	ips_div = in_be32(&im->clk.scfr[0]);
	ips_div &= ~(SCFR1_IPS_DIV_MASK);
	ips_div |= SCFR1_IPS_DIV << SCFR1_IPS_DIV_SHIFT;
	out_be32(&im->clk.scfr[0], ips_div);

#ifdef SCFR1_LPC_DIV
	clrsetbits_be32(&im->clk.scfr[0], SCFR1_LPC_DIV_MASK,
			SCFR1_LPC_DIV << SCFR1_LPC_DIV_SHIFT);
#endif

#ifdef SCFR1_NFC_DIV
	clrsetbits_be32(&im->clk.scfr[0], SCFR1_NFC_DIV_MASK,
			SCFR1_NFC_DIV << SCFR1_NFC_DIV_SHIFT);
#endif

#ifdef SCFR1_DIU_DIV
	clrsetbits_be32(&im->clk.scfr[0], SCFR1_DIU_DIV_MASK,
			SCFR1_DIU_DIV << SCFR1_DIU_DIV_SHIFT);
#endif

	/*
	 * Enable Time Base/Decrementer
	 *
	 * NOTICE: TB needs to be enabled as early as possible in order to
	 * have udelay() working; if not enabled, usually leads to a hang, like
	 * during FLASH chip identification etc.
	 */
	setbits_be32(&im->sysconf.spcr, SPCR_TBEN);

	/*
	 * Enable clocks
	 */
	out_be32(&im->clk.sccr[0], SCCR1_CLOCKS_EN);
	out_be32(&im->clk.sccr[1], SCCR2_CLOCKS_EN);
#if defined(CONFIG_FSL_IIM) || defined(CONFIG_CMD_FUSE)
	setbits_be32(&im->clk.sccr[1], CLOCK_SCCR2_IIM_EN);
#endif
}

int cpu_init_r (void)
{
	return 0;
}
