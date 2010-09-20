/*
 * (C) Copyright 2010
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
 */

#ifndef _PPC440GP_H_
#define _PPC440GP_H_

#define CONFIG_SDRAM_PPC4xx_IBM_DDR	/* IBM DDR controller */

/*
 * Some SoC specific registers (not common for all 440 SoC's)
 */

/* Memory mapped register */
#define CONFIG_SYS_PERIPHERAL_BASE	0xe0000000 /* Internal Peripherals */

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_PERIPHERAL_BASE + 0x0200)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_PERIPHERAL_BASE + 0x0300)

#define GPIO0_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x0700)

#define SDR0_PCI0	0x0300

#define CPC0_STRP1_PAE_MASK		(0x80000000 >> 11)
#define CPC0_STRP1_PISE_MASK		(0x80000000 >> 13)

#define CNTRL_DCR_BASE	0x0b0

#define CPC0_SYS0	(CNTRL_DCR_BASE + 0x30)	/* System configuration reg 0 */
#define CPC0_SYS1	(CNTRL_DCR_BASE + 0x31)	/* System configuration reg 1 */

#define CPC0_STRP0	(CNTRL_DCR_BASE + 0x34)	/* Power-on config reg 0 (RO) */
#define CPC0_STRP1	(CNTRL_DCR_BASE + 0x35)	/* Power-on config reg 1 (RO) */

#define CPC0_GPIO	(CNTRL_DCR_BASE + 0x38)	/* GPIO config reg (440GP) */

#define CPC0_CR0	(CNTRL_DCR_BASE + 0x3b)	/* Control 0 register */
#define CPC0_CR1	(CNTRL_DCR_BASE + 0x3a)	/* Control 1 register */

#define PLLSYS0_TUNE_MASK	0xffc00000	/* PLL TUNE bits	    */
#define PLLSYS0_FB_DIV_MASK	0x003c0000	/* Feedback divisor	    */
#define PLLSYS0_FWD_DIV_A_MASK	0x00038000	/* Forward divisor A	    */
#define PLLSYS0_FWD_DIV_B_MASK	0x00007000	/* Forward divisor B	    */
#define PLLSYS0_OPB_DIV_MASK	0x00000c00	/* OPB divisor		    */
#define PLLSYS0_EPB_DIV_MASK	0x00000300	/* EPB divisor		    */
#define PLLSYS0_EXTSL_MASK	0x00000080	/* PerClk feedback path	    */
#define PLLSYS0_RW_MASK		0x00000060	/* ROM width		    */
#define PLLSYS0_RL_MASK		0x00000010	/* ROM location		    */
#define PLLSYS0_ZMII_SEL_MASK	0x0000000c	/* ZMII selection	    */
#define PLLSYS0_BYPASS_MASK	0x00000002	/* Bypass PLL		    */
#define PLLSYS0_NTO1_MASK	0x00000001	/* CPU:PLB N-to-1 ratio	    */

#define PCIL0_BRDGOPT1		(PCIL0_CFGBASE + 0x0040)
#define PCIL0_BRDGOPT2		(PCIL0_CFGBASE + 0x0044)

#endif /* _PPC440GP_H_ */
