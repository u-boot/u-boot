/* Gaisler.com GRETH 10/100/1000 Ethernet MAC driver
 *
 * Driver use polling mode (no Interrupt)
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com
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

/* #define DEBUG */

#include <common.h>
#include <command.h>
#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <asm/processor.h>
#include <ambapp.h>
#include <asm/leon.h>

#include "greth.h"

/* Default to 3s timeout on autonegotiation */
#ifndef GRETH_PHY_TIMEOUT_MS
#define GRETH_PHY_TIMEOUT_MS 3000
#endif

/* Default to PHY adrress 0 not not specified */
#ifdef CONFIG_SYS_GRLIB_GRETH_PHYADDR
#define GRETH_PHY_ADR_DEFAULT CONFIG_SYS_GRLIB_GRETH_PHYADDR
#else
#define GRETH_PHY_ADR_DEFAULT 0
#endif

/* ByPass Cache when reading regs */
#define GRETH_REGLOAD(addr)		SPARC_NOCACHE_READ(addr)
/* Write-through cache ==> no bypassing needed on writes */
#define GRETH_REGSAVE(addr,data) (*(volatile unsigned int *)(addr) = (data))
#define GRETH_REGORIN(addr,data) GRETH_REGSAVE(addr,GRETH_REGLOAD(addr)|data)
#define GRETH_REGANDIN(addr,data) GRETH_REGSAVE(addr,GRETH_REGLOAD(addr)&data)

#define GRETH_RXBD_CNT 4
#define GRETH_TXBD_CNT 1

#define GRETH_RXBUF_SIZE 1540
#define GRETH_BUF_ALIGN 4
#define GRETH_RXBUF_EFF_SIZE \
	( (GRETH_RXBUF_SIZE&~(GRETH_BUF_ALIGN-1))+GRETH_BUF_ALIGN )

typedef struct {
	greth_regs *regs;
	int irq;
	struct eth_device *dev;

	/* Hardware info */
	unsigned char phyaddr;
	int gbit_mac;

	/* Current operating Mode */
	int gb;			/* GigaBit */
	int fd;			/* Full Duplex */
	int sp;			/* 10/100Mbps speed (1=100,0=10) */
	int auto_neg;		/* Auto negotiate done */

	unsigned char hwaddr[6];	/* MAC Address */

	/* Descriptors */
	greth_bd *rxbd_base, *rxbd_max;
	greth_bd *txbd_base, *txbd_max;

	greth_bd *rxbd_curr;

	/* rx buffers in rx descriptors */
	void *rxbuf_base;	/* (GRETH_RXBUF_SIZE+ALIGNBYTES) * GRETH_RXBD_CNT */

	/* unused for gbit_mac, temp buffer for sending packets with unligned
	 * start.
	 * Pointer to packet allocated with malloc.
	 */
	void *txbuf;

	struct {
		/* rx status */
		unsigned int rx_packets,
		    rx_crc_errors, rx_frame_errors, rx_length_errors, rx_errors;

		/* tx stats */
		unsigned int tx_packets,
		    tx_latecol_errors,
		    tx_underrun_errors, tx_limit_errors, tx_errors;
	} stats;
} greth_priv;

/* Read MII register 'addr' from core 'regs' */
static int read_mii(int phyaddr, int regaddr, volatile greth_regs * regs)
{
	while (GRETH_REGLOAD(&regs->mdio) & GRETH_MII_BUSY) {
	}

	GRETH_REGSAVE(&regs->mdio, ((phyaddr & 0x1F) << 11) | ((regaddr & 0x1F) << 6) | 2);

	while (GRETH_REGLOAD(&regs->mdio) & GRETH_MII_BUSY) {
	}

	if (!(GRETH_REGLOAD(&regs->mdio) & GRETH_MII_NVALID)) {
		return (GRETH_REGLOAD(&regs->mdio) >> 16) & 0xFFFF;
	} else {
		return -1;
	}
}

