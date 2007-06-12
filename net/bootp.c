/*
 *	Based on LiMon - BOOTP.
 *
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 *	Copyright 2000-2004 Wolfgang Denk, wd@denx.de
 */

#if 0
#define DEBUG		1	/* general debug */
#define DEBUG_BOOTP_EXT 1	/* Debug received vendor fields */
#endif

#ifdef DEBUG_BOOTP_EXT
#define debug_ext(fmt,args...)	printf (fmt ,##args)
#else
#define debug_ext(fmt,args...)
#endif

#include <common.h>
#include <command.h>
#include <net.h>
#include "bootp.h"
#include "tftp.h"
#include "nfs.h"
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif

#define BOOTP_VENDOR_MAGIC	0x63825363	/* RFC1048 Magic Cookie		*/

#if (CONFIG_COMMANDS & CFG_CMD_NET) || defined(CONFIG_CMD_NET)

#define TIMEOUT		5		/* Seconds before trying BOOTP again	*/
#ifndef CONFIG_NET_RETRY_COUNT
# define TIMEOUT_COUNT	5		/* # of timeouts before giving up  */
#else
# define TIMEOUT_COUNT	(CONFIG_NET_RETRY_COUNT)
#endif

#define PORT_BOOTPS	67		/* BOOTP server UDP port		*/
#define PORT_BOOTPC	68		/* BOOTP client UDP port		*/

#ifndef CONFIG_DHCP_MIN_EXT_LEN		/* minimal length of extension list	*/
#define CONFIG_DHCP_MIN_EXT_LEN 64
#endif

ulong		BootpID;
int		BootpTry;
#ifdef CONFIG_BOOTP_RANDOM_DELAY
ulong		seed1, seed2;
#endif

#if (CONFIG_COMMANDS & CFG_CMD_DHCP) || defined(CONFIG_CMD_DHCP)
dhcp_state_t dhcp_state = INIT;
unsigned long dhcp_leasetime = 0;
IPaddr_t NetDHCPServerIP = 0;
static void DhcpHandler(uchar * pkt, unsigned dest, unsigned src, unsigned len);

/* For Debug */
#if 0
static char *dhcpmsg2str(int type)
{
	switch (type) {
	case 1:	 return "DHCPDISCOVER"; break;
	case 2:	 return "DHCPOFFER";	break;
	case 3:	 return "DHCPREQUEST";	break;
	case 4:	 return "DHCPDECLINE";	break;
	case 5:	 return "DHCPACK";	break;
	case 6:	 return "DHCPNACK";	break;
	case 7:	 return "DHCPRELEASE";	break;
	default: return "UNKNOWN/INVALID MSG TYPE"; break;
	}
}
#endif

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_VENDOREX)
extern u8 *dhcp_vendorex_prep (u8 *e); /*rtn new e after add own opts. */
extern u8 *dhcp_vendorex_proc (u8 *e); /*rtn next e if mine,else NULL  */
#endif

#endif	/* CFG_CMD_DHCP */

static int BootpCheckPkt(uchar *pkt, unsigned dest, unsigned src, unsigned len)
{
	Bootp_t *bp = (Bootp_t *) pkt;
	int retval = 0;

	if (dest != PORT_BOOTPC || src != PORT_BOOTPS)
		retval = -1;
	else if (len < sizeof (Bootp_t) - OPT_SIZE)
		retval = -2;
	else if (bp->bp_op != OP_BOOTREQUEST &&
	    bp->bp_op != OP_BOOTREPLY &&
	    bp->bp_op != DHCP_OFFER &&
	    bp->bp_op != DHCP_ACK &&
	    bp->bp_op != DHCP_NAK ) {
		retval = -3;
	}
	else if (bp->bp_htype != HWT_ETHER)
		retval = -4;
	else if (bp->bp_hlen != HWL_ETHER)
		retval = -5;
	else if (NetReadLong((ulong*)&bp->bp_id) != BootpID) {
		retval = -6;
	}

	debug ("Filtering pkt = %d\n", retval);

	return retval;
}

/*
 * Copy parameters of interest from BOOTP_REPLY/DHCP_OFFER packet
 */
