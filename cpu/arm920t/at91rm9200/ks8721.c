/*
 * (C) Copyright 2006
 * Author : Eric Benard (Eukrea Electromatique)
 * based on dm9161.c which is :
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
#include <ks8721.h>

#ifdef CONFIG_DRIVER_ETHER

#if defined(CONFIG_CMD_NET)

/*
 * Name:
 *	ks8721_isphyconnected
 * Description:
 *	Reads the 2 PHY ID registers
 * Arguments:
 *	p_mac - pointer to AT91S_EMAC struct
 * Return value:
 *	1 - if id read successfully
 *	0 - if error
 */
unsigned int ks8721_isphyconnected(AT91PS_EMAC p_mac)
{
	unsigned short id1, id2;

	at91rm9200_EmacEnableMDIO(p_mac);
	at91rm9200_EmacReadPhy(p_mac,
		CONFIG_PHY_ADDRESS | KS8721_PHYID1, &id1);
	at91rm9200_EmacReadPhy(p_mac,
		CONFIG_PHY_ADDRESS | KS8721_PHYID2, &id2);
	at91rm9200_EmacDisableMDIO(p_mac);

	if ((id1 == (KS8721_PHYID_OUI >> 6)) &&
		((id2 >> 10) == (KS8721_PHYID_OUI & KS8721_LSB_MASK))) {
		if ((id2 & KS8721_MODELMASK) == KS8721BL_MODEL)
			printf("Micrel KS8721bL PHY detected : ");
		else
			printf("Unknown Micrel PHY detected : ");
		return 1;
	}
	return 0;
}

/*
 * Name:
 *	ks8721_getlinkspeed
 * Description:
 *	Link parallel detection status of MAC is checked and set in the
 *	MAC configuration registers
 * Arguments:
 *	p_mac - pointer to MAC
 * Return value:
 *	1 - if link status set succesfully
 *	0 - if link status not set
 */
unsigned char ks8721_getlinkspeed(AT91PS_EMAC p_mac)
{
	unsigned short stat1;

	if (!at91rm9200_EmacReadPhy(p_mac, KS8721_BMSR, &stat1))
		return 0;

	if (!(stat1 & KS8721_LINK_STATUS)) {
		/* link status up? */
		printf("Link Down !\n");
		return 0;
	}

	if (stat1 & KS8721_100BASE_TX_FD) {
		/* set Emac for 100BaseTX and Full Duplex */
		printf("100BT FD\n");
		p_mac->EMAC_CFG |= AT91C_EMAC_SPD | AT91C_EMAC_FD;
		return 1;
	}

	if (stat1 & KS8721_10BASE_T_FD) {
		/* set MII for 10BaseT and Full Duplex */
		printf("10BT FD\n");
		p_mac->EMAC_CFG = (p_mac->EMAC_CFG &
				~(AT91C_EMAC_SPD | AT91C_EMAC_FD))
				| AT91C_EMAC_FD;
		return 1;
	}

	if (stat1 & KS8721_100BASE_T4_HD) {
		/* set MII for 100BaseTX and Half Duplex */
		printf("100BT HD\n");
		p_mac->EMAC_CFG = (p_mac->EMAC_CFG &
				~(AT91C_EMAC_SPD | AT91C_EMAC_FD))
				| AT91C_EMAC_SPD;
		return 1;
	}

	if (stat1 & KS8721_10BASE_T_HD) {
		/* set MII for 10BaseT and Half Duplex */
		printf("10BT HD\n");
		p_mac->EMAC_CFG &= ~(AT91C_EMAC_SPD | AT91C_EMAC_FD);
		return 1;
	}
	return 0;
}

/*
 * Name:
 *	ks8721_initphy
 * Description:
 *	MAC starts checking its link by using parallel detection and
 *	Autonegotiation and the same is set in the MAC configuration registers
 * Arguments:
 *	p_mac - pointer to struct AT91S_EMAC
 * Return value:
 *	1 - if link status set succesfully
 *	0 - if link status not set
 */
