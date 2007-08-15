/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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

#include <common.h>
#include <malloc.h>

#include <asm/fec.h>
#include <asm/immap.h>

#include <command.h>
#include <config.h>
#include <net.h>
#include <miiphy.h>

#ifdef CONFIG_MCFFEC
#undef	ET_DEBUG
#undef	MII_DEBUG

/* Ethernet Transmit and Receive Buffers */
#define DBUF_LENGTH		1520
#define TX_BUF_CNT		2
#define PKT_MAXBUF_SIZE		1518
#define PKT_MINBUF_SIZE		64
#define PKT_MAXBLR_SIZE		1520
#define LAST_PKTBUFSRX		PKTBUFSRX - 1
#define BD_ENET_RX_W_E		(BD_ENET_RX_WRAP | BD_ENET_RX_EMPTY)
#define BD_ENET_TX_RDY_LST	(BD_ENET_TX_READY | BD_ENET_TX_LAST)

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CMD_NET) && defined(CONFIG_NET_MULTI)

struct fec_info_s fec_info[] = {
#ifdef CFG_FEC0_IOBASE
	{
	 0,			/* index */
	 CFG_FEC0_IOBASE,	/* io base */
	 CFG_FEC0_PINMUX,	/* gpio pin muxing */
	 CFG_FEC0_MIIBASE,	/* mii base */
	 -1,			/* phy_addr */
	 0,			/* duplex and speed */
	 0,			/* phy name */
	 0,			/* phyname init */
	 0,			/* RX BD */
	 0,			/* TX BD */
	 0,			/* rx Index */
	 0,			/* tx Index */
	 0,			/* tx buffer */
	 0,			/* initialized flag */
	 },
#endif
#ifdef CFG_FEC1_IOBASE
	{
	 1,			/* index */
	 CFG_FEC1_IOBASE,	/* io base */
	 CFG_FEC1_PINMUX,	/* gpio pin muxing */
	 CFG_FEC1_MIIBASE,	/* mii base */
	 -1,			/* phy_addr */
	 0,			/* duplex and speed */
	 0,			/* phy name */
	 0,			/* phy name init */
	 0,			/* RX BD */
	 0,			/* TX BD */
	 0,			/* rx Index */
	 0,			/* tx Index */
	 0,			/* tx buffer */
	 0,			/* initialized flag */
	 }
#endif
};

int fec_send(struct eth_device *dev, volatile void *packet, int length);
int fec_recv(struct eth_device *dev);
int fec_init(struct eth_device *dev, bd_t * bd);
void fec_halt(struct eth_device *dev);
void fec_reset(struct eth_device *dev);

extern int fecpin_setclear(struct eth_device *dev, int setclear);

#ifdef CFG_DISCOVER_PHY
extern void __mii_init(void);
extern uint mii_send(uint mii_cmd);
extern int mii_discover_phy(struct eth_device *dev);
extern int mcffec_miiphy_read(char *devname, unsigned char addr,
			      unsigned char reg, unsigned short *value);
extern int mcffec_miiphy_write(char *devname, unsigned char addr,
			       unsigned char reg, unsigned short value);
#endif

void setFecDuplexSpeed(volatile fec_t * fecp, bd_t * bd, int dup_spd)
{
	if ((dup_spd >> 16) == FULL) {
		/* Set maximum frame length */
		fecp->rcr = FEC_RCR_MAX_FL(PKT_MAXBUF_SIZE) | FEC_RCR_MII_MODE |
		    FEC_RCR_PROM | 0x100;
		fecp->tcr = FEC_TCR_FDEN;
	} else {
		/* Half duplex mode */
		fecp->rcr = FEC_RCR_MAX_FL(PKT_MAXBUF_SIZE) |
		    FEC_RCR_MII_MODE | FEC_RCR_DRT;
		fecp->tcr &= ~FEC_TCR_FDEN;
	}

	if ((dup_spd & 0xFFFF) == _100BASET) {
#ifdef MII_DEBUG
		printf("100Mbps\n");
#endif
		bd->bi_ethspeed = 100;
	} else {
#ifdef MII_DEBUG
		printf("10Mbps\n");
#endif
		bd->bi_ethspeed = 10;
	}
}

