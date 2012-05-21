/*
 * Copyright (C) 2009 BuS Elektronik GmbH & Co. KG
 * Jens Scharsig (esw@bus-elektronik.de)
 *
 * (C) Copyright 2003
 * Author : Hamid Ikdoumi (Atmel)

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
#include <asm/io.h>
#ifndef CONFIG_AT91_LEGACY
#include <asm/arch/hardware.h>
#include <asm/arch/at91_emac.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_pio.h>
#else
/* remove next 5 lines, if all RM9200 boards convert to at91 arch */
#include <asm/arch-at91/at91rm9200.h>
#include <asm/arch-at91/hardware.h>
#include <asm/arch-at91/at91_emac.h>
#include <asm/arch-at91/at91_pmc.h>
#include <asm/arch-at91/at91_pio.h>
#endif
#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <miiphy.h>
#include <linux/mii.h>

#undef MII_DEBUG
#undef ET_DEBUG

#if (CONFIG_SYS_RX_ETH_BUFFER > 1024)
#error AT91 EMAC supports max 1024 RX buffers. \
	Please decrease the CONFIG_SYS_RX_ETH_BUFFER value
#endif

#ifndef CONFIG_DRIVER_AT91EMAC_PHYADDR
#define CONFIG_DRIVER_AT91EMAC_PHYADDR	0
#endif

/* MDIO clock must not exceed 2.5 MHz, so enable MCK divider */
#if (AT91C_MASTER_CLOCK > 80000000)
	#define HCLK_DIV	AT91_EMAC_CFG_MCLK_64
#elif (AT91C_MASTER_CLOCK > 40000000)
	#define HCLK_DIV	AT91_EMAC_CFG_MCLK_32
#elif (AT91C_MASTER_CLOCK > 20000000)
	#define HCLK_DIV	AT91_EMAC_CFG_MCLK_16
#else
	#define HCLK_DIV	AT91_EMAC_CFG_MCLK_8
#endif

#ifdef ET_DEBUG
#define DEBUG_AT91EMAC	1
#else
#define DEBUG_AT91EMAC	0
#endif

#ifdef MII_DEBUG
#define DEBUG_AT91PHY	1
#else
#define DEBUG_AT91PHY	0
#endif

#ifndef CONFIG_DRIVER_AT91EMAC_QUIET
#define VERBOSEP	1
#else
#define VERBOSEP	0
#endif

#define RBF_ADDR      0xfffffffc
#define RBF_OWNER     (1<<0)
#define RBF_WRAP      (1<<1)
#define RBF_BROADCAST (1<<31)
#define RBF_MULTICAST (1<<30)
#define RBF_UNICAST   (1<<29)
#define RBF_EXTERNAL  (1<<28)
#define RBF_UNKNOWN   (1<<27)
#define RBF_SIZE      0x07ff
#define RBF_LOCAL4    (1<<26)
#define RBF_LOCAL3    (1<<25)
#define RBF_LOCAL2    (1<<24)
#define RBF_LOCAL1    (1<<23)

#define RBF_FRAMEMAX CONFIG_SYS_RX_ETH_BUFFER
#define RBF_FRAMELEN 0x600

typedef struct {
	unsigned long addr, size;
} rbf_t;

typedef struct {
	rbf_t 		rbfdt[RBF_FRAMEMAX];
	unsigned long	rbindex;
} emac_device;

void at91emac_EnableMDIO(at91_emac_t *at91mac)
{
	/* Mac CTRL reg set for MDIO enable */
	writel(readl(&at91mac->ctl) | AT91_EMAC_CTL_MPE, &at91mac->ctl);
}

void at91emac_DisableMDIO(at91_emac_t *at91mac)
{
	/* Mac CTRL reg set for MDIO disable */
	writel(readl(&at91mac->ctl) & ~AT91_EMAC_CTL_MPE, &at91mac->ctl);
}

