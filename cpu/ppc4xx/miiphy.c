/*-----------------------------------------------------------------------------+
  |   This source code is dual-licensed.  You may use it under the terms of the
  |   GNU General Public License version 2, or under the license below.
  |
  |	  This source code has been made available to you by IBM on an AS-IS
  |	  basis.  Anyone receiving this source is licensed under IBM
  |	  copyrights to use it in any way he or she deems fit, including
  |	  copying it, modifying it, compiling it, and redistributing it either
  |	  with or without modifications.  No license under IBM patents or
  |	  patent applications is to be implied by the copyright license.
  |
  |	  Any user of this software should understand that IBM cannot provide
  |	  technical support for this software and will not be responsible for
  |	  any consequences resulting from the use of this software.
  |
  |	  Any person who transfers this source code or any derivative work
  |	  must include the IBM copyright notice, this paragraph, and the
  |	  preceding two paragraphs in the transferred software.
  |
  |	  COPYRIGHT   I B M   CORPORATION 1995
  |	  LICENSED MATERIAL  -	PROGRAM PROPERTY OF I B M
  +-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------+
  |
  |  File Name:	 miiphy.c
  |
  |  Function:	 This module has utilities for accessing the MII PHY through
  |	       the EMAC3 macro.
  |
  |  Author:	 Mark Wisner
  |
  +-----------------------------------------------------------------------------*/

/* define DEBUG for debugging output (obviously ;-)) */
#if 0
#define DEBUG
#endif

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <ppc_asm.tmpl>
#include <commproc.h>
#include <ppc4xx_enet.h>
#include <405_mal.h>
#include <miiphy.h>

#if !defined(CONFIG_PHY_CLK_FREQ)
#define CONFIG_PHY_CLK_FREQ	0
#endif

/***********************************************************/
/* Dump out to the screen PHY regs			   */
/***********************************************************/

void miiphy_dump (char *devname, unsigned char addr)
{
	unsigned long i;
	unsigned short data;

	for (i = 0; i < 0x1A; i++) {
		if (miiphy_read (devname, addr, i, &data)) {
			printf ("read error for reg %lx\n", i);
			return;
		}
		printf ("Phy reg %lx ==> %4x\n", i, data);

		/* jump to the next set of regs */
		if (i == 0x07)
			i = 0x0f;

	}			/* end for loop */
}				/* end dump */

/***********************************************************/
/* (Re)start autonegotiation				   */
/***********************************************************/
int phy_setup_aneg (char *devname, unsigned char addr)
{
	u16 bmcr;

#if defined(CONFIG_PHY_DYNAMIC_ANEG)
	/*
	 * Set up advertisement based on capablilities reported by the PHY.
	 * This should work for both copper and fiber.
	 */
	u16 bmsr;
#if defined(CONFIG_PHY_GIGE)
	u16 exsr = 0x0000;
#endif

	miiphy_read (devname, addr, PHY_BMSR, &bmsr);

#if defined(CONFIG_PHY_GIGE)
	if (bmsr & PHY_BMSR_EXT_STAT)
		miiphy_read (devname, addr, PHY_EXSR, &exsr);

	if (exsr & (PHY_EXSR_1000XF | PHY_EXSR_1000XH)) {
		/* 1000BASE-X */
		u16 anar = 0x0000;

		if (exsr & PHY_EXSR_1000XF)
			anar |= PHY_X_ANLPAR_FD;

		if (exsr & PHY_EXSR_1000XH)
			anar |= PHY_X_ANLPAR_HD;

		miiphy_write (devname, addr, PHY_ANAR, anar);
	} else
#endif
	{
		u16 anar, btcr;

		miiphy_read (devname, addr, PHY_ANAR, &anar);
		anar &= ~(0x5000 | PHY_ANLPAR_T4 | PHY_ANLPAR_TXFD |
			  PHY_ANLPAR_TX | PHY_ANLPAR_10FD | PHY_ANLPAR_10);

		miiphy_read (devname, addr, PHY_1000BTCR, &btcr);
		btcr &= ~(0x00FF | PHY_1000BTCR_1000FD | PHY_1000BTCR_1000HD);

		if (bmsr & PHY_BMSR_100T4)
			anar |= PHY_ANLPAR_T4;

		if (bmsr & PHY_BMSR_100TXF)
			anar |= PHY_ANLPAR_TXFD;

		if (bmsr & PHY_BMSR_100TXH)
			anar |= PHY_ANLPAR_TX;

		if (bmsr & PHY_BMSR_10TF)
			anar |= PHY_ANLPAR_10FD;

		if (bmsr & PHY_BMSR_10TH)
			anar |= PHY_ANLPAR_10;

		miiphy_write (devname, addr, PHY_ANAR, anar);

#if defined(CONFIG_PHY_GIGE)
		if (exsr & PHY_EXSR_1000TF)
			btcr |= PHY_1000BTCR_1000FD;

		if (exsr & PHY_EXSR_1000TH)
			btcr |= PHY_1000BTCR_1000HD;

		miiphy_write (devname, addr, PHY_1000BTCR, btcr);
#endif
	}

#else /* defined(CONFIG_PHY_DYNAMIC_ANEG) */
	/*
	 * Set up standard advertisement
	 */
	u16 adv;

	miiphy_read (devname, addr, PHY_ANAR, &adv);
	adv |= (PHY_ANLPAR_ACK  | PHY_ANLPAR_TXFD | PHY_ANLPAR_TX |
		PHY_ANLPAR_10FD | PHY_ANLPAR_10);
	miiphy_write (devname, addr, PHY_ANAR, adv);

	miiphy_read (devname, addr, PHY_1000BTCR, &adv);
	adv |= (0x0300);
	miiphy_write (devname, addr, PHY_1000BTCR, adv);

#endif /* defined(CONFIG_PHY_DYNAMIC_ANEG) */

	/* Start/Restart aneg */
	miiphy_read (devname, addr, PHY_BMCR, &bmcr);
	bmcr |= (PHY_BMCR_AUTON | PHY_BMCR_RST_NEG);
	miiphy_write (devname, addr, PHY_BMCR, bmcr);

	return 0;
}