static void BootpCopyNetParams(Bootp_t *bp)
{
	IPaddr_t tmp_ip;

	NetCopyIP(&NetOurIP, &bp->bp_yiaddr);
	NetCopyIP(&tmp_ip, &bp->bp_siaddr);
	if (tmp_ip != 0)
		NetCopyIP(&NetServerIP, &bp->bp_siaddr);
	memcpy (NetServerEther, ((Ethernet_t *)NetRxPkt)->et_src, 6);
	if (strlen(bp->bp_file) > 0)
		copy_filename (BootFile, bp->bp_file, sizeof(BootFile));

	debug ("Bootfile: %s\n", BootFile);

	/* Propagate to environment:
	 * don't delete exising entry when BOOTP / DHCP reply does
	 * not contain a new value
	 */
	if (*BootFile) {
		setenv ("bootfile", BootFile);
	}
}

static int truncate_sz (const char *name, int maxlen, int curlen)
{
	if (curlen >= maxlen) {
		printf("*** WARNING: %s is too long (%d - max: %d) - truncated\n",
			name, curlen, maxlen);
		curlen = maxlen - 1;
	}
	return (curlen);
}

#if !((CONFIG_COMMANDS & CFG_CMD_DHCP) || defined(CONFIG_CMD_DHCP))

static void BootpVendorFieldProcess (u8 * ext)
{
	int size = *(ext + 1);

	debug_ext ("[BOOTP] Processing extension %d... (%d bytes)\n", *ext,
		   *(ext + 1));

	NetBootFileSize = 0;

	switch (*ext) {
		/* Fixed length fields */
	case 1:			/* Subnet mask                                  */
		if (NetOurSubnetMask == 0)
			NetCopyIP (&NetOurSubnetMask, (IPaddr_t *) (ext + 2));
		break;
	case 2:			/* Time offset - Not yet supported              */
		break;
		/* Variable length fields */
	case 3:			/* Gateways list                                */
		if (NetOurGatewayIP == 0) {
			NetCopyIP (&NetOurGatewayIP, (IPaddr_t *) (ext + 2));
		}
		break;
	case 4:			/* Time server - Not yet supported              */
		break;
	case 5:			/* IEN-116 name server - Not yet supported      */
		break;
	case 6:
		if (NetOurDNSIP == 0) {
			NetCopyIP (&NetOurDNSIP, (IPaddr_t *) (ext + 2));
		}
#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_DNS2)
		if ((NetOurDNS2IP == 0) && (size > 4)) {
			NetCopyIP (&NetOurDNS2IP, (IPaddr_t *) (ext + 2 + 4));
		}
#endif
		break;
	case 7:			/* Log server - Not yet supported               */
		break;
	case 8:			/* Cookie/Quote server - Not yet supported      */
		break;
	case 9:			/* LPR server - Not yet supported               */
		break;
	case 10:		/* Impress server - Not yet supported           */
		break;
	case 11:		/* RPL server - Not yet supported               */
		break;
	case 12:		/* Host name                                    */
		if (NetOurHostName[0] == 0) {
			size = truncate_sz ("Host Name", sizeof (NetOurHostName), size);
			memcpy (&NetOurHostName, ext + 2, size);
			NetOurHostName[size] = 0;
		}
		break;
	case 13:		/* Boot file size                               */
		if (size == 2)
			NetBootFileSize = ntohs (*(ushort *) (ext + 2));
		else if (size == 4)
			NetBootFileSize = ntohl (*(ulong *) (ext + 2));
		break;
	case 14:		/* Merit dump file - Not yet supported          */
		break;
	case 15:		/* Domain name - Not yet supported              */
		break;
	case 16:		/* Swap server - Not yet supported              */
		break;
	case 17:		/* Root path                                    */
		if (NetOurRootPath[0] == 0) {
			size = truncate_sz ("Root Path", sizeof (NetOurRootPath), size);
			memcpy (&NetOurRootPath, ext + 2, size);
			NetOurRootPath[size] = 0;
		}
		break;
	case 18:		/* Extension path - Not yet supported           */
		/*
		 * This can be used to send the information of the
		 * vendor area in another file that the client can
		 * access via TFTP.
		 */
		break;
		/* IP host layer fields */
	case 40:		/* NIS Domain name                              */
		if (NetOurNISDomain[0] == 0) {
			size = truncate_sz ("NIS Domain Name", sizeof (NetOurNISDomain), size);
			memcpy (&NetOurNISDomain, ext + 2, size);
			NetOurNISDomain[size] = 0;
		}
		break;
		/* Application layer fields */
	case 43:		/* Vendor specific info - Not yet supported     */
		/*
		 * Binary information to exchange specific
		 * product information.
		 */
		break;
		/* Reserved (custom) fields (128..254) */
	}
}

