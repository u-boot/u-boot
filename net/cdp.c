/*
 *	Copied from Linux Monitor (LiMon) - Networking.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 *	Copyright 2000-2002 Wolfgang Denk, wd@denx.de
 */

#include <common.h>
#include <net.h>
#if defined(CONFIG_CDP_VERSION)
#include <timestamp.h>
#endif

#include "cdp.h"

/* Ethernet bcast address */
const uchar NetCDPAddr[6] = { 0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc };

#define CDP_DEVICE_ID_TLV		0x0001
#define CDP_ADDRESS_TLV			0x0002
#define CDP_PORT_ID_TLV			0x0003
#define CDP_CAPABILITIES_TLV		0x0004
#define CDP_VERSION_TLV			0x0005
#define CDP_PLATFORM_TLV		0x0006
#define CDP_NATIVE_VLAN_TLV		0x000a
#define CDP_APPLIANCE_VLAN_TLV		0x000e
#define CDP_TRIGGER_TLV			0x000f
#define CDP_POWER_CONSUMPTION_TLV	0x0010
#define CDP_SYSNAME_TLV			0x0014
#define CDP_SYSOBJECT_TLV		0x0015
#define CDP_MANAGEMENT_ADDRESS_TLV	0x0016

#define CDP_TIMEOUT			250UL	/* one packet every 250ms */

static int CDPSeq;
static int CDPOK;

ushort CDPNativeVLAN;
ushort CDPApplianceVLAN;

static const uchar CDP_SNAP_hdr[8] = {
	0xAA, 0xAA, 0x03, 0x00, 0x00, 0x0C, 0x20, 0x00 };

static ushort
CDP_compute_csum(const uchar *buff, ushort len)
{
	ushort csum;
	int     odd;
	ulong   result = 0;
	ushort  leftover;
	ushort *p;

	if (len > 0) {
		odd = 1 & (ulong)buff;
		if (odd) {
			result = *buff << 8;
			len--;
			buff++;
		}
		while (len > 1) {
			p = (ushort *)buff;
			result += *p++;
			buff = (uchar *)p;
			if (result & 0x80000000)
				result = (result & 0xFFFF) + (result >> 16);
			len -= 2;
		}
		if (len) {
			leftover = (signed short)(*(const signed char *)buff);
			/*
			 * CISCO SUCKS big time! (and blows too):
			 * CDP uses the IP checksum algorithm with a twist;
			 * for the last byte it *sign* extends and sums.
			 */
			result = (result & 0xffff0000) |
				 ((result + leftover) & 0x0000ffff);
		}
		while (result >> 16)
			result = (result & 0xFFFF) + (result >> 16);

		if (odd)
			result = ((result >> 8) & 0xff) |
				 ((result & 0xff) << 8);
	}

	/* add up 16-bit and 17-bit words for 17+c bits */
	result = (result & 0xffff) + (result >> 16);
	/* add up 16-bit and 2-bit for 16+c bit */
	result = (result & 0xffff) + (result >> 16);
	/* add up carry.. */
	result = (result & 0xffff) + (result >> 16);

	/* negate */
	csum = ~(ushort)result;

	/* run time endian detection */
	if (csum != htons(csum))	/* little endian */
		csum = htons(csum);

	return csum;
}

static int
CDPSendTrigger(void)
{
	uchar *pkt;
	ushort *s;
	ushort *cp;
	struct ethernet_hdr *et;
	int len;
	ushort chksum;
#if	defined(CONFIG_CDP_DEVICE_ID) || defined(CONFIG_CDP_PORT_ID)   || \
	defined(CONFIG_CDP_VERSION)   || defined(CONFIG_CDP_PLATFORM)
	char buf[32];
#endif

	pkt = NetTxPacket;
	et = (struct ethernet_hdr *)pkt;

	/* NOTE: trigger sent not on any VLAN */

	/* form ethernet header */
	memcpy(et->et_dest, NetCDPAddr, 6);
	memcpy(et->et_src, NetOurEther, 6);

	pkt += ETHER_HDR_SIZE;

	/* SNAP header */
	memcpy((uchar *)pkt, CDP_SNAP_hdr, sizeof(CDP_SNAP_hdr));
	pkt += sizeof(CDP_SNAP_hdr);

	/* CDP header */
	*pkt++ = 0x02;				/* CDP version 2 */
	*pkt++ = 180;				/* TTL */
	s = (ushort *)pkt;
	cp = s;
	/* checksum (0 for later calculation) */
	*s++ = htons(0);

	/* CDP fields */
#ifdef CONFIG_CDP_DEVICE_ID
	*s++ = htons(CDP_DEVICE_ID_TLV);
	*s++ = htons(CONFIG_CDP_DEVICE_ID);
	sprintf(buf, CONFIG_CDP_DEVICE_ID_PREFIX "%pm", NetOurEther);
	memcpy((uchar *)s, buf, 16);
	s += 16 / 2;
#endif

#ifdef CONFIG_CDP_PORT_ID
	*s++ = htons(CDP_PORT_ID_TLV);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, CONFIG_CDP_PORT_ID, eth_get_dev_index());
	len = strlen(buf);
	if (len & 1)	/* make it even */
		len++;
	*s++ = htons(len + 4);
	memcpy((uchar *)s, buf, len);
	s += len / 2;
#endif

#ifdef CONFIG_CDP_CAPABILITIES
	*s++ = htons(CDP_CAPABILITIES_TLV);
	*s++ = htons(8);
	*(ulong *)s = htonl(CONFIG_CDP_CAPABILITIES);
	s += 2;
#endif

