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
#include <asm/arch/emac_defs.h>

#ifdef CONFIG_DRIVER_TI_EMAC

#ifdef CONFIG_CMD_NET

unsigned int	emac_dbg = 0;
#define debug_emac(fmt,args...)	if (emac_dbg) printf(fmt,##args)

/* Internal static functions */
static int dm644x_eth_hw_init (void);
static int dm644x_eth_open (void);
static int dm644x_eth_close (void);
static int dm644x_eth_send_packet (volatile void *packet, int length);
static int dm644x_eth_rcv_packet (void);
static void dm644x_eth_mdio_enable(void);

static int gen_init_phy(int phy_addr);
static int gen_is_phy_connected(int phy_addr);
static int gen_get_link_speed(int phy_addr);
static int gen_auto_negotiate(int phy_addr);

/* Wrappers exported to the U-Boot proper */
int eth_hw_init(void)
{
	return(dm644x_eth_hw_init());
}

int eth_init(bd_t * bd)
{
	return(dm644x_eth_open());
}

void eth_halt(void)
{
	dm644x_eth_close();
}

int eth_send(volatile void *packet, int length)
{
	return(dm644x_eth_send_packet(packet, length));
}

int eth_rx(void)
{
	return(dm644x_eth_rcv_packet());
}

void eth_mdio_enable(void)
{
	dm644x_eth_mdio_enable();
}
/* End of wrappers */


