/*
 * Ethernet driver for TI TMS320DM644x (DaVinci) chips.
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Parts shamelessly stolen from TI's dm644x_emac.c. Original copyright
 * follows:
 *
 * ----------------------------------------------------------------------------
 *
 * dm644x_emac.c
 *
 * TI DaVinci (DM644X) EMAC peripheral driver source for DV-EVM
 *
 * Copyright (C) 2005 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------

 * Modifications:
 * ver. 1.0: Sep 2005, Anant Gole - Created EMAC version for uBoot.
 * ver  1.1: Nov 2005, Anant Gole - Extended the RX logic for multiple descriptors
 *
 */
#include <common.h>
#include <command.h>
#include <net.h>
#include <miiphy.h>
#include <malloc.h>
#include <asm/arch/emac_defs.h>
#include <asm/io.h>

unsigned int	emac_dbg = 0;
#define debug_emac(fmt,args...)	if (emac_dbg) printf(fmt,##args)

#ifdef DAVINCI_EMAC_GIG_ENABLE
#define emac_gigabit_enable()	davinci_eth_gigabit_enable()
#else
#define emac_gigabit_enable()	/* no gigabit to enable */
#endif

static void davinci_eth_mdio_enable(void);

static int gen_init_phy(int phy_addr);
static int gen_is_phy_connected(int phy_addr);
static int gen_get_link_speed(int phy_addr);
static int gen_auto_negotiate(int phy_addr);

void eth_mdio_enable(void)
{
	davinci_eth_mdio_enable();
}