#ifdef CONFIG_CDP_VERSION
	*s++ = htons(CDP_VERSION_TLV);
	memset(buf, 0, sizeof(buf));
	strcpy(buf, CONFIG_CDP_VERSION);
	len = strlen(buf);
	if (len & 1)	/* make it even */
		len++;
	*s++ = htons(len + 4);
	memcpy((uchar *)s, buf, len);
	s += len / 2;
#endif

#ifdef CONFIG_CDP_PLATFORM
	*s++ = htons(CDP_PLATFORM_TLV);
	memset(buf, 0, sizeof(buf));
	strcpy(buf, CONFIG_CDP_PLATFORM);
	len = strlen(buf);
	if (len & 1)	/* make it even */
		len++;
	*s++ = htons(len + 4);
	memcpy((uchar *)s, buf, len);
	s += len / 2;
#endif

#ifdef CONFIG_CDP_TRIGGER
	*s++ = htons(CDP_TRIGGER_TLV);
	*s++ = htons(8);
	*(ulong *)s = htonl(CONFIG_CDP_TRIGGER);
	s += 2;
#endif

#ifdef CONFIG_CDP_POWER_CONSUMPTION
	*s++ = htons(CDP_POWER_CONSUMPTION_TLV);
	*s++ = htons(6);
	*s++ = htons(CONFIG_CDP_POWER_CONSUMPTION);
#endif

	/* length of ethernet packet */
	len = (uchar *)s - ((uchar *)NetTxPacket + ETHER_HDR_SIZE);
	et->et_protlen = htons(len);

	len = ETHER_HDR_SIZE + sizeof(CDP_SNAP_hdr);
	chksum = CDP_compute_csum((uchar *)NetTxPacket + len,
				  (uchar *)s - (NetTxPacket + len));
	if (chksum == 0)
		chksum = 0xFFFF;
	*cp = htons(chksum);

	NetSendPacket(NetTxPacket, (uchar *)s - NetTxPacket);
	return 0;
}

static void
CDPTimeout(void)
{
	CDPSeq++;

	if (CDPSeq < 3) {
		NetSetTimeout(CDP_TIMEOUT, CDPTimeout);
		CDPSendTrigger();
		return;
	}

	/* if not OK try again */
	if (!CDPOK)
		NetStartAgain();
	else
		net_set_state(NETLOOP_SUCCESS);
}

void cdp_receive(const uchar *pkt, unsigned len)
{
	const uchar *t;
	const ushort *ss;
	ushort type, tlen;
	ushort vlan, nvlan;

	/* minimum size? */
	if (len < sizeof(CDP_SNAP_hdr) + 4)
		goto pkt_short;

	/* check for valid CDP SNAP header */
	if (memcmp(pkt, CDP_SNAP_hdr, sizeof(CDP_SNAP_hdr)) != 0)
		return;

	pkt += sizeof(CDP_SNAP_hdr);
	len -= sizeof(CDP_SNAP_hdr);

	/* Version of CDP protocol must be >= 2 and TTL != 0 */
	if (pkt[0] < 0x02 || pkt[1] == 0)
		return;

	/*
	 * if version is greater than 0x02 maybe we'll have a problem;
	 * output a warning
	 */
	if (pkt[0] != 0x02)
		printf("**WARNING: CDP packet received with a protocol version "
				"%d > 2\n", pkt[0] & 0xff);

	if (CDP_compute_csum(pkt, len) != 0)
		return;

	pkt += 4;
	len -= 4;

	vlan = htons(-1);
	nvlan = htons(-1);
	while (len > 0) {
		if (len < 4)
			goto pkt_short;

		ss = (const ushort *)pkt;
		type = ntohs(ss[0]);
		tlen = ntohs(ss[1]);
		if (tlen > len)
			goto pkt_short;

		pkt += tlen;
		len -= tlen;

		ss += 2;	/* point ss to the data of the TLV */
		tlen -= 4;

		switch (type) {
		case CDP_DEVICE_ID_TLV:
			break;
		case CDP_ADDRESS_TLV:
			break;
		case CDP_PORT_ID_TLV:
			break;
		case CDP_CAPABILITIES_TLV:
			break;
		case CDP_VERSION_TLV:
			break;
		case CDP_PLATFORM_TLV:
			break;
		case CDP_NATIVE_VLAN_TLV:
			nvlan = *ss;
			break;
		case CDP_APPLIANCE_VLAN_TLV:
			t = (const uchar *)ss;
			while (tlen > 0) {
				if (tlen < 3)
					goto pkt_short;

				ss = (const ushort *)(t + 1);

#ifdef CONFIG_CDP_APPLIANCE_VLAN_TYPE
				if (t[0] == CONFIG_CDP_APPLIANCE_VLAN_TYPE)
					vlan = *ss;
#else
				/* XXX will this work; dunno */
				vlan = ntohs(*ss);
#endif
				t += 3; tlen -= 3;
			}
			break;
		case CDP_TRIGGER_TLV:
			break;
		case CDP_POWER_CONSUMPTION_TLV:
			break;
		case CDP_SYSNAME_TLV:
			break;
		case CDP_SYSOBJECT_TLV:
			break;
		case CDP_MANAGEMENT_ADDRESS_TLV:
			break;
		}
	}

	CDPApplianceVLAN = vlan;
	CDPNativeVLAN = nvlan;

	CDPOK = 1;
	return;

 pkt_short:
	printf("** CDP packet is too short\n");
	return;
}

void
CDPStart(void)
{
	printf("Using %s device\n", eth_get_name());
	CDPSeq = 0;
	CDPOK = 0;

	CDPNativeVLAN = htons(-1);
	CDPApplianceVLAN = htons(-1);

	NetSetTimeout(CDP_TIMEOUT, CDPTimeout);

	CDPSendTrigger();
}
