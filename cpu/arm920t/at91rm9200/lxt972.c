/*
 *
 * (C) Copyright 2003
 * Author : Hamid Ikdoumi (Atmel)
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

/*
 * Adapted for KwikByte KB920x board: 22APR2005
 */

#include <common.h>
#include <at91rm9200_net.h>
#include <net.h>
#include <miiphy.h>
#include <lxt971a.h>

#ifdef CONFIG_DRIVER_ETHER

#if defined(CONFIG_CMD_NET)

/*
 * Name:
 *	lxt972_IsPhyConnected
 * Description:
 *	Reads the 2 PHY ID registers
 * Arguments:
 *	p_mac - pointer to AT91S_EMAC struct
 * Return value:
 *	TRUE - if id read successfully
 *	FALSE- if error
 */
unsigned int lxt972_IsPhyConnected (AT91PS_EMAC p_mac)
{
	unsigned short Id1, Id2;

	at91rm9200_EmacEnableMDIO (p_mac);
	at91rm9200_EmacReadPhy(p_mac, PHY_PHYIDR1, &Id1);
	at91rm9200_EmacReadPhy(p_mac, PHY_PHYIDR2, &Id2);
	at91rm9200_EmacDisableMDIO (p_mac);

	if ((Id1 == (0x0013)) && ((Id2  & 0xFFF0) == 0x78E0))
		return TRUE;

	return FALSE;
}

/*
 * Name:
 *	lxt972_GetLinkSpeed
 * Description:
 *	Link parallel detection status of MAC is checked and set in the
 *	MAC configuration registers
 * Arguments:
 *	p_mac - pointer to MAC
 * Return value:
 *	TRUE - if link status set succesfully
 *	FALSE - if link status not set
 */
UCHAR lxt972_GetLinkSpeed (AT91PS_EMAC p_mac)
{
	unsigned short stat1;

	if (!at91rm9200_EmacReadPhy (p_mac, PHY_LXT971_STAT2, &stat1))
		return FALSE;

	if (!(stat1 & PHY_LXT971_STAT2_LINK))	/* link status up? */
		return FALSE;

	if (stat1 & PHY_LXT971_STAT2_100BTX) {

		if (stat1 & PHY_LXT971_STAT2_DUPLEX_MODE) {

			/*set Emac for 100BaseTX and Full Duplex  */
			p_mac->EMAC_CFG |= AT91C_EMAC_SPD | AT91C_EMAC_FD;
		} else {

			/*set Emac for 100BaseTX and Half Duplex  */
			p_mac->EMAC_CFG = (p_mac->EMAC_CFG &
					~(AT91C_EMAC_SPD | AT91C_EMAC_FD))
					| AT91C_EMAC_SPD;
		}

		return TRUE;

	} else {

		if (stat1 & PHY_LXT971_STAT2_DUPLEX_MODE) {

			/*set MII for 10BaseT and Full Duplex  */
			p_mac->EMAC_CFG = (p_mac->EMAC_CFG &
					~(AT91C_EMAC_SPD | AT91C_EMAC_FD))
					| AT91C_EMAC_FD;
		} else {

			/*set MII for 10BaseT and Half Duplex  */
			p_mac->EMAC_CFG &= ~(AT91C_EMAC_SPD | AT91C_EMAC_FD);
		}

		return TRUE;
	}

	return FALSE;
}


/*
 * Name:
 *	lxt972_InitPhy
 * Description:
 *	MAC starts checking its link by using parallel detection and
 *	Autonegotiation and the same is set in the MAC configuration registers
 * Arguments:
 *	p_mac - pointer to struct AT91S_EMAC
 * Return value:
 *	TRUE - if link status set succesfully
 *	FALSE - if link status not set
 */
UCHAR lxt972_InitPhy (AT91PS_EMAC p_mac)
{
	UCHAR ret = TRUE;

	at91rm9200_EmacEnableMDIO (p_mac);

	if (!lxt972_GetLinkSpeed (p_mac)) {
		/* Try another time */
		ret = lxt972_GetLinkSpeed (p_mac);
	}

	/* Disable PHY Interrupts */
	at91rm9200_EmacWritePhy (p_mac, PHY_LXT971_INT_ENABLE, 0);

	at91rm9200_EmacDisableMDIO (p_mac);

	return (ret);
}


/*
 * Name:
 *	lxt972_AutoNegotiate
 * Description:
 *	MAC Autonegotiates with the partner status of same is set in the
 *	MAC configuration registers
 * Arguments:
 *	dev - pointer to struct net_device
 * Return value:
 *	TRUE - if link status set successfully
 *	FALSE - if link status not set
 */
UCHAR lxt972_AutoNegotiate (AT91PS_EMAC p_mac, int *status)
{
	unsigned short value;

	/* Set lxt972 control register */
	if (!at91rm9200_EmacReadPhy (p_mac, PHY_BMCR, &value))
		return FALSE;

	/* Restart Auto_negotiation  */
	value |= PHY_BMCR_RST_NEG;
	if (!at91rm9200_EmacWritePhy (p_mac, PHY_BMCR, &value))
		return FALSE;

	/*check AutoNegotiate complete */
	udelay (10000);
	at91rm9200_EmacReadPhy(p_mac, PHY_BMSR, &value);
	if (!(value & PHY_BMSR_AUTN_COMP))
		return FALSE;

	return (lxt972_GetLinkSpeed (p_mac));
}

#endif

#endif	/* CONFIG_DRIVER_ETHER */
