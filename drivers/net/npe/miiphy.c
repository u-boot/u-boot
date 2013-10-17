/*
 * SPDX-License-Identifier:	GPL-2.0	IBM-pibs
 */
/*-----------------------------------------------------------------------------+
  |
  |  File Name:	 miiphy.c
  |
  |  Function:	 This module has utilities for accessing the MII PHY through
  |	       the EMAC3 macro.
  |
  |  Author:	 Mark Wisner
  |
  |  Change Activity-
  |
  |  Date	 Description of Change					     BY
  |  ---------	 ---------------------					     ---
  |  05-May-99	 Created						     MKW
  |  01-Jul-99	 Changed clock setting of sta_reg from 66MHz to 50MHz to
  |		 better match OPB speed. Also modified delay times.	     JWB
  |  29-Jul-99	 Added Full duplex support				     MKW
  |  24-Aug-99	 Removed printf from dp83843_duplex()			     JWB
  |  19-Jul-00	 Ported to esd cpci405					     sr
  |  23-Dec-03	 Ported from miiphy.c to 440GX Travis Sawyer		     TBS
  |		 <travis.sawyer@sandburst.com>
  |
  +-----------------------------------------------------------------------------*/

#include <common.h>
#include <miiphy.h>
#include "IxOsal.h"
#include "IxEthAcc.h"
#include "IxEthAcc_p.h"
#include "IxEthAccMac_p.h"
#include "IxEthAccMii_p.h"

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
	unsigned short ctl, adv;

	/* Setup standard advertise */
	miiphy_read (devname, addr, MII_ADVERTISE, &adv);
	adv |= (LPA_LPACK | LPA_RFAULT | LPA_100BASE4 |
		LPA_100FULL | LPA_100HALF | LPA_10FULL |
		LPA_10HALF);
	miiphy_write (devname, addr, MII_ADVERTISE, adv);

	/* Start/Restart aneg */
	miiphy_read (devname, addr, MII_BMCR, &ctl);
	ctl |= (BMCR_ANENABLE | BMCR_ANRESTART);
	miiphy_write (devname, addr, MII_BMCR, ctl);

	return 0;
}


int npe_miiphy_read (const char *devname, unsigned char addr,
		     unsigned char reg, unsigned short *value)
{
	u16 val;

	ixEthAccMiiReadRtn(addr, reg, &val);
	*value = val;

	return 0;
}				/* phy_read */


int npe_miiphy_write (const char *devname, unsigned char addr,
		      unsigned char reg, unsigned short value)
{
	ixEthAccMiiWriteRtn(addr, reg, value);
	return 0;
}				/* phy_write */