/***********************************************************/
/* read a phy reg and return the value with a rc	   */
/***********************************************************/
/* AMCC_TODO:
 * Find out of the choice for the emac for MDIO is from the bridges,
 * i.e. ZMII or RGMII as approporiate.  If the bridges are not used
 * to determine the emac for MDIO, then is the SDR0_ETH_CFG[MDIO_SEL]
 * used?  If so, then this routine below does not apply to the 460EX/GT.
 *
 * sr: Currently on 460EX only EMAC0 works with MDIO, so we always
 * return EMAC0 offset here
 * vg: For 460EX/460GT if internal GPCS PHY address is specified
 * return appropriate EMAC offset
 */
unsigned int miiphy_getemac_offset(u8 addr)
{
#if (defined(CONFIG_440) && \
    !defined(CONFIG_440SP) && !defined(CONFIG_440SPE) && \
    !defined(CONFIG_460EX) && !defined(CONFIG_460GT)) && \
    defined(CONFIG_NET_MULTI)
	unsigned long zmii;
	unsigned long eoffset;

	/* Need to find out which mdi port we're using */
	zmii = in_be32((void *)ZMII_FER);

	if (zmii & (ZMII_FER_MDI << ZMII_FER_V (0)))
		/* using port 0 */
		eoffset = 0;

	else if (zmii & (ZMII_FER_MDI << ZMII_FER_V (1)))
		/* using port 1 */
		eoffset = 0x100;

	else if (zmii & (ZMII_FER_MDI << ZMII_FER_V (2)))
		/* using port 2 */
		eoffset = 0x400;

	else if (zmii & (ZMII_FER_MDI << ZMII_FER_V (3)))
		/* using port 3 */
		eoffset = 0x600;

	else {
		/* None of the mdi ports are enabled! */
		/* enable port 0 */
		zmii |= ZMII_FER_MDI << ZMII_FER_V (0);
		out_be32((void *)ZMII_FER, zmii);
		eoffset = 0;
		/* need to soft reset port 0 */
		zmii = in_be32((void *)EMAC_M0);
		zmii |= EMAC_M0_SRST;
		out_be32((void *)EMAC_M0, zmii);
	}

	return (eoffset);
#else

#if defined(CONFIG_NET_MULTI) && defined(CONFIG_405EX)
	unsigned long rgmii;
	int devnum = 1;

	rgmii = in_be32((void *)RGMII_FER);
	if (rgmii & (1 << (19 - devnum)))
		return 0x100;
#endif

