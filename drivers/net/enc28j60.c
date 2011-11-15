/*
 * (C) Copyright 2010
 * Reinhard Meyer, EMK Elektronik, reinhard.meyer@emk-elektronik.de
 * Martin Krause, Martin.Krause@tqs.de
 * reworked original enc28j60.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <net.h>
#include <spi.h>
#include <malloc.h>
#include <netdev.h>
#include <miiphy.h>
#include "enc28j60.h"

/*
 * IMPORTANT: spi_claim_bus() and spi_release_bus()
 * are called at begin and end of each of the following functions:
 * enc_miiphy_read(), enc_miiphy_write(), enc_write_hwaddr(),
 * enc_init(), enc_recv(), enc_send(), enc_halt()
 * ALL other functions assume that the bus has already been claimed!
 * Since NetReceive() might call enc_send() in return, the bus must be
 * released, NetReceive() called and claimed again.
 */

/*
 * Controller memory layout.
 * We only allow 1 frame for transmission and reserve the rest
 * for reception to handle as many broadcast packets as possible.
 * Also use the memory from 0x0000 for receiver buffer. See errata pt. 5
 * 0x0000 - 0x19ff 6656 bytes receive buffer
 * 0x1a00 - 0x1fff 1536 bytes transmit buffer =
 * control(1)+frame(1518)+status(7)+reserve(10).
 */
#define ENC_RX_BUF_START	0x0000
#define ENC_RX_BUF_END		0x19ff
#define ENC_TX_BUF_START	0x1a00
#define ENC_TX_BUF_END		0x1fff
#define ENC_MAX_FRM_LEN		1518
#define RX_RESET_COUNTER	1000

/*
 * For non data transfer functions, like phy read/write, set hwaddr, init
 * we do not need a full, time consuming init including link ready wait.
 * This enum helps to bring the chip through the minimum necessary inits.
 */
enum enc_initstate {none=0, setupdone, linkready};
typedef struct enc_device {
	struct eth_device	*dev;	/* back pointer */
	struct spi_slave	*slave;
	int			rx_reset_counter;
	u16			next_pointer;
	u8			bank;	/* current bank in enc28j60 */
	enum enc_initstate	initstate;
} enc_dev_t;

/*
 * enc_bset:		set bits in a common register
 * enc_bclr:		clear bits in a common register
 *
 * making the reg parameter u8 will give a compile time warning if the
 * functions are called with a register not accessible in all Banks
 */
static void enc_bset(enc_dev_t *enc, const u8 reg, const u8 data)
{
	u8 dout[2];

	dout[0] = CMD_BFS(reg);
	dout[1] = data;
	spi_xfer(enc->slave, 2 * 8, dout, NULL,
		SPI_XFER_BEGIN | SPI_XFER_END);
}

static void enc_bclr(enc_dev_t *enc, const u8 reg, const u8 data)
{
	u8 dout[2];

	dout[0] = CMD_BFC(reg);
	dout[1] = data;
	spi_xfer(enc->slave, 2 * 8, dout, NULL,
		SPI_XFER_BEGIN | SPI_XFER_END);
}

/*
 * high byte of the register contains bank number:
 * 0: no bank switch necessary
 * 1: switch to bank 0
 * 2: switch to bank 1
 * 3: switch to bank 2
 * 4: switch to bank 3
 */
static void enc_set_bank(enc_dev_t *enc, const u16 reg)
{
	u8 newbank = reg >> 8;

	if (newbank == 0 || newbank == enc->bank)
		return;
	switch (newbank) {
	case 1:
		enc_bclr(enc, CTL_REG_ECON1,
			ENC_ECON1_BSEL0 | ENC_ECON1_BSEL1);
		break;
	case 2:
		enc_bset(enc, CTL_REG_ECON1, ENC_ECON1_BSEL0);
		enc_bclr(enc, CTL_REG_ECON1, ENC_ECON1_BSEL1);
		break;
	case 3:
		enc_bclr(enc, CTL_REG_ECON1, ENC_ECON1_BSEL0);
		enc_bset(enc, CTL_REG_ECON1, ENC_ECON1_BSEL1);
		break;
	case 4:
		enc_bset(enc, CTL_REG_ECON1,
			ENC_ECON1_BSEL0 | ENC_ECON1_BSEL1);
		break;
	}
	enc->bank = newbank;
}