static void BootpVendorProcess (u8 * ext, int size)
{
	u8 *end = ext + size;

	debug_ext ("[BOOTP] Checking extension (%d bytes)...\n", size);

	while ((ext < end) && (*ext != 0xff)) {
		if (*ext == 0) {
			ext++;
		} else {
			u8 *opt = ext;

			ext += ext[1] + 2;
			if (ext <= end)
				BootpVendorFieldProcess (opt);
		}
	}

#ifdef DEBUG_BOOTP_EXT
	puts ("[BOOTP] Received fields: \n");
	if (NetOurSubnetMask) {
		puts ("NetOurSubnetMask : ");
		print_IPaddr (NetOurSubnetMask);
		putc ('\n');
	}

	if (NetOurGatewayIP) {
		puts ("NetOurGatewayIP	: ");
		print_IPaddr (NetOurGatewayIP);
		putc ('\n');
	}

	if (NetBootFileSize) {
		printf ("NetBootFileSize : %d\n", NetBootFileSize);
	}

	if (NetOurHostName[0]) {
		printf ("NetOurHostName  : %s\n", NetOurHostName);
	}

	if (NetOurRootPath[0]) {
		printf ("NetOurRootPath  : %s\n", NetOurRootPath);
	}

	if (NetOurNISDomain[0]) {
		printf ("NetOurNISDomain : %s\n", NetOurNISDomain);
	}

	if (NetBootFileSize) {
		printf ("NetBootFileSize: %d\n", NetBootFileSize);
	}
#endif /* DEBUG_BOOTP_EXT */
}
/*
 *	Handle a BOOTP received packet.
 */
static void
BootpHandler(uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	Bootp_t *bp;
	char	*s;

	debug ("got BOOTP packet (src=%d, dst=%d, len=%d want_len=%d)\n",
		src, dest, len, sizeof (Bootp_t));

	bp = (Bootp_t *)pkt;

	if (BootpCheckPkt(pkt, dest, src, len)) /* Filter out pkts we don't want */
		return;

	/*
	 *	Got a good BOOTP reply.	 Copy the data into our variables.
	 */
#ifdef CONFIG_STATUS_LED
	status_led_set (STATUS_LED_BOOT, STATUS_LED_OFF);
#endif

	BootpCopyNetParams(bp);		/* Store net parameters from reply */

	/* Retrieve extended information (we must parse the vendor area) */
	if (NetReadLong((ulong*)&bp->bp_vend[0]) == htonl(BOOTP_VENDOR_MAGIC))
		BootpVendorProcess((uchar *)&bp->bp_vend[4], len);

	NetSetTimeout(0, (thand_f *)0);

	debug ("Got good BOOTP\n");

	if ((s = getenv("autoload")) != NULL) {
		if (*s == 'n') {
			/*
			 * Just use BOOTP to configure system;
			 * Do not use TFTP to load the bootfile.
			 */
			NetState = NETLOOP_SUCCESS;
			return;
#if (CONFIG_COMMANDS & CFG_CMD_NFS) || defined(CONFIG_CMD_NFS)
		} else if (strcmp(s, "NFS") == 0) {
			/*
			 * Use NFS to load the bootfile.
			 */
			NfsStart();
			return;
#endif
		}
	}

	TftpStart();
}
#endif	/* !CFG_CMD_DHCP */

/*
 *	Timeout on BOOTP/DHCP request.
 */
static void
BootpTimeout(void)
{
	if (BootpTry >= TIMEOUT_COUNT) {
		puts ("\nRetry count exceeded; starting again\n");
		NetStartAgain ();
	} else {
		NetSetTimeout (TIMEOUT * CFG_HZ, BootpTimeout);
		BootpRequest ();
	}
}

/*
 *	Initialize BOOTP extension fields in the request.
 */