static void write_mii(int phyaddr, int regaddr, int data, volatile greth_regs * regs)
{
	while (GRETH_REGLOAD(&regs->mdio) & GRETH_MII_BUSY) {
	}

	GRETH_REGSAVE(&regs->mdio,
		      ((data & 0xFFFF) << 16) | ((phyaddr & 0x1F) << 11) |
		      ((regaddr & 0x1F) << 6) | 1);

	while (GRETH_REGLOAD(&regs->mdio) & GRETH_MII_BUSY) {
	}

}

/* init/start hardware and allocate descriptor buffers for rx side
 *
 */
int greth_init(struct eth_device *dev, bd_t * bis)
{
	int i;

	greth_priv *greth = dev->priv;
	greth_regs *regs = greth->regs;

	debug("greth_init\n");

	/* Reset core */
	GRETH_REGSAVE(&regs->control, (GRETH_RESET | (greth->gb << 8) |
		(greth->sp << 7) | (greth->fd << 4)));

	/* Wait for Reset to complete */
	while ( GRETH_REGLOAD(&regs->control) & GRETH_RESET) ;

	GRETH_REGSAVE(&regs->control,
		((greth->gb << 8) | (greth->sp << 7) | (greth->fd << 4)));

	if (!greth->rxbd_base) {

		/* allocate descriptors */
		greth->rxbd_base = (greth_bd *)
		    memalign(0x1000, GRETH_RXBD_CNT * sizeof(greth_bd));
		greth->txbd_base = (greth_bd *)
		    memalign(0x1000, GRETH_TXBD_CNT * sizeof(greth_bd));

		/* allocate buffers to all descriptors  */
		greth->rxbuf_base =
		    malloc(GRETH_RXBUF_EFF_SIZE * GRETH_RXBD_CNT);
	}

	/* initate rx decriptors */
	for (i = 0; i < GRETH_RXBD_CNT; i++) {
		greth->rxbd_base[i].addr = (unsigned int)
		    greth->rxbuf_base + (GRETH_RXBUF_EFF_SIZE * i);
		/* enable desciptor & set wrap bit if last descriptor */
		if (i >= (GRETH_RXBD_CNT - 1)) {
			greth->rxbd_base[i].stat = GRETH_BD_EN | GRETH_BD_WR;
		} else {
			greth->rxbd_base[i].stat = GRETH_BD_EN;
		}
	}

	/* initiate indexes */
	greth->rxbd_curr = greth->rxbd_base;
	greth->rxbd_max = greth->rxbd_base + (GRETH_RXBD_CNT - 1);
	greth->txbd_max = greth->txbd_base + (GRETH_TXBD_CNT - 1);
	/*
	 * greth->txbd_base->addr = 0;
	 * greth->txbd_base->stat = GRETH_BD_WR;
	 */

	/* initate tx decriptors */
	for (i = 0; i < GRETH_TXBD_CNT; i++) {
		greth->txbd_base[i].addr = 0;
		/* enable desciptor & set wrap bit if last descriptor */
		if (i >= (GRETH_TXBD_CNT - 1)) {
			greth->txbd_base[i].stat = GRETH_BD_WR;
		} else {
			greth->txbd_base[i].stat = 0;
		}
	}

	/**** SET HARDWARE REGS ****/

	/* Set pointer to tx/rx descriptor areas */
	GRETH_REGSAVE(&regs->rx_desc_p, (unsigned int)&greth->rxbd_base[0]);
	GRETH_REGSAVE(&regs->tx_desc_p, (unsigned int)&greth->txbd_base[0]);

	/* Enable Transmitter, GRETH will now scan descriptors for packets
	 * to transmitt */
	debug("greth_init: enabling receiver\n");
	GRETH_REGORIN(&regs->control, GRETH_RXEN);

	return 0;
}

/* Initiate PHY to a relevant speed
 * return:
 *  - 0 = success
 *  - 1 = timeout/fail
 */
