/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 *
 * Based in part on arch/powerpc/cpu/mpc8260/ether_scc.c.
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
#include <net.h>

#ifdef CONFIG_KEYMILE_HDLC_ENET
#ifdef TEST_IT
#include <command.h>
#endif

#include "keymile_hdlc_enet.h"

extern char keymile_slot;	/* our slot number in the backplane */

/* Allow up to about 50 ms for sending */
#define TOUT_LOOP	50000

/*
 * Since, except during initialization, ethact is always HDLC
 * while we're in the driver, just use serial_printf() everywhere for
 * output.  This avoids possible conflicts when netconsole is being
 * used.
 */
#define dprintf(fmt, args...)	serial_printf(fmt, ##args)

/* Cannot use the storage from net.c because we allocate larger buffers */
static volatile uchar MyPktBuf[HDLC_PKTBUFSRX * PKT_MAXBLR_SIZE + PKTALIGN];
static volatile uchar *MyRxPackets[HDLC_PKTBUFSRX]; /* Receive packet */

static unsigned int keymile_rxIdx;	/* index of the current RX buffer */

static IPaddr_t cachedNumbers[CACHEDNUMBERS]; /* 4 bytes per entry */
void initCachedNumbers(int);

/*
  * SCC Ethernet Tx and Rx buffer descriptors allocated at the
  *  immr->udata_bd address on Dual-Port RAM
  * Provide for Double Buffering
  */
typedef volatile struct CommonBufferDescriptor {
    cbd_t txbd;			/* Tx BD */
    cbd_t rxbd[HDLC_PKTBUFSRX];	/* Rx BD */
} RTXBD;

/*
 * This must be extern because it is allocated in DPRAM using CPM-sepcific
 * code.
 */
static RTXBD *rtx;

static int keymile_hdlc_enet_send(struct eth_device *, volatile void *, int);
static int keymile_hdlc_enet_recv(struct eth_device *);
void keymile_hdlc_enet_init_bds(RTXBD *);
extern int keymile_hdlc_enet_init(struct eth_device *, bd_t *);
extern void keymile_hdlc_enet_halt(struct eth_device *);

/* flags in the buffer descriptor not defined anywhere else */
#define BD_SC_CT	BD_SC_CD
#define BD_SC_CR	0x04
#define BD_SC_DE	0x80
#ifndef BD_SC_TC
#define BD_SC_TC	((ushort)0x0400)	/* Transmit CRC */
#endif
#define BD_SC_FIRST	BD_SC_TC
#define BD_SC_STATS (BD_SC_BR | BD_SC_FR | BD_SC_PR | BD_SC_CR | BD_SC_CD \
		| BD_SC_OV | BD_SC_DE)

#if defined(TEST_RX) || defined(TEST_TX) || defined(TEST_IT)
static void hexdump(unsigned char *buf, int len)
{
	int i;
	const int bytesPerLine = 32;

	if (len > 4 * bytesPerLine)
		len = 4 * bytesPerLine;
	dprintf("\t address: %08x\n", (unsigned int)buf);
	for (i = 0; i < len; i++) {
		if (i % bytesPerLine == 0)
			dprintf("%04x: ", (unsigned short)i);
		dprintf("%02x ", buf[i]);
		if ((i + 1) % bytesPerLine == 0) {
			dprintf("\n");
			continue;
		}
		if ((i + 1) % 8 == 0)
			printf(" ");
	}
	if (len % bytesPerLine)
		dprintf("\n");
}
#endif

int keymile_hdlc_enet_initialize(bd_t *bis)
{
	struct eth_device *dev;

	dev = (struct eth_device *) malloc(sizeof *dev);
	memset(dev, 0, sizeof *dev);
#ifdef TEST_IT
	seth = dev;
#endif

	sprintf(dev->name, "HDLC");
	dev->init   = keymile_hdlc_enet_init;
	dev->halt   = keymile_hdlc_enet_halt;
	dev->send   = keymile_hdlc_enet_send;
	dev->recv   = keymile_hdlc_enet_recv;

	eth_register(dev);

	return 1;
}

/*
 * This is called from the board-specific driver after rtx is allocated.
 */