/*
 * local functions to access SPI
 *
 * reg: register inside ENC28J60
 * data: 8/16 bits to write
 * c: number of retries
 *
 * enc_r8:		read 8 bits
 * enc_r16:		read 16 bits
 * enc_w8:		write 8 bits
 * enc_w16:		write 16 bits
 * enc_w8_retry:	write 8 bits, verify and retry
 * enc_rbuf:		read from ENC28J60 into buffer
 * enc_wbuf:		write from buffer into ENC28J60
 */

/*
 * MAC and MII registers need a 3 byte SPI transfer to read,
 * all other registers need a 2 byte SPI transfer.
 */
static int enc_reg2nbytes(const u16 reg)
{
	/* check if MAC or MII register */
	return ((reg >= CTL_REG_MACON1 && reg <= CTL_REG_MIRDH) ||
		(reg >= CTL_REG_MAADR1 && reg <= CTL_REG_MAADR4) ||
		(reg == CTL_REG_MISTAT)) ? 3 : 2;
}

/*
 * Read a byte register
 */
static u8 enc_r8(enc_dev_t *enc, const u16 reg)
{
	u8 dout[3];
	u8 din[3];
	int nbytes = enc_reg2nbytes(reg);

	enc_set_bank(enc, reg);
	dout[0] = CMD_RCR(reg);
	spi_xfer(enc->slave, nbytes * 8, dout, din,
		SPI_XFER_BEGIN | SPI_XFER_END);
	return din[nbytes-1];
}

/*
 * Read a L/H register pair and return a word.
 * Must be called with the L register's address.
 */
static u16 enc_r16(enc_dev_t *enc, const u16 reg)
{
	u8 dout[3];
	u8 din[3];
	u16 result;
	int nbytes = enc_reg2nbytes(reg);

	enc_set_bank(enc, reg);
	dout[0] = CMD_RCR(reg);
	spi_xfer(enc->slave, nbytes * 8, dout, din,
		SPI_XFER_BEGIN | SPI_XFER_END);
	result = din[nbytes-1];
	dout[0]++; /* next register */
	spi_xfer(enc->slave, nbytes * 8, dout, din,
		SPI_XFER_BEGIN | SPI_XFER_END);
	result |= din[nbytes-1] << 8;
	return result;
}

/*
 * Write a byte register
 */
static void enc_w8(enc_dev_t *enc, const u16 reg, const u8 data)
{
	u8 dout[2];

	enc_set_bank(enc, reg);
	dout[0] = CMD_WCR(reg);
	dout[1] = data;
	spi_xfer(enc->slave, 2 * 8, dout, NULL,
		SPI_XFER_BEGIN | SPI_XFER_END);
}

/*
 * Write a L/H register pair.
 * Must be called with the L register's address.
 */
static void enc_w16(enc_dev_t *enc, const u16 reg, const u16 data)
{
	u8 dout[2];

	enc_set_bank(enc, reg);
	dout[0] = CMD_WCR(reg);
	dout[1] = data;
	spi_xfer(enc->slave, 2 * 8, dout, NULL,
		SPI_XFER_BEGIN | SPI_XFER_END);
	dout[0]++; /* next register */
	dout[1] = data >> 8;
	spi_xfer(enc->slave, 2 * 8, dout, NULL,
		SPI_XFER_BEGIN | SPI_XFER_END);
}

/*
 * Write a byte register, verify and retry
 */
static void enc_w8_retry(enc_dev_t *enc, const u16 reg, const u8 data, const int c)
{
	u8 dout[2];
	u8 readback;
	int i;

	enc_set_bank(enc, reg);
	for (i = 0; i < c; i++) {
		dout[0] = CMD_WCR(reg);
		dout[1] = data;
		spi_xfer(enc->slave, 2 * 8, dout, NULL,
			SPI_XFER_BEGIN | SPI_XFER_END);
		readback = enc_r8(enc, reg);
		if (readback == data)
			break;
		/* wait 1ms */
		udelay(1000);
	}
	if (i == c) {
		printf("%s: write reg 0x%03x failed\n", enc->dev->name, reg);
	}
}