int greth_init_phy(greth_priv * dev, bd_t * bis)
{
	greth_regs *regs = dev->regs;
	int tmp, tmp1, tmp2, i;
	unsigned int start, timeout;
	int phyaddr = GRETH_PHY_ADR_DEFAULT;

#ifndef CONFIG_SYS_GRLIB_GRETH_PHYADDR
	/* If BSP doesn't provide a hardcoded PHY address the driver will
	 * try to autodetect PHY address by stopping the search on the first
	 * PHY address which has REG0 implemented.
	 */
	for (i=0; i<32; i++) {
		tmp = read_mii(i, 0, regs);
		if ( (tmp != 0) && (tmp != 0xffff) ) {
			phyaddr = i;
			break;
		}
	}
#endif

	/* Save PHY Address */
	dev->phyaddr = phyaddr;

	debug("GRETH PHY ADDRESS: %d\n", phyaddr);

	/* X msecs to ticks */
	timeout = usec2ticks(GRETH_PHY_TIMEOUT_MS * 1000);

	/* Get system timer0 current value
	 * Total timeout is 5s
	 */
	start = get_timer(0);

	/* get phy control register default values */

	while ((tmp = read_mii(phyaddr, 0, regs)) & 0x8000) {
		if (get_timer(start) > timeout) {
			debug("greth_init_phy: PHY read 1 failed\n");
			return 1;	/* Fail */
		}
	}

	/* reset PHY and wait for completion */
	write_mii(phyaddr, 0, 0x8000 | tmp, regs);

	while (((tmp = read_mii(phyaddr, 0, regs))) & 0x8000) {
		if (get_timer(start) > timeout) {
			debug("greth_init_phy: PHY read 2 failed\n");
			return 1;	/* Fail */
		}
	}

	/* Check if PHY is autoneg capable and then determine operating
	 * mode, otherwise force it to 10 Mbit halfduplex
	 */
	dev->gb = 0;
	dev->fd = 0;
	dev->sp = 0;
	dev->auto_neg = 0;
	if (!((tmp >> 12) & 1)) {
		write_mii(phyaddr, 0, 0, regs);
	} else {
		/* wait for auto negotiation to complete and then check operating mode */
		dev->auto_neg = 1;
		i = 0;
		while (!(((tmp = read_mii(phyaddr, 1, regs)) >> 5) & 1)) {
			if (get_timer(start) > timeout) {
				printf("Auto negotiation timed out. "
				       "Selecting default config\n");
				tmp = read_mii(phyaddr, 0, regs);
				dev->gb = ((tmp >> 6) & 1)
				    && !((tmp >> 13) & 1);
				dev->sp = !((tmp >> 6) & 1)
				    && ((tmp >> 13) & 1);
				dev->fd = (tmp >> 8) & 1;
				goto auto_neg_done;
			}
		}
		if ((tmp >> 8) & 1) {
			tmp1 = read_mii(phyaddr, 9, regs);
			tmp2 = read_mii(phyaddr, 10, regs);
			if ((tmp1 & GRETH_MII_EXTADV_1000FD) &&
			    (tmp2 & GRETH_MII_EXTPRT_1000FD)) {
				dev->gb = 1;
				dev->fd = 1;
			}
			if ((tmp1 & GRETH_MII_EXTADV_1000HD) &&
			    (tmp2 & GRETH_MII_EXTPRT_1000HD)) {
				dev->gb = 1;
				dev->fd = 0;
			}
		}
		if ((dev->gb == 0) || ((dev->gb == 1) && (dev->gbit_mac == 0))) {
			tmp1 = read_mii(phyaddr, 4, regs);
			tmp2 = read_mii(phyaddr, 5, regs);
			if ((tmp1 & GRETH_MII_100TXFD) &&
			    (tmp2 & GRETH_MII_100TXFD)) {
				dev->sp = 1;
				dev->fd = 1;
			}
			if ((tmp1 & GRETH_MII_100TXHD) &&
			    (tmp2 & GRETH_MII_100TXHD)) {
				dev->sp = 1;
				dev->fd = 0;
			}
			if ((tmp1 & GRETH_MII_10FD) && (tmp2 & GRETH_MII_10FD)) {
				dev->fd = 1;
			}
			if ((dev->gb == 1) && (dev->gbit_mac == 0)) {
				dev->gb = 0;
				dev->fd = 0;
				write_mii(phyaddr, 0, dev->sp << 13, regs);
			}
		}

	}
      auto_neg_done:
	debug("%s GRETH Ethermac at [0x%x] irq %d. Running \
		%d Mbps %s duplex\n", dev->gbit_mac ? "10/100/1000" : "10/100", (unsigned int)(regs), (unsigned int)(dev->irq), dev->gb ? 1000 : (dev->sp ? 100 : 10), dev->fd ? "full" : "half");
	/* Read out PHY info if extended registers are available */
	if (tmp & 1) {
		tmp1 = read_mii(phyaddr, 2, regs);
		tmp2 = read_mii(phyaddr, 3, regs);
		tmp1 = (tmp1 << 6) | ((tmp2 >> 10) & 0x3F);
		tmp = tmp2 & 0xF;

		tmp2 = (tmp2 >> 4) & 0x3F;
		debug("PHY: Vendor %x   Device %x    Revision %d\n", tmp1,
		       tmp2, tmp);
	} else {
		printf("PHY info not available\n");
	}

	/* set speed and duplex bits in control register */
	GRETH_REGORIN(&regs->control,
		      (dev->gb << 8) | (dev->sp << 7) | (dev->fd << 4));

	return 0;
}

