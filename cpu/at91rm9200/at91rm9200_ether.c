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

/* ----- Ethernet Buffer definitions ----- */

typedef struct {
	unsigned long addr, size;
} rbf_t;

#define RBF_ADDR      0xfffffffc
#define RBF_OWNER     (1<<0)
#define RBF_WRAP      (1<<1)
#define RBF_BROADCAST (1<<31)
#define RBF_MULTICAST (1<<30)
#define RBF_UNICAST   (1<<29)
#define RBF_EXTERNAL  (1<<28)
#define RBF_UNKOWN    (1<<27)
#define RBF_SIZE      0x07ff
#define RBF_LOCAL4    (1<<26)
#define RBF_LOCAL3    (1<<25)
#define RBF_LOCAL2    (1<<24)
#define RBF_LOCAL1    (1<<23)

/* Emac Buffers in last 512KBytes of SDRAM*/
/* Be careful, buffer size is limited to 512KBytes !!! */
#define RBF_FRAMEMAX 100
/*#define RBF_FRAMEMEM 0x200000 */
#define RBF_FRAMEMEM 0x21F80000
#define RBF_FRAMELEN 0x600

#define RBF_FRAMEBTD RBF_FRAMEMEM
#define RBF_FRAMEBUF (RBF_FRAMEMEM + RBF_FRAMEMAX*sizeof(rbf_t))


#ifdef CONFIG_DRIVER_ETHER

#if (CONFIG_COMMANDS & CFG_CMD_NET)

/* structure to interface the PHY */
AT91S_PhyOps AT91S_Dm9161Ops;
AT91PS_PhyOps pPhyOps;

AT91PS_EMAC p_mac;