unsigned char ks8721_initphy(AT91PS_EMAC p_mac)
{
	unsigned char ret = 1;
	unsigned short intvalue;

	at91rm9200_EmacEnableMDIO(p_mac);

	/* Try another time */
	if (!ks8721_getlinkspeed(p_mac))
		ret = ks8721_getlinkspeed(p_mac);

	/* Disable PHY Interrupts */
	intvalue = 0;
	at91rm9200_EmacWritePhy(p_mac,
		CONFIG_PHY_ADDRESS | KS8721_MDINTR, &intvalue);
	at91rm9200_EmacDisableMDIO(p_mac);

	return ret;
}

/*
 * Name:
 *	ks8721_autonegotiate
 * Description:
 *	MAC Autonegotiates with the partner status of same is set in the
 *	MAC configuration registers
 * Arguments:
 *	dev - pointer to struct net_device
 * Return value:
 *	1 - if link status set successfully
 *	0 - if link status not set
 */
unsigned char ks8721_autonegotiate(AT91PS_EMAC p_mac, int *status)
{
	unsigned short value;
	unsigned short phyanar;
	unsigned short phyanalpar;

	/* Set ks8721 control register */
	if (!at91rm9200_EmacReadPhy(p_mac,
		CONFIG_PHY_ADDRESS | KS8721_BMCR, &value))
		return 0;

	/* remove autonegotiation enable */
	value &= ~KS8721_AUTONEG;
	/* Electrically isolate PHY */
	value |= KS8721_ISOLATE;
	if (!at91rm9200_EmacWritePhy(p_mac,
		CONFIG_PHY_ADDRESS | KS8721_BMCR, &value)) {
		return 0;
	}
	/*
	 * Set the Auto_negotiation Advertisement Register
	 * MII advertising for Next page, 100BaseTxFD and HD,
	 * 10BaseTFD and HD, IEEE 802.3
	 */
	phyanar = KS8721_NP | KS8721_TX_FDX | KS8721_TX_HDX |
		  KS8721_10_FDX | KS8721_10_HDX | KS8721_AN_IEEE_802_3;
	if (!at91rm9200_EmacWritePhy(p_mac,
		CONFIG_PHY_ADDRESS | KS8721_ANAR, &phyanar)) {
		return 0;
	}
	/* Read the Control Register */
	if (!at91rm9200_EmacReadPhy(p_mac,
		CONFIG_PHY_ADDRESS | KS8721_BMCR, &value)) {
		return 0;
	}
	value |= KS8721_SPEED_SELECT | KS8721_AUTONEG | KS8721_DUPLEX_MODE;
	if (!at91rm9200_EmacWritePhy(p_mac,
		CONFIG_PHY_ADDRESS | KS8721_BMCR, &value)) {
		return 0;
	}
	/* Restart Auto_negotiation */
	value |= KS8721_RESTART_AUTONEG;
	value &= ~KS8721_ISOLATE;
	if (!at91rm9200_EmacWritePhy(p_mac,
		CONFIG_PHY_ADDRESS | KS8721_BMCR, &value)) {
		return 0;
	}
	/* Check AutoNegotiate complete */
	udelay(10000);
	at91rm9200_EmacReadPhy(p_mac,
		CONFIG_PHY_ADDRESS | KS8721_BMSR, &value);
	if (!(value & KS8721_AUTONEG_COMP))
		return 0;

	/* Get the AutoNeg Link partner base page */
	if (!at91rm9200_EmacReadPhy(p_mac,
		CONFIG_PHY_ADDRESS | KS8721_ANLPAR, &phyanalpar)) {
		return 0;
	}

	if ((phyanar & KS8721_TX_FDX) && (phyanalpar & KS8721_TX_FDX)) {
		/* Set MII for 100BaseTX and Full Duplex */
		p_mac->EMAC_CFG |= AT91C_EMAC_SPD | AT91C_EMAC_FD;
		return 1;
	}

	if ((phyanar & KS8721_10_FDX) && (phyanalpar & KS8721_10_FDX)) {
		/* Set MII for 10BaseT and Full Duplex */
		p_mac->EMAC_CFG = (p_mac->EMAC_CFG &
				~(AT91C_EMAC_SPD | AT91C_EMAC_FD))
				| AT91C_EMAC_FD;
		return 1;
	}
	return 0;
}

#endif	/* CONFIG_CMD_NET */

#endif	/* CONFIG_DRIVER_ETHER */