void greth_halt(struct eth_device *dev)
{
	greth_priv *greth;
	greth_regs *regs;
	int i;

	debug("greth_halt\n");

	if (!dev || !dev->priv)
		return;

	greth = dev->priv;
	regs = greth->regs;

	if (!regs)
		return;

	/* disable receiver/transmitter by clearing the enable bits */
	GRETH_REGANDIN(&regs->control, ~(GRETH_RXEN | GRETH_TXEN));

	/* reset rx/tx descriptors */
	if (greth->rxbd_base) {
		for (i = 0; i < GRETH_RXBD_CNT; i++) {
			greth->rxbd_base[i].stat =
			    (i >= (GRETH_RXBD_CNT - 1)) ? GRETH_BD_WR : 0;
		}
	}

	if (greth->txbd_base) {
		for (i = 0; i < GRETH_TXBD_CNT; i++) {
			greth->txbd_base[i].stat =
			    (i >= (GRETH_TXBD_CNT - 1)) ? GRETH_BD_WR : 0;
		}
	}
}

int greth_send(struct eth_device *dev, volatile void *eth_data, int data_length)
{
	greth_priv *greth = dev->priv;
	greth_regs *regs = greth->regs;
	greth_bd *txbd;
	void *txbuf;
	unsigned int status;

	debug("greth_send\n");

	/* send data, wait for data to be sent, then return */
	if (((unsigned int)eth_data & (GRETH_BUF_ALIGN - 1))
	    && !greth->gbit_mac) {
		/* data not aligned as needed by GRETH 10/100, solve this by allocating 4 byte aligned buffer
		 * and copy data to before giving it to GRETH.
		 */
		if (!greth->txbuf) {
			greth->txbuf = malloc(GRETH_RXBUF_SIZE);
		}

		txbuf = greth->txbuf;

		/* copy data info buffer */
		memcpy((char *)txbuf, (char *)eth_data, data_length);

		/* keep buffer to next time */
	} else {
		txbuf = (void *)eth_data;
	}
	/* get descriptor to use, only 1 supported... hehe easy */
	txbd = greth->txbd_base;

	/* setup descriptor to wrap around to it self */
	txbd->addr = (unsigned int)txbuf;
	txbd->stat = GRETH_BD_EN | GRETH_BD_WR | data_length;

	/* Remind Core which descriptor to use when sending */
	GRETH_REGSAVE(&regs->tx_desc_p, (unsigned int)txbd);

	/* initate send by enabling transmitter */
	GRETH_REGORIN(&regs->control, GRETH_TXEN);

	/* Wait for data to be sent */
	while ((status = GRETH_REGLOAD(&txbd->stat)) & GRETH_BD_EN) {
		;
	}

	/* was the packet transmitted succesfully? */
	if (status & GRETH_TXBD_ERR_AL) {
		greth->stats.tx_limit_errors++;
	}

	if (status & GRETH_TXBD_ERR_UE) {
		greth->stats.tx_underrun_errors++;
	}

	if (status & GRETH_TXBD_ERR_LC) {
		greth->stats.tx_latecol_errors++;
	}

	if (status &
	    (GRETH_TXBD_ERR_LC | GRETH_TXBD_ERR_UE | GRETH_TXBD_ERR_AL)) {
		/* any error */
		greth->stats.tx_errors++;
		return -1;
	}

	/* bump tx packet counter */
	greth->stats.tx_packets++;

	/* return succefully */
	return 0;
}

