/*
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000, 2001 DENX Software Engineering, Wolfgang Denk, wd@denx.de
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include "tftp.h"
#include "bootp.h"

#undef	ET_DEBUG

#if (CONFIG_COMMANDS & CFG_CMD_NET)

#define WELL_KNOWN_PORT	69		/* Well known TFTP port #		*/
#define TIMEOUT		2		/* Seconds to timeout for a lost pkt	*/
#ifndef	CONFIG_NET_RETRY_COUNT
# define TIMEOUT_COUNT	10		/* # of timeouts before giving up  */
#else
# define TIMEOUT_COUNT  (CONFIG_NET_RETRY_COUNT * 2)
#endif
					/* (for checking the image size)	*/
#define HASHES_PER_LINE	65		/* Number of "loading" hashes per line	*/

/*
 *	TFTP operations.
 */
#define TFTP_RRQ	1
#define TFTP_WRQ	2
#define TFTP_DATA	3
#define TFTP_ACK	4
#define TFTP_ERROR	5


static int	TftpServerPort;		/* The UDP port at their end		*/
static int	TftpOurPort;		/* The UDP port at our end		*/
static int	TftpTimeoutCount;
static unsigned	TftpBlock;
static unsigned	TftpLastBlock;
static int	TftpState;
#define STATE_RRQ	1
#define STATE_DATA	2
#define STATE_TOO_LARGE	3
#define STATE_BAD_MAGIC	4

#define DEFAULT_NAME_LEN	(8 + 4 + 1)
static char default_filename[DEFAULT_NAME_LEN];
static char *tftp_filename;

#ifdef CFG_DIRECT_FLASH_TFTP
extern flash_info_t flash_info[CFG_MAX_FLASH_BANKS];
#endif

static __inline__ void
store_block (unsigned block, uchar * src, unsigned len)
{
	ulong offset = block * 512, newsize = offset + len;
#ifdef CFG_DIRECT_FLASH_TFTP
	int i, rc = 0;

	for (i=0; i<CFG_MAX_FLASH_BANKS; i++) {
		/* start address in flash? */
		if (load_addr + offset >= flash_info[i].start[0]) {
			rc = 1;
			break;
		}
	}

	if (rc) { /* Flash is destination for this packet */
		rc = flash_write ((uchar *)src, (ulong)(load_addr+offset), len);
		if (rc) {
			flash_perror (rc);
			NetState = NETLOOP_FAIL;
			return;
		}
	}
	else
#endif /* CFG_DIRECT_FLASH_TFTP */
	{
		(void)memcpy((void *)(load_addr + offset), src, len);
	}

	if (NetBootFileXferSize < newsize)
		NetBootFileXferSize = newsize;
}

static void TftpSend (void);
static void TftpTimeout (void);

/**********************************************************************/

static void
TftpSend (void)
{
	volatile uchar *	pkt;
	volatile uchar *	xp;
	int			len = 0;

	/*
	 *	We will always be sending some sort of packet, so
	 *	cobble together the packet headers now.
	 */
	pkt = NetTxPacket + ETHER_HDR_SIZE + IP_HDR_SIZE;

	switch (TftpState) {

	case STATE_RRQ:
		xp = pkt;
		*((ushort *)pkt)++ = htons(TFTP_RRQ);
		strcpy ((char *)pkt, tftp_filename);
		pkt += strlen(tftp_filename) + 1;
		strcpy ((char *)pkt, "octet");
		pkt += 5 /*strlen("octet")*/ + 1;
		len = pkt - xp;
		break;

	case STATE_DATA:
		xp = pkt;
		*((ushort *)pkt)++ = htons(TFTP_ACK);
		*((ushort *)pkt)++ = htons(TftpBlock);
		len = pkt - xp;
		break;

	case STATE_TOO_LARGE:
		xp = pkt;
		*((ushort *)pkt)++ = htons(TFTP_ERROR);
		*((ushort *)pkt)++ = htons(3);
		strcpy ((char *)pkt, "File too large");
		pkt += 14 /*strlen("File too large")*/ + 1;
		len = pkt - xp;
		break;

	case STATE_BAD_MAGIC:
		xp = pkt;
		*((ushort *)pkt)++ = htons(TFTP_ERROR);
		*((ushort *)pkt)++ = htons(2);
		strcpy ((char *)pkt, "File has bad magic");
		pkt += 18 /*strlen("File has bad magic")*/ + 1;
		len = pkt - xp;
		break;
	}

	NetSetEther (NetTxPacket, NetServerEther, PROT_IP);
	NetSetIP (NetTxPacket + ETHER_HDR_SIZE, NetServerIP,
					TftpServerPort, TftpOurPort, len);
	NetSendPacket (NetTxPacket, ETHER_HDR_SIZE + IP_HDR_SIZE + len);
}


