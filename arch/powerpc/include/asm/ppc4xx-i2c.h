/*
 * (C) Copyright 2007-2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _4xx_i2c_h_
#define _4xx_i2c_h_

#define IIC_OK		0
#define IIC_NOK		1
#define IIC_NOK_LA	2		/* Lost arbitration */
#define IIC_NOK_ICT	3		/* Incomplete transfer */
#define IIC_NOK_XFRA	4		/* Transfer aborted */
#define IIC_NOK_DATA	5		/* No data in buffer */
#define IIC_NOK_TOUT	6		/* Transfer timeout */

#define IIC_TIMEOUT	1		/* 1 second */

struct ppc4xx_i2c {
	u8 mdbuf;
	u8 res1;
	u8 sdbuf;
	u8 res2;
	u8 lmadr;
	u8 hmadr;
	u8 cntl;
	u8 mdcntl;
	u8 sts;
	u8 extsts;
	u8 lsadr;
	u8 hsadr;
	u8 clkdiv;
	u8 intrmsk;
	u8 xfrcnt;
	u8 xtcntlss;
	u8 directcntl;
	u8 intr;
};

/* MDCNTL Register Bit definition */
#define IIC_MDCNTL_HSCL		0x01
#define IIC_MDCNTL_EUBS		0x02
#define IIC_MDCNTL_EINT		0x04
#define IIC_MDCNTL_ESM		0x08
#define IIC_MDCNTL_FSM		0x10
#define IIC_MDCNTL_EGC		0x20
#define IIC_MDCNTL_FMDB		0x40
#define IIC_MDCNTL_FSDB		0x80

/* CNTL Register Bit definition */
#define IIC_CNTL_PT		0x01
#define IIC_CNTL_READ		0x02
#define IIC_CNTL_CHT		0x04
#define IIC_CNTL_RPST		0x08
/* bit 2/3 for Transfer count*/
#define IIC_CNTL_AMD		0x40
#define IIC_CNTL_HMT		0x80

/* STS Register Bit definition */
#define IIC_STS_PT		0x01
#define IIC_STS_IRQA		0x02
#define IIC_STS_ERR		0x04
#define IIC_STS_SCMP		0x08
#define IIC_STS_MDBF		0x10
#define IIC_STS_MDBS		0x20
#define IIC_STS_SLPR		0x40
#define IIC_STS_SSS		0x80

/* EXTSTS Register Bit definition */
#define IIC_EXTSTS_XFRA		0x01
#define IIC_EXTSTS_ICT		0x02
#define IIC_EXTSTS_LA		0x04
#define IIC_EXTSTS_BCS_MASK	0x70
#define IIC_EXTSTS_BCS_FREE	0x40

/* XTCNTLSS Register Bit definition */
#define IIC_XTCNTLSS_SRST	0x01
#define IIC_XTCNTLSS_EPI	0x02
#define IIC_XTCNTLSS_SDBF	0x04
#define IIC_XTCNTLSS_SBDD	0x08
#define IIC_XTCNTLSS_SWS	0x10
#define IIC_XTCNTLSS_SWC	0x20
#define IIC_XTCNTLSS_SRS	0x40
#define IIC_XTCNTLSS_SRC	0x80

/* IICx_DIRECTCNTL register */
#define IIC_DIRCNTL_SDAC	0x08
#define IIC_DIRCNTL_SCC		0x04
#define IIC_DIRCNTL_MSDA	0x02
#define IIC_DIRCNTL_MSC		0x01

#define DIRCTNL_FREE(v)		(((v) & 0x0f) == 0x0f)
#endif