#if (CONFIG_COMMANDS & CFG_CMD_DHCP) || defined(CONFIG_CMD_DHCP)
static int DhcpExtended (u8 * e, int message_type, IPaddr_t ServerID, IPaddr_t RequestedIP)
{
	u8 *start = e;
	u8 *cnt;

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_VENDOREX)
	u8 *x;
#endif
#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_SEND_HOSTNAME)
	char *hostname;
#endif

	*e++ = 99;		/* RFC1048 Magic Cookie */
	*e++ = 130;
	*e++ = 83;
	*e++ = 99;

	*e++ = 53;		/* DHCP Message Type */
	*e++ = 1;
	*e++ = message_type;

	*e++ = 57;		/* Maximum DHCP Message Size */
	*e++ = 2;
	*e++ = (576 - 312 + OPT_SIZE) >> 8;
	*e++ = (576 - 312 + OPT_SIZE) & 0xff;

	if (ServerID) {
		int tmp = ntohl (ServerID);

		*e++ = 54;	/* ServerID */
		*e++ = 4;
		*e++ = tmp >> 24;
		*e++ = tmp >> 16;
		*e++ = tmp >> 8;
		*e++ = tmp & 0xff;
	}

	if (RequestedIP) {
		int tmp = ntohl (RequestedIP);

		*e++ = 50;	/* Requested IP */
		*e++ = 4;
		*e++ = tmp >> 24;
		*e++ = tmp >> 16;
		*e++ = tmp >> 8;
		*e++ = tmp & 0xff;
	}
#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_SEND_HOSTNAME)
	if ((hostname = getenv ("hostname"))) {
		int hostnamelen = strlen (hostname);

		*e++ = 12;	/* Hostname */
		*e++ = hostnamelen;
		memcpy (e, hostname, hostnamelen);
		e += hostnamelen;
	}
#endif

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_VENDOREX)
	if ((x = dhcp_vendorex_prep (e)))
		return x - start;
#endif

	*e++ = 55;		/* Parameter Request List */
	 cnt = e++;		/* Pointer to count of requested items */
	*cnt = 0;
#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_SUBNETMASK)
	*e++  = 1;		/* Subnet Mask */
	*cnt += 1;
#endif
#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_TIMEOFFSET)
	*e++  = 2;
	*cnt += 1;
#endif
#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_GATEWAY)
	*e++  = 3;		/* Router Option */
	*cnt += 1;
#endif
#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_DNS)
	*e++  = 6;		/* DNS Server(s) */
	*cnt += 1;
#endif
#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_HOSTNAME)
	*e++  = 12;		/* Hostname */
	*cnt += 1;
#endif
#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_BOOTFILESIZE)
	*e++  = 13;		/* Boot File Size */
	*cnt += 1;
#endif
#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_BOOTPATH)
	*e++  = 17;		/* Boot path */
	*cnt += 1;
#endif
#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_NISDOMAIN)
	*e++  = 40;		/* NIS Domain name request */
	*cnt += 1;
#endif
#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_NTPSERVER)
	*e++  = 42;
	*cnt += 1;
#endif
	*e++  = 255;		/* End of the list */

	/* Pad to minimal length */
#ifdef	CONFIG_DHCP_MIN_EXT_LEN
	while ((e - start) <= CONFIG_DHCP_MIN_EXT_LEN)
		*e++ = 0;
#endif

	return e - start;
}

#else	/* CFG_CMD_DHCP */
/*
 *	Warning: no field size check - change CONFIG_BOOTP_MASK at your own risk!
 */
static int BootpExtended (u8 * e)
{
	u8 *start = e;

	*e++ = 99;		/* RFC1048 Magic Cookie */
	*e++ = 130;
	*e++ = 83;
	*e++ = 99;

#if (CONFIG_COMMANDS & CFG_CMD_DHCP) || defined(CONFIG_CMD_DHCP)
	*e++ = 53;		/* DHCP Message Type */
	*e++ = 1;
	*e++ = DHCP_DISCOVER;

	*e++ = 57;		/* Maximum DHCP Message Size */
	*e++ = 2;
	*e++ = (576 - 312 + OPT_SIZE) >> 16;
	*e++ = (576 - 312 + OPT_SIZE) & 0xff;
#endif /* CFG_CMD_DHCP */

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_SUBNETMASK)
	*e++ = 1;		/* Subnet mask request */
	*e++ = 4;
	e   += 4;