int  at91emac_read(at91_emac_t *at91mac, unsigned char addr,
		unsigned char reg, unsigned short *value)
{
	unsigned long netstat;
	at91emac_EnableMDIO(at91mac);

	writel(AT91_EMAC_MAN_HIGH | AT91_EMAC_MAN_RW_R |
		AT91_EMAC_MAN_REGA(reg) | AT91_EMAC_MAN_CODE_802_3 |
		AT91_EMAC_MAN_PHYA(addr),
		&at91mac->man);

	do {
		netstat = readl(&at91mac->sr);
		debug_cond(DEBUG_AT91PHY, "poll SR %08lx\n", netstat);
	} while (!(netstat & AT91_EMAC_SR_IDLE));

	*value = readl(&at91mac->man) & AT91_EMAC_MAN_DATA_MASK;

	at91emac_DisableMDIO(at91mac);

	debug_cond(DEBUG_AT91PHY,
		"AT91PHY read %p REG(%d)=%x\n", at91mac, reg, *value);

	return 0;
}

int  at91emac_write(at91_emac_t *at91mac, unsigned char addr,
		unsigned char reg, unsigned short value)
{
	unsigned long netstat;
	debug_cond(DEBUG_AT91PHY,
		"AT91PHY write %p REG(%d)=%p\n", at91mac, reg, &value);

	at91emac_EnableMDIO(at91mac);

	writel(AT91_EMAC_MAN_HIGH | AT91_EMAC_MAN_RW_W |
		AT91_EMAC_MAN_REGA(reg) | AT91_EMAC_MAN_CODE_802_3 |
		AT91_EMAC_MAN_PHYA(addr) | (value & AT91_EMAC_MAN_DATA_MASK),
		&at91mac->man);

	do {
		netstat = readl(&at91mac->sr);
		debug_cond(DEBUG_AT91PHY, "poll SR %08lx\n", netstat);
	} while (!(netstat & AT91_EMAC_SR_IDLE));

	at91emac_DisableMDIO(at91mac);

	return 0;
}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)

at91_emac_t *get_emacbase_by_name(const char *devname)
{
	struct eth_device *netdev;

	netdev = eth_get_dev_by_name(devname);
	return (at91_emac_t *) netdev->iobase;
}

int  at91emac_mii_read(const char *devname, unsigned char addr,
		unsigned char reg, unsigned short *value)
{
	at91_emac_t *emac;

	emac = get_emacbase_by_name(devname);
	at91emac_read(emac , addr, reg, value);
	return 0;
}


int  at91emac_mii_write(const char *devname, unsigned char addr,
		unsigned char reg, unsigned short value)
{
	at91_emac_t *emac;

	emac = get_emacbase_by_name(devname);
	at91emac_write(emac, addr, reg, value);
	return 0;
}

#endif

static int at91emac_phy_reset(struct eth_device *netdev)
{
	int i;
	u16 status, adv;
	at91_emac_t *emac;

	emac = (at91_emac_t *) netdev->iobase;

	adv = ADVERTISE_CSMA | ADVERTISE_ALL;
	at91emac_write(emac, CONFIG_DRIVER_AT91EMAC_PHYADDR,
		MII_ADVERTISE, adv);
	debug_cond(VERBOSEP, "%s: Starting autonegotiation...\n", netdev->name);
	at91emac_write(emac, CONFIG_DRIVER_AT91EMAC_PHYADDR, MII_BMCR,
		(BMCR_ANENABLE | BMCR_ANRESTART));

	for (i = 0; i < 30000; i++) {
		at91emac_read(emac, CONFIG_DRIVER_AT91EMAC_PHYADDR,
			MII_BMSR, &status);
		if (status & BMSR_ANEGCOMPLETE)
			break;
		udelay(100);
	}

	if (status & BMSR_ANEGCOMPLETE) {
		debug_cond(VERBOSEP,
			"%s: Autonegotiation complete\n", netdev->name);
	} else {
		printf("%s: Autonegotiation timed out (status=0x%04x)\n",
		       netdev->name, status);
		return -1;
	}
	return 0;
}