/*
 * Read ENC RAM into buffer
 */
static void enc_rbuf(enc_dev_t *enc, const u16 length, u8 *buf)
{
	u8 dout[1];

	dout[0] = CMD_RBM;
	spi_xfer(enc->slave, 8, dout, NULL, SPI_XFER_BEGIN);
	spi_xfer(enc->slave, length * 8, NULL, buf, SPI_XFER_END);
#ifdef DEBUG
	puts("Rx:\n");
	print_buffer(0, buf, 1, length, 0);
#endif
}

/*
 * Write buffer into ENC RAM
 */
static void enc_wbuf(enc_dev_t *enc, const u16 length, const u8 *buf, const u8 control)
{
	u8 dout[2];
	dout[0] = CMD_WBM;
	dout[1] = control;
	spi_xfer(enc->slave, 2 * 8, dout, NULL, SPI_XFER_BEGIN);
	spi_xfer(enc->slave, length * 8, buf, NULL, SPI_XFER_END);
#ifdef DEBUG
	puts("Tx:\n");
	print_buffer(0, buf, 1, length, 0);
#endif
}

/*
 * Try to claim the SPI bus.
 * Print error message on failure.
 */
static int enc_claim_bus(enc_dev_t *enc)
{
	int rc = spi_claim_bus(enc->slave);
	if (rc)
		printf("%s: failed to claim SPI bus\n", enc->dev->name);
	return rc;
}

/*
 * Release previously claimed SPI bus.
 * This function is mainly for symmetry to enc_claim_bus().
 * Let the toolchain decide to inline it...
 */
static void enc_release_bus(enc_dev_t *enc)
{
	spi_release_bus(enc->slave);
}

/*
 * Read PHY register
 */
static u16 enc_phy_read(enc_dev_t *enc, const u8 addr)
{
	uint64_t etime;
	u8 status;

	enc_w8(enc, CTL_REG_MIREGADR, addr);
	enc_w8(enc, CTL_REG_MICMD, ENC_MICMD_MIIRD);
	/* 1 second timeout - only happens on hardware problem */
	etime = get_ticks() + get_tbclk();
	/* poll MISTAT.BUSY bit until operation is complete */
	do
	{
		status = enc_r8(enc, CTL_REG_MISTAT);
	} while (get_ticks() <= etime && (status & ENC_MISTAT_BUSY));
	if (status & ENC_MISTAT_BUSY) {
		printf("%s: timeout reading phy\n", enc->dev->name);
		return 0;
	}
	enc_w8(enc, CTL_REG_MICMD, 0);
	return enc_r16(enc, CTL_REG_MIRDL);
}

/*
 * Write PHY register
 */
static void enc_phy_write(enc_dev_t *enc, const u8 addr, const u16 data)
{
	uint64_t etime;
	u8 status;

	enc_w8(enc, CTL_REG_MIREGADR, addr);
	enc_w16(enc, CTL_REG_MIWRL, data);
	/* 1 second timeout - only happens on hardware problem */
	etime = get_ticks() + get_tbclk();
	/* poll MISTAT.BUSY bit until operation is complete */
	do
	{
		status = enc_r8(enc, CTL_REG_MISTAT);
	} while (get_ticks() <= etime && (status & ENC_MISTAT_BUSY));
	if (status & ENC_MISTAT_BUSY) {
		printf("%s: timeout writing phy\n", enc->dev->name);
		return;
	}
}

/*
 * Verify link status, wait if necessary
 *
 * Note: with a 10 MBit/s only PHY there is no autonegotiation possible,
 * half/full duplex is a pure setup matter. For the time being, this driver
 * will setup in half duplex mode only.
 */