#endif

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_GATEWAY)
	*e++ = 3;		/* Default gateway request */
	*e++ = 4;
	e   += 4;
#endif

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_DNS)
	*e++ = 6;		/* Domain Name Server */
	*e++ = 4;
	e   += 4;
#endif

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_HOSTNAME)
	*e++ = 12;		/* Host name request */
	*e++ = 32;
	e   += 32;
#endif

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_BOOTFILESIZE)
	*e++ = 13;		/* Boot file size */
	*e++ = 2;
	e   += 2;
#endif

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_BOOTPATH)
	*e++ = 17;		/* Boot path */
	*e++ = 32;
	e   += 32;
#endif

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_NISDOMAIN)
	*e++ = 40;		/* NIS Domain name request */
	*e++ = 32;
	e   += 32;
#endif

	*e++ = 255;		/* End of the list */

	return e - start;
}
#endif	/* CFG_CMD_DHCP */

void
BootpRequest (void)
{
	volatile uchar *pkt, *iphdr;
	Bootp_t *bp;
	int ext_len, pktlen, iplen;

#if (CONFIG_COMMANDS & CFG_CMD_DHCP) || defined(CONFIG_CMD_DHCP)
	dhcp_state = INIT;
#endif

#ifdef CONFIG_BOOTP_RANDOM_DELAY		/* Random BOOTP delay */
	unsigned char bi_enetaddr[6];
	int   reg;
	char  *e,*s;
	char tmp[64];
	ulong tst1, tst2, sum, m_mask, m_value = 0;

	if (BootpTry ==0) {
		/* get our mac */
		reg = getenv_r ("ethaddr", tmp, sizeof(tmp));
		s = (reg > 0) ? tmp : NULL;

		for (reg=0; reg<6; ++reg) {
			bi_enetaddr[reg] = s ? simple_strtoul(s, &e, 16) : 0;
			if (s) {
				s = (*e) ? e+1 : e;
			}
		}
#ifdef DEBUG
		puts ("BootpRequest => Our Mac: ");
		for (reg=0; reg<6; reg++) {
			printf ("%x%c",
				bi_enetaddr[reg],
				reg==5 ? '\n' : ':');
		}
#endif /* DEBUG */

		/* Mac-Manipulation 2 get seed1 */
		tst1=0;
		tst2=0;
		for (reg=2; reg<6; reg++) {
			tst1 = tst1 << 8;
			tst1 = tst1 | bi_enetaddr[reg];
		}
		for (reg=0; reg<2; reg++) {
			tst2 = tst2 | bi_enetaddr[reg];
			tst2 = tst2 << 8;
		}

		seed1 = tst1^tst2;

		/* Mirror seed1*/
		m_mask=0x1;
		for (reg=1;reg<=32;reg++) {
			m_value |= (m_mask & seed1);
			seed1 = seed1 >> 1;
			m_value = m_value << 1;
		}
		seed1 = m_value;
		seed2 = 0xB78D0945;
	}

	/* Random Number Generator */

	for (reg=0;reg<=0;reg++) {
		sum = seed1 + seed2;
		if (sum < seed1 || sum < seed2)
			sum++;
		seed2 = seed1;
		seed1 = sum;

		if (BootpTry<=2) {	/* Start with max 1024 * 1ms */
			sum = sum >> (22-BootpTry);
		} else {		/*After 3rd BOOTP request max 8192 * 1ms */
			sum = sum >> 19;
		}
	}

	printf ("Random delay: %ld ms...\n", sum);
	for (reg=0; reg <sum; reg++) {
		udelay(1000); /*Wait 1ms*/
	}
#endif	/* CONFIG_BOOTP_RANDOM_DELAY */

	printf("BOOTP broadcast %d\n", ++BootpTry);
	pkt = NetTxPacket;
	memset ((void*)pkt, 0, PKTSIZE);

	pkt += NetSetEther(pkt, NetBcastAddr, PROT_IP);

	/*
	 * Next line results in incorrect packet size being transmitted, resulting
	 * in errors in some DHCP servers, reporting missing bytes.  Size must be
	 * set in packet header after extension length has been determined.
	 * C. Hallinan, DS4.COM, Inc.
	 */
	/* NetSetIP(pkt, 0xFFFFFFFFL, PORT_BOOTPS, PORT_BOOTPC, sizeof (Bootp_t)); */
	iphdr = pkt;	/* We need this later for NetSetIP() */
	pkt += IP_HDR_SIZE;

	bp = (Bootp_t *)pkt;
	bp->bp_op = OP_BOOTREQUEST;
	bp->bp_htype = HWT_ETHER;
	bp->bp_hlen = HWL_ETHER;
	bp->bp_hops = 0;
	bp->bp_secs = htons(get_timer(0) / CFG_HZ);
	NetWriteIP(&bp->bp_ciaddr, 0);
	NetWriteIP(&bp->bp_yiaddr, 0);
	NetWriteIP(&bp->bp_siaddr, 0);
	NetWriteIP(&bp->bp_giaddr, 0);
	memcpy (bp->bp_chaddr, NetOurEther, 6);
	copy_filename (bp->bp_file, BootFile, sizeof(bp->bp_file));

	/* Request additional information from the BOOTP/DHCP server */
#if (CONFIG_COMMANDS & CFG_CMD_DHCP) || defined(CONFIG_CMD_DHCP)
	ext_len = DhcpExtended((u8 *)bp->bp_vend, DHCP_DISCOVER, 0, 0);
#else
	ext_len = BootpExtended((u8 *)bp->bp_vend);
#endif	/* CFG_CMD_DHCP */

	/*
	 *	Bootp ID is the lower 4 bytes of our ethernet address
	 *	plus the current time in HZ.
	 */
	BootpID = ((ulong)NetOurEther[2] << 24)
		| ((ulong)NetOurEther[3] << 16)
		| ((ulong)NetOurEther[4] << 8)
		| (ulong)NetOurEther[5];
	BootpID += get_timer(0);
	BootpID	 = htonl(BootpID);
	NetCopyLong(&bp->bp_id, &BootpID);

	/*
	 * Calculate proper packet lengths taking into account the
	 * variable size of the options field
	 */
	pktlen = BOOTP_SIZE - sizeof(bp->bp_vend) + ext_len;
	iplen = BOOTP_HDR_SIZE - sizeof(bp->bp_vend) + ext_len;
	NetSetIP(iphdr, 0xFFFFFFFFL, PORT_BOOTPS, PORT_BOOTPC, iplen);
	NetSetTimeout(SELECT_TIMEOUT * CFG_HZ, BootpTimeout);

#if (CONFIG_COMMANDS & CFG_CMD_DHCP) || defined(CONFIG_CMD_DHCP)
	dhcp_state = SELECTING;
	NetSetHandler(DhcpHandler);
#else
	NetSetHandler(BootpHandler);
#endif	/* CFG_CMD_DHCP */
	NetSendPacket(NetTxPacket, pktlen);
}

