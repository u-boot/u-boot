/*
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

#include <at91rm9200_net.h>
#include <net.h>
#include <dm9161.h>

#ifdef CONFIG_DRIVER_ETHER

#if defined(CONFIG_CMD_NET)

/*
 * Name:
 *	dm9161_IsPhyConnected
 * Description:
 *	Reads the 2 PHY ID registers
 * Arguments:
 *	p_mac - pointer to AT91S_EMAC struct
 * Return value:
 *	TRUE - if id read successfully
 *	FALSE- if error
 */
unsigned int dm9161_IsPhyConnected (AT91PS_EMAC p_mac)
{
	unsigned short Id1, Id2;

	at91rm9200_EmacEnableMDIO (p_mac);
	at91rm9200_EmacReadPhy (p_mac, DM9161_PHYID1, &Id1);
	at91rm9200_EmacReadPhy (p_mac, DM9161_PHYID2, &Id2);
	at91rm9200_EmacDisableMDIO (p_mac);

	if ((Id1 == (DM9161_PHYID1_OUI >> 6)) &&
		((Id2 >> 10) == (DM9161_PHYID1_OUI & DM9161_LSB_MASK)))
		return TRUE;

	return FALSE;
}

/*
 * Name:
 *	dm9161_GetLinkSpeed
 * Description:
 *	Link parallel detection status of MAC is checked and set in the
 *	MAC configuration registers
 * Arguments:
 *	p_mac - pointer to MAC
 * Return value:
 *	TRUE - if link status set succesfully
 *	FALSE - if link status not set
 */
UCHAR dm9161_GetLinkSpeed (AT91PS_EMAC p_mac)
{
	unsigned short stat1, stat2;

	if (!at91rm9200_EmacReadPhy (p_mac, DM9161_BMSR, &stat1))
		return FALSE;

	if (!(stat1 & DM9161_LINK_STATUS))	/* link status up? */
		return FALSE;

	if (!at91rm9200_EmacReadPhy (p_mac, DM9161_DSCSR, &stat2))
		return FALSE;

	if ((stat1 & DM9161_100BASE_TX_FD) && (stat2 & DM9161_100FDX)) {
		/*set Emac for 100BaseTX and Full Duplex  */
		p_mac->EMAC_CFG |= AT91C_EMAC_SPD | AT91C_EMAC_FD;
		return TRUE;
	}

	if ((stat1 & DM9161_10BASE_T_FD) && (stat2 & DM9161_10FDX)) {
		/*set MII for 10BaseT and Full Duplex  */
		p_mac->EMAC_CFG = (p_mac->EMAC_CFG &
				~(AT91C_EMAC_SPD | AT91C_EMAC_FD))
				| AT91C_EMAC_FD;
		return TRUE;
	}

	if ((stat1 & DM9161_100BASE_T4_HD) && (stat2 & DM9161_100HDX)) {
		/*set MII for 100BaseTX and Half Duplex  */
		p_mac->EMAC_CFG = (p_mac->EMAC_CFG &
				~(AT91C_EMAC_SPD | AT91C_EMAC_FD))
				| AT91C_EMAC_SPD;
		return TRUE;
	}

	if ((stat1 & DM9161_10BASE_T_HD) && (stat2 & DM9161_10HDX)) {
		/*set MII for 10BaseT and Half Duplex  */
		p_mac->EMAC_CFG &= ~(AT91C_EMAC_SPD | AT91C_EMAC_FD);
		return TRUE;
	}
	return FALSE;
}


/*
 * Name:
 *	dm9161_InitPhy
 * Description:
 *	MAC starts checking its link by using parallel detection and
 *	Autonegotiation and the same is set in the MAC configuration registers
 * Arguments:
 *	p_mac - pointer to struct AT91S_EMAC
 * Return value:
 *	TRUE - if link status set succesfully
 *	FALSE - if link status not set
 */