int greth_recv(struct eth_device *dev)
{
	greth_priv *greth = dev->priv;
	greth_regs *regs = greth->regs;
	greth_bd *rxbd;
	unsigned int status, len = 0, bad;
	unsigned char *d;
	int enable = 0;
	int i;

	/* Receive One packet only, but clear as many error packets as there are
	 * available.
	 */
	{
		/* current receive descriptor */
		rxbd = greth->rxbd_curr;

		/* get status of next received packet */
		status = GRETH_REGLOAD(&rxbd->stat);

		bad = 0;

		/* stop if no more packets received */
		if (status & GRETH_BD_EN) {
			goto done;
		}

		debug("greth_recv: packet 0x%lx, 0x%lx, len: %d\n",
		       (unsigned int)rxbd, status, status & GRETH_BD_LEN);

		/* Check status for errors.
		 */
		if (status & GRETH_RXBD_ERR_FT) {
			greth->stats.rx_length_errors++;
			bad = 1;
		}
		if (status & (GRETH_RXBD_ERR_AE | GRETH_RXBD_ERR_OE)) {
			greth->stats.rx_frame_errors++;
			bad = 1;
		}
		if (status & GRETH_RXBD_ERR_CRC) {
			greth->stats.rx_crc_errors++;
			bad = 1;
		}
		if (bad) {
			greth->stats.rx_errors++;
			printf
			    ("greth_recv: Bad packet (%d, %d, %d, 0x%08x, %d)\n",
			     greth->stats.rx_length_errors,
			     greth->stats.rx_frame_errors,
			     greth->stats.rx_crc_errors, status,
			     greth->stats.rx_packets);
			/* print all rx descriptors */
			for (i = 0; i < GRETH_RXBD_CNT; i++) {
				printf("[%d]: Stat=0x%lx, Addr=0x%lx\n", i,
				       GRETH_REGLOAD(&greth->rxbd_base[i].stat),
				       GRETH_REGLOAD(&greth->rxbd_base[i].addr));
			}
		} else {
			/* Process the incoming packet. */
			len = status & GRETH_BD_LEN;
			d = (char *)rxbd->addr;

			debug
			    ("greth_recv: new packet, length: %d. data: %x %x %x %x %x %x %x %x\n",
			     len, d[0], d[1], d[2], d[3], d[4], d[5], d[6],
			     d[7]);

			/* flush all data cache to make sure we're not reading old packet data */
			sparc_dcache_flush_all();

			/* pass packet on to network subsystem */
			NetReceive((void *)d, len);

			/* bump stats counters */
			greth->stats.rx_packets++;

			/* bad is now 0 ==> will stop loop */
		}

		/* reenable descriptor to receive more packet with this descriptor, wrap around if needed */
		rxbd->stat =
		    GRETH_BD_EN |
		    (((unsigned int)greth->rxbd_curr >=
		      (unsigned int)greth->rxbd_max) ? GRETH_BD_WR : 0);
		enable = 1;

		/* increase index */
		greth->rxbd_curr =
		    ((unsigned int)greth->rxbd_curr >=
		     (unsigned int)greth->rxbd_max) ? greth->
		    rxbd_base : (greth->rxbd_curr + 1);

	}

	if (enable) {
		GRETH_REGORIN(&regs->control, GRETH_RXEN);
	}
      done:
	/* return positive length of packet or 0 if non received */
	return len;
}