static int at91emac_phy_init(struct eth_device *netdev)
{
	u16 phy_id, status, adv, lpa;
	int media, speed, duplex;
	int i;
	at91_emac_t *emac;

	emac = (at91_emac_t *) netdev->iobase;

	/* Check if the PHY is up to snuff... */
	at91emac_read(emac, CONFIG_DRIVER_AT91EMAC_PHYADDR,
		MII_PHYSID1, &phy_id);
	if (phy_id == 0xffff) {
		printf("%s: No PHY present\n", netdev->name);
		return -1;
	}

	at91emac_read(emac, CONFIG_DRIVER_AT91EMAC_PHYADDR,
		MII_BMSR, &status);

	if (!(status & BMSR_LSTATUS)) {
		/* Try to re-negotiate if we don't have link already. */
		if (at91emac_phy_reset(netdev))
			return -2;

		for (i = 0; i < 100000 / 100; i++) {
			at91emac_read(emac, CONFIG_DRIVER_AT91EMAC_PHYADDR,
				MII_BMSR, &status);
			if (status & BMSR_LSTATUS)
				break;
			udelay(100);
		}
	}
	if (!(status & BMSR_LSTATUS)) {
		debug_cond(VERBOSEP, "%s: link down\n", netdev->name);
		return -3;
	} else {
		at91emac_read(emac, CONFIG_DRIVER_AT91EMAC_PHYADDR,
			MII_ADVERTISE, &adv);
		at91emac_read(emac, CONFIG_DRIVER_AT91EMAC_PHYADDR,
			MII_LPA, &lpa);
		media = mii_nway_result(lpa & adv);
		speed = (media & (ADVERTISE_100FULL | ADVERTISE_100HALF)
			 ? 1 : 0);
		duplex = (media & ADVERTISE_FULL) ? 1 : 0;
		debug_cond(VERBOSEP, "%s: link up, %sMbps %s-duplex\n",
		       netdev->name,
		       speed ? "100" : "10",
		       duplex ? "full" : "half");
	}
	return 0;
}

int at91emac_UpdateLinkSpeed(at91_emac_t *emac)
{
	unsigned short stat1;

	at91emac_read(emac, CONFIG_DRIVER_AT91EMAC_PHYADDR, MII_BMSR, &stat1);

	if (!(stat1 & BMSR_LSTATUS))	/* link status up? */
		return -1;

	if (stat1 & BMSR_100FULL) {
		/*set Emac for 100BaseTX and Full Duplex  */
		writel(readl(&emac->cfg) |
			AT91_EMAC_CFG_SPD | AT91_EMAC_CFG_FD,
			&emac->cfg);
		return 0;
	}

	if (stat1 & BMSR_10FULL) {
		/*set MII for 10BaseT and Full Duplex  */
		writel((readl(&emac->cfg) &
			~(AT91_EMAC_CFG_SPD | AT91_EMAC_CFG_FD)
			) | AT91_EMAC_CFG_FD,
			&emac->cfg);
		return 0;
	}

	if (stat1 & BMSR_100HALF) {
		/*set MII for 100BaseTX and Half Duplex  */
		writel((readl(&emac->cfg) &
			~(AT91_EMAC_CFG_SPD | AT91_EMAC_CFG_FD)
			) | AT91_EMAC_CFG_SPD,
			&emac->cfg);
		return 0;
	}

	if (stat1 & BMSR_10HALF) {
		/*set MII for 10BaseT and Half Duplex  */
		writel((readl(&emac->cfg) &
			~(AT91_EMAC_CFG_SPD | AT91_EMAC_CFG_FD)),
			&emac->cfg);
		return 0;
	}
	return 0;
}

