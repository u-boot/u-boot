/*
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

#include <common.h>
#include <asm/fec.h>
#include <asm/immap.h>

#include <config.h>
#include <net.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CMD_NET) && defined(CONFIG_NET_MULTI)
#undef MII_DEBUG
#undef ET_DEBUG

int fecpin_setclear(struct eth_device *dev, int setclear)
{
	if (setclear) {
		/* Enable Ethernet pins */
		mbar_writeByte(MCF_GPIO_PAR_FECI2C, CFG_FECI2C);
	} else {
	}

	return 0;
}

#if defined(CFG_DISCOVER_PHY) || defined(CONFIG_CMD_MII)
#include <miiphy.h>

/* Make MII read/write commands for the FEC. */
#define mk_mii_read(ADDR, REG)	(0x60020000 | ((ADDR << 23) | (REG & 0x1f) << 18))

#define mk_mii_write(ADDR, REG, VAL)	(0x50020000 | ((ADDR << 23) | (REG & 0x1f) << 18) | (VAL & 0xffff))

/* PHY identification */
#define PHY_ID_LXT970		0x78100000	/* LXT970 */
#define PHY_ID_LXT971		0x001378e0	/* LXT971 and 972 */
#define PHY_ID_82555		0x02a80150	/* Intel 82555 */
#define PHY_ID_QS6612		0x01814400	/* QS6612 */
#define PHY_ID_AMD79C784	0x00225610	/* AMD 79C784 */
#define PHY_ID_LSI80225		0x0016f870	/* LSI 80225 */
#define PHY_ID_LSI80225B	0x0016f880	/* LSI 80225/B */
#define PHY_ID_DP83848VV	0x20005C90	/* National 83848 */
#define PHY_ID_DP83849		0x20005CA2	/* National 82849 */
#define PHY_ID_KS8721BL		0x00221619	/* Micrel KS8721BL/SL */

#define STR_ID_LXT970		"LXT970"
#define STR_ID_LXT971		"LXT971"
#define STR_ID_82555		"Intel82555"
#define STR_ID_QS6612		"QS6612"
#define STR_ID_AMD79C784	"AMD79C784"
#define STR_ID_LSI80225		"LSI80225"
#define STR_ID_LSI80225B	"LSI80225/B"
#define STR_ID_DP83848VV	"N83848"
#define STR_ID_DP83849		"N83849"
#define STR_ID_KS8721BL		"KS8721BL"

/****************************************************************************
 * mii_init -- Initialize the MII for MII command without ethernet
 * This function is a subset of eth_init
 ****************************************************************************
 */
void mii_reset(struct fec_info_s *info)
{
	volatile fec_t *fecp = (fec_t *) (info->miibase);
	int i;

	fecp->ecr = FEC_ECR_RESET;
	for (i = 0; (fecp->ecr & FEC_ECR_RESET) && (i < FEC_RESET_DELAY); ++i) {
		udelay(1);
	}
	if (i == FEC_RESET_DELAY) {
		printf("FEC_RESET_DELAY timeout\n");
	}
}

/* send command to phy using mii, wait for result */
uint mii_send(uint mii_cmd)
{
	struct fec_info_s *info;
	struct eth_device *dev;
	volatile fec_t *ep;
	uint mii_reply;
	int j = 0;

	/* retrieve from register structure */
	dev = eth_get_dev();
	info = dev->priv;

	ep = (fec_t *) info->miibase;

	ep->mmfr = mii_cmd;	/* command to phy */

	/* wait for mii complete */
	while (!(ep->eir & FEC_EIR_MII) && (j < MCFFEC_TOUT_LOOP)) {
		udelay(1);
		j++;
	}
	if (j >= MCFFEC_TOUT_LOOP) {
		printf("MII not complete\n");
		return -1;
	}

	mii_reply = ep->mmfr;	/* result from phy */
	ep->eir = FEC_EIR_MII;	/* clear MII complete */
#ifdef ET_DEBUG
	printf("%s[%d] %s: sent=0x%8.8x, reply=0x%8.8x\n",
	       __FILE__, __LINE__, __FUNCTION__, mii_cmd, mii_reply);
#endif

	return (mii_reply & 0xffff);	/* data read from phy */
}
#endif				/* CFG_DISCOVER_PHY || CONFIG_CMD_MII */