void greth_set_hwaddr(greth_priv * greth, unsigned char *mac)
{
	/* save new MAC address */
	greth->dev->enetaddr[0] = greth->hwaddr[0] = mac[0];
	greth->dev->enetaddr[1] = greth->hwaddr[1] = mac[1];
	greth->dev->enetaddr[2] = greth->hwaddr[2] = mac[2];
	greth->dev->enetaddr[3] = greth->hwaddr[3] = mac[3];
	greth->dev->enetaddr[4] = greth->hwaddr[4] = mac[4];
	greth->dev->enetaddr[5] = greth->hwaddr[5] = mac[5];
	greth->regs->esa_msb = (mac[0] << 8) | mac[1];
	greth->regs->esa_lsb =
	    (mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5];

	debug("GRETH: New MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
	       mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

int greth_initialize(bd_t * bis)
{
	greth_priv *greth;
	ambapp_apbdev apbdev;
	struct eth_device *dev;
	int i;
	char *addr_str, *end;
	unsigned char addr[6];

	debug("Scanning for GRETH\n");

	/* Find Device & IRQ via AMBA Plug&Play information */
	if (ambapp_apb_first(VENDOR_GAISLER, GAISLER_ETHMAC, &apbdev) != 1) {
		return -1;	/* GRETH not found */
	}

	greth = (greth_priv *) malloc(sizeof(greth_priv));
	dev = (struct eth_device *)malloc(sizeof(struct eth_device));
	memset(dev, 0, sizeof(struct eth_device));
	memset(greth, 0, sizeof(greth_priv));

	greth->regs = (greth_regs *) apbdev.address;
	greth->irq = apbdev.irq;
	debug("Found GRETH at 0x%lx, irq %d\n", greth->regs, greth->irq);
	dev->priv = (void *)greth;
	dev->iobase = (unsigned int)greth->regs;
	dev->init = greth_init;
	dev->halt = greth_halt;
	dev->send = greth_send;
	dev->recv = greth_recv;
	greth->dev = dev;

	/* Reset Core */
	GRETH_REGSAVE(&greth->regs->control, GRETH_RESET);

	/* Wait for core to finish reset cycle */
	while (GRETH_REGLOAD(&greth->regs->control) & GRETH_RESET) ;

	/* Get the phy address which assumed to have been set
	   correctly with the reset value in hardware */
	greth->phyaddr = (GRETH_REGLOAD(&greth->regs->mdio) >> 11) & 0x1F;

	/* Check if mac is gigabit capable */
	greth->gbit_mac = (GRETH_REGLOAD(&greth->regs->control) >> 27) & 1;

	/* Make descriptor string */
	if (greth->gbit_mac) {
		sprintf(dev->name, "GRETH_10/100/GB");
	} else {
		sprintf(dev->name, "GRETH_10/100");
	}

	/* initiate PHY, select speed/duplex depending on connected PHY */
	if (greth_init_phy(greth, bis)) {
		/* Failed to init PHY (timedout) */
		debug("GRETH[0x%08x]: Failed to init PHY\n", greth->regs);
		return -1;
	}

	/* Register Device to EtherNet subsystem  */
	eth_register(dev);

	/* Get MAC address */
	if ((addr_str = getenv("ethaddr")) != NULL) {
		for (i = 0; i < 6; i++) {
			addr[i] =
			    addr_str ? simple_strtoul(addr_str, &end, 16) : 0;
			if (addr_str) {
				addr_str = (*end) ? end + 1 : end;
			}
		}
	} else {
		/* HW Address not found in environment, Set default HW address */
		addr[0] = GRETH_HWADDR_0;	/* MSB */
		addr[1] = GRETH_HWADDR_1;
		addr[2] = GRETH_HWADDR_2;
		addr[3] = GRETH_HWADDR_3;
		addr[4] = GRETH_HWADDR_4;
		addr[5] = GRETH_HWADDR_5;	/* LSB */
	}

	/* set and remember MAC address */
	greth_set_hwaddr(greth, addr);

	debug("GRETH[0x%08x]: Initialized successfully\n", greth->regs);
	return 0;
}