static int at91emac_init(struct eth_device *netdev, bd_t *bd)
{
	int i;
	u32 value;
	emac_device *dev;
	at91_emac_t *emac;
	at91_pio_t *pio = (at91_pio_t *) ATMEL_BASE_PIO;
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;

	emac = (at91_emac_t *) netdev->iobase;
	dev = (emac_device *) netdev->priv;

	/* PIO Disable Register */
	value =	ATMEL_PMX_AA_EMDIO |	ATMEL_PMX_AA_EMDC |
		ATMEL_PMX_AA_ERXER |	ATMEL_PMX_AA_ERX1 |
		ATMEL_PMX_AA_ERX0 |	ATMEL_PMX_AA_ECRS |
		ATMEL_PMX_AA_ETX1 |	ATMEL_PMX_AA_ETX0 |
		ATMEL_PMX_AA_ETXEN |	ATMEL_PMX_AA_EREFCK;

	writel(value, &pio->pioa.pdr);
	writel(value, &pio->pioa.asr);

#ifdef CONFIG_RMII
	value = ATMEL_PMX_BA_ERXCK;
#else
	value = ATMEL_PMX_BA_ERXCK |	ATMEL_PMX_BA_ECOL |
		ATMEL_PMX_BA_ERXDV |	ATMEL_PMX_BA_ERX3 |
		ATMEL_PMX_BA_ERX2 |	ATMEL_PMX_BA_ETXER |
		ATMEL_PMX_BA_ETX3 |	ATMEL_PMX_BA_ETX2;
#endif
	writel(value, &pio->piob.pdr);
	writel(value, &pio->piob.bsr);

	writel(1 << ATMEL_ID_EMAC, &pmc->pcer);
	writel(readl(&emac->ctl) | AT91_EMAC_CTL_CSR, &emac->ctl);

	/* Init Ethernet buffers */
	for (i = 0; i < RBF_FRAMEMAX; i++) {
		dev->rbfdt[i].addr = (unsigned long) NetRxPackets[i];
		dev->rbfdt[i].size = 0;
	}
	dev->rbfdt[RBF_FRAMEMAX - 1].addr |= RBF_WRAP;
	dev->rbindex = 0;
	writel((u32) &(dev->rbfdt[0]), &emac->rbqp);

	writel(readl(&emac->rsr) &
		~(AT91_EMAC_RSR_OVR | AT91_EMAC_RSR_REC | AT91_EMAC_RSR_BNA),
		&emac->rsr);

	value = AT91_EMAC_CFG_CAF |	AT91_EMAC_CFG_NBC |
		HCLK_DIV;
#ifdef CONFIG_RMII
	value |= AT91_EMAC_CFG_RMII;
#endif
	writel(value, &emac->cfg);

	writel(readl(&emac->ctl) | AT91_EMAC_CTL_TE | AT91_EMAC_CTL_RE,
		&emac->ctl);

	if (!at91emac_phy_init(netdev)) {
		at91emac_UpdateLinkSpeed(emac);
		return 0;
	}
	return -1;
}

static void at91emac_halt(struct eth_device *netdev)
{
	at91_emac_t *emac;

	emac = (at91_emac_t *) netdev->iobase;
	writel(readl(&emac->ctl) & ~(AT91_EMAC_CTL_TE | AT91_EMAC_CTL_RE),
		&emac->ctl);
	debug_cond(DEBUG_AT91EMAC, "halt MAC\n");
}

static int at91emac_send(struct eth_device *netdev, void *packet, int length)
{
	at91_emac_t *emac;

	emac = (at91_emac_t *) netdev->iobase;

	while (!(readl(&emac->tsr) & AT91_EMAC_TSR_BNQ))
		;
	writel((u32) packet, &emac->tar);
	writel(AT91_EMAC_TCR_LEN(length), &emac->tcr);
	while (AT91_EMAC_TCR_LEN(readl(&emac->tcr)))
		;
	debug_cond(DEBUG_AT91EMAC, "Send %d\n", length);
	writel(readl(&emac->tsr) | AT91_EMAC_TSR_COMP, &emac->tsr);
	return 0;
}

