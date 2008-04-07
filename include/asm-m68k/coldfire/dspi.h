/*
 * MCF5227x Internal Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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

#ifndef __DSPI_H__
#define __DSPI_H__

/*********************************************************************
* DMA Serial Peripheral Interface (DSPI)
*********************************************************************/

typedef struct dspi {
	u32 dmcr;
	u8 resv0[0x4];
	u32 dtcr;
	u32 dctar0;
	u32 dctar1;
	u32 dctar2;
	u32 dctar3;
	u32 dctar4;
	u32 dctar5;
	u32 dctar6;
	u32 dctar7;
	u32 dsr;
	u32 dirsr;
	u32 dtfr;
	u32 drfr;
	u32 dtfdr0;
	u32 dtfdr1;
	u32 dtfdr2;
	u32 dtfdr3;
	u8 resv1[0x30];
	u32 drfdr0;
	u32 drfdr1;
	u32 drfdr2;
	u32 drfdr3;
} dspi_t;

/* Bit definitions and macros for DMCR */
#define DSPI_DMCR_HALT			(0x00000001)
#define DSPI_DMCR_SMPL_PT(x)		(((x)&0x00000003)<<8)
#define DSPI_DMCR_CRXF			(0x00000400)
#define DSPI_DMCR_CTXF			(0x00000800)
#define DSPI_DMCR_DRXF			(0x00001000)
#define DSPI_DMCR_DTXF			(0x00002000)
#define DSPI_DMCR_MDIS			(0x00004000)
#define DSPI_DMCR_CSIS0			(0x00010000)
#define DSPI_DMCR_CSIS1			(0x00020000)
#define DSPI_DMCR_CSIS2			(0x00040000)
#define DSPI_DMCR_CSIS3			(0x00080000)
#define DSPI_DMCR_CSIS4			(0x00100000)
#define DSPI_DMCR_CSIS5			(0x00200000)
#define DSPI_DMCR_CSIS6			(0x00400000)
#define DSPI_DMCR_CSIS7			(0x00800000)
#define DSPI_DMCR_ROOE			(0x01000000)
#define DSPI_DMCR_PCSSE			(0x02000000)
#define DSPI_DMCR_MTFE			(0x04000000)
#define DSPI_DMCR_FRZ			(0x08000000)
#define DSPI_DMCR_DCONF(x)		(((x)&0x00000003)<<28)
#define DSPI_DMCR_CSCK			(0x40000000)
#define DSPI_DMCR_MSTR			(0x80000000)

/* Bit definitions and macros for DTCR */
#define DSPI_DTCR_SPI_TCNT(x)		(((x)&0x0000FFFF)<<16)

/* Bit definitions and macros for DCTAR group */
#define DSPI_DCTAR_BR(x)		(((x)&0x0000000F))
#define DSPI_DCTAR_DT(x)		(((x)&0x0000000F)<<4)
#define DSPI_DCTAR_ASC(x)		(((x)&0x0000000F)<<8)
#define DSPI_DCTAR_CSSCK(x)		(((x)&0x0000000F)<<12)
#define DSPI_DCTAR_PBR(x)		(((x)&0x00000003)<<16)
#define DSPI_DCTAR_PDT(x)		(((x)&0x00000003)<<18)
#define DSPI_DCTAR_PASC(x)		(((x)&0x00000003)<<20)
#define DSPI_DCTAR_PCSSCK(x)		(((x)&0x00000003)<<22)
#define DSPI_DCTAR_LSBFE		(0x01000000)
#define DSPI_DCTAR_CPHA			(0x02000000)
#define DSPI_DCTAR_CPOL			(0x04000000)
#define DSPI_DCTAR_TRSZ(x)		(((x)&0x0000000F)<<27)
#define DSPI_DCTAR_DBR			(0x80000000)
#define DSPI_DCTAR_PCSSCK_1CLK		(0x00000000)
#define DSPI_DCTAR_PCSSCK_3CLK		(0x00400000)
#define DSPI_DCTAR_PCSSCK_5CLK		(0x00800000)
#define DSPI_DCTAR_PCSSCK_7CLK		(0x00A00000)
#define DSPI_DCTAR_PASC_1CLK		(0x00000000)
#define DSPI_DCTAR_PASC_3CLK		(0x00100000)
#define DSPI_DCTAR_PASC_5CLK		(0x00200000)
#define DSPI_DCTAR_PASC_7CLK		(0x00300000)
#define DSPI_DCTAR_PDT_1CLK		(0x00000000)
#define DSPI_DCTAR_PDT_3CLK		(0x00040000)
#define DSPI_DCTAR_PDT_5CLK		(0x00080000)
#define DSPI_DCTAR_PDT_7CLK		(0x000A0000)
#define DSPI_DCTAR_PBR_1CLK		(0x00000000)
#define DSPI_DCTAR_PBR_3CLK		(0x00010000)
#define DSPI_DCTAR_PBR_5CLK		(0x00020000)
#define DSPI_DCTAR_PBR_7CLK		(0x00030000)