static u_int8_t dm644x_eth_mac_addr[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

/*
 * This function must be called before emac_open() if you want to override
 * the default mac address.
 */
void dm644x_eth_set_mac_addr(const u_int8_t *addr)
{
	int i;

	for (i = 0; i < sizeof (dm644x_eth_mac_addr); i++) {
		dm644x_eth_mac_addr[i] = addr[i];
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

static void dm644x_eth_mdio_enable(void)
{
	u_int32_t	clkdiv;

	clkdiv = (EMAC_MDIO_BUS_FREQ / EMAC_MDIO_CLOCK_FREQ) - 1;

	adap_mdio->CONTROL = (clkdiv & 0xff) |
		MDIO_CONTROL_ENABLE |
		MDIO_CONTROL_FAULT |
		MDIO_CONTROL_FAULT_ENABLE;

	while (adap_mdio->CONTROL & MDIO_CONTROL_IDLE) {;}
}

/*
 * Tries to find an active connected PHY. Returns 1 if address if found.
 * If no active PHY (or more than one PHY) found returns 0.
 * Sets active_phy_addr variable.
 */
static int dm644x_eth_phy_detect(void)
{
	u_int32_t	phy_act_state;
	int		i;

	active_phy_addr = 0xff;

	if ((phy_act_state = adap_mdio->ALIVE) == 0)
		return(0);				/* No active PHYs */

	debug_emac("dm644x_eth_phy_detect(), ALIVE = 0x%08x\n", phy_act_state);

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
int dm644x_eth_phy_read(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t *data)
{
	int	tmp;

	while (adap_mdio->USERACCESS0 & MDIO_USERACCESS0_GO) {;}

	adap_mdio->USERACCESS0 = MDIO_USERACCESS0_GO |
				MDIO_USERACCESS0_WRITE_READ |
				((reg_num & 0x1f) << 21) |
				((phy_addr & 0x1f) << 16);

	/* Wait for command to complete */
	while ((tmp = adap_mdio->USERACCESS0) & MDIO_USERACCESS0_GO) {;}

	if (tmp & MDIO_USERACCESS0_ACK) {
		*data = tmp & 0xffff;
		return(1);
	}

	*data = -1;
	return(0);
}

/* Write to a PHY register via MDIO inteface. Blocks until operation is complete. */
int dm644x_eth_phy_write(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t data)
{

	while (adap_mdio->USERACCESS0 & MDIO_USERACCESS0_GO) {;}

	adap_mdio->USERACCESS0 = MDIO_USERACCESS0_GO |
				MDIO_USERACCESS0_WRITE_WRITE |
				((reg_num & 0x1f) << 21) |
				((phy_addr & 0x1f) << 16) |
				(data & 0xffff);

	/* Wait for command to complete */
	while (adap_mdio->USERACCESS0 & MDIO_USERACCESS0_GO) {;}

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

	return(dm644x_eth_phy_read(phy_addr, PHY_PHYIDR1, &dummy));
}

static int gen_get_link_speed(int phy_addr)
{
	u_int16_t	tmp;

	if (dm644x_eth_phy_read(phy_addr, MII_STATUS_REG, &tmp) && (tmp & 0x04))
		return(1);

	return(0);
}

static int gen_auto_negotiate(int phy_addr)
{
	u_int16_t	tmp;

	if (!dm644x_eth_phy_read(phy_addr, PHY_BMCR, &tmp))
		return(0);

	/* Restart Auto_negotiation  */
	tmp |= PHY_BMCR_AUTON;
	dm644x_eth_phy_write(phy_addr, PHY_BMCR, tmp);

	/*check AutoNegotiate complete */
	udelay (10000);
	if (!dm644x_eth_phy_read(phy_addr, PHY_BMSR, &tmp))
		return(0);

	if (!(tmp & PHY_BMSR_AUTN_COMP))
		return(0);

	return(gen_get_link_speed(phy_addr));
}
/* End of generic PHY functions */


#if defined(CONFIG_MII) || (CONFIG_COMMANDS & CFG_CMD_MII)
static int dm644x_mii_phy_read(char *devname, unsigned char addr, unsigned char reg, unsigned short *value)
{
	return(dm644x_eth_phy_read(addr, reg, value) ? 0 : 1);
}

static int dm644x_mii_phy_write(char *devname, unsigned char addr, unsigned char reg, unsigned short value)
{
	return(dm644x_eth_phy_write(addr, reg, value) ? 0 : 1);
}

int dm644x_eth_miiphy_initialize(bd_t *bis)
{
	miiphy_register(phy.name, dm644x_mii_phy_read, dm644x_mii_phy_write);

	return(1);
}
#endif

/*
 * This function initializes the emac hardware. It does NOT initialize
 * EMAC modules power or pin multiplexors, that is done by board_init()
 * much earlier in bootup process. Returns 1 on success, 0 otherwise.
 */
static int dm644x_eth_hw_init(void)
{
	u_int32_t	phy_id;
	u_int16_t	tmp;
	int		i;

	dm644x_eth_mdio_enable();

	for (i = 0; i < 256; i++) {
		if (adap_mdio->ALIVE)
			break;
		udelay(10);
	}

	if (i >= 256) {
		printf("No ETH PHY detected!!!\n");
		return(0);
	}

	/* Find if a PHY is connected and get it's address */
	if (!dm644x_eth_phy_detect())
		return(0);

	/* Get PHY ID and initialize phy_ops for a detected PHY */
	if (!dm644x_eth_phy_read(active_phy_addr, PHY_PHYIDR1, &tmp)) {
		active_phy_addr = 0xff;
		return(0);
	}

	phy_id = (tmp << 16) & 0xffff0000;

	if (!dm644x_eth_phy_read(active_phy_addr, PHY_PHYIDR2, &tmp)) {
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

	return(1);
}


/* Eth device open */
static int dm644x_eth_open(void)
{
	dv_reg_p		addr;
	u_int32_t		clkdiv, cnt;
	volatile emac_desc	*rx_desc;

	debug_emac("+ emac_open\n");

	/* Reset EMAC module and disable interrupts in wrapper */
	adap_emac->SOFTRESET = 1;
	while (adap_emac->SOFTRESET != 0) {;}
	adap_ewrap->EWCTL = 0;
	for (cnt = 0; cnt < 5; cnt++) {
		clkdiv = adap_ewrap->EWCTL;
	}

	rx_desc = emac_rx_desc;

	adap_emac->TXCONTROL = 0x01;
	adap_emac->RXCONTROL = 0x01;

	/* Set MAC Addresses & Init multicast Hash to 0 (disable any multicast receive) */
	/* Using channel 0 only - other channels are disabled */
	adap_emac->MACINDEX = 0;
	adap_emac->MACADDRHI =
		(dm644x_eth_mac_addr[3] << 24) |
		(dm644x_eth_mac_addr[2] << 16) |
		(dm644x_eth_mac_addr[1] << 8)  |
		(dm644x_eth_mac_addr[0]);
	adap_emac->MACADDRLO =
		(dm644x_eth_mac_addr[5] << 8) |
		(dm644x_eth_mac_addr[4]);

	adap_emac->MACHASH1 = 0;
	adap_emac->MACHASH2 = 0;

	/* Set source MAC address - REQUIRED */
	adap_emac->MACSRCADDRHI =
		(dm644x_eth_mac_addr[3] << 24) |
		(dm644x_eth_mac_addr[2] << 16) |
		(dm644x_eth_mac_addr[1] << 8)  |
		(dm644x_eth_mac_addr[0]);
	adap_emac->MACSRCADDRLO =
		(dm644x_eth_mac_addr[4] << 8) |
		(dm644x_eth_mac_addr[5]);

	/* Set DMA 8 TX / 8 RX Head pointers to 0 */
	addr = &adap_emac->TX0HDP;
	for(cnt = 0; cnt < 16; cnt++)
		*addr++ = 0;

	addr = &adap_emac->RX0HDP;
	for(cnt = 0; cnt < 16; cnt++)
		*addr++ = 0;

	/* Clear Statistics (do this before setting MacControl register) */
	addr = &adap_emac->RXGOODFRAMES;
	for(cnt = 0; cnt < EMAC_NUM_STATS; cnt++)
		*addr++ = 0;

	/* No multicast addressing */
	adap_emac->MACHASH1 = 0;
	adap_emac->MACHASH2 = 0;

	/* Create RX queue and set receive process in place */
	emac_rx_active_head = emac_rx_desc;
	for (cnt = 0; cnt < EMAC_MAX_RX_BUFFERS; cnt++) {
		rx_desc->next = (u_int32_t)(rx_desc + 1);
		rx_desc->buffer = &emac_rx_buffers[cnt * (EMAC_MAX_ETHERNET_PKT_SIZE + EMAC_PKT_ALIGN)];
		rx_desc->buff_off_len = EMAC_MAX_ETHERNET_PKT_SIZE;
		rx_desc->pkt_flag_len = EMAC_CPPI_OWNERSHIP_BIT;
		rx_desc++;
	}

	/* Set the last descriptor's "next" parameter to 0 to end the RX desc list */
	rx_desc--;
	rx_desc->next = 0;
	emac_rx_active_tail = rx_desc;
	emac_rx_queue_active = 1;

	/* Enable TX/RX */
	adap_emac->RXMAXLEN = EMAC_MAX_ETHERNET_PKT_SIZE;
	adap_emac->RXBUFFEROFFSET = 0;

	/* No fancy configs - Use this for promiscous for debug - EMAC_RXMBPENABLE_RXCAFEN_ENABLE */
	adap_emac->RXMBPENABLE = EMAC_RXMBPENABLE_RXBROADEN;

	/* Enable ch 0 only */
	adap_emac->RXUNICASTSET = 0x01;

	/* Enable MII interface and Full duplex mode */
	adap_emac->MACCONTROL = (EMAC_MACCONTROL_MIIEN_ENABLE | EMAC_MACCONTROL_FULLDUPLEX_ENABLE);

	/* Init MDIO & get link state */
	clkdiv = (EMAC_MDIO_BUS_FREQ / EMAC_MDIO_CLOCK_FREQ) - 1;
	adap_mdio->CONTROL = ((clkdiv & 0xff) | MDIO_CONTROL_ENABLE | MDIO_CONTROL_FAULT);

	if (!phy.get_link_speed(active_phy_addr))
		return(0);

	/* Start receive process */
	adap_emac->RX0HDP = (u_int32_t)emac_rx_desc;

	debug_emac("- emac_open\n");

	return(1);
}

/* EMAC Channel Teardown */
static void dm644x_eth_ch_teardown(int ch)
{
	dv_reg		dly = 0xff;
	dv_reg		cnt;

	debug_emac("+ emac_ch_teardown\n");

	if (ch == EMAC_CH_TX) {
		/* Init TX channel teardown */
		adap_emac->TXTEARDOWN = 1;
		for(cnt = 0; cnt != 0xfffffffc; cnt = adap_emac->TX0CP) {
			/* Wait here for Tx teardown completion interrupt to occur
			 * Note: A task delay can be called here to pend rather than
			 * occupying CPU cycles - anyway it has been found that teardown
			 * takes very few cpu cycles and does not affect functionality */
			 dly--;
			 udelay(1);
			 if (dly == 0)
			 	break;
		}
		adap_emac->TX0CP = cnt;
		adap_emac->TX0HDP = 0;
	} else {
		/* Init RX channel teardown */
		adap_emac->RXTEARDOWN = 1;
		for(cnt = 0; cnt != 0xfffffffc; cnt = adap_emac->RX0CP) {
			/* Wait here for Rx teardown completion interrupt to occur
			 * Note: A task delay can be called here to pend rather than
			 * occupying CPU cycles - anyway it has been found that teardown
			 * takes very few cpu cycles and does not affect functionality */
			 dly--;
			 udelay(1);
			 if (dly == 0)
			 	break;
		}
		adap_emac->RX0CP = cnt;
		adap_emac->RX0HDP = 0;
	}

	debug_emac("- emac_ch_teardown\n");
}

/* Eth device close */
static int dm644x_eth_close(void)
{
	debug_emac("+ emac_close\n");

	dm644x_eth_ch_teardown(EMAC_CH_TX);	/* TX Channel teardown */
	dm644x_eth_ch_teardown(EMAC_CH_RX);	/* RX Channel teardown */

	/* Reset EMAC module and disable interrupts in wrapper */
	adap_emac->SOFTRESET = 1;
	adap_ewrap->EWCTL = 0;

	debug_emac("- emac_close\n");
	return(1);
}

static int tx_send_loop = 0;

/*
 * This function sends a single packet on the network and returns
 * positive number (number of bytes transmitted) or negative for error
 */
static int dm644x_eth_send_packet(volatile void *packet, int length)
{
	int ret_status = -1;
	tx_send_loop = 0;

	/* Return error if no link */
	if (!phy.get_link_speed(active_phy_addr))
	{
		printf("WARN: emac_send_packet: No link\n");
		return (ret_status);
	}

	/* Check packet size and if < EMAC_MIN_ETHERNET_PKT_SIZE, pad it up */
	if (length < EMAC_MIN_ETHERNET_PKT_SIZE)
	{
		length = EMAC_MIN_ETHERNET_PKT_SIZE;
	}

	/* Populate the TX descriptor */
	emac_tx_desc->next         = 0;
	emac_tx_desc->buffer       = (u_int8_t *)packet;
	emac_tx_desc->buff_off_len = (length & 0xffff);
	emac_tx_desc->pkt_flag_len = ((length & 0xffff) |
			EMAC_CPPI_SOP_BIT |
			EMAC_CPPI_OWNERSHIP_BIT |
			EMAC_CPPI_EOP_BIT);
	/* Send the packet */
	adap_emac->TX0HDP = (unsigned int)emac_tx_desc;

	/* Wait for packet to complete or link down */
	while (1) {
	        if (!phy.get_link_speed(active_phy_addr)) {
	        	dm644x_eth_ch_teardown(EMAC_CH_TX);
	        	return (ret_status);
	        }
	        if (adap_emac->TXINTSTATRAW & 0x01) {
	        	ret_status = length;
	        	break;
		}
	        tx_send_loop++;
	}

	return(ret_status);
}

/*
 * This function handles receipt of a packet from the network
 */
static int dm644x_eth_rcv_packet(void)
{
	volatile emac_desc	*rx_curr_desc;
	volatile emac_desc	*curr_desc;
	volatile emac_desc	*tail_desc;
	int			status, ret = -1;

	rx_curr_desc = emac_rx_active_head;
	status = rx_curr_desc->pkt_flag_len;
	if ((rx_curr_desc) && ((status & EMAC_CPPI_OWNERSHIP_BIT) == 0)) {
	        if (status & EMAC_CPPI_RX_ERROR_FRAME) {
	        	/* Error in packet - discard it and requeue desc */
	        	printf("WARN: emac_rcv_pkt: Error in packet\n");
		} else {
			NetReceive(rx_curr_desc->buffer, (rx_curr_desc->buff_off_len & 0xffff));
			ret = rx_curr_desc->buff_off_len & 0xffff;
	        }

	        /* Ack received packet descriptor */
	        adap_emac->RX0CP = (unsigned int)rx_curr_desc;
	        curr_desc = rx_curr_desc;
	        emac_rx_active_head = (volatile emac_desc *)rx_curr_desc->next;

	        if (status & EMAC_CPPI_EOQ_BIT) {
	        	if (emac_rx_active_head) {
	        		adap_emac->RX0HDP = (unsigned int)emac_rx_active_head;
			} else {
				emac_rx_queue_active = 0;
				printf("INFO:emac_rcv_packet: RX Queue not active\n");
			}
		}

		/* Recycle RX descriptor */
		rx_curr_desc->buff_off_len = EMAC_MAX_ETHERNET_PKT_SIZE;
		rx_curr_desc->pkt_flag_len = EMAC_CPPI_OWNERSHIP_BIT;
		rx_curr_desc->next = 0;

		if (emac_rx_active_head == 0) {
			printf("INFO: emac_rcv_pkt: active queue head = 0\n");
			emac_rx_active_head = curr_desc;
			emac_rx_active_tail = curr_desc;
			if (emac_rx_queue_active != 0) {
				adap_emac->RX0HDP = (unsigned int)emac_rx_active_head;
				printf("INFO: emac_rcv_pkt: active queue head = 0, HDP fired\n");
				emac_rx_queue_active = 1;
			}
		} else {
			tail_desc = emac_rx_active_tail;
			emac_rx_active_tail = curr_desc;
			tail_desc->next = (unsigned int)curr_desc;
			status = tail_desc->pkt_flag_len;
			if (status & EMAC_CPPI_EOQ_BIT) {
				adap_emac->RX0HDP = (unsigned int)curr_desc;
				status &= ~EMAC_CPPI_EOQ_BIT;
				tail_desc->pkt_flag_len = status;
			}
		}
		return(ret);
	}
	return(0);
}

#endif /* CONFIG_CMD_NET */

#endif /* CONFIG_DRIVER_TI_EMAC */