static int at91emac_recv(struct eth_device *netdev)
{
	emac_device *dev;
	at91_emac_t *emac;
	rbf_t *rbfp;
	int size;

	emac = (at91_emac_t *) netdev->iobase;
	dev = (emac_device *) netdev->priv;

	rbfp = &dev->rbfdt[dev->rbindex];
	while (rbfp->addr & RBF_OWNER)	{
		size = rbfp->size & RBF_SIZE;
		NetReceive(NetRxPackets[dev->rbindex], size);

		debug_cond(DEBUG_AT91EMAC, "Recv[%ld]: %d bytes @ %lx\n",
			dev->rbindex, size, rbfp->addr);

		rbfp->addr &= ~RBF_OWNER;
		rbfp->size = 0;
		if (dev->rbindex < (RBF_FRAMEMAX-1))
			dev->rbindex++;
		else
			dev->rbindex = 0;

		rbfp = &(dev->rbfdt[dev->rbindex]);
		if (!(rbfp->addr & RBF_OWNER))
			writel(readl(&emac->rsr) | AT91_EMAC_RSR_REC,
				&emac->rsr);
	}

	if (readl(&emac->isr) & AT91_EMAC_IxR_RBNA) {
		/* EMAC silicon bug 41.3.1 workaround 1 */
		writel(readl(&emac->ctl) & ~AT91_EMAC_CTL_RE, &emac->ctl);
		writel(readl(&emac->ctl) | AT91_EMAC_CTL_RE, &emac->ctl);
		dev->rbindex = 0;
		printf("%s: reset receiver (EMAC dead lock bug)\n",
			netdev->name);
	}
	return 0;
}

static int at91emac_write_hwaddr(struct eth_device *netdev)
{
	at91_emac_t *emac;
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;
	emac = (at91_emac_t *) netdev->iobase;

	writel(1 << ATMEL_ID_EMAC, &pmc->pcer);
	debug_cond(DEBUG_AT91EMAC,
		"init MAC-ADDR %02x:%02x:%02x:%02x:%02x:%02x\n",
		netdev->enetaddr[5], netdev->enetaddr[4], netdev->enetaddr[3],
		netdev->enetaddr[2], netdev->enetaddr[1], netdev->enetaddr[0]);
	writel( (netdev->enetaddr[0] | netdev->enetaddr[1] << 8 |
			netdev->enetaddr[2] << 16 | netdev->enetaddr[3] << 24),
			&emac->sa2l);
	writel((netdev->enetaddr[4] | netdev->enetaddr[5] << 8), &emac->sa2h);
	debug_cond(DEBUG_AT91EMAC, "init MAC-ADDR %x%x\n",
		readl(&emac->sa2h), readl(&emac->sa2l));
	return 0;
}

int at91emac_register(bd_t *bis, unsigned long iobase)
{
	emac_device *emac;
	emac_device *emacfix;
	struct eth_device *dev;

	if (iobase == 0)
		iobase = ATMEL_BASE_EMAC;
	emac = malloc(sizeof(*emac)+512);
	if (emac == NULL)
		return -1;
	dev = malloc(sizeof(*dev));
	if (dev == NULL) {
		free(emac);
		return -1;
	}
	/* alignment as per Errata (64 bytes) is insufficient! */
	emacfix = (emac_device *) (((unsigned long) emac + 0x1ff) & 0xFFFFFE00);
	memset(emacfix, 0, sizeof(emac_device));

	memset(dev, 0, sizeof(*dev));
	sprintf(dev->name, "emac");
	dev->iobase = iobase;
	dev->priv = emacfix;
	dev->init = at91emac_init;
	dev->halt = at91emac_halt;
	dev->send = at91emac_send;
	dev->recv = at91emac_recv;
	dev->write_hwaddr = at91emac_write_hwaddr;

	eth_register(dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	miiphy_register(dev->name, at91emac_mii_read, at91emac_mii_write);
#endif
	return 1;
}