int fec_send(struct eth_device *dev, volatile void *packet, int length)
{
	struct fec_info_s *info = dev->priv;
	volatile fec_t *fecp = (fec_t *) (info->iobase);
	int j, rc;
	u16 phyStatus;

	miiphy_read(dev->name, info->phy_addr, PHY_BMSR, &phyStatus);

	/* section 16.9.23.3
	 * Wait for ready
	 */
	j = 0;
	while ((info->txbd[info->txIdx].cbd_sc & BD_ENET_TX_READY) &&
	       (j < MCFFEC_TOUT_LOOP)) {
		udelay(1);
		j++;
	}
	if (j >= MCFFEC_TOUT_LOOP) {
		printf("TX not ready\n");
	}

	info->txbd[info->txIdx].cbd_bufaddr = (uint) packet;
	info->txbd[info->txIdx].cbd_datlen = length;
	info->txbd[info->txIdx].cbd_sc |= BD_ENET_TX_RDY_LST;

	/* Activate transmit Buffer Descriptor polling */
	fecp->tdar = 0x01000000;	/* Descriptor polling active    */

#ifdef CFG_UNIFY_CACHE
	icache_invalid();
#endif
	j = 0;
	while ((info->txbd[info->txIdx].cbd_sc & BD_ENET_TX_READY) &&
	       (j < MCFFEC_TOUT_LOOP)) {
		udelay(1);
		j++;
	}
	if (j >= MCFFEC_TOUT_LOOP) {
		printf("TX timeout\n");
	}

#ifdef ET_DEBUG
	printf("%s[%d] %s: cycles: %d    status: %x  retry cnt: %d\n",
	       __FILE__, __LINE__, __FUNCTION__, j,
	       info->txbd[info->txIdx].cbd_sc,
	       (info->txbd[info->txIdx].cbd_sc & 0x003C) >> 2);
#endif

	/* return only status bits */
	rc = (info->txbd[info->txIdx].cbd_sc & BD_ENET_TX_STATS);
	info->txIdx = (info->txIdx + 1) % TX_BUF_CNT;

	return rc;
}

int fec_recv(struct eth_device *dev)
{
	struct fec_info_s *info = dev->priv;
	volatile fec_t *fecp = (fec_t *) (info->iobase);
	int length;

	for (;;) {
#ifdef CFG_UNIFY_CACHE
       		icache_invalid();
#endif
		/* section 16.9.23.2 */
		if (info->rxbd[info->rxIdx].cbd_sc & BD_ENET_RX_EMPTY) {
			length = -1;
			break;	/* nothing received - leave for() loop */
		}

		length = info->rxbd[info->rxIdx].cbd_datlen;

		if (info->rxbd[info->rxIdx].cbd_sc & 0x003f) {
			printf("%s[%d] err: %x\n",
			       __FUNCTION__, __LINE__,
			       info->rxbd[info->rxIdx].cbd_sc);
#ifdef ET_DEBUG
			printf("%s[%d] err: %x\n",
			       __FUNCTION__, __LINE__,
			       info->rxbd[info->rxIdx].cbd_sc);
#endif
		} else {

			length -= 4;
			/* Pass the packet up to the protocol layers. */
			NetReceive(NetRxPackets[info->rxIdx], length);

			fecp->eir |= FEC_EIR_RXF;
		}

		/* Give the buffer back to the FEC. */
		info->rxbd[info->rxIdx].cbd_datlen = 0;

		/* wrap around buffer index when necessary */
		if (info->rxIdx == LAST_PKTBUFSRX) {
			info->rxbd[PKTBUFSRX - 1].cbd_sc = BD_ENET_RX_W_E;
			info->rxIdx = 0;
		} else {
			info->rxbd[info->rxIdx].cbd_sc = BD_ENET_RX_EMPTY;
			info->rxIdx++;
		}

		/* Try to fill Buffer Descriptors */
		fecp->rdar = 0x01000000;	/* Descriptor polling active    */
	}

	return length;
}