static void
TftpHandler (uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	ushort proto;

	if (dest != TftpOurPort) {
		return;
	}
	if (TftpState != STATE_RRQ && src != TftpServerPort) {
		return;
	}

	if (len < 2) {
		return;
	}
	len -= 2;
	/* warning: don't use increment (++) in ntohs() macros!! */
	proto = *((ushort *)pkt)++;
	switch (ntohs(proto)) {

	case TFTP_RRQ:
	case TFTP_WRQ:
	case TFTP_ACK:
		break;
	default:
		break;

	case TFTP_DATA:
		if (len < 2)
			return;
		len -= 2;
		TftpBlock = ntohs(*(ushort *)pkt);
		if (((TftpBlock - 1) % 10) == 0) {
			putc ('#');
		} else if ((TftpBlock % (10 * HASHES_PER_LINE)) == 0) {
			puts ("\n\t ");
		}

		if (TftpState == STATE_RRQ) {
			TftpState = STATE_DATA;
			TftpServerPort = src;
			TftpLastBlock = 0;

			if (TftpBlock != 1) {	/* Assertion */
				printf ("\nTFTP error: "
					"First block is not block 1 (%d)\n"
					"Starting again\n\n",
					TftpBlock);
				NetStartAgain ();
				break;
			}
		}

		if (TftpBlock == TftpLastBlock) {
			/*
			 *	Same block again; ignore it.
			 */
			break;
		}

		TftpLastBlock = TftpBlock;
		NetSetTimeout (TIMEOUT * CFG_HZ, TftpTimeout);

		store_block (TftpBlock - 1, pkt + 2, len);

		/*
		 *	Acknoledge the block just received, which will prompt
		 *	the server for the next one.
		 */
		TftpSend ();

		if (len < 512) {
			/*
			 *	We received the whole thing.  Try to
			 *	run it.
			 */
			puts ("\ndone\n");
			NetState = NETLOOP_SUCCESS;
		}
		break;

	case TFTP_ERROR:
		printf ("\nTFTP error: '%s' (%d)\n",
					pkt + 2, ntohs(*(ushort *)pkt));
		puts ("Starting again\n\n");
		NetStartAgain ();
		break;
	}
}


static void
TftpTimeout (void)
{
	if (++TftpTimeoutCount >= TIMEOUT_COUNT) {
		puts ("\nRetry count exceeded; starting again\n");
		NetStartAgain ();
	} else {
		puts ("T ");
		NetSetTimeout (TIMEOUT * CFG_HZ, TftpTimeout);
		TftpSend ();
	}
}


void
TftpStart (void)
{
#ifdef ET_DEBUG
	printf ("\nServer ethernet address %02x:%02x:%02x:%02x:%02x:%02x\n",
		NetServerEther[0],
		NetServerEther[1],
		NetServerEther[2],
		NetServerEther[3],
		NetServerEther[4],
		NetServerEther[5]
	);
#endif /* DEBUG */

	if (BootFile[0] == '\0') {
		IPaddr_t OurIP = ntohl(NetOurIP);

		sprintf(default_filename, "%02lX%02lX%02lX%02lX.img",
			OurIP & 0xFF,
			(OurIP >>  8) & 0xFF,
			(OurIP >> 16) & 0xFF,
			(OurIP >> 24) & 0xFF	);
		tftp_filename = default_filename;

		printf ("*** Warning: no boot file name; using '%s'\n",
			tftp_filename);
	} else {
		tftp_filename = BootFile;
	}

	puts ("TFTP from server ");	print_IPaddr (NetServerIP);
	puts ("; our IP address is ");	print_IPaddr (NetOurIP);

	/* Check if we need to send across this subnet */
	if (NetOurGatewayIP && NetOurSubnetMask) {
	    IPaddr_t OurNet 	= NetOurIP    & NetOurSubnetMask;
	    IPaddr_t ServerNet 	= NetServerIP & NetOurSubnetMask;

	    if (OurNet != ServerNet) {
		puts ("; sending through gateway ");
		print_IPaddr (NetOurGatewayIP) ;
	    }
	}
	putc ('\n');

	printf ("Filename '%s'.", tftp_filename);

	if (NetBootFileSize) {
		printf (" Size is 0x%x Bytes = ", NetBootFileSize<<9);
		print_size (NetBootFileSize<<9, "");
	}

	putc ('\n');

	printf ("Load address: 0x%lx\n", load_addr);

	puts ("Loading: *\b");

	NetSetTimeout (TIMEOUT * CFG_HZ, TftpTimeout);
	NetSetHandler (TftpHandler);

	TftpServerPort = WELL_KNOWN_PORT;
	TftpTimeoutCount = 0;
	TftpState = STATE_RRQ;
	TftpOurPort = 1024 + (get_timer(0) % 3072);

	TftpSend ();
}

#endif /* CFG_CMD_NET */