void keymile_hdlc_enet_init_bds(RTXBD *board_rtx)
{
	volatile cbd_t *bdp;
	int i;

	rtx = board_rtx;
	keymile_rxIdx = 0;
	/*
	 * Initialize the buffer descriptors.
	 */
	bdp = &rtx->txbd;
	bdp->cbd_sc = 0;
	bdp->cbd_bufaddr = 0;
	bdp->cbd_sc = BD_SC_WRAP;

	/*
	 *	Setup RX packet buffers, aligned correctly.
	 *	Borrowed from net/net.c.
	 */
	MyRxPackets[0] = &MyPktBuf[0] + (PKTALIGN - 1);
	MyRxPackets[0] -= (ulong)MyRxPackets[0] % PKTALIGN;
	for (i = 1; i < HDLC_PKTBUFSRX; i++)
		MyRxPackets[i] = MyRxPackets[0] + i * PKT_MAXBLR_SIZE;

	bdp = &rtx->rxbd[0];
	for (i = 0; i < HDLC_PKTBUFSRX; i++) {
		bdp->cbd_sc = BD_SC_EMPTY;
		/* Leave space at the start for INET header. */
		bdp->cbd_bufaddr = (unsigned int)(MyRxPackets[i] +
			INET_HDR_ALIGN);
		bdp++;
	}
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;
}

/*
 * This returns the current port number for NETCONSOLE.  If nc_port
 * in netconsole.c weren't declared static we wouldn't need this.
 */
static short get_netcons_port(void)
{
	char *p;
	short nc_port;

	nc_port = 6666; /* default */

	p = getenv("ncip");
	if (p != NULL) {
		p = strchr(p, ':');
		if (p != NULL)
			nc_port = simple_strtoul(p + 1, NULL, 10);
	}

	return htons(nc_port);
}

/*
 * Read the port numbers from the variables
 */
void initCachedNumbers(int verbose)
{
	char *str;
	ushort port;

	/* already in network order */
	cachedNumbers[IP_ADDR] = getenv_IPaddr("ipaddr");
	/* already in network order */
	cachedNumbers[IP_SERVER] = getenv_IPaddr("serverip");
	str = getenv("tftpsrcp");
	if (str != NULL) {
		/* avoid doing htons() again and again */
		port = htons((ushort)simple_strtol(str, NULL, 10));
		cachedNumbers[TFTP_SRC_PORT] = port;
	} else
		/* this can never be a valid port number */
		cachedNumbers[TFTP_SRC_PORT] = (ulong)-1;
	str = getenv("tftpdstp");
	if (str != NULL) {
		/* avoid doing htons() again and again */
		port = htons((ushort)simple_strtol(str, NULL, 10));
		cachedNumbers[TFTP_DST_PORT] = port;
	} else
		/* this is the default value */
		cachedNumbers[TFTP_DST_PORT] = htons(WELL_KNOWN_PORT);
	/* already in network order */
	cachedNumbers[NETCONS_PORT] = get_netcons_port();
	if (verbose) {
		dprintf("\nIP Number Initialization:\n");
		dprintf(" ip address          %08lx\n", cachedNumbers[IP_ADDR]);
		dprintf(" server ip address   %08lx\n",
			cachedNumbers[IP_SERVER]);
		dprintf(" tftp client port    %ld\n",
			cachedNumbers[TFTP_SRC_PORT]);
		dprintf(" tftp server port    %ld\n",
			cachedNumbers[TFTP_DST_PORT]);
		dprintf(" netcons port        %ld\n",
			cachedNumbers[NETCONS_PORT]);
		dprintf(" slot number (hex)   %02x\n", keymile_slot);
	}
}

static void keymile_hdlc_enet_doarp(volatile void *packet, int len)
{
	ARP_t *arp;
	IPaddr_t src_ip; /* U-Boot's IP */
	IPaddr_t dest_ip; /* the mgcoge's IP */
	unsigned char *packet_copy = malloc(len);

	/*
	 * Handling an ARP request means that a new transfer has started.
	 * Update our cached parameters now.
	 */
	initCachedNumbers(0); /* may reinit port numbers */

	/* special handling required for ARP */
	arp = (ARP_t *)(packet + ETHER_HDR_SIZE);
	/*
	 *	XXXX
	 * This is pretty dirty!  NetReceive only uses
	 * a few fields when handling an ARP reply, so
	 * we only modify those here.  This could
	 * result in catastrophic failure at a later
	 * time if the handler is modified!
	 */
	arp->ar_op = htons(ARPOP_REPLY);
	/* save his/our IP */
	src_ip = NetReadIP(&arp->ar_data[6]);
	dest_ip = NetReadIP(&arp->ar_data[16]);
	/* copy target IP to source IP */
	NetCopyIP(&arp->ar_data[6], &dest_ip);
	/* copy our IP to the right place */
	NetCopyIP(&arp->ar_data[16], &src_ip);
	/* always use 0x7f as the MAC for the coge */
	arp->ar_data[0] = HDLC_UACUA;
	/*
	 * copy the packet
	 * if NetReceive wants to write to stdout, it may overwrite packet
	 * especially if stdout is set to nc!
	 *
	 * However, if the malloc() above fails then we can still try the
	 * original packet, rather than causing the transfer to fail.
	 */
	if (packet_copy != NULL) {
		memcpy(packet_copy, (char *)packet, len);
		NetReceive(packet_copy, len);
		free(packet_copy);
	} else
		NetReceive(packet, len);
}