#if (CONFIG_COMMANDS & CFG_CMD_DHCP) || defined(CONFIG_CMD_DHCP)
static void DhcpOptionsProcess (uchar * popt, Bootp_t *bp)
{
	uchar *end = popt + BOOTP_HDR_SIZE;
	int oplen, size;

	while (popt < end && *popt != 0xff) {
		oplen = *(popt + 1);
		switch (*popt) {
		case 1:
			NetCopyIP (&NetOurSubnetMask, (popt + 2));
			break;
#if ((CONFIG_COMMANDS & CFG_CMD_SNTP) || defined(CONFIG_CMD_SNTP)) && (CONFIG_BOOTP_MASK & CONFIG_BOOTP_TIMEOFFSET)
		case 2:		/* Time offset	*/
			NetCopyLong (&NetTimeOffset, (ulong *) (popt + 2));
			NetTimeOffset = ntohl (NetTimeOffset);
			break;
#endif
		case 3:
			NetCopyIP (&NetOurGatewayIP, (popt + 2));
			break;
		case 6:
			NetCopyIP (&NetOurDNSIP, (popt + 2));
#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_DNS2)
			if (*(popt + 1) > 4) {
				NetCopyIP (&NetOurDNS2IP, (popt + 2 + 4));
			}
#endif
			break;
		case 12:
			size = truncate_sz ("Host Name", sizeof (NetOurHostName), oplen);
			memcpy (&NetOurHostName, popt + 2, size);
			NetOurHostName[size] = 0;
			break;
		case 15:	/* Ignore Domain Name Option */
			break;
		case 17:
			size = truncate_sz ("Root Path", sizeof (NetOurRootPath), oplen);
			memcpy (&NetOurRootPath, popt + 2, size);
			NetOurRootPath[size] = 0;
			break;
#if ((CONFIG_COMMANDS & CFG_CMD_SNTP) || defined(CONFIG_CMD_SNTP)) && (CONFIG_BOOTP_MASK & CONFIG_BOOTP_NTPSERVER)
		case 42:	/* NTP server IP */
			NetCopyIP (&NetNtpServerIP, (popt + 2));
			break;
#endif
		case 51:
			NetCopyLong (&dhcp_leasetime, (ulong *) (popt + 2));
			break;
		case 53:	/* Ignore Message Type Option */
			break;
		case 54:
			NetCopyIP (&NetDHCPServerIP, (popt + 2));
			break;
		case 58:	/* Ignore Renewal Time Option */
			break;
		case 59:	/* Ignore Rebinding Time Option */
			break;
		case 66:	/* Ignore TFTP server name */
			break;
		case 67:	/* vendor opt bootfile */
			/*
			 * I can't use dhcp_vendorex_proc here because I need
			 * to write into the bootp packet - even then I had to
			 * pass the bootp packet pointer into here as the
			 * second arg
			 */
			size = truncate_sz ("Opt Boot File",
					    sizeof(bp->bp_file),
					    oplen);
			if (bp->bp_file[0] == '\0' && size > 0) {
				/*
				 * only use vendor boot file if we didn't
				 * receive a boot file in the main non-vendor
				 * part of the packet - god only knows why
				 * some vendors chose not to use this perfectly
				 * good spot to store the boot file (join on
				 * Tru64 Unix) it seems mind bogglingly crazy
				 * to me
				 */
				printf("*** WARNING: using vendor "
					"optional boot file\n");
				memcpy(bp->bp_file, popt + 2, size);
				bp->bp_file[size] = '\0';
			}
			break;
		default:
#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_VENDOREX)
			if (dhcp_vendorex_proc (popt))
				break;
#endif
			printf ("*** Unhandled DHCP Option in OFFER/ACK: %d\n", *popt);
			break;
		}
		popt += oplen + 2;	/* Process next option */
	}
}

