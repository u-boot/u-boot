/*
 * Broadcom BCM5221 Ethernet PHY
 *
 * (C) Copyright 2005 REA Elektronik GmbH <www.rea.de>
 * Anders Larsen <alarsen@rea.de>
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

#include <at91rm9200_net.h>
#include <net.h>
#include <bcm5221.h>

#ifdef CONFIG_DRIVER_ETHER

#if defined(CONFIG_CMD_NET)

/*
 * Name:
 *	bcm5221_IsPhyConnected
 * Description:
 *	Reads the 2 PHY ID registers
 * Arguments:
 *	p_mac - pointer to AT91S_EMAC struct
 * Return value:
 *	TRUE - if id read successfully
 *	FALSE- if error
 */
unsigned int bcm5221_IsPhyConnected (AT91PS_EMAC p_mac)
{
	unsigned short Id1, Id2;

	at91rm9200_EmacEnableMDIO (p_mac);
	at91rm9200_EmacReadPhy (p_mac, BCM5221_PHYID1, &Id1);
	at91rm9200_EmacReadPhy (p_mac, BCM5221_PHYID2, &Id2);
	at91rm9200_EmacDisableMDIO (p_mac);

	if ((Id1 == (BCM5221_PHYID1_OUI >> 6)) &&
		((Id2 >> 10) == (BCM5221_PHYID1_OUI & BCM5221_LSB_MASK)))
		return TRUE;

	return FALSE;
}

/*
 * Name:
 *	bcm5221_GetLinkSpeed
 * Description:
 *	Link parallel detection status of MAC is checked and set in the
 *	MAC configuration registers
 * Arguments:
 *	p_mac - pointer to MAC
 * Return value:
 *	TRUE - if link status set succesfully
 *	FALSE - if link status not set
 */
unsigned char bcm5221_GetLinkSpeed (AT91PS_EMAC p_mac)
{
	unsigned short stat1, stat2;

	if (!at91rm9200_EmacReadPhy (p_mac, BCM5221_BMSR, &stat1))
		return FALSE;

	if (!(stat1 & BCM5221_LINK_STATUS))	/* link status up? */
		return FALSE;

	if (!at91rm9200_EmacReadPhy (p_mac, BCM5221_ACSR, &stat2))
		return FALSE;

	if ((stat1 & BCM5221_100BASE_TX_FD) && (stat2 & BCM5221_100) && (stat2 & BCM5221_FDX)) {
		/*set Emac for 100BaseTX and Full Duplex  */
		p_mac->EMAC_CFG |= AT91C_EMAC_SPD | AT91C_EMAC_FD;
		return TRUE;
	}

	if ((stat1 & BCM5221_10BASE_T_FD) && !(stat2 & BCM5221_100) && (stat2 & BCM5221_FDX)) {
		/*set MII for 10BaseT and Full Duplex  */
		p_mac->EMAC_CFG = (p_mac->EMAC_CFG &
				~(AT91C_EMAC_SPD | AT91C_EMAC_FD))
				| AT91C_EMAC_FD;
		return TRUE;
	}

	if ((stat1 & BCM5221_100BASE_TX_HD) && (stat2 & BCM5221_100) && !(stat2 & BCM5221_FDX)) {
		/*set MII for 100BaseTX and Half Duplex  */
		p_mac->EMAC_CFG = (p_mac->EMAC_CFG &
				~(AT91C_EMAC_SPD | AT91C_EMAC_FD))
				| AT91C_EMAC_SPD;
		return TRUE;
	}

	if ((stat1 & BCM5221_10BASE_T_HD) && !(stat2 & BCM5221_100) && !(stat2 & BCM5221_FDX)) {
		/*set MII for 10BaseT and Half Duplex  */
		p_mac->EMAC_CFG &= ~(AT91C_EMAC_SPD | AT91C_EMAC_FD);
		return TRUE;
	}
	return FALSE;
}


/*
 * Name:
 *	bcm5221_InitPhy
 * Description:
 *	MAC starts checking its link by using parallel detection and
 *	Autonegotiation and the same is set in the MAC configuration registers
 * Arguments:
 *	p_mac - pointer to struct AT91S_EMAC
 * Return value:
 *	TRUE - if link status set succesfully
 *	FALSE - if link status not set
 */