/*
 * NOTE all callers ignore the returned value!
 * At the moment this only handles ARP Requests, TFTP and NETCONSOLE.
 */
static int keymile_hdlc_enet_send(struct eth_device *dev, volatile void *packet,
	int len)
{
	int j;
	uint data_addr;
	int data_len;
	struct icn_hdr header;
	struct icn_frame *frame;
	Ethernet_t *et;
	ARP_t *arp;
	IP_t *ip;

	if (len > (MAX_FRAME_LENGTH - sizeof(header)))
		return -1;

	frame = NULL;
	et = NULL;
	arp = NULL;
	ip = NULL;

	j = 0;
	while ((rtx->txbd.cbd_sc & BD_SC_READY) && (j < TOUT_LOOP)) {
		/* will also trigger Wd if needed, but maybe too often  */
		udelay(1);
		j++;
	}
	if (j >= TOUT_LOOP) {
		dprintf("TX not ready sc %x\n", rtx->txbd.cbd_sc);
		return -1;
	}

	/*
	 * First check for an ARP Request since this requires special handling.
	 */
	if (len >= (ARP_HDR_SIZE + ETHER_HDR_SIZE)) {
		et = (Ethernet_t *)packet;
		arp = (ARP_t *)(((char *)et) + ETHER_HDR_SIZE);
		/* ARP and REQUEST? */
		if (et->et_protlen == PROT_ARP &&
			arp->ar_op == htons(ARPOP_REQUEST)) {
			/* just short-circuit the request on the U-Boot side */
			keymile_hdlc_enet_doarp(packet, len);
			return 0;
		}
	}

	/*
	 * GJ - I suppose the assumption here that len will always be
	 * > INET_HDR_SIZE is alright as long as the network stack
	 * isn't changed.
	 * Do not send INET header.
	 */
	data_len = len + sizeof(header) - INET_HDR_SIZE;
	frame = (struct icn_frame *) (((char *)packet) + INET_HDR_SIZE -
		sizeof(header));

#ifdef TEST_TX
	printf("frame: %08x, ", frame);
	hexdump((unsigned char *)packet, data_len + INET_HDR_SIZE);
#endif

	data_addr = (uint)frame;
	if (len >= (IP_HDR_SIZE + ETHER_HDR_SIZE))
		ip = (IP_t *)(packet + ETHER_HDR_SIZE);
	/* Is it TFTP? TFTP always uses UDP and the cached dport */
	if (ip != NULL && ip->ip_p == IPPROTO_UDP && ip->udp_dst ==
			(ushort)cachedNumbers[TFTP_DST_PORT]) {
		/* just in case the port wasn't set in the environment */
		if (cachedNumbers[TFTP_SRC_PORT] == (ulong)-1)
			cachedNumbers[TFTP_SRC_PORT] = ip->udp_src;
		frame->hdr.application = MGS_TFTP;
	}
	/*
	 * Is it NETCONSOLE?  NETCONSOLE always uses UDP.
	 */
	else if (ip != NULL && ip->ip_p == IPPROTO_UDP
		&& ip->udp_dst == (ushort)cachedNumbers[NETCONS_PORT]) {
			frame->hdr.application = MGS_NETCONS;
	} else {
		/* reject unknown packets */
		/* may do some check on frame->hdr.application */
		dprintf("Unknown packet type in %s, rejected\n",
			__func__);
		return -1;
	}
	/*
	 * Could extract the target's slot ID from its MAC here,
	 * but u-boot only wants to talk to the active server.
	 *
	 * avoid setting new source address when moving to another slot
	 */
	frame->hdr.src_addr = keymile_slot;
	frame->hdr.dest_addr = HDLC_UACUA;
#ifdef TEST_TX
	{
		dprintf("TX: ");
		hexdump((unsigned char *)data_addr, data_len);
	}
#endif

	flush_cache(data_addr, data_len);
	rtx->txbd.cbd_bufaddr = data_addr;
	rtx->txbd.cbd_datlen = data_len;
	rtx->txbd.cbd_sc |= (BD_SC_READY | BD_SC_TC | BD_SC_LAST | BD_SC_WRAP);

	while ((rtx->txbd.cbd_sc & BD_SC_READY) && (j < TOUT_LOOP)) {
		/* will also trigger Wd if needed, but maybe too often  */
		udelay(1);
		j++;
	}
	if (j >= TOUT_LOOP)
		dprintf("TX timeout\n");
#ifdef ET_DEBUG
	dprintf("cycles: %d    status: %x\n", j, rtx->txbd.cbd_sc);
#endif
	j = (rtx->txbd.cbd_sc & BD_SC_STATS); /* return only status bits */
	return j;
}