#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
	u32 eoffset = 0;

	switch (addr) {
#if defined(CONFIG_HAS_ETH1) && defined(CONFIG_GPCS_PHY1_ADDR)
	case CONFIG_GPCS_PHY1_ADDR:
		if (addr == EMAC_M1_IPPA_GET(in_be32((void *)EMAC_M1 + 0x100)))
			eoffset = 0x100;
		break;
#endif
#if defined(CONFIG_HAS_ETH2) && defined(CONFIG_GPCS_PHY2_ADDR)
	case CONFIG_GPCS_PHY2_ADDR:
		if (addr == EMAC_M1_IPPA_GET(in_be32((void *)EMAC_M1 + 0x300)))
			eoffset = 0x300;
		break;
#endif
#if defined(CONFIG_HAS_ETH3) && defined(CONFIG_GPCS_PHY3_ADDR)
	case CONFIG_GPCS_PHY3_ADDR:
		if (addr == EMAC_M1_IPPA_GET(in_be32((void *)EMAC_M1 + 0x400)))
			eoffset = 0x400;
		break;
#endif
	default:
		eoffset = 0;
		break;
	}
	return eoffset;
#endif

	return 0;
#endif
}

static int emac_miiphy_wait(u32 emac_reg)
{
	u32 sta_reg;
	int i;

	/* wait for completion */
	i = 0;
	do {
		sta_reg = in_be32((void *)EMAC_STACR + emac_reg);
		if (i++ > 5) {
			debug("%s [%d]: Timeout! EMAC_STACR=0x%0x\n", __func__,
			      __LINE__, sta_reg);
			return -1;
		}
		udelay(10);
	} while ((sta_reg & EMAC_STACR_OC) == EMAC_STACR_OC_MASK);

	return 0;
}

static int emac_miiphy_command(u8 addr, u8 reg, int cmd, u16 value)
{
	u32 emac_reg;
	u32 sta_reg;

	emac_reg = miiphy_getemac_offset(addr);

	/* wait for completion */
	if (emac_miiphy_wait(emac_reg) != 0)
		return -1;

	sta_reg = reg;		/* reg address */

	/* set clock (50MHz) and read flags */
#if defined(CONFIG_440GX) || defined(CONFIG_440SPE) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT) || \
    defined(CONFIG_405EX)
#if defined(CONFIG_IBM_EMAC4_V4)	/* EMAC4 V4 changed bit setting */
	sta_reg = (sta_reg & ~EMAC_STACR_OP_MASK) | cmd;
#else
	sta_reg |= cmd;
#endif
#else
	sta_reg = (sta_reg | cmd) & ~EMAC_STACR_CLK_100MHZ;
#endif

	/* Some boards (mainly 405EP based) define the PHY clock freqency fixed */
	sta_reg = sta_reg | CONFIG_PHY_CLK_FREQ;
	sta_reg = sta_reg | ((u32)addr << 5);	/* Phy address */
	sta_reg = sta_reg | EMAC_STACR_OC_MASK;	/* new IBM emac v4 */
	if (cmd == EMAC_STACR_WRITE)
		memcpy(&sta_reg, &value, 2);	/* put in data */

	out_be32((void *)EMAC_STACR + emac_reg, sta_reg);
	debug("%s [%d]: sta_reg=%08x\n", __func__, __LINE__, sta_reg);

	/* wait for completion */
	if (emac_miiphy_wait(emac_reg) != 0)
		return -1;

	debug("%s [%d]: sta_reg=%08x\n", __func__, __LINE__, sta_reg);
	if ((sta_reg & EMAC_STACR_PHYE) != 0)
		return -1;

	return 0;
}

int emac4xx_miiphy_read (char *devname, unsigned char addr, unsigned char reg,
			 unsigned short *value)
{
	unsigned long sta_reg;
	unsigned long emac_reg;

	emac_reg = miiphy_getemac_offset(addr);

	if (emac_miiphy_command(addr, reg, EMAC_STACR_READ, 0) != 0)
		return -1;

	sta_reg = in_be32((void *)EMAC_STACR + emac_reg);
	*value = *(u16 *)(&sta_reg);

	return 0;
}

/***********************************************************/
/* write a phy reg and return the value with a rc	    */
/***********************************************************/

int emac4xx_miiphy_write (char *devname, unsigned char addr, unsigned char reg,
			  unsigned short value)
{
	return emac_miiphy_command(addr, reg, EMAC_STACR_WRITE, value);
}