unsigned char bcm5221_InitPhy (AT91PS_EMAC p_mac)
{
	unsigned char ret = TRUE;
	unsigned short IntValue;

	at91rm9200_EmacEnableMDIO (p_mac);

	if (!bcm5221_GetLinkSpeed (p_mac)) {
		/* Try another time */
		ret = bcm5221_GetLinkSpeed (p_mac);
	}

	/* Disable PHY Interrupts */
	at91rm9200_EmacReadPhy (p_mac, BCM5221_INTR, &IntValue);
	/* clear FDX LED and INTR Enable */
	IntValue &= ~(BCM5221_FDX_LED | BCM5221_INTR_ENABLE);
	/* set FDX, SPD, Link, INTR masks */
	IntValue |= (BCM5221_FDX_MASK  | BCM5221_SPD_MASK |
		     BCM5221_LINK_MASK | BCM5221_INTR_MASK);
	at91rm9200_EmacWritePhy (p_mac, BCM5221_INTR, &IntValue);
	at91rm9200_EmacDisableMDIO (p_mac);

	return (ret);
}


/*
 * Name:
 *	bcm5221_AutoNegotiate
 * Description:
 *	MAC Autonegotiates with the partner status of same is set in the
 *	MAC configuration registers
 * Arguments:
 *	dev - pointer to struct net_device
 * Return value:
 *	TRUE - if link status set successfully
 *	FALSE - if link status not set
 */
unsigned char bcm5221_AutoNegotiate (AT91PS_EMAC p_mac, int *status)
{
	unsigned short value;
	unsigned short PhyAnar;
	unsigned short PhyAnalpar;

	/* Set bcm5221 control register */
	if (!at91rm9200_EmacReadPhy (p_mac, BCM5221_BMCR, &value))
		return FALSE;
	value &= ~BCM5221_AUTONEG;	/* remove autonegotiation enable */
	value |=  BCM5221_ISOLATE;	/* Electrically isolate PHY */
	if (!at91rm9200_EmacWritePhy (p_mac, BCM5221_BMCR, &value))
		return FALSE;

	/* Set the Auto_negotiation Advertisement Register */
	/* MII advertising for 100BaseTxFD and HD, 10BaseTFD and HD, IEEE 802.3 */
	PhyAnar = BCM5221_TX_FDX | BCM5221_TX_HDX |
		  BCM5221_10_FDX | BCM5221_10_HDX | BCM5221_AN_IEEE_802_3;
	if (!at91rm9200_EmacWritePhy (p_mac, BCM5221_ANAR, &PhyAnar))
		return FALSE;

	/* Read the Control Register     */
	if (!at91rm9200_EmacReadPhy (p_mac, BCM5221_BMCR, &value))
		return FALSE;

	value |= BCM5221_SPEED_SELECT | BCM5221_AUTONEG | BCM5221_DUPLEX_MODE;
	if (!at91rm9200_EmacWritePhy (p_mac, BCM5221_BMCR, &value))
		return FALSE;
	/* Restart Auto_negotiation  */
	value |= BCM5221_RESTART_AUTONEG;
	value &= ~BCM5221_ISOLATE;
	if (!at91rm9200_EmacWritePhy (p_mac, BCM5221_BMCR, &value))
		return FALSE;

	/*check AutoNegotiate complete */
	udelay (10000);
	at91rm9200_EmacReadPhy (p_mac, BCM5221_BMSR, &value);
	if (!(value & BCM5221_AUTONEG_COMP))
		return FALSE;

	/* Get the AutoNeg Link partner base page */
	if (!at91rm9200_EmacReadPhy (p_mac, BCM5221_ANLPAR, &PhyAnalpar))
		return FALSE;

	if ((PhyAnar & BCM5221_TX_FDX) && (PhyAnalpar & BCM5221_TX_FDX)) {
		/*set MII for 100BaseTX and Full Duplex  */
		p_mac->EMAC_CFG |= AT91C_EMAC_SPD | AT91C_EMAC_FD;
		return TRUE;
	}

	if ((PhyAnar & BCM5221_10_FDX) && (PhyAnalpar & BCM5221_10_FDX)) {
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