/*
 * During a receive, the RxIdx points to the current incoming buffer.
 * When we update through the ring, if the next incoming buffer has
 * not been given to the system, we just set the empty indicator,
 * effectively tossing the packet.
 */
static int keymile_hdlc_enet_recv(struct eth_device *dev)
{
	int length;
	unsigned char app;
	struct icn_frame *fp;
	Ethernet_t *ep;
	IP_t *ip;

	for (;;) {
		if (rtx->rxbd[keymile_rxIdx].cbd_sc & BD_SC_EMPTY) {
			length = -1;
			break;	/* nothing received - leave for() loop */
		}

		length = rtx->rxbd[keymile_rxIdx].cbd_datlen;
#ifdef TEST_RX
		dprintf("packet %d bytes long\n", length);
#endif

		/*
		 * BD_SC_BR -> LG bit
		 * BD_SC_FR -> NO bit
		 * BD_SC_PR -> AB bit
		 * BD_SC_NAK -> CR bit
		 * 0x80 -> DE bit
		 */
		if (rtx->rxbd[keymile_rxIdx].cbd_sc & BD_SC_STATS) {
#ifdef ET_DEBUG
			dprintf("err: %x\n", rtx->rxbd[keymile_rxIdx].cbd_sc);
#endif
		} else if (length > MAX_FRAME_LENGTH) { /* can't happen */
#ifdef ET_DEBUG
			dprintf("err: packet too big\n");
#endif
		} else {
			fp = (struct icn_frame *)(MyRxPackets[keymile_rxIdx] +
				INET_HDR_ALIGN - INET_HDR_SIZE);
#ifdef TEST_RX
			dprintf("RX %d: ", keymile_rxIdx);
			hexdump((unsigned char *)MyRxPackets[keymile_rxIdx],
				INET_HDR_ALIGN + INET_HDR_SIZE + 4);
#endif
			/* copy icn header to the beginning */
			memcpy(fp, ((char *)fp + INET_HDR_SIZE),
				sizeof(struct icn_hdr));
			app = fp->hdr.application;
			if (app == MGS_NETCONS || app == MGS_TFTP) {
				struct icn_hdr *ih = &fp->hdr;
				unsigned char icn_src_addr = ih->src_addr;
				unsigned char icn_dest_addr = ih->dest_addr;

				/*
				 * expand header by INET_HDR_SIZE
				 */
				length += INET_HDR_SIZE;
				/* initalize header */
				memset((char *)fp->data, 0x00, INET_HDR_SIZE);
				ep = (Ethernet_t *)fp->data;
				/* set MACs */
				ep->et_dest[0] = icn_dest_addr;
				ep->et_src[0] = icn_src_addr;
				ep->et_protlen = htons(PROT_IP);
				/* set ip stuff */
				ip = (IP_t *)(fp->data + ETHER_HDR_SIZE);
				/* set ip addresses */
				ip->ip_src = cachedNumbers[IP_SERVER];
				ip->ip_dst = cachedNumbers[IP_ADDR];
				/* ip length */
				ip->ip_len = htons(length - ETHER_HDR_SIZE -
					REMOVE);
				/* ip proto */
				ip->ip_p = IPPROTO_UDP;
				switch (app) {
				case MGS_TFTP:
					/* swap src/dst port numbers */
					ip->udp_src = (ushort)
						cachedNumbers[TFTP_DST_PORT];
					ip->udp_dst = (ushort)
						cachedNumbers[TFTP_SRC_PORT];
					ip->udp_len = ip->ip_len -
						IP_HDR_SIZE_NO_UDP;
					ip->udp_xsum = 0;
					break;
				case MGS_NETCONS:
					ip->udp_src = (ushort)
						cachedNumbers[NETCONS_PORT];
					/*
					 * in drivers/net/netconsole.c src port
					 * equals dest port
					 */
					ip->udp_dst = ip->udp_src;
					ip->udp_len = ip->ip_len -
						IP_HDR_SIZE_NO_UDP;
					ip->udp_xsum = 0;
					break;
				}
				/* ip version */
				ip->ip_hl_v = (0x40) | (0x0f &
					(IP_HDR_SIZE_NO_UDP / 4));
				ip->ip_tos = 0;
				ip->ip_id = 0;
				/* flags, fragment offset */
				ip->ip_off = htons(0x4000);
				ip->ip_ttl = 255; /* time to live */
				/* have to fixup the checksum */
				ip->ip_sum = ~NetCksum((uchar *)ip,
					IP_HDR_SIZE_NO_UDP / 2);
				/*
				 * Pass the packet up to the protocol layers
				 * but remove dest_addr, src_addr, application
				 * and the CRC.
				 */
#ifdef TEST_RX
				hexdump((unsigned char *)fp->data,
					INET_HDR_SIZE + 4);
#endif
				NetReceive(fp->data, length - REMOVE);
			} else {
				/*
				 * the other application types are not yet
				 * supported by u-boot.
				 */
				/* normally drop it */
#ifdef TEST_NO
				/* send it anyway */
				fp = (struct icn_frame *)
					(MyRxPackets[keymile_rxIdx] +
						INET_HDR_ALIGN);
				NetReceive(fp->data, length - REMOVE);
#endif
			}
		}

		/* Give the buffer back to the SCC. */
		rtx->rxbd[keymile_rxIdx].cbd_datlen = 0;

		/* wrap around buffer index when necessary */
		if ((keymile_rxIdx + 1) >= HDLC_PKTBUFSRX) {
			rtx->rxbd[HDLC_PKTBUFSRX - 1].cbd_sc =
				(BD_SC_WRAP | BD_SC_EMPTY);
			keymile_rxIdx = 0;
		} else {
			rtx->rxbd[keymile_rxIdx].cbd_sc = BD_SC_EMPTY;
			keymile_rxIdx++;
		}
	}
	return length;
}