static int enc_phy_link_wait(enc_dev_t *enc)
{
	u16 status;
	int duplex;
	uint64_t etime;

#ifdef CONFIG_ENC_SILENTLINK
	/* check if we have a link, then just return */
	status = enc_phy_read(enc, PHY_REG_PHSTAT1);
	if (status & ENC_PHSTAT1_LLSTAT)
		return 0;
#endif

	/* wait for link with 1 second timeout */
	etime = get_ticks() + get_tbclk();
	while (get_ticks() <= etime) {
		status = enc_phy_read(enc, PHY_REG_PHSTAT1);
		if (status & ENC_PHSTAT1_LLSTAT) {
			/* now we have a link */
			status = enc_phy_read(enc, PHY_REG_PHSTAT2);
			duplex = (status & ENC_PHSTAT2_DPXSTAT) ? 1 : 0;
			printf("%s: link up, 10Mbps %s-duplex\n",
				enc->dev->name, duplex ? "full" : "half");
			return 0;
		}
		udelay(1000);
	}

	/* timeout occured */
	printf("%s: link down\n", enc->dev->name);
	return 1;
}

/*
 * This function resets the receiver only.
 */
static void enc_reset_rx(enc_dev_t *enc)
{
	u8 econ1;

	econ1 = enc_r8(enc, CTL_REG_ECON1);
	if ((econ1 & ENC_ECON1_RXRST) == 0) {
		enc_bset(enc, CTL_REG_ECON1, ENC_ECON1_RXRST);
		enc->rx_reset_counter = RX_RESET_COUNTER;
	}
}

/*
 * Reset receiver and reenable it.
 */
static void enc_reset_rx_call(enc_dev_t *enc)
{
	enc_bclr(enc, CTL_REG_ECON1, ENC_ECON1_RXRST);
	enc_bset(enc, CTL_REG_ECON1, ENC_ECON1_RXEN);
}

/*
 * Copy a packet from the receive ring and forward it to
 * the protocol stack.
 */
static void enc_receive(enc_dev_t *enc)
{
	u8 *packet = (u8 *)NetRxPackets[0];
	u16 pkt_len;
	u16 copy_len;
	u16 status;
	u8 pkt_cnt = 0;
	u16 rxbuf_rdpt;
	u8 hbuf[6];

	enc_w16(enc, CTL_REG_ERDPTL, enc->next_pointer);
	do {
		enc_rbuf(enc, 6, hbuf);
		enc->next_pointer = hbuf[0] | (hbuf[1] << 8);
		pkt_len = hbuf[2] | (hbuf[3] << 8);
		status = hbuf[4] | (hbuf[5] << 8);
		debug("next_pointer=$%04x pkt_len=%u status=$%04x\n",
			enc->next_pointer, pkt_len, status);
		if (pkt_len <= ENC_MAX_FRM_LEN)
			copy_len = pkt_len;
		else
			copy_len = 0;
		if ((status & (1L << 7)) == 0) /* check Received Ok bit */
			copy_len = 0;
		/* check if next pointer is resonable */
		if (enc->next_pointer >= ENC_TX_BUF_START)
			copy_len = 0;
		if (copy_len > 0) {
			enc_rbuf(enc, copy_len, packet);
		}
		/* advance read pointer to next pointer */
		enc_w16(enc, CTL_REG_ERDPTL, enc->next_pointer);
		/* decrease packet counter */
		enc_bset(enc, CTL_REG_ECON2, ENC_ECON2_PKTDEC);
		/*
		 * Only odd values should be written to ERXRDPTL,
		 * see errata B4 pt.13
		 */
		rxbuf_rdpt = enc->next_pointer - 1;
		if ((rxbuf_rdpt < enc_r16(enc, CTL_REG_ERXSTL)) ||
			(rxbuf_rdpt > enc_r16(enc, CTL_REG_ERXNDL))) {
			enc_w16(enc, CTL_REG_ERXRDPTL,
				enc_r16(enc, CTL_REG_ERXNDL));
		} else {
			enc_w16(enc, CTL_REG_ERXRDPTL, rxbuf_rdpt);
		}
		/* read pktcnt */
		pkt_cnt = enc_r8(enc, CTL_REG_EPKTCNT);
		if (copy_len == 0) {
			(void)enc_r8(enc, CTL_REG_EIR);
			enc_reset_rx(enc);
			printf("%s: receive copy_len=0\n", enc->dev->name);
			continue;
		}
		/*
		 * Because NetReceive() might call enc_send(), we need to
		 * release the SPI bus, call NetReceive(), reclaim the bus
		 */
		enc_release_bus(enc);
		NetReceive(packet, pkt_len);
		if (enc_claim_bus(enc))
			return;
		(void)enc_r8(enc, CTL_REG_EIR);
	} while (pkt_cnt);
	/* Use EPKTCNT not EIR.PKTIF flag, see errata pt. 6 */
}