#ifdef ET_DEBUG
void dbgFecRegs(struct eth_device *dev)
{
	struct fec_info_s *info = dev->priv;
	volatile fec_t *fecp = (fec_t *) (info->iobase);

	printf("=====\n");
	printf("ievent       %x - %x\n", (int)&fecp->eir, fecp->eir);
	printf("imask        %x - %x\n", (int)&fecp->eimr, fecp->eimr);
	printf("r_des_active %x - %x\n", (int)&fecp->rdar, fecp->rdar);
	printf("x_des_active %x - %x\n", (int)&fecp->tdar, fecp->tdar);
	printf("ecntrl       %x - %x\n", (int)&fecp->ecr, fecp->ecr);
	printf("mii_mframe   %x - %x\n", (int)&fecp->mmfr, fecp->mmfr);
	printf("mii_speed    %x - %x\n", (int)&fecp->mscr, fecp->mscr);
	printf("mii_ctrlstat %x - %x\n", (int)&fecp->mibc, fecp->mibc);
	printf("r_cntrl      %x - %x\n", (int)&fecp->rcr, fecp->rcr);
	printf("x_cntrl      %x - %x\n", (int)&fecp->tcr, fecp->tcr);
	printf("padr_l       %x - %x\n", (int)&fecp->palr, fecp->palr);
	printf("padr_u       %x - %x\n", (int)&fecp->paur, fecp->paur);
	printf("op_pause     %x - %x\n", (int)&fecp->opd, fecp->opd);
	printf("iadr_u       %x - %x\n", (int)&fecp->iaur, fecp->iaur);
	printf("iadr_l       %x - %x\n", (int)&fecp->ialr, fecp->ialr);
	printf("gadr_u       %x - %x\n", (int)&fecp->gaur, fecp->gaur);
	printf("gadr_l       %x - %x\n", (int)&fecp->galr, fecp->galr);
	printf("x_wmrk       %x - %x\n", (int)&fecp->tfwr, fecp->tfwr);
	printf("r_bound      %x - %x\n", (int)&fecp->frbr, fecp->frbr);
	printf("r_fstart     %x - %x\n", (int)&fecp->frsr, fecp->frsr);
	printf("r_drng       %x - %x\n", (int)&fecp->erdsr, fecp->erdsr);
	printf("x_drng       %x - %x\n", (int)&fecp->etdsr, fecp->etdsr);
	printf("r_bufsz      %x - %x\n", (int)&fecp->emrbr, fecp->emrbr);

	printf("\n");
	printf("rmon_t_drop        %x - %x\n", (int)&fecp->rmon_t_drop,
	       fecp->rmon_t_drop);
	printf("rmon_t_packets     %x - %x\n", (int)&fecp->rmon_t_packets,
	       fecp->rmon_t_packets);
	printf("rmon_t_bc_pkt      %x - %x\n", (int)&fecp->rmon_t_bc_pkt,
	       fecp->rmon_t_bc_pkt);
	printf("rmon_t_mc_pkt      %x - %x\n", (int)&fecp->rmon_t_mc_pkt,
	       fecp->rmon_t_mc_pkt);
	printf("rmon_t_crc_align   %x - %x\n", (int)&fecp->rmon_t_crc_align,
	       fecp->rmon_t_crc_align);
	printf("rmon_t_undersize   %x - %x\n", (int)&fecp->rmon_t_undersize,
	       fecp->rmon_t_undersize);
	printf("rmon_t_oversize    %x - %x\n", (int)&fecp->rmon_t_oversize,
	       fecp->rmon_t_oversize);
	printf("rmon_t_frag        %x - %x\n", (int)&fecp->rmon_t_frag,
	       fecp->rmon_t_frag);
	printf("rmon_t_jab         %x - %x\n", (int)&fecp->rmon_t_jab,
	       fecp->rmon_t_jab);
	printf("rmon_t_col         %x - %x\n", (int)&fecp->rmon_t_col,
	       fecp->rmon_t_col);
	printf("rmon_t_p64         %x - %x\n", (int)&fecp->rmon_t_p64,
	       fecp->rmon_t_p64);
	printf("rmon_t_p65to127    %x - %x\n", (int)&fecp->rmon_t_p65to127,
	       fecp->rmon_t_p65to127);
	printf("rmon_t_p128to255   %x - %x\n", (int)&fecp->rmon_t_p128to255,
	       fecp->rmon_t_p128to255);
	printf("rmon_t_p256to511   %x - %x\n", (int)&fecp->rmon_t_p256to511,
	       fecp->rmon_t_p256to511);
	printf("rmon_t_p512to1023  %x - %x\n", (int)&fecp->rmon_t_p512to1023,
	       fecp->rmon_t_p512to1023);
	printf("rmon_t_p1024to2047 %x - %x\n", (int)&fecp->rmon_t_p1024to2047,
	       fecp->rmon_t_p1024to2047);
	printf("rmon_t_p_gte2048   %x - %x\n", (int)&fecp->rmon_t_p_gte2048,
	       fecp->rmon_t_p_gte2048);
	printf("rmon_t_octets      %x - %x\n", (int)&fecp->rmon_t_octets,
	       fecp->rmon_t_octets);

	printf("\n");
	printf("ieee_t_drop      %x - %x\n", (int)&fecp->ieee_t_drop,
	       fecp->ieee_t_drop);
	printf("ieee_t_frame_ok  %x - %x\n", (int)&fecp->ieee_t_frame_ok,
	       fecp->ieee_t_frame_ok);
	printf("ieee_t_1col      %x - %x\n", (int)&fecp->ieee_t_1col,
	       fecp->ieee_t_1col);
	printf("ieee_t_mcol      %x - %x\n", (int)&fecp->ieee_t_mcol,
	       fecp->ieee_t_mcol);
	printf("ieee_t_def       %x - %x\n", (int)&fecp->ieee_t_def,
	       fecp->ieee_t_def);
	printf("ieee_t_lcol      %x - %x\n", (int)&fecp->ieee_t_lcol,
	       fecp->ieee_t_lcol);
	printf("ieee_t_excol     %x - %x\n", (int)&fecp->ieee_t_excol,
	       fecp->ieee_t_excol);
	printf("ieee_t_macerr    %x - %x\n", (int)&fecp->ieee_t_macerr,
	       fecp->ieee_t_macerr);
	printf("ieee_t_cserr     %x - %x\n", (int)&fecp->ieee_t_cserr,
	       fecp->ieee_t_cserr);
	printf("ieee_t_sqe       %x - %x\n", (int)&fecp->ieee_t_sqe,
	       fecp->ieee_t_sqe);
	printf("ieee_t_fdxfc     %x - %x\n", (int)&fecp->ieee_t_fdxfc,
	       fecp->ieee_t_fdxfc);
	printf("ieee_t_octets_ok %x - %x\n", (int)&fecp->ieee_t_octets_ok,
	       fecp->ieee_t_octets_ok);

	printf("\n");
	printf("rmon_r_drop        %x - %x\n", (int)&fecp->rmon_r_drop,
	       fecp->rmon_r_drop);
	printf("rmon_r_packets     %x - %x\n", (int)&fecp->rmon_r_packets,
	       fecp->rmon_r_packets);
	printf("rmon_r_bc_pkt      %x - %x\n", (int)&fecp->rmon_r_bc_pkt,
	       fecp->rmon_r_bc_pkt);
	printf("rmon_r_mc_pkt      %x - %x\n", (int)&fecp->rmon_r_mc_pkt,
	       fecp->rmon_r_mc_pkt);
	printf("rmon_r_crc_align   %x - %x\n", (int)&fecp->rmon_r_crc_align,
	       fecp->rmon_r_crc_align);
	printf("rmon_r_undersize   %x - %x\n", (int)&fecp->rmon_r_undersize,
	       fecp->rmon_r_undersize);
	printf("rmon_r_oversize    %x - %x\n", (int)&fecp->rmon_r_oversize,
	       fecp->rmon_r_oversize);
	printf("rmon_r_frag        %x - %x\n", (int)&fecp->rmon_r_frag,
	       fecp->rmon_r_frag);
	printf("rmon_r_jab         %x - %x\n", (int)&fecp->rmon_r_jab,
	       fecp->rmon_r_jab);
	printf("rmon_r_p64         %x - %x\n", (int)&fecp->rmon_r_p64,
	       fecp->rmon_r_p64);
	printf("rmon_r_p65to127    %x - %x\n", (int)&fecp->rmon_r_p65to127,
	       fecp->rmon_r_p65to127);
	printf("rmon_r_p128to255   %x - %x\n", (int)&fecp->rmon_r_p128to255,
	       fecp->rmon_r_p128to255);
	printf("rmon_r_p256to511   %x - %x\n", (int)&fecp->rmon_r_p256to511,
	       fecp->rmon_r_p256to511);
	printf("rmon_r_p512to1023  %x - %x\n", (int)&fecp->rmon_r_p512to1023,
	       fecp->rmon_r_p512to1023);
	printf("rmon_r_p1024to2047 %x - %x\n", (int)&fecp->rmon_r_p1024to2047,
	       fecp->rmon_r_p1024to2047);
	printf("rmon_r_p_gte2048   %x - %x\n", (int)&fecp->rmon_r_p_gte2048,
	       fecp->rmon_r_p_gte2048);
	printf("rmon_r_octets      %x - %x\n", (int)&fecp->rmon_r_octets,
	       fecp->rmon_r_octets);

	printf("\n");
	printf("ieee_r_drop      %x - %x\n", (int)&fecp->ieee_r_drop,
	       fecp->ieee_r_drop);
	printf("ieee_r_frame_ok  %x - %x\n", (int)&fecp->ieee_r_frame_ok,
	       fecp->ieee_r_frame_ok);
	printf("ieee_r_crc       %x - %x\n", (int)&fecp->ieee_r_crc,
	       fecp->ieee_r_crc);
	printf("ieee_r_align     %x - %x\n", (int)&fecp->ieee_r_align,
	       fecp->ieee_r_align);
	printf("ieee_r_macerr    %x - %x\n", (int)&fecp->ieee_r_macerr,
	       fecp->ieee_r_macerr);
	printf("ieee_r_fdxfc     %x - %x\n", (int)&fecp->ieee_r_fdxfc,
	       fecp->ieee_r_fdxfc);
	printf("ieee_r_octets_ok %x - %x\n", (int)&fecp->ieee_r_octets_ok,
	       fecp->ieee_r_octets_ok);

	printf("\n\n\n");
}
#endif

