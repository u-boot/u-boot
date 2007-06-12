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
#include <miiphy.h>

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

#define RBF_FRAMEMAX 64
#define RBF_FRAMELEN 0x600

#ifdef CONFIG_DRIVER_ETHER

#if (CONFIG_COMMANDS & CFG_CMD_NET) || defined(CONFIG_CMD_NET)

/* alignment as per Errata #11 (64 bytes) is insufficient! */
rbf_t rbfdt[RBF_FRAMEMAX] __attribute((aligned(512)));
rbf_t *rbfp;

unsigned char rbf_framebuf[RBF_FRAMEMAX][RBF_FRAMELEN] __attribute((aligned(4)));

/* structure to interface the PHY */
AT91S_PhyOps PhyOps;

AT91PS_EMAC p_mac;

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
void at91rm9200_EmacEnableMDIO (AT91PS_EMAC p_mac)
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
void at91rm9200_EmacDisableMDIO (AT91PS_EMAC p_mac)
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
UCHAR at91rm9200_EmacReadPhy (AT91PS_EMAC p_mac,
				     unsigned char RegisterAddress,
				     unsigned short *pInput)
{
	p_mac->EMAC_MAN = (AT91C_EMAC_HIGH & ~AT91C_EMAC_LOW) |
			  (AT91C_EMAC_RW_R) |
			  (RegisterAddress << 18) |
			  (AT91C_EMAC_CODE_802_3);

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
UCHAR at91rm9200_EmacWritePhy (AT91PS_EMAC p_mac,
				      unsigned char RegisterAddress,
				      unsigned short *pOutput)
{
	p_mac->EMAC_MAN = (AT91C_EMAC_HIGH & ~AT91C_EMAC_LOW) |
			AT91C_EMAC_CODE_802_3 | AT91C_EMAC_RW_W |
			(RegisterAddress << 18) | *pOutput;

	udelay (10000);

	return TRUE;
}

int eth_init (bd_t * bd)
{
	int ret;
	int i;

	p_mac = AT91C_BASE_EMAC;

	/* PIO Disable Register */
	*AT91C_PIOA_PDR = AT91C_PA16_EMDIO | AT91C_PA15_EMDC | AT91C_PA14_ERXER |
			  AT91C_PA13_ERX1 | AT91C_PA12_ERX0 | AT91C_PA11_ECRS_ECRSDV |
			  AT91C_PA10_ETX1 | AT91C_PA9_ETX0 | AT91C_PA8_ETXEN |
			  AT91C_PA7_ETXCK_EREFCK;

#ifdef CONFIG_AT91C_USE_RMII
	*AT91C_PIOB_PDR = AT91C_PB19_ERXCK;
	*AT91C_PIOB_BSR = AT91C_PB19_ERXCK;
#else
	*AT91C_PIOB_PDR = AT91C_PB19_ERXCK | AT91C_PB18_ECOL | AT91C_PB17_ERXDV |
			  AT91C_PB16_ERX3 | AT91C_PB15_ERX2 | AT91C_PB14_ETXER |
			  AT91C_PB13_ETX3 | AT91C_PB12_ETX2;

	/* Select B Register */
	*AT91C_PIOB_BSR = AT91C_PB19_ERXCK | AT91C_PB18_ECOL |
			  AT91C_PB17_ERXDV | AT91C_PB16_ERX3 | AT91C_PB15_ERX2 |
			  AT91C_PB14_ETXER | AT91C_PB13_ETX3 | AT91C_PB12_ETX2;
#endif

	*AT91C_PMC_PCER = 1 << AT91C_ID_EMAC;	/* Peripheral Clock Enable Register */

	p_mac->EMAC_CFG |= AT91C_EMAC_CSR;	/* Clear statistics */

	/* Init Ehternet buffers */
	for (i = 0; i < RBF_FRAMEMAX; i++) {
		rbfdt[i].addr = (unsigned long)rbf_framebuf[i];
		rbfdt[i].size = 0;
	}
	rbfdt[RBF_FRAMEMAX - 1].addr |= RBF_WRAP;
	rbfp = &rbfdt[0];

	p_mac->EMAC_SA2L = (bd->bi_enetaddr[3] << 24) | (bd->bi_enetaddr[2] << 16)
			 | (bd->bi_enetaddr[1] <<  8) | (bd->bi_enetaddr[0]);
	p_mac->EMAC_SA2H = (bd->bi_enetaddr[5] <<  8) | (bd->bi_enetaddr[4]);

	p_mac->EMAC_RBQP = (long) (&rbfdt[0]);
	p_mac->EMAC_RSR &= ~(AT91C_EMAC_RSR_OVR | AT91C_EMAC_REC | AT91C_EMAC_BNA);

	p_mac->EMAC_CFG = (p_mac->EMAC_CFG | AT91C_EMAC_CAF | AT91C_EMAC_NBC)
			& ~AT91C_EMAC_CLK;

#ifdef CONFIG_AT91C_USE_RMII
	p_mac->EMAC_CFG |= AT91C_EMAC_RMII;
#endif

#if (AT91C_MASTER_CLOCK > 40000000)
	/* MDIO clock must not exceed 2.5 MHz, so enable MCK divider */
	p_mac->EMAC_CFG |= AT91C_EMAC_CLK_HCLK_64;
#endif

	p_mac->EMAC_CTL |= AT91C_EMAC_TE | AT91C_EMAC_RE;

	at91rm9200_GetPhyInterface (& PhyOps);

	if (!PhyOps.IsPhyConnected (p_mac))
		printf ("PHY not connected!!\n\r");

	/* MII management start from here */
	if (!(p_mac->EMAC_SR & AT91C_EMAC_LINK)) {
		if (!(ret = PhyOps.Init (p_mac))) {
			printf ("MAC: error during MII initialization\n");
			return 0;
		}
	} else {
		printf ("No link\n\r");
		return 0;
	}

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

#if defined(CONFIG_MII) || (CONFIG_COMMANDS & CFG_CMD_MII) || defined(CONFIG_CMD_MII)
int  at91rm9200_miiphy_read(char *devname, unsigned char addr,
		unsigned char reg, unsigned short * value)
{
	at91rm9200_EmacEnableMDIO (p_mac);
	at91rm9200_EmacReadPhy (p_mac, reg, value);
	at91rm9200_EmacDisableMDIO (p_mac);
	return 0;
}

int  at91rm9200_miiphy_write(char *devname, unsigned char addr,
		unsigned char reg, unsigned short value)
{
	at91rm9200_EmacEnableMDIO (p_mac);
	at91rm9200_EmacWritePhy (p_mac, reg, &value);
	at91rm9200_EmacDisableMDIO (p_mac);
	return 0;
}

#endif	/* defined(CONFIG_MII) || (CONFIG_COMMANDS & CFG_CMD_MII) */

int at91rm9200_miiphy_initialize(bd_t *bis)
{
#if defined(CONFIG_MII) || (CONFIG_COMMANDS & CFG_CMD_MII) || defined(CONFIG_CMD_MII)
	miiphy_register("at91rm9200phy", at91rm9200_miiphy_read, at91rm9200_miiphy_write);
#endif
	return 0;
}

#endif	/* CONFIG_COMMANDS & CFG_CMD_NET */

#endif	/* CONFIG_DRIVER_ETHER */