/*
 * Poll for completely received packets.
 */
static void enc_poll(enc_dev_t *enc)
{
	u8 eir_reg;
	u8 pkt_cnt;

#ifdef CONFIG_USE_IRQ
	/* clear global interrupt enable bit in enc28j60 */
	enc_bclr(enc, CTL_REG_EIE, ENC_EIE_INTIE);
#endif
	(void)enc_r8(enc, CTL_REG_ESTAT);
	eir_reg = enc_r8(enc, CTL_REG_EIR);
	if (eir_reg & ENC_EIR_TXIF) {
		/* clear TXIF bit in EIR */
		enc_bclr(enc, CTL_REG_EIR, ENC_EIR_TXIF);
	}
	/* We have to use pktcnt and not pktif bit, see errata pt. 6 */
	pkt_cnt = enc_r8(enc, CTL_REG_EPKTCNT);
	if (pkt_cnt > 0) {
		if ((eir_reg & ENC_EIR_PKTIF) == 0) {
			debug("enc_poll: pkt cnt > 0, but pktif not set\n");
		}
		enc_receive(enc);
		/*
		 * clear PKTIF bit in EIR, this should not need to be done
		 * but it seems like we get problems if we do not
		 */
		enc_bclr(enc, CTL_REG_EIR, ENC_EIR_PKTIF);
	}
	if (eir_reg & ENC_EIR_RXERIF) {
		printf("%s: rx error\n", enc->dev->name);
		enc_bclr(enc, CTL_REG_EIR, ENC_EIR_RXERIF);
	}
	if (eir_reg & ENC_EIR_TXERIF) {
		printf("%s: tx error\n", enc->dev->name);
		enc_bclr(enc, CTL_REG_EIR, ENC_EIR_TXERIF);
	}
#ifdef CONFIG_USE_IRQ
	/* set global interrupt enable bit in enc28j60 */
	enc_bset(enc, CTL_REG_EIE, ENC_EIE_INTIE);
#endif
}

/*
 * Completely Reset the ENC
 */
static void enc_reset(enc_dev_t *enc)
{
	u8 dout[1];

	dout[0] = CMD_SRC;
	spi_xfer(enc->slave, 8, dout, NULL,
		SPI_XFER_BEGIN | SPI_XFER_END);
	/* sleep 1 ms. See errata pt. 2 */
	udelay(1000);
}

/*
 * Initialisation data for most of the ENC registers
 */