int fec_init(struct eth_device *dev, bd_t * bd)
{
	struct fec_info_s *info = dev->priv;
	volatile fec_t *fecp = (fec_t *) (info->iobase);
	int i;
	u8 *ea = NULL;

	fecpin_setclear(dev, 1);

	fec_reset(dev);

#if defined(CONFIG_CMD_MII) || defined (CONFIG_MII) || \
	defined (CFG_DISCOVER_PHY)

	mii_init();

	setFecDuplexSpeed(fecp, bd, info->dup_spd);
#else
#ifndef CFG_DISCOVER_PHY
	setFecDuplexSpeed(fecp, bd, (FECDUPLEX << 16) | FECSPEED);
#endif				/* ifndef CFG_DISCOVER_PHY */
#endif				/* CONFIG_CMD_MII || CONFIG_MII */

	/* We use strictly polling mode only */
	fecp->eimr = 0;

	/* Clear any pending interrupt */
	fecp->eir = 0xffffffff;

	/* Set station address   */
	if ((u32) fecp == CFG_FEC0_IOBASE) {
		ea = &bd->bi_enetaddr[0];
	} else {
#ifdef CFG_FEC1_IOBASE
		ea = &bd->bi_enet1addr[0];
#endif
	}

	fecp->palr = (ea[0] << 24) | (ea[1] << 16) | (ea[2] << 8) | (ea[3]);
	fecp->paur = (ea[4] << 24) | (ea[5] << 16);
#ifdef ET_DEBUG
	printf("Eth Addrs: %02x:%02x:%02x:%02x:%02x:%02x\n",
	       ea[0], ea[1], ea[2], ea[3], ea[4], ea[5]);
#endif

	/* Clear unicast address hash table */
	fecp->iaur = 0;
	fecp->ialr = 0;

	/* Clear multicast address hash table */
	fecp->gaur = 0;
	fecp->galr = 0;

	/* Set maximum receive buffer size. */
	fecp->emrbr = PKT_MAXBLR_SIZE;

	/*
	 * Setup Buffers and Buffer Desriptors
	 */
	info->rxIdx = 0;
	info->txIdx = 0;

	/*
	 * Setup Receiver Buffer Descriptors (13.14.24.18)
	 * Settings:
	 *     Empty, Wrap
	 */
	for (i = 0; i < PKTBUFSRX; i++) {
		info->rxbd[i].cbd_sc = BD_ENET_RX_EMPTY;
		info->rxbd[i].cbd_datlen = 0;	/* Reset */
		info->rxbd[i].cbd_bufaddr = (uint) NetRxPackets[i];
	}
	info->rxbd[PKTBUFSRX - 1].cbd_sc |= BD_ENET_RX_WRAP;

	/*
	 * Setup Ethernet Transmitter Buffer Descriptors (13.14.24.19)
	 * Settings:
	 *    Last, Tx CRC
	 */
	for (i = 0; i < TX_BUF_CNT; i++) {
		info->txbd[i].cbd_sc = BD_ENET_TX_LAST | BD_ENET_TX_TC;
		info->txbd[i].cbd_datlen = 0;	/* Reset */
		info->txbd[i].cbd_bufaddr = (uint) (&info->txbuf[0]);
	}
	info->txbd[TX_BUF_CNT - 1].cbd_sc |= BD_ENET_TX_WRAP;

	/* Set receive and transmit descriptor base */
	fecp->erdsr = (unsigned int)(&info->rxbd[0]);
	fecp->etdsr = (unsigned int)(&info->txbd[0]);

	/* Now enable the transmit and receive processing */
	fecp->ecr |= FEC_ECR_ETHER_EN;

	/* And last, try to fill Rx Buffer Descriptors */
	fecp->rdar = 0x01000000;	/* Descriptor polling active    */

	return 1;
}