#if defined(CFG_DISCOVER_PHY)
int mii_discover_phy(struct eth_device *dev)
{
#define MAX_PHY_PASSES 11
	struct fec_info_s *info = dev->priv;
	int phyaddr, pass;
	uint phyno, phytype;

	if (info->phyname_init)
		return info->phy_addr;

	phyaddr = -1;		/* didn't find a PHY yet */
	for (pass = 1; pass <= MAX_PHY_PASSES && phyaddr < 0; ++pass) {
		if (pass > 1) {
			/* PHY may need more time to recover from reset.
			 * The LXT970 needs 50ms typical, no maximum is
			 * specified, so wait 10ms before try again.
			 * With 11 passes this gives it 100ms to wake up.
			 */
			udelay(10000);	/* wait 10ms */
		}

		for (phyno = 0; phyno < 32 && phyaddr < 0; ++phyno) {

			phytype = mii_send(mk_mii_read(phyno, PHY_PHYIDR1));
#ifdef ET_DEBUG
			printf("PHY type 0x%x pass %d type\n", phytype, pass);
#endif
			if (phytype != 0xffff) {
				phyaddr = phyno;
				phytype <<= 16;
				phytype |=
				    mii_send(mk_mii_read(phyno, PHY_PHYIDR2));

				switch (phytype & 0xffffffff) {
				case PHY_ID_KS8721BL:
					strcpy(info->phy_name,
					       STR_ID_KS8721BL);
					info->phyname_init = 1;
					break;
				default:
					strcpy(info->phy_name, "unknown");
					info->phyname_init = 1;
					break;
				}

#ifdef ET_DEBUG
				printf("PHY @ 0x%x pass %d type ", phyno, pass);
				switch (phytype & 0xffffffff) {
				case PHY_ID_KS8721BL:
					printf(STR_ID_KS8721BL);
					break;
				default:
					printf("0x%08x\n", phytype);
					break;
				}
#endif
			}
		}
	}
	if (phyaddr < 0)
		printf("No PHY device found.\n");

	return phyaddr;
}
#endif				/* CFG_DISCOVER_PHY */

int mii_init(void) __attribute__((weak,alias("__mii_init")));

void __mii_init(void)
{
	volatile fec_t *fecp;
	struct fec_info_s *info;
	struct eth_device *dev;
	int miispd = 0, i = 0;
	u16 autoneg = 0;

	/* retrieve from register structure */
	dev = eth_get_dev();
	info = dev->priv;

	fecp = (fec_t *) info->miibase;

	fecpin_setclear(dev, 1);

	mii_reset(info);

	/* We use strictly polling mode only */
	fecp->eimr = 0;

	/* Clear any pending interrupt */
	fecp->eir = 0xffffffff;

	/* Set MII speed */
	miispd = (gd->bus_clk / 1000000) / 5;
	fecp->mscr = miispd << 1;

	info->phy_addr = mii_discover_phy(dev);

#define AUTONEGLINK		(PHY_BMSR_AUTN_COMP | PHY_BMSR_LS)
	while (i < MCFFEC_TOUT_LOOP) {
		autoneg = 0;
		miiphy_read(dev->name, info->phy_addr, PHY_BMSR, &autoneg);
		i++;

		if ((autoneg & AUTONEGLINK) == AUTONEGLINK)
			break;

		udelay(500);
	}
	if (i >= MCFFEC_TOUT_LOOP) {
		printf("Auto Negotiation not complete\n");
	}

	/* adapt to the half/full speed settings */
	info->dup_spd = miiphy_duplex(dev->name, info->phy_addr) << 16;
	info->dup_spd |= miiphy_speed(dev->name, info->phy_addr);
}

/*****************************************************************************
 * Read and write a MII PHY register, routines used by MII Utilities
 *
 * FIXME: These routines are expected to return 0 on success, but mii_send
 *	  does _not_ return an error code. Maybe 0xFFFF means error, i.e.
 *	  no PHY connected...
 *	  For now always return 0.
 * FIXME: These routines only work after calling eth_init() at least once!
 *	  Otherwise they hang in mii_send() !!! Sorry!
 *****************************************************************************/

int mcffec_miiphy_read(char *devname, unsigned char addr, unsigned char reg,
		       unsigned short *value)
{
	short rdreg;		/* register working value */

#ifdef MII_DEBUG
	printf("miiphy_read(0x%x) @ 0x%x = ", reg, addr);
#endif
	rdreg = mii_send(mk_mii_read(addr, reg));

	*value = rdreg;

#ifdef MII_DEBUG
	printf("0x%04x\n", *value);
#endif

	return 0;
}

int mcffec_miiphy_write(char *devname, unsigned char addr, unsigned char reg,
			unsigned short value)
{
	short rdreg;		/* register working value */

#ifdef MII_DEBUG
	printf("miiphy_write(0x%x) @ 0x%x = ", reg, addr);
#endif

	rdreg = mii_send(mk_mii_write(addr, reg, value));

#ifdef MII_DEBUG
	printf("0x%04x\n", value);
#endif

	return 0;
}

#endif				/* CONFIG_CMD_NET, FEC_ENET & NET_MULTI */