/* Bit definitions and macros for DSR */
#define DSPI_DSR_RXPTR(x)		(((x)&0x0000000F))
#define DSPI_DSR_RXCTR(x)		(((x)&0x0000000F)<<4)
#define DSPI_DSR_TXPTR(x)		(((x)&0x0000000F)<<8)
#define DSPI_DSR_TXCTR(x)		(((x)&0x0000000F)<<12)
#define DSPI_DSR_RFDF			(0x00020000)
#define DSPI_DSR_RFOF			(0x00080000)
#define DSPI_DSR_TFFF			(0x02000000)
#define DSPI_DSR_TFUF			(0x08000000)
#define DSPI_DSR_EOQF			(0x10000000)
#define DSPI_DSR_TXRXS			(0x40000000)
#define DSPI_DSR_TCF			(0x80000000)

/* Bit definitions and macros for DIRSR */
#define DSPI_DIRSR_RFDFS		(0x00010000)
#define DSPI_DIRSR_RFDFE		(0x00020000)
#define DSPI_DIRSR_RFOFE		(0x00080000)
#define DSPI_DIRSR_TFFFS		(0x01000000)
#define DSPI_DIRSR_TFFFE		(0x02000000)
#define DSPI_DIRSR_TFUFE		(0x08000000)
#define DSPI_DIRSR_EOQFE		(0x10000000)
#define DSPI_DIRSR_TCFE			(0x80000000)

/* Bit definitions and macros for DTFR */
#define DSPI_DTFR_TXDATA(x)		(((x)&0x0000FFFF))
#define DSPI_DTFR_CS0			(0x00010000)
#define DSPI_DTFR_CS2			(0x00040000)
#define DSPI_DTFR_CS3			(0x00080000)
#define DSPI_DTFR_CS5			(0x00200000)
#define DSPI_DTFR_CTCNT			(0x04000000)
#define DSPI_DTFR_EOQ			(0x08000000)
#define DSPI_DTFR_CTAS(x)		(((x)&0x00000007)<<28)
#define DSPI_DTFR_CONT			(0x80000000)

/* Bit definitions and macros for DRFR */
#define DSPI_DRFR_RXDATA(x)		(((x)&0x0000FFFF))

/* Bit definitions and macros for DTFDR group */
#define DSPI_DTFDR_TXDATA(x)		(((x)&0x0000FFFF))
#define DSPI_DTFDR_TXCMD(x)		(((x)&0x0000FFFF)<<16)

/* Bit definitions and macros for DRFDR group */
#define DSPI_DRFDR_RXDATA(x)		(((x)&0x0000FFFF))

void dspi_init(void);
void dspi_tx(int chipsel, u8 attrib, u16 data);
u16 dspi_rx(void);

#endif				/* __DSPI_H__ */