static u_int8_t davinci_eth_mac_addr[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

/*
 * This function must be called before emac_open() if you want to override
 * the default mac address.
 */
void davinci_eth_set_mac_addr(const u_int8_t *addr)
{
	int i;

	for (i = 0; i < sizeof (davinci_eth_mac_addr); i++) {
		davinci_eth_mac_addr[i] = addr[i];
	}
}

/* EMAC Addresses */
static volatile emac_regs	*adap_emac = (emac_regs *)EMAC_BASE_ADDR;
static volatile ewrap_regs	*adap_ewrap = (ewrap_regs *)EMAC_WRAPPER_BASE_ADDR;
static volatile mdio_regs	*adap_mdio = (mdio_regs *)EMAC_MDIO_BASE_ADDR;

/* EMAC descriptors */
static volatile emac_desc	*emac_rx_desc = (emac_desc *)(EMAC_WRAPPER_RAM_ADDR + EMAC_RX_DESC_BASE);
static volatile emac_desc	*emac_tx_desc = (emac_desc *)(EMAC_WRAPPER_RAM_ADDR + EMAC_TX_DESC_BASE);
static volatile emac_desc	*emac_rx_active_head = 0;
static volatile emac_desc	*emac_rx_active_tail = 0;
static int			emac_rx_queue_active = 0;

/* Receive packet buffers */
static unsigned char		emac_rx_buffers[EMAC_MAX_RX_BUFFERS * (EMAC_MAX_ETHERNET_PKT_SIZE + EMAC_PKT_ALIGN)];

/* PHY address for a discovered PHY (0xff - not found) */
static volatile u_int8_t	active_phy_addr = 0xff;

phy_t				phy;

static void davinci_eth_mdio_enable(void)
{
	u_int32_t	clkdiv;

	clkdiv = (EMAC_MDIO_BUS_FREQ / EMAC_MDIO_CLOCK_FREQ) - 1;

	writel((clkdiv & 0xff) |
	       MDIO_CONTROL_ENABLE |
	       MDIO_CONTROL_FAULT |
	       MDIO_CONTROL_FAULT_ENABLE,
	       &adap_mdio->CONTROL);

	while (readl(&adap_mdio->CONTROL) & MDIO_CONTROL_IDLE)
		;
}

/*
 * Tries to find an active connected PHY. Returns 1 if address if found.
 * If no active PHY (or more than one PHY) found returns 0.
 * Sets active_phy_addr variable.
 */
static int davinci_eth_phy_detect(void)
{
	u_int32_t	phy_act_state;
	int		i;

	active_phy_addr = 0xff;

	phy_act_state = readl(&adap_mdio->ALIVE) & EMAC_MDIO_PHY_MASK;
	if (phy_act_state == 0)
		return(0);				/* No active PHYs */

	debug_emac("davinci_eth_phy_detect(), ALIVE = 0x%08x\n", phy_act_state);

	for (i = 0; i < 32; i++) {
		if (phy_act_state & (1 << i)) {
			if (phy_act_state & ~(1 << i))
				return(0);		/* More than one PHY */
			else {
				active_phy_addr = i;
				return(1);
			}
		}
	}

	return(0);	/* Just to make GCC happy */
}


/* Read a PHY register via MDIO inteface. Returns 1 on success, 0 otherwise */
int davinci_eth_phy_read(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t *data)
{
	int	tmp;

	while (readl(&adap_mdio->USERACCESS0) & MDIO_USERACCESS0_GO)
		;

	writel(MDIO_USERACCESS0_GO |
	       MDIO_USERACCESS0_WRITE_READ |
	       ((reg_num & 0x1f) << 21) |
	       ((phy_addr & 0x1f) << 16),
	       &adap_mdio->USERACCESS0);

	/* Wait for command to complete */
	while ((tmp = readl(&adap_mdio->USERACCESS0)) & MDIO_USERACCESS0_GO)
		;

	if (tmp & MDIO_USERACCESS0_ACK) {
		*data = tmp & 0xffff;
		return(1);
	}

	*data = -1;
	return(0);
}

/* Write to a PHY register via MDIO inteface. Blocks until operation is complete. */
int davinci_eth_phy_write(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t data)
{

	while (readl(&adap_mdio->USERACCESS0) & MDIO_USERACCESS0_GO)
		;

	writel(MDIO_USERACCESS0_GO |
	       MDIO_USERACCESS0_WRITE_WRITE |
	       ((reg_num & 0x1f) << 21) |
	       ((phy_addr & 0x1f) << 16) |
	       (data & 0xffff),
	       &adap_mdio->USERACCESS0);

	/* Wait for command to complete */
	while (readl(&adap_mdio->USERACCESS0) & MDIO_USERACCESS0_GO)
		;

	return(1);
}

/* PHY functions for a generic PHY */
static int gen_init_phy(int phy_addr)
{
	int	ret = 1;

	if (gen_get_link_speed(phy_addr)) {
		/* Try another time */
		ret = gen_get_link_speed(phy_addr);
	}

	return(ret);
}

static int gen_is_phy_connected(int phy_addr)
{
	u_int16_t	dummy;

	return(davinci_eth_phy_read(phy_addr, PHY_PHYIDR1, &dummy));
}

static int gen_get_link_speed(int phy_addr)
{
	u_int16_t	tmp;

	if (davinci_eth_phy_read(phy_addr, MII_STATUS_REG, &tmp) && (tmp & 0x04))
		return(1);

	return(0);
}

static int gen_auto_negotiate(int phy_addr)
{
	u_int16_t	tmp;

	if (!davinci_eth_phy_read(phy_addr, PHY_BMCR, &tmp))
		return(0);

	/* Restart Auto_negotiation  */
	tmp |= PHY_BMCR_AUTON;
	davinci_eth_phy_write(phy_addr, PHY_BMCR, tmp);

	/*check AutoNegotiate complete */
	udelay (10000);
	if (!davinci_eth_phy_read(phy_addr, PHY_BMSR, &tmp))
		return(0);

	if (!(tmp & PHY_BMSR_AUTN_COMP))
		return(0);

	return(gen_get_link_speed(phy_addr));
}
/* End of generic PHY functions */


#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
static int davinci_mii_phy_read(const char *devname, unsigned char addr, unsigned char reg, unsigned short *value)
{
	return(davinci_eth_phy_read(addr, reg, value) ? 0 : 1);
}

static int davinci_mii_phy_write(const char *devname, unsigned char addr, unsigned char reg, unsigned short value)
{
	return(davinci_eth_phy_write(addr, reg, value) ? 0 : 1);
}
#endif

static void  __attribute__((unused)) davinci_eth_gigabit_enable(void)
{
	u_int16_t data;

	if (davinci_eth_phy_read(EMAC_MDIO_PHY_NUM, 0, &data)) {
		if (data & (1 << 6)) { /* speed selection MSB */
			/*
			 * Check if link detected is giga-bit
			 * If Gigabit mode detected, enable gigbit in MAC
			 */
			writel(EMAC_MACCONTROL_GIGFORCE |
			       EMAC_MACCONTROL_GIGABIT_ENABLE,
			       &adap_emac->MACCONTROL);
		}
	}
}

/* Eth device open */
static int davinci_eth_open(struct eth_device *dev, bd_t *bis)
{
	dv_reg_p		addr;
	u_int32_t		clkdiv, cnt;
	volatile emac_desc	*rx_desc;
	unsigned long		mac_hi;
	unsigned long		mac_lo;

	debug_emac("+ emac_open\n");

	/* Reset EMAC module and disable interrupts in wrapper */
	writel(1, &adap_emac->SOFTRESET);
	while (readl(&adap_emac->SOFTRESET) != 0)
		;
#if defined(DAVINCI_EMAC_VERSION2)
	writel(1, &adap_ewrap->softrst);
	while (readl(&adap_ewrap->softrst) != 0)
		;
#else
	writel(0, &adap_ewrap->EWCTL);
	for (cnt = 0; cnt < 5; cnt++) {
		clkdiv = readl(&adap_ewrap->EWCTL);
	}
#endif

	rx_desc = emac_rx_desc;

	writel(1, &adap_emac->TXCONTROL);
	writel(1, &adap_emac->RXCONTROL);

	/* Set MAC Addresses & Init multicast Hash to 0 (disable any multicast receive) */
	/* Using channel 0 only - other channels are disabled */
	writel(0, &adap_emac->MACINDEX);
	mac_hi = (davinci_eth_mac_addr[3] << 24) |
		 (davinci_eth_mac_addr[2] << 16) |
		 (davinci_eth_mac_addr[1] << 8)  |
		 (davinci_eth_mac_addr[0]);
	mac_lo = (davinci_eth_mac_addr[5] << 8) |
		 (davinci_eth_mac_addr[4]);

	writel(mac_hi, &adap_emac->MACADDRHI);
#if defined(DAVINCI_EMAC_VERSION2)
	writel(mac_lo | EMAC_MAC_ADDR_IS_VALID | EMAC_MAC_ADDR_MATCH,
	       &adap_emac->MACADDRLO);
#else
	writel(mac_lo, &adap_emac->MACADDRLO);
#endif

	writel(0, &adap_emac->MACHASH1);
	writel(0, &adap_emac->MACHASH2);

	/* Set source MAC address - REQUIRED */
	writel(mac_hi, &adap_emac->MACSRCADDRHI);
	writel(mac_lo, &adap_emac->MACSRCADDRLO);

	/* Set DMA 8 TX / 8 RX Head pointers to 0 */
	addr = &adap_emac->TX0HDP;
	for(cnt = 0; cnt < 16; cnt++)
		writel(0, addr++);

	addr = &adap_emac->RX0HDP;
	for(cnt = 0; cnt < 16; cnt++)
		writel(0, addr++);

	/* Clear Statistics (do this before setting MacControl register) */
	addr = &adap_emac->RXGOODFRAMES;
	for(cnt = 0; cnt < EMAC_NUM_STATS; cnt++)
		writel(0, addr++);

	/* No multicast addressing */
	writel(0, &adap_emac->MACHASH1);
	writel(0, &adap_emac->MACHASH2);

	/* Create RX queue and set receive process in place */
	emac_rx_active_head = emac_rx_desc;
	for (cnt = 0; cnt < EMAC_MAX_RX_BUFFERS; cnt++) {
		rx_desc->next = (u_int32_t)(rx_desc + 1);
		rx_desc->buffer = &emac_rx_buffers[cnt * (EMAC_MAX_ETHERNET_PKT_SIZE + EMAC_PKT_ALIGN)];
		rx_desc->buff_off_len = EMAC_MAX_ETHERNET_PKT_SIZE;
		rx_desc->pkt_flag_len = EMAC_CPPI_OWNERSHIP_BIT;
		rx_desc++;
	}

	/* Finalize the rx desc list */
	rx_desc--;
	rx_desc->next = 0;
	emac_rx_active_tail = rx_desc;
	emac_rx_queue_active = 1;

	/* Enable TX/RX */
	writel(EMAC_MAX_ETHERNET_PKT_SIZE, &adap_emac->RXMAXLEN);
	writel(0, &adap_emac->RXBUFFEROFFSET);

	/*
	 * No fancy configs - Use this for promiscous debug
	 *   - EMAC_RXMBPENABLE_RXCAFEN_ENABLE
	 */
	writel(EMAC_RXMBPENABLE_RXBROADEN, &adap_emac->RXMBPENABLE);

	/* Enable ch 0 only */
	writel(1, &adap_emac->RXUNICASTSET);

	/* Enable MII interface and Full duplex mode */
#ifdef CONFIG_SOC_DA8XX
	writel((EMAC_MACCONTROL_MIIEN_ENABLE |
		EMAC_MACCONTROL_FULLDUPLEX_ENABLE |
		EMAC_MACCONTROL_RMIISPEED_100),
	       &adap_emac->MACCONTROL);
#else
	writel((EMAC_MACCONTROL_MIIEN_ENABLE |
		EMAC_MACCONTROL_FULLDUPLEX_ENABLE),
	       &adap_emac->MACCONTROL);
#endif

	/* Init MDIO & get link state */
	clkdiv = (EMAC_MDIO_BUS_FREQ / EMAC_MDIO_CLOCK_FREQ) - 1;
	writel((clkdiv & 0xff) | MDIO_CONTROL_ENABLE | MDIO_CONTROL_FAULT,
	       &adap_mdio->CONTROL);

	/* We need to wait for MDIO to start */
	udelay(1000);

	if (!phy.get_link_speed(active_phy_addr))
		return(0);

	emac_gigabit_enable();

	/* Start receive process */
	writel((u_int32_t)emac_rx_desc, &adap_emac->RX0HDP);

	debug_emac("- emac_open\n");

	return(1);
}

/* EMAC Channel Teardown */
static void davinci_eth_ch_teardown(int ch)
{
	dv_reg		dly = 0xff;
	dv_reg		cnt;

	debug_emac("+ emac_ch_teardown\n");

	if (ch == EMAC_CH_TX) {
		/* Init TX channel teardown */
		writel(1, &adap_emac->TXTEARDOWN);
		do {
			/*
			 * Wait here for Tx teardown completion interrupt to
			 * occur. Note: A task delay can be called here to pend
			 * rather than occupying CPU cycles - anyway it has
			 * been found that teardown takes very few cpu cycles
			 * and does not affect functionality
			 */
			dly--;
			udelay(1);
			if (dly == 0)
				break;
			cnt = readl(&adap_emac->TX0CP);
		} while (cnt != 0xfffffffc);
		writel(cnt, &adap_emac->TX0CP);
		writel(0, &adap_emac->TX0HDP);
	} else {
		/* Init RX channel teardown */
		writel(1, &adap_emac->RXTEARDOWN);
		do {
			/*
			 * Wait here for Rx teardown completion interrupt to
			 * occur. Note: A task delay can be called here to pend
			 * rather than occupying CPU cycles - anyway it has
			 * been found that teardown takes very few cpu cycles
			 * and does not affect functionality
			 */
			dly--;
			udelay(1);
			if (dly == 0)
				break;
			cnt = readl(&adap_emac->RX0CP);
		} while (cnt != 0xfffffffc);
		writel(cnt, &adap_emac->RX0CP);
		writel(0, &adap_emac->RX0HDP);
	}

	debug_emac("- emac_ch_teardown\n");
}

/* Eth device close */
static void davinci_eth_close(struct eth_device *dev)
{
	debug_emac("+ emac_close\n");

	davinci_eth_ch_teardown(EMAC_CH_TX);	/* TX Channel teardown */
	davinci_eth_ch_teardown(EMAC_CH_RX);	/* RX Channel teardown */

	/* Reset EMAC module and disable interrupts in wrapper */
	writel(1, &adap_emac->SOFTRESET);
#if defined(DAVINCI_EMAC_VERSION2)
	writel(1, &adap_ewrap->softrst);
#else
	writel(0, &adap_ewrap->EWCTL);
#endif

	debug_emac("- emac_close\n");
}

static int tx_send_loop = 0;

/*
 * This function sends a single packet on the network and returns
 * positive number (number of bytes transmitted) or negative for error
 */
static int davinci_eth_send_packet (struct eth_device *dev,
					volatile void *packet, int length)
{
	int ret_status = -1;

	tx_send_loop = 0;

	/* Return error if no link */
	if (!phy.get_link_speed (active_phy_addr)) {
		printf ("WARN: emac_send_packet: No link\n");
		return (ret_status);
	}

	emac_gigabit_enable();

	/* Check packet size and if < EMAC_MIN_ETHERNET_PKT_SIZE, pad it up */
	if (length < EMAC_MIN_ETHERNET_PKT_SIZE) {
		length = EMAC_MIN_ETHERNET_PKT_SIZE;
	}

	/* Populate the TX descriptor */
	emac_tx_desc->next = 0;
	emac_tx_desc->buffer = (u_int8_t *) packet;
	emac_tx_desc->buff_off_len = (length & 0xffff);
	emac_tx_desc->pkt_flag_len = ((length & 0xffff) |
				      EMAC_CPPI_SOP_BIT |
				      EMAC_CPPI_OWNERSHIP_BIT |
				      EMAC_CPPI_EOP_BIT);
	/* Send the packet */
	writel((unsigned long)emac_tx_desc, &adap_emac->TX0HDP);

	/* Wait for packet to complete or link down */
	while (1) {
		if (!phy.get_link_speed (active_phy_addr)) {
			davinci_eth_ch_teardown (EMAC_CH_TX);
			return (ret_status);
		}

		emac_gigabit_enable();

		if (readl(&adap_emac->TXINTSTATRAW) & 0x01) {
			ret_status = length;
			break;
		}
		tx_send_loop++;
	}

	return (ret_status);
}

/*
 * This function handles receipt of a packet from the network
 */
static int davinci_eth_rcv_packet (struct eth_device *dev)
{
	volatile emac_desc *rx_curr_desc;
	volatile emac_desc *curr_desc;
	volatile emac_desc *tail_desc;
	int status, ret = -1;

	rx_curr_desc = emac_rx_active_head;
	status = rx_curr_desc->pkt_flag_len;
	if ((rx_curr_desc) && ((status & EMAC_CPPI_OWNERSHIP_BIT) == 0)) {
		if (status & EMAC_CPPI_RX_ERROR_FRAME) {
			/* Error in packet - discard it and requeue desc */
			printf ("WARN: emac_rcv_pkt: Error in packet\n");
		} else {
			NetReceive (rx_curr_desc->buffer,
				    (rx_curr_desc->buff_off_len & 0xffff));
			ret = rx_curr_desc->buff_off_len & 0xffff;
		}

		/* Ack received packet descriptor */
		writel((unsigned long)rx_curr_desc, &adap_emac->RX0CP);
		curr_desc = rx_curr_desc;
		emac_rx_active_head =
			(volatile emac_desc *) rx_curr_desc->next;

		if (status & EMAC_CPPI_EOQ_BIT) {
			if (emac_rx_active_head) {
				writel((unsigned long)emac_rx_active_head,
				       &adap_emac->RX0HDP);
			} else {
				emac_rx_queue_active = 0;
				printf ("INFO:emac_rcv_packet: RX Queue not active\n");
			}
		}

		/* Recycle RX descriptor */
		rx_curr_desc->buff_off_len = EMAC_MAX_ETHERNET_PKT_SIZE;
		rx_curr_desc->pkt_flag_len = EMAC_CPPI_OWNERSHIP_BIT;
		rx_curr_desc->next = 0;

		if (emac_rx_active_head == 0) {
			printf ("INFO: emac_rcv_pkt: active queue head = 0\n");
			emac_rx_active_head = curr_desc;
			emac_rx_active_tail = curr_desc;
			if (emac_rx_queue_active != 0) {
				writel((unsigned long)emac_rx_active_head,
				       &adap_emac->RX0HDP);
				printf ("INFO: emac_rcv_pkt: active queue head = 0, HDP fired\n");
				emac_rx_queue_active = 1;
			}
		} else {
			tail_desc = emac_rx_active_tail;
			emac_rx_active_tail = curr_desc;
			tail_desc->next = (unsigned int) curr_desc;
			status = tail_desc->pkt_flag_len;
			if (status & EMAC_CPPI_EOQ_BIT) {
				writel((unsigned long)curr_desc,
				       &adap_emac->RX0HDP);
				status &= ~EMAC_CPPI_EOQ_BIT;
				tail_desc->pkt_flag_len = status;
			}
		}
		return (ret);
	}
	return (0);
}

/*
 * This function initializes the emac hardware. It does NOT initialize
 * EMAC modules power or pin multiplexors, that is done by board_init()
 * much earlier in bootup process. Returns 1 on success, 0 otherwise.
 */
int davinci_emac_initialize(void)
{
	u_int32_t	phy_id;
	u_int16_t	tmp;
	int		i;
	struct eth_device *dev;

	dev = malloc(sizeof *dev);

	if (dev == NULL)
		return -1;

	memset(dev, 0, sizeof *dev);

	dev->iobase = 0;
	dev->init = davinci_eth_open;
	dev->halt = davinci_eth_close;
	dev->send = davinci_eth_send_packet;
	dev->recv = davinci_eth_rcv_packet;

	eth_register(dev);

	davinci_eth_mdio_enable();

	for (i = 0; i < 256; i++) {
		if (readl(&adap_mdio->ALIVE))
			break;
		udelay(10);
	}

	if (i >= 256) {
		printf("No ETH PHY detected!!!\n");
		return(0);
	}

	/* Find if a PHY is connected and get it's address */
	if (!davinci_eth_phy_detect())
		return(0);

	/* Get PHY ID and initialize phy_ops for a detected PHY */
	if (!davinci_eth_phy_read(active_phy_addr, PHY_PHYIDR1, &tmp)) {
		active_phy_addr = 0xff;
		return(0);
	}

	phy_id = (tmp << 16) & 0xffff0000;

	if (!davinci_eth_phy_read(active_phy_addr, PHY_PHYIDR2, &tmp)) {
		active_phy_addr = 0xff;
		return(0);
	}

	phy_id |= tmp & 0x0000ffff;

	switch (phy_id) {
		case PHY_LXT972:
			sprintf(phy.name, "LXT972 @ 0x%02x", active_phy_addr);
			phy.init = lxt972_init_phy;
			phy.is_phy_connected = lxt972_is_phy_connected;
			phy.get_link_speed = lxt972_get_link_speed;
			phy.auto_negotiate = lxt972_auto_negotiate;
			break;
		case PHY_DP83848:
			sprintf(phy.name, "DP83848 @ 0x%02x", active_phy_addr);
			phy.init = dp83848_init_phy;
			phy.is_phy_connected = dp83848_is_phy_connected;
			phy.get_link_speed = dp83848_get_link_speed;
			phy.auto_negotiate = dp83848_auto_negotiate;
			break;
		default:
			sprintf(phy.name, "GENERIC @ 0x%02x", active_phy_addr);
			phy.init = gen_init_phy;
			phy.is_phy_connected = gen_is_phy_connected;
			phy.get_link_speed = gen_get_link_speed;
			phy.auto_negotiate = gen_auto_negotiate;
	}

	printf("Ethernet PHY: %s\n", phy.name);

	miiphy_register(phy.name, davinci_mii_phy_read, davinci_mii_phy_write);
	return(1);
}