#ifdef TEST_IT
/* simple send test routine */
int hdlc_enet_stest(struct cmd_tbl_s *a, int b, int c, char **d)
{
	unsigned char pkt[2];
	int ret;

	dprintf("enter stest\n");
	/* may have to initialize things */
	if (seth->state != ETH_STATE_ACTIVE) {
		/* the bd_t* is not used */
		if (seth->init(seth, NULL) >= 0)
			seth->state = ETH_STATE_ACTIVE;
	}
	pkt[0] = 0xea;
	pkt[1] = 0xae;
	ret = keymile_hdlc_enet_send(seth, pkt, 2);
	dprintf("return from send %x\n", ret);
	dprintf("exit stest\n");
	return ret;
}
U_BOOT_CMD(
	stest, 1, 1, hdlc_enet_stest,
	"simple send test for hdlc_enet",
	""
);
/* simple receive test routine */
int hdlc_enet_rtest(struct cmd_tbl_s *a, int b, int c, char **d)
{
	int ret;

	dprintf("enter rtest\n");
	/* may have to initialize things */
	if (seth->state != ETH_STATE_ACTIVE) {
		/* the bd_t* is not used */
		if (seth->init(seth, NULL) >= 0)
			seth->state = ETH_STATE_ACTIVE;
	}
	ret = keymile_hdlc_enet_recv(seth);
	dprintf("return from recv %x\n", ret);
	dprintf("exit rtest\n");
	return ret;
}
U_BOOT_CMD(
	rtest, 1, 1, hdlc_enet_rtest,
	"simple receive test for hdlc_enet",
	""
);
#endif

#endif /* CONFIG_KEYMILE_HDLC_ENET */