/*************************** Phy layer functions ************************/
/** functions to interface the DAVICOM 10/100Mbps ethernet phy **********/

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
static unsigned int dm9161_IsPhyConnected (AT91PS_EMAC p_mac)
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
static UCHAR dm9161_GetLinkSpeed (AT91PS_EMAC p_mac)
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
static UCHAR dm9161_InitPhy (AT91PS_EMAC p_mac)
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
	/* clear FDX, SPD, Link, INTR masks */
	IntValue &= ~(DM9161_FDX_MASK | DM9161_SPD_MASK |
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
static UCHAR dm9161_AutoNegotiate (AT91PS_EMAC p_mac, int *status)
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




/*********** EMAC Phy layer Management functions *************************/
/*
 * Name: 
 *	at91rm9200_EmacEnableMDIO
 * Description: 
 *	Enables the MDIO bit in MAC control register
 * Arguments: 
 *	p_mac - pointer to struct AT91S_EMAC
 * Return value: 
 *	none
 */
static void at91rm9200_EmacEnableMDIO (AT91PS_EMAC p_mac)
{
	/* Mac CTRL reg set for MDIO enable */
	p_mac->EMAC_CTL |= AT91C_EMAC_MPE;	/* Management port enable */
}

/*
 * Name: 
 *	at91rm9200_EmacDisableMDIO
 * Description: 
 *	Disables the MDIO bit in MAC control register
 * Arguments: 
 *	p_mac - pointer to struct AT91S_EMAC
 * Return value: 
 *	none
 */
static void at91rm9200_EmacDisableMDIO (AT91PS_EMAC p_mac)
{
	/* Mac CTRL reg set for MDIO disable */
	p_mac->EMAC_CTL &= ~AT91C_EMAC_MPE;	/* Management port disable */
}


/*
 * Name: 
 *	at91rm9200_EmacReadPhy
 * Description: 
 *	Reads data from the PHY register
 * Arguments: 
 *	dev - pointer to struct net_device
 *	RegisterAddress - unsigned char
 * 	pInput - pointer to value read from register 
 * Return value: 
 *	TRUE - if data read successfully
 */
static UCHAR at91rm9200_EmacReadPhy (AT91PS_EMAC p_mac,
				     unsigned char RegisterAddress,
				     unsigned short *pInput)
{
	p_mac->EMAC_MAN = (AT91C_EMAC_HIGH & ~AT91C_EMAC_LOW) |
			  (AT91C_EMAC_CODE_802_3) | (AT91C_EMAC_RW_R) |
			  (RegisterAddress << 18);

	udelay (10000);

	*pInput = (unsigned short) p_mac->EMAC_MAN;

	return TRUE;
}


/*
 * Name: 
 *	at91rm9200_EmacWritePhy
 * Description: 
 *	Writes data to the PHY register
 * Arguments: 
 *	dev - pointer to struct net_device
 *	RegisterAddress - unsigned char
 * 	pOutput - pointer to value to be written in the register 
 * Return value: 
 *	TRUE - if data read successfully
 */
static UCHAR at91rm9200_EmacWritePhy (AT91PS_EMAC p_mac,
									  unsigned char RegisterAddress,
									  unsigned short *pOutput)
{
	p_mac->EMAC_MAN = (AT91C_EMAC_HIGH & ~AT91C_EMAC_LOW) |
			AT91C_EMAC_CODE_802_3 | AT91C_EMAC_RW_W |
			(RegisterAddress << 18);

	udelay (10000);

	return TRUE;
}

/*
 * Name: 
 *	at91rm92000_GetPhyInterface
 * Description: 
 *	Initialise the interface functions to the PHY 
 * Arguments: 
 *	None
 * Return value: 
 *	None
 */
void at91rm92000_GetPhyInterface (void)
{
	AT91S_Dm9161Ops.Init = dm9161_InitPhy;
	AT91S_Dm9161Ops.IsPhyConnected = dm9161_IsPhyConnected;
	AT91S_Dm9161Ops.GetLinkSpeed = dm9161_GetLinkSpeed;
	AT91S_Dm9161Ops.AutoNegotiate = dm9161_AutoNegotiate;

	pPhyOps = (AT91PS_PhyOps) & AT91S_Dm9161Ops;
}


rbf_t *rbfdt;
rbf_t *rbfp;

int eth_init (bd_t * bd)
{
	int ret;
	int i;

	p_mac = AT91C_BASE_EMAC;

	*AT91C_PIOA_PDR = AT91C_PA16_EMDIO | AT91C_PA15_EMDC | AT91C_PA14_ERXER | AT91C_PA13_ERX1 | AT91C_PA12_ERX0 | AT91C_PA11_ECRS_ECRSDV | AT91C_PA10_ETX1 | AT91C_PA9_ETX0 | AT91C_PA8_ETXEN | AT91C_PA7_ETXCK_EREFCK;	/* PIO Disable Register */

	*AT91C_PIOB_PDR = AT91C_PB25_EF100 |
			AT91C_PB19_ERXCK | AT91C_PB18_ECOL | AT91C_PB17_ERXDV |
			AT91C_PB16_ERX3 | AT91C_PB15_ERX2 | AT91C_PB14_ETXER |
			AT91C_PB13_ETX3 | AT91C_PB12_ETX2;

	*AT91C_PIOB_BSR = AT91C_PB25_EF100 | AT91C_PB19_ERXCK | AT91C_PB18_ECOL | AT91C_PB17_ERXDV | AT91C_PB16_ERX3 | AT91C_PB15_ERX2 | AT91C_PB14_ETXER | AT91C_PB13_ETX3 | AT91C_PB12_ETX2;	/* Select B Register */

	*AT91C_PMC_PCER = 1 << AT91C_ID_EMAC;	/* Peripheral Clock Enable Register */

	p_mac->EMAC_CFG |= AT91C_EMAC_CSR;	/* Clear statistics */

	/* Init Ehternet buffers */
	rbfdt = (rbf_t *) RBF_FRAMEBTD;
	for (i = 0; i < RBF_FRAMEMAX; i++) {
		rbfdt[i].addr = RBF_FRAMEBUF + RBF_FRAMELEN * i;
		rbfdt[i].size = 0;
	}
	rbfdt[RBF_FRAMEMAX - 1].addr |= RBF_WRAP;
	rbfp = &rbfdt[0];

	at91rm92000_GetPhyInterface ();

	if (!pPhyOps->IsPhyConnected (p_mac))
		printf ("PHY not connected!!\n\r");

	/* MII management start from here */
	if (!(p_mac->EMAC_SR & AT91C_EMAC_LINK)) {
		if (!(ret = pPhyOps->Init (p_mac))) {
			printf ("MAC: error during MII initialization\n");
			return 0;
		}
	} else {
		printf ("No link\n\r");
		return 0;
	}

	p_mac->EMAC_SA2L = (bd->bi_enetaddr[3] << 24) | (bd->bi_enetaddr[2] << 16)
			 | (bd->bi_enetaddr[1] << 8) | (bd->bi_enetaddr[0]);
	p_mac->EMAC_SA2H = (bd->bi_enetaddr[5] << 8) | (bd->bi_enetaddr[4]);

	p_mac->EMAC_RBQP = (long) (&rbfdt[0]);
	p_mac->EMAC_RSR &= ~(AT91C_EMAC_RSR_OVR | AT91C_EMAC_REC | AT91C_EMAC_BNA);
	p_mac->EMAC_CFG = (p_mac->EMAC_CFG | AT91C_EMAC_CAF | AT91C_EMAC_NBC | AT91C_EMAC_RMII)
			& ~AT91C_EMAC_CLK;
	p_mac->EMAC_CTL |= AT91C_EMAC_TE | AT91C_EMAC_RE;

	return 0;
}

int eth_send (volatile void *packet, int length)
{
	while (!(p_mac->EMAC_TSR & AT91C_EMAC_BNQ));
	p_mac->EMAC_TAR = (long) packet;
	p_mac->EMAC_TCR = length;
	while (p_mac->EMAC_TCR & 0x7ff);
	p_mac->EMAC_TSR |= AT91C_EMAC_COMP;
	return 0;
}

int eth_rx (void)
{
	int size;

	if (!(rbfp->addr & RBF_OWNER))
		return 0;

	size = rbfp->size & RBF_SIZE;
	NetReceive ((volatile uchar *) (rbfp->addr & RBF_ADDR), size);

	rbfp->addr &= ~RBF_OWNER;
	if (rbfp->addr & RBF_WRAP)
		rbfp = &rbfdt[0];
	else
		rbfp++;

	p_mac->EMAC_RSR |= AT91C_EMAC_REC;

	return size;
}

void eth_halt (void)
{
};
#endif	/* CONFIG_COMMANDS & CFG_CMD_NET */
#endif	/* CONFIG_DRIVER_ETHER */