static int DhcpMessageType(unsigned char *popt)
{
	if (NetReadLong((ulong*)popt) != htonl(BOOTP_VENDOR_MAGIC))
		return -1;

	popt += 4;
	while ( *popt != 0xff ) {
		if ( *popt == 53 )	/* DHCP Message Type */
			return *(popt + 2);
		popt += *(popt + 1) + 2;	/* Scan through all options */
	}
	return -1;
}

static void DhcpSendRequestPkt(Bootp_t *bp_offer)
{
	volatile uchar *pkt, *iphdr;
	Bootp_t *bp;
	int pktlen, iplen, extlen;
	IPaddr_t OfferedIP;

	debug ("DhcpSendRequestPkt: Sending DHCPREQUEST\n");
	pkt = NetTxPacket;
	memset ((void*)pkt, 0, PKTSIZE);

	pkt += NetSetEther(pkt, NetBcastAddr, PROT_IP);

	iphdr = pkt;		/* We'll need this later to set proper pkt size */
	pkt += IP_HDR_SIZE;

	bp = (Bootp_t *)pkt;
	bp->bp_op = OP_BOOTREQUEST;
	bp->bp_htype = HWT_ETHER;
	bp->bp_hlen = HWL_ETHER;
	bp->bp_hops = 0;
	bp->bp_secs = htons(get_timer(0) / CFG_HZ);
	NetCopyIP(&bp->bp_ciaddr, &bp_offer->bp_ciaddr); /* both in network byte order */
	NetCopyIP(&bp->bp_yiaddr, &bp_offer->bp_yiaddr);
	NetCopyIP(&bp->bp_siaddr, &bp_offer->bp_siaddr);
	/*
	 * RFC3046 requires Relay Agents to discard packets with
	 * nonzero and offered giaddr
	 */
	NetWriteIP(&bp->bp_giaddr, 0);

	memcpy (bp->bp_chaddr, NetOurEther, 6);

	/*
	 * ID is the id of the OFFER packet
	 */

	NetCopyLong(&bp->bp_id, &bp_offer->bp_id);

	/*
	 * Copy options from OFFER packet if present
	 */
	NetCopyIP(&OfferedIP, &bp->bp_yiaddr);
	extlen = DhcpExtended((u8 *)bp->bp_vend, DHCP_REQUEST, NetDHCPServerIP, OfferedIP);

	pktlen = BOOTP_SIZE - sizeof(bp->bp_vend) + extlen;
	iplen = BOOTP_HDR_SIZE - sizeof(bp->bp_vend) + extlen;
	NetSetIP(iphdr, 0xFFFFFFFFL, PORT_BOOTPS, PORT_BOOTPC, iplen);

	debug ("Transmitting DHCPREQUEST packet: len = %d\n", pktlen);
	NetSendPacket(NetTxPacket, pktlen);
}