static const u16 enc_initdata[] = {
	/*
	 * Setup the buffer space. The reset values are valid for the
	 * other pointers.
	 *
	 * We shall not write to ERXST, see errata pt. 5. Instead we
	 * have to make sure that ENC_RX_BUS_START is 0.
	 */
	CTL_REG_ERXSTL, ENC_RX_BUF_START,
	CTL_REG_ERXSTH, ENC_RX_BUF_START >> 8,
	CTL_REG_ERXNDL, ENC_RX_BUF_END,
	CTL_REG_ERXNDH, ENC_RX_BUF_END >> 8,
	CTL_REG_ERDPTL, ENC_RX_BUF_START,
	CTL_REG_ERDPTH, ENC_RX_BUF_START >> 8,
	/*
	 * Set the filter to receive only good-CRC, unicast and broadcast
	 * frames.
	 * Note: some DHCP servers return their answers as broadcasts!
	 * So its unwise to remove broadcast from this. This driver
	 * might incur receiver overruns with packet loss on a broadcast
	 * flooded network.
	 */
	CTL_REG_ERXFCON, ENC_RFR_BCEN | ENC_RFR_UCEN | ENC_RFR_CRCEN,

	/* enable MAC to receive frames */
	CTL_REG_MACON1,
		ENC_MACON1_MARXEN | ENC_MACON1_TXPAUS | ENC_MACON1_RXPAUS,

	/* configure pad, tx-crc and duplex */
	CTL_REG_MACON3,
		ENC_MACON3_PADCFG0 | ENC_MACON3_TXCRCEN |
		ENC_MACON3_FRMLNEN,

	/* Allow infinite deferals if the medium is continously busy */
	CTL_REG_MACON4, ENC_MACON4_DEFER,

	/* Late collisions occur beyond 63 bytes */
	CTL_REG_MACLCON2, 63,

	/*
	 * Set (low byte) Non-Back-to_Back Inter-Packet Gap.
	 * Recommended 0x12
	 */
	CTL_REG_MAIPGL, 0x12,

	/*
	 * Set (high byte) Non-Back-to_Back Inter-Packet Gap.
	 * Recommended 0x0c for half-duplex. Nothing for full-duplex
	 */
	CTL_REG_MAIPGH, 0x0C,

	/* set maximum frame length */
	CTL_REG_MAMXFLL, ENC_MAX_FRM_LEN,
	CTL_REG_MAMXFLH, ENC_MAX_FRM_LEN >> 8,

	/*
	 * Set MAC back-to-back inter-packet gap.
	 * Recommended 0x12 for half duplex
	 * and 0x15 for full duplex.
	 */
	CTL_REG_MABBIPG, 0x12,

	/* end of table */
	0xffff
};

/*
 * Wait for the XTAL oscillator to become ready
 */
static int enc_clock_wait(enc_dev_t *enc)
{
	uint64_t etime;

	/* one second timeout */
	etime = get_ticks() + get_tbclk();

	/*
	 * Wait for CLKRDY to become set (i.e., check that we can
	 * communicate with the ENC)
	 */
	do
	{
		if (enc_r8(enc, CTL_REG_ESTAT) & ENC_ESTAT_CLKRDY)
			return 0;
	} while (get_ticks() <= etime);

	printf("%s: timeout waiting for CLKRDY\n", enc->dev->name);
	return -1;
}

/*
 * Write the MAC address into the ENC
 */
static int enc_write_macaddr(enc_dev_t *enc)
{
	unsigned char *p = enc->dev->enetaddr;

	enc_w8_retry(enc, CTL_REG_MAADR5, *p++, 5);
	enc_w8_retry(enc, CTL_REG_MAADR4, *p++, 5);
	enc_w8_retry(enc, CTL_REG_MAADR3, *p++, 5);
	enc_w8_retry(enc, CTL_REG_MAADR2, *p++, 5);
	enc_w8_retry(enc, CTL_REG_MAADR1, *p++, 5);
	enc_w8_retry(enc, CTL_REG_MAADR0, *p, 5);
	return 0;
}

/*
 * Setup most of the ENC registers
 */
static int enc_setup(enc_dev_t *enc)
{
	u16 phid1 = 0;
	u16 phid2 = 0;
	const u16 *tp;

	/* reset enc struct values */
	enc->next_pointer = ENC_RX_BUF_START;
	enc->rx_reset_counter = RX_RESET_COUNTER;
	enc->bank = 0xff;	/* invalidate current bank in enc28j60 */

	/* verify PHY identification */
	phid1 = enc_phy_read(enc, PHY_REG_PHID1);
	phid2 = enc_phy_read(enc, PHY_REG_PHID2) & ENC_PHID2_MASK;
	if (phid1 != ENC_PHID1_VALUE || phid2 != ENC_PHID2_VALUE) {
		printf("%s: failed to identify PHY. Found %04x:%04x\n",
			enc->dev->name, phid1, phid2);
		return -1;
	}

	/* now program registers */
	for (tp = enc_initdata; *tp != 0xffff; tp += 2)
		enc_w8_retry(enc, tp[0], tp[1], 10);

	/*
	 * Prevent automatic loopback of data beeing transmitted by setting
	 * ENC_PHCON2_HDLDIS
	 */
	enc_phy_write(enc, PHY_REG_PHCON2, (1<<8));

	/*
	 * LEDs configuration
	 * LEDA: LACFG = 0100 -> display link status
	 * LEDB: LBCFG = 0111 -> display TX & RX activity
	 * STRCH = 1 -> LED pulses
	 */
	enc_phy_write(enc, PHY_REG_PHLCON, 0x0472);

	/* Reset PDPXMD-bit => half duplex */
	enc_phy_write(enc, PHY_REG_PHCON1, 0);

#ifdef CONFIG_USE_IRQ
	/* enable interrupts */
	enc_bset(enc, CTL_REG_EIE, ENC_EIE_PKTIE);
	enc_bset(enc, CTL_REG_EIE, ENC_EIE_TXIE);
	enc_bset(enc, CTL_REG_EIE, ENC_EIE_RXERIE);
	enc_bset(enc, CTL_REG_EIE, ENC_EIE_TXERIE);
	enc_bset(enc, CTL_REG_EIE, ENC_EIE_INTIE);
#endif

	return 0;
}