void fec_reset(struct eth_device *dev)
{
	struct fec_info_s *info = dev->priv;
	volatile fec_t *fecp = (fec_t *) (info->iobase);
	int i;

	fecp->ecr = FEC_ECR_RESET;
	for (i = 0; (fecp->ecr & FEC_ECR_RESET) && (i < FEC_RESET_DELAY); ++i) {
		udelay(1);
	}
	if (i == FEC_RESET_DELAY) {
		printf("FEC_RESET_DELAY timeout\n");
	}
}

void fec_halt(struct eth_device *dev)
{
	struct fec_info_s *info = dev->priv;

	fec_reset(dev);

	fecpin_setclear(dev, 0);

	info->rxIdx = info->txIdx = 0;
	memset(info->rxbd, 0, PKTBUFSRX * sizeof(cbd_t));
	memset(info->txbd, 0, TX_BUF_CNT * sizeof(cbd_t));
	memset(info->txbuf, 0, DBUF_LENGTH);
}

int mcffec_initialize(bd_t * bis)
{
	struct eth_device *dev;
	int i;

	for (i = 0; i < sizeof(fec_info) / sizeof(fec_info[0]); i++) {

		dev =
		    (struct eth_device *)memalign(CFG_CACHELINE_SIZE,
						  sizeof *dev);
		if (dev == NULL)
			hang();

		memset(dev, 0, sizeof(*dev));

		sprintf(dev->name, "FEC%d", fec_info[i].index);

		dev->priv = &fec_info[i];
		dev->init = fec_init;
		dev->halt = fec_halt;
		dev->send = fec_send;
		dev->recv = fec_recv;

		/* setup Receive and Transmit buffer descriptor */
		fec_info[i].rxbd =
		    (cbd_t *) memalign(CFG_CACHELINE_SIZE,
				       (PKTBUFSRX * sizeof(cbd_t)));
		fec_info[i].txbd =
		    (cbd_t *) memalign(CFG_CACHELINE_SIZE,
				       (TX_BUF_CNT * sizeof(cbd_t)));
		fec_info[i].txbuf =
		    (char *)memalign(CFG_CACHELINE_SIZE, DBUF_LENGTH);
#ifdef ET_DEBUG
		printf("rxbd %x txbd %x\n",
		       (int)fec_info[i].rxbd, (int)fec_info[i].txbd);
#endif

		fec_info[i].phy_name = (char *)memalign(CFG_CACHELINE_SIZE, 32);

		eth_register(dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
		miiphy_register(dev->name,
				mcffec_miiphy_read, mcffec_miiphy_write);
#endif
	}

	/* default speed */
	bis->bi_ethspeed = 10;

	return 1;
}

#endif				/* CONFIG_CMD_NET, FEC_ENET & NET_MULTI */
#endif				/* CONFIG_MCFFEC */