/*
 *	Handle DHCP received packets.
 */
static void
DhcpHandler(uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	Bootp_t *bp = (Bootp_t *)pkt;

	debug ("DHCPHandler: got packet: (src=%d, dst=%d, len=%d) state: %d\n",
		src, dest, len, dhcp_state);

	if (BootpCheckPkt(pkt, dest, src, len)) /* Filter out pkts we don't want */
		return;

	debug ("DHCPHandler: got DHCP packet: (src=%d, dst=%d, len=%d) state: %d\n",
		src, dest, len, dhcp_state);

	switch (dhcp_state) {
	case SELECTING:
		/*
		 * Wait an appropriate time for any potential DHCPOFFER packets
		 * to arrive.  Then select one, and generate DHCPREQUEST response.
		 * If filename is in format we recognize, assume it is a valid
		 * OFFER from a server we want.
		 */
		debug ("DHCP: state=SELECTING bp_file: \"%s\"\n", bp->bp_file);
#ifdef CFG_BOOTFILE_PREFIX
		if (strncmp(bp->bp_file,
			    CFG_BOOTFILE_PREFIX,
			    strlen(CFG_BOOTFILE_PREFIX)) == 0 ) {
#endif	/* CFG_BOOTFILE_PREFIX */

			debug ("TRANSITIONING TO REQUESTING STATE\n");
			dhcp_state = REQUESTING;

			if (NetReadLong((ulong*)&bp->bp_vend[0]) == htonl(BOOTP_VENDOR_MAGIC))
				DhcpOptionsProcess((u8 *)&bp->bp_vend[4], bp);

			BootpCopyNetParams(bp); /* Store net params from reply */

			NetSetTimeout(TIMEOUT * CFG_HZ, BootpTimeout);
			DhcpSendRequestPkt(bp);
#ifdef CFG_BOOTFILE_PREFIX
		}
#endif	/* CFG_BOOTFILE_PREFIX */

		return;
		break;
	case REQUESTING:
		debug ("DHCP State: REQUESTING\n");

		if ( DhcpMessageType((u8 *)bp->bp_vend) == DHCP_ACK ) {
			char *s;

			if (NetReadLong((ulong*)&bp->bp_vend[0]) == htonl(BOOTP_VENDOR_MAGIC))
				DhcpOptionsProcess((u8 *)&bp->bp_vend[4], bp);
			BootpCopyNetParams(bp); /* Store net params from reply */
			dhcp_state = BOUND;
			puts ("DHCP client bound to address ");
			print_IPaddr(NetOurIP);
			putc ('\n');

			/* Obey the 'autoload' setting */
			if ((s = getenv("autoload")) != NULL) {
				if (*s == 'n') {
					/*
					 * Just use BOOTP to configure system;
					 * Do not use TFTP to load the bootfile.
					 */
					NetState = NETLOOP_SUCCESS;
					return;
#if (CONFIG_COMMANDS & CFG_CMD_NFS) || defined(CONFIG_CMD_NFS)
				} else if (strcmp(s, "NFS") == 0) {
					/*
					 * Use NFS to load the bootfile.
					 */
					NfsStart();
					return;
#endif
				}
			}
			TftpStart();
			return;
		}
		break;
	default:
		puts ("DHCP: INVALID STATE\n");
		break;
	}

}

void DhcpRequest(void)
{
	BootpRequest();
}
#endif	/* CFG_CMD_DHCP */

#endif /* CFG_CMD_NET */