/*
 * Check if ENC has been initialized.
 * If not, try to initialize it.
 * Remember initialized state in struct.
 */
static int enc_initcheck(enc_dev_t *enc, const enum enc_initstate requiredstate)
{
	if (enc->initstate >= requiredstate)
		return 0;

	if (enc->initstate < setupdone) {
		/* Initialize the ENC only */
		enc_reset(enc);
		/* if any of functions fails, skip the rest and return an error */
		if (enc_clock_wait(enc) || enc_setup(enc) || enc_write_macaddr(enc)) {
			return -1;
		}
		enc->initstate = setupdone;
	}
	/* if that's all we need, return here */
	if (enc->initstate >= requiredstate)
		return 0;

	/* now wait for link ready condition */
	if (enc_phy_link_wait(enc)) {
		return -1;
	}
	enc->initstate = linkready;
	return 0;
}

#if defined(CONFIG_CMD_MII)
/*
 * Read a PHY register.
 *
 * This function is registered with miiphy_register().
 */
int enc_miiphy_read(const char *devname, u8 phy_adr, u8 reg, u16 *value)
{
	struct eth_device *dev = eth_get_dev_by_name(devname);
	enc_dev_t *enc;

	if (!dev || phy_adr != 0)
		return -1;

	enc = dev->priv;
	if (enc_claim_bus(enc))
		return -1;
	if (enc_initcheck(enc, setupdone)) {
		enc_release_bus(enc);
		return -1;
	}
	*value = enc_phy_read(enc, reg);
	enc_release_bus(enc);
	return 0;
}

/*
 * Write a PHY register.
 *
 * This function is registered with miiphy_register().
 */
int enc_miiphy_write(const char *devname, u8 phy_adr, u8 reg, u16 value)
{
	struct eth_device *dev = eth_get_dev_by_name(devname);
	enc_dev_t *enc;

	if (!dev || phy_adr != 0)
		return -1;

	enc = dev->priv;
	if (enc_claim_bus(enc))
		return -1;
	if (enc_initcheck(enc, setupdone)) {
		enc_release_bus(enc);
		return -1;
	}
	enc_phy_write(enc, reg, value);
	enc_release_bus(enc);
	return 0;
}
#endif

/*
 * Write hardware (MAC) address.
 *
 * This function entered into eth_device structure.
 */
static int enc_write_hwaddr(struct eth_device *dev)
{
	enc_dev_t *enc = dev->priv;

	if (enc_claim_bus(enc))
		return -1;
	if (enc_initcheck(enc, setupdone)) {
		enc_release_bus(enc);
		return -1;
	}
	enc_release_bus(enc);
	return 0;
}

/*
 * Initialize ENC28J60 for use.
 *
 * This function entered into eth_device structure.
 */
static int enc_init(struct eth_device *dev, bd_t *bis)
{
	enc_dev_t *enc = dev->priv;

	if (enc_claim_bus(enc))
		return -1;
	if (enc_initcheck(enc, linkready)) {
		enc_release_bus(enc);
		return -1;
	}
	/* enable receive */
	enc_bset(enc, CTL_REG_ECON1, ENC_ECON1_RXEN);
	enc_release_bus(enc);
	return 0;
}

/*
 * Check for received packets.
 *
 * This function entered into eth_device structure.
 */
