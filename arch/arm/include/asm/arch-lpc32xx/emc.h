/*
 * Copyright (C) 2011 by Vladimir Zapolskiy <vz@mleia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#ifndef _LPC32XX_EMC_H
#define _LPC32XX_EMC_H

#include <asm/types.h>

/* EMC Registers */
struct emc_regs {
	u32 ctrl;		/* Controls operation of the EMC             */
	u32 status;		/* Provides EMC status information           */
	u32 config;		/* Configures operation of the EMC           */
	u32 reserved0[5];
	u32 control;		/* Controls dyn memory operation             */
	u32 refresh;		/* Configures dyn memory refresh operation   */
	u32 read_config;	/* Configures the dyn memory read strategy   */
	u32 reserved1;
	u32 t_rp;		/* Precharge command period                  */
	u32 t_ras;		/* Active to precharge command period        */
	u32 t_srex;		/* Self-refresh exit time                    */
	u32 reserved2[2];
	u32 t_wr;		/* Write recovery time                       */
	u32 t_rc;		/* Active to active command period           */
	u32 t_rfc;		/* Auto-refresh period                       */
	u32 t_xsr;		/* Exit self-refresh to active command time  */
	u32 t_rrd;		/* Active bank A to active bank B latency    */
	u32 t_mrd;		/* Load mode register to active command time */
	u32 t_cdlr;		/* Last data in to read command time         */
	u32 reserved3[8];
	u32 extended_wait;	/* time for static memory rd/wr transfers    */
	u32 reserved4[31];
	u32 config0;		/* Configuration information for the SDRAM   */
	u32 rascas0;		/* RAS and CAS latencies for the SDRAM       */
	u32 reserved5[6];
	u32 config1;		/* Configuration information for the SDRAM   */
	u32 rascas1;		/* RAS and CAS latencies for the SDRAM       */
	u32 reserved6[54];
	struct emc_stat_t {
		u32 config;	/* Static memory configuration               */
		u32 waitwen;	/* Delay from chip select to write enable    */
		u32 waitoen;	/* Delay to output enable                    */
		u32 waitrd;	/* Delay to a read access                    */
		u32 waitpage;	/* Delay for async page mode read            */
		u32 waitwr;	/* Delay to a write access                   */
		u32 waitturn;	/* Number of bus turnaround cycles           */
		u32 reserved;
	} stat[4];
	u32 reserved7[96];
	struct emc_ahb_t {
		u32 control;	/* Control register for AHB                  */
		u32 status;	/* Status register for AHB                   */
		u32 timeout;	/* Timeout register for AHB                  */
		u32 reserved[5];
	} ahb[5];
};

/* Static Memory Configuration Register bits */
#define EMC_STAT_CONFIG_WP		(1 << 20)
#define EMC_STAT_CONFIG_EW		(1 << 8)
#define EMC_STAT_CONFIG_PB		(1 << 7)
#define EMC_STAT_CONFIG_PC		(1 << 6)
#define EMC_STAT_CONFIG_PM		(1 << 3)
#define EMC_STAT_CONFIG_32BIT		(2 << 0)
#define EMC_STAT_CONFIG_16BIT		(1 << 0)
#define EMC_STAT_CONFIG_8BIT		(0 << 0)

/* Static Memory Delay Registers */
#define EMC_STAT_WAITWEN(n)		(((n) - 1) & 0x0F)
#define EMC_STAT_WAITOEN(n)		(((n) - 1) & 0x0F)
#define EMC_STAT_WAITRD(n)		(((n) - 1) & 0x1F)
#define EMC_STAT_WAITPAGE(n)		(((n) - 1) & 0x1F)
#define EMC_STAT_WAITWR(n)		(((n) - 2) & 0x1F)
#define EMC_STAT_WAITTURN(n)		(((n) - 1) & 0x0F)

#endif /* _LPC32XX_EMC_H */
