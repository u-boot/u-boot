/*-----------------------------------------------------------------------------+
  |
  |       This source code has been made available to you by IBM on an AS-IS
  |       basis.  Anyone receiving this source is licensed under IBM
  |       copyrights to use it in any way he or she deems fit, including
  |       copying it, modifying it, compiling it, and redistributing it either
  |       with or without modifications.  No license under IBM patents or
  |       patent applications is to be implied by the copyright license.
  |
  |       Any user of this software should understand that IBM cannot provide
  |       technical support for this software and will not be responsible for
  |       any consequences resulting from the use of this software.
  |
  |       Any person who transfers this source code or any derivative work
  |       must include the IBM copyright notice, this paragraph, and the
  |       preceding two paragraphs in the transferred software.
  |
  |       COPYRIGHT   I B M   CORPORATION 1995
  |       LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
  +-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------+
  |
  |  File Name:  miiphy.c
  |
  |  Function:   This module has utilities for accessing the MII PHY through
  |	       the EMAC3 macro.
  |
  |  Author:     Mark Wisner
  |
  |  Change Activity-
  |
  |  Date        Description of Change                                       BY
  |  ---------   ---------------------                                       ---
  |  05-May-99   Created                                                     MKW
  |  01-Jul-99   Changed clock setting of sta_reg from 66Mhz to 50Mhz to
  |              better match OPB speed. Also modified delay times.      	   JWB
  |  29-Jul-99   Added Full duplex support                                   MKW
  |  24-Aug-99   Removed printf from dp83843_duplex()                      JWB
  |  19-Jul-00   Ported to esd cpci405                                       sr
  |
  +-----------------------------------------------------------------------------*/

#include <common.h>
#include <asm/processor.h>
#include <ppc_asm.tmpl>
#include <commproc.h>
#include <405gp_enet.h>
#include <405_mal.h>
#include <miiphy.h>

#if defined(CONFIG_405GP) || defined(CONFIG_405EP) || \
  (defined(CONFIG_440) && !defined(CONFIG_NET_MULTI))

/***********************************************************/
/* Dump out to the screen PHY regs                         */
/***********************************************************/

void miiphy_dump (unsigned char addr)
{
	unsigned long i;
	unsigned short data;


	for (i = 0; i < 0x1A; i++) {
		if (miiphy_read (addr, i, &data)) {
			printf ("read error for reg %lx\n", i);
			return;
		}
		printf ("Phy reg %lx ==> %4x\n", i, data);

		/* jump to the next set of regs */
		if (i == 0x07)
			i = 0x0f;

	} /* end for loop */
} /* end dump */


/***********************************************************/
/* read a phy reg and return the value with a rc           */
/* Note: We are referencing to EMAC_STACR register         */
/* @(EMAC_BASE + 92) because  of:                          */
/* - 405EP has only STACR for EMAC0 pinned out             */
/* - 405GP has onle one EMAC0                              */
/* - For 440 this module gets compiled only for            */
/*   !CONFIG_NET_MULTI, i.e. only EMAC0 is supported.      */
/***********************************************************/

int miiphy_read (unsigned char addr, unsigned char reg,
				 unsigned short *value)
{
	unsigned long sta_reg;		/* STA scratch area */
	unsigned long i;

	/* see if it is ready for 1000 nsec */
	i = 0;

	/* see if it is ready for  sec */
	while ((in32 (EMAC_STACR) & EMAC_STACR_OC) == 0) {
		udelay (7);
		if (i > 5) {
#if 0	/* test-only */
			printf ("read err 1\n");
#endif
			return -1;
		}
		i++;
	}
	sta_reg = reg;				/* reg address */
	/* set clock (50Mhz) and read flags */
	sta_reg = (sta_reg | EMAC_STACR_READ) & ~EMAC_STACR_CLK_100MHZ;
#ifdef CONFIG_PHY_CLK_FREQ
	sta_reg = sta_reg | CONFIG_PHY_CLK_FREQ;
#endif
	sta_reg = sta_reg | (addr << 5);	/* Phy address */

	out32 (EMAC_STACR, sta_reg);
#if 0	/* test-only */
	printf ("a2: write: EMAC_STACR=0x%0x\n", sta_reg);	/* test-only */
#endif

#ifdef CONFIG_PHY_CMD_DELAY
	udelay (CONFIG_PHY_CMD_DELAY);		/* Intel LXT971A needs this */
#endif
	sta_reg = in32 (EMAC_STACR);
	i = 0;
	while ((sta_reg & EMAC_STACR_OC) == 0) {
		udelay (7);
		if (i > 5) {
#if 0	/* test-only */
			printf ("read err 2\n");
#endif
			return -1;
		}
		i++;
		sta_reg = in32 (EMAC_STACR);
	}
	if ((sta_reg & EMAC_STACR_PHYE) != 0) {
#if 0	/* test-only */
		printf ("read err 3\n");
		printf ("a2: read: EMAC_STACR=0x%0lx, i=%d\n",
			sta_reg, (int) i);	/* test-only */
#endif
		return -1;
	}

	*value = *(short *) (&sta_reg);
	return 0;


} /* phy_read */


/***********************************************************/
/* write a phy reg and return the value with a rc           */
/***********************************************************/

int miiphy_write (unsigned char addr, unsigned char reg,
		  unsigned short value)
{
	unsigned long sta_reg;		/* STA scratch area */
	unsigned long i;

	/* see if it is ready for 1000 nsec */
	i = 0;

	while ((in32 (EMAC_STACR) & EMAC_STACR_OC) == 0) {
		if (i > 5)
			return -1;
		udelay (7);
		i++;
	}
	sta_reg = 0;
	sta_reg = reg;				/* reg address */
	/* set clock (50Mhz) and read flags */
	sta_reg = (sta_reg | EMAC_STACR_WRITE) & ~EMAC_STACR_CLK_100MHZ;
#ifdef CONFIG_PHY_CLK_FREQ
	sta_reg = sta_reg | CONFIG_PHY_CLK_FREQ; /* Set clock frequency (PLB freq. dependend) */
#endif
	sta_reg = sta_reg | ((unsigned long) addr << 5);	/* Phy address */
	memcpy (&sta_reg, &value, 2);	/* put in data */

	out32 (EMAC_STACR, sta_reg);

#ifdef CONFIG_PHY_CMD_DELAY
	udelay (CONFIG_PHY_CMD_DELAY);		/* Intel LXT971A needs this */
#endif
	/* wait for completion */
	i = 0;
	sta_reg = in32 (EMAC_STACR);
	while ((sta_reg & EMAC_STACR_OC) == 0) {
		udelay (7);
		if (i > 5)
			return -1;
		i++;
		sta_reg = in32 (EMAC_STACR);
	}

	if ((sta_reg & EMAC_STACR_PHYE) != 0)
		return -1;
	return 0;

} /* phy_read */

#endif	/* CONFIG_405GP */