UCHAR dm9161_InitPhy (AT91PS_EMAC p_mac)
{
	UCHAR ret = TRUE;
	unsigned short IntValue;

	at91rm9200_EmacEnableMDIO (p_mac);

	if (!dm9161_GetLinkSpeed (p_mac)) {
		/* Try another time */
		ret = dm9161_GetLinkSpeed (p_mac);
	}

	/* Disable PHY Interrupts */
	at91rm9200_EmacReadPhy (p_mac, DM9161_MDINTR, &IntValue);
	/* set FDX, SPD, Link, INTR masks */
	IntValue |= (DM9161_FDX_MASK | DM9161_SPD_MASK |
		     DM9161_LINK_MASK | DM9161_INTR_MASK);
	at91rm9200_EmacWritePhy (p_mac, DM9161_MDINTR, &IntValue);
	at91rm9200_EmacDisableMDIO (p_mac);

	return (ret);
}


/*
 * Name:
 *	dm9161_AutoNegotiate
 * Description:
 *	MAC Autonegotiates with the partner status of same is set in the
 *	MAC configuration registers
 * Arguments:
 *	dev - pointer to struct net_device
 * Return value:
 *	TRUE - if link status set successfully
 *	FALSE - if link status not set
 */
UCHAR dm9161_AutoNegotiate (AT91PS_EMAC p_mac, int *status)
{
	unsigned short value;
	unsigned short PhyAnar;
	unsigned short PhyAnalpar;

	/* Set dm9161 control register */
	if (!at91rm9200_EmacReadPhy (p_mac, DM9161_BMCR, &value))
		return FALSE;
	value &= ~DM9161_AUTONEG;	/* remove autonegotiation enable */
	value |= DM9161_ISOLATE;	/* Electrically isolate PHY */
	if (!at91rm9200_EmacWritePhy (p_mac, DM9161_BMCR, &value))
		return FALSE;

	/* Set the Auto_negotiation Advertisement Register */
	/* MII advertising for Next page, 100BaseTxFD and HD, 10BaseTFD and HD, IEEE 802.3 */
	PhyAnar = DM9161_NP | DM9161_TX_FDX | DM9161_TX_HDX |
		  DM9161_10_FDX | DM9161_10_HDX | DM9161_AN_IEEE_802_3;
	if (!at91rm9200_EmacWritePhy (p_mac, DM9161_ANAR, &PhyAnar))
		return FALSE;

	/* Read the Control Register     */
	if (!at91rm9200_EmacReadPhy (p_mac, DM9161_BMCR, &value))
		return FALSE;

	value |= DM9161_SPEED_SELECT | DM9161_AUTONEG | DM9161_DUPLEX_MODE;
	if (!at91rm9200_EmacWritePhy (p_mac, DM9161_BMCR, &value))
		return FALSE;
	/* Restart Auto_negotiation  */
	value |= DM9161_RESTART_AUTONEG;
	value &= ~DM9161_ISOLATE;
	if (!at91rm9200_EmacWritePhy (p_mac, DM9161_BMCR, &value))
		return FALSE;

	/*check AutoNegotiate complete */
	udelay (10000);
	at91rm9200_EmacReadPhy (p_mac, DM9161_BMSR, &value);
	if (!(value & DM9161_AUTONEG_COMP))
		return FALSE;

	/* Get the AutoNeg Link partner base page */
	if (!at91rm9200_EmacReadPhy (p_mac, DM9161_ANLPAR, &PhyAnalpar))
		return FALSE;

	if ((PhyAnar & DM9161_TX_FDX) && (PhyAnalpar & DM9161_TX_FDX)) {
		/*set MII for 100BaseTX and Full Duplex  */
		p_mac->EMAC_CFG |= AT91C_EMAC_SPD | AT91C_EMAC_FD;
		return TRUE;
	}

	if ((PhyAnar & DM9161_10_FDX) && (PhyAnalpar & DM9161_10_FDX)) {
		/*set MII for 10BaseT and Full Duplex  */
		p_mac->EMAC_CFG = (p_mac->EMAC_CFG &
				~(AT91C_EMAC_SPD | AT91C_EMAC_FD))
				| AT91C_EMAC_FD;
		return TRUE;
	}
	return FALSE;
}

#endif

#endif	/* CONFIG_DRIVER_ETHER */