static int enc_recv(struct eth_device *dev)
{
	enc_dev_t *enc = dev->priv;

	if (enc_claim_bus(enc))
		return -1;
	if (enc_initcheck(enc, linkready)) {
		enc_release_bus(enc);
		return -1;
	}
	/* Check for dead receiver */
	if (enc->rx_reset_counter > 0)
		enc->rx_reset_counter--;
	else
		enc_reset_rx_call(enc);
	enc_poll(enc);
	enc_release_bus(enc);
	return 0;
}

/*
 * Send a packet.
 *
 * This function entered into eth_device structure.
 *
 * Should we wait here until we have a Link? Or shall we leave that to
 * protocol retries?
 */
static int enc_send(
	struct eth_device *dev,
	volatile void *packet,
	int length)
{
	enc_dev_t *enc = dev->priv;

	if (enc_claim_bus(enc))
		return -1;
	if (enc_initcheck(enc, linkready)) {
		enc_release_bus(enc);
		return -1;
	}
	/* setup transmit pointers */
	enc_w16(enc, CTL_REG_EWRPTL, ENC_TX_BUF_START);
	enc_w16(enc, CTL_REG_ETXNDL, length + ENC_TX_BUF_START);
	enc_w16(enc, CTL_REG_ETXSTL, ENC_TX_BUF_START);
	/* write packet to ENC */
	enc_wbuf(enc, length, (u8 *) packet, 0x00);
	/*
	 * Check that the internal transmit logic has not been altered
	 * by excessive collisions. Reset transmitter if so.
	 * See Errata B4 12 and 14.
	 */
	if (enc_r8(enc, CTL_REG_EIR) & ENC_EIR_TXERIF) {
		enc_bset(enc, CTL_REG_ECON1, ENC_ECON1_TXRST);
		enc_bclr(enc, CTL_REG_ECON1, ENC_ECON1_TXRST);
	}
	enc_bclr(enc, CTL_REG_EIR, (ENC_EIR_TXERIF | ENC_EIR_TXIF));
	/* start transmitting */
	enc_bset(enc, CTL_REG_ECON1, ENC_ECON1_TXRTS);
	enc_release_bus(enc);
	return 0;
}

/*
 * Finish use of ENC.
 *
 * This function entered into eth_device structure.
 */
static void enc_halt(struct eth_device *dev)
{
	enc_dev_t *enc = dev->priv;

	if (enc_claim_bus(enc))
		return;
	/* Just disable receiver */
	enc_bclr(enc, CTL_REG_ECON1, ENC_ECON1_RXEN);
	enc_release_bus(enc);
}

/*
 * This is the only exported function.
 *
 * It may be called several times with different bus:cs combinations.
 */
int enc28j60_initialize(unsigned int bus, unsigned int cs,
	unsigned int max_hz, unsigned int mode)
{
	struct eth_device *dev;
	enc_dev_t *enc;

	/* try to allocate, check and clear eth_device object */
	dev = malloc(sizeof(*dev));
	if (!dev) {
		return -1;
	}
	memset(dev, 0, sizeof(*dev));

	/* try to allocate, check and clear enc_dev_t object */
	enc = malloc(sizeof(*enc));
	if (!enc) {
		free(dev);
		return -1;
	}
	memset(enc, 0, sizeof(*enc));

	/* try to setup the SPI slave */
	enc->slave = spi_setup_slave(bus, cs, max_hz, mode);
	if (!enc->slave) {
		printf("enc28j60: invalid SPI device %i:%i\n", bus, cs);
		free(enc);
		free(dev);
		return -1;
	}

	enc->dev = dev;
	/* now fill the eth_device object */
	dev->priv = enc;
	dev->init = enc_init;
	dev->halt = enc_halt;
	dev->send = enc_send;
	dev->recv = enc_recv;
	dev->write_hwaddr = enc_write_hwaddr;
	sprintf(dev->name, "enc%i.%i", bus, cs);
	eth_register(dev);
#if defined(CONFIG_CMD_MII)
	miiphy_register(dev->name, enc_miiphy_read, enc_miiphy_write);
#endif
	return 0;
}
