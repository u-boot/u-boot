/*
 * Copyright 1994, 1995, 2000 Neil Russell.
 * (See License)
 * Copyright 2000, 2001 DENX Software Engineering, Wolfgang Denk, wd@denx.de
 * Copyright 2011 Comelit Group SpA,
 *                Luca Ceresoli <luca.ceresoli@comelit.it>
 */
#include <common.h>
#include <command.h>
#include <display_options.h>
#include <efi_loader.h>
#include <env.h>
#include <image.h>
#include <lmb.h>
#include <log.h>
#include <mapmem.h>
#include <net.h>
#include <net6.h>
#include <asm/global_data.h>
#include <net/tftp.h>
#include "bootp.h"

DECLARE_GLOBAL_DATA_PTR;

/* Well known TFTP port # */
#define WELL_KNOWN_PORT	69
/* Millisecs to timeout for lost pkt */
#define TIMEOUT		5000UL
/* Number of "loading" hashes per line (for checking the image size) */
#define HASHES_PER_LINE	65

/*
 *	TFTP operations.
 */
#define TFTP_RRQ	1
#define TFTP_WRQ	2
#define TFTP_DATA	3
#define TFTP_ACK	4
#define TFTP_ERROR	5
#define TFTP_OACK	6

static ulong timeout_ms = TIMEOUT;
static int timeout_count_max = (CONFIG_NET_RETRY_COUNT * 2);
static ulong time_start;   /* Record time we started tftp */
static struct in6_addr tftp_remote_ip6;

/*
 * These globals govern the timeout behavior when attempting a connection to a
 * TFTP server. tftp_timeout_ms specifies the number of milliseconds to
 * wait for the server to respond to initial connection. Second global,
 * tftp_timeout_count_max, gives the number of such connection retries.
 * tftp_timeout_count_max must be non-negative and tftp_timeout_ms must be
 * positive. The globals are meant to be set (and restored) by code needing
 * non-standard timeout behavior when initiating a TFTP transfer.
 */
ulong tftp_timeout_ms = TIMEOUT;
int tftp_timeout_count_max = (CONFIG_NET_RETRY_COUNT * 2);

enum {
	TFTP_ERR_UNDEFINED           = 0,
	TFTP_ERR_FILE_NOT_FOUND      = 1,
	TFTP_ERR_ACCESS_DENIED       = 2,
	TFTP_ERR_DISK_FULL           = 3,
	TFTP_ERR_UNEXPECTED_OPCODE   = 4,
	TFTP_ERR_UNKNOWN_TRANSFER_ID  = 5,
	TFTP_ERR_FILE_ALREADY_EXISTS = 6,
	TFTP_ERR_OPTION_NEGOTIATION = 8,
};

static struct in_addr tftp_remote_ip;
/* The UDP port at their end */
static int	tftp_remote_port;
/* The UDP port at our end */
static int	tftp_our_port;
static int	timeout_count;
/* packet sequence number */
static ulong	tftp_cur_block;
/* last packet sequence number received */
static ulong	tftp_prev_block;
/* count of sequence number wraparounds */
static ulong	tftp_block_wrap;
/* memory offset due to wrapping */
static ulong	tftp_block_wrap_offset;
static int	tftp_state;
static ulong	tftp_load_addr;
#ifdef CONFIG_LMB
static ulong	tftp_load_size;
#endif
#ifdef CONFIG_TFTP_TSIZE
/* The file size reported by the server */
static int	tftp_tsize;
/* The number of hashes we printed */
static short	tftp_tsize_num_hash;
#endif
/* The window size negotiated */
static ushort	tftp_windowsize;
/* Next block to send ack to */
static ushort	tftp_next_ack;
/* Last nack block we send */
static ushort	tftp_last_nack;
#ifdef CONFIG_CMD_TFTPPUT
/* 1 if writing, else 0 */
static int	tftp_put_active;
/* 1 if we have sent the last block */
static int	tftp_put_final_block_sent;
#else
#define tftp_put_active	0
#endif

#define STATE_SEND_RRQ	1
#define STATE_DATA	2
#define STATE_TOO_LARGE	3
#define STATE_BAD_MAGIC	4
#define STATE_OACK	5
#define STATE_RECV_WRQ	6
#define STATE_SEND_WRQ	7
#define STATE_INVALID_OPTION	8

/* default TFTP block size */
#define TFTP_BLOCK_SIZE		512
#define TFTP_MTU_BLOCKSIZE6 (CONFIG_TFTP_BLOCKSIZE - 20)
/* sequence number is 16 bit */
#define TFTP_SEQUENCE_SIZE	((ulong)(1<<16))

#define DEFAULT_NAME_LEN	(8 + 4 + 1)
static char default_filename[DEFAULT_NAME_LEN];

#ifndef CONFIG_TFTP_FILE_NAME_MAX_LEN
#define MAX_LEN 128
#else
#define MAX_LEN CONFIG_TFTP_FILE_NAME_MAX_LEN
#endif

static char tftp_filename[MAX_LEN];

/* 512 is poor choice for ethernet, MTU is typically 1500.
 * Minus eth.hdrs thats 1468.  Can get 2x better throughput with
 * almost-MTU block sizes.  At least try... fall back to 512 if need be.
 * (but those using CONFIG_IP_DEFRAG may want to set a larger block in cfg file)
 */

/* When windowsize is defined to 1,
 * tftp behaves the same way as it was
 * never declared
 */
#ifdef CONFIG_TFTP_WINDOWSIZE
#define TFTP_WINDOWSIZE CONFIG_TFTP_WINDOWSIZE
#else
#define TFTP_WINDOWSIZE 1
#endif

static unsigned short tftp_block_size = TFTP_BLOCK_SIZE;
static unsigned short tftp_block_size_option = CONFIG_TFTP_BLOCKSIZE;
static unsigned short tftp_window_size_option = TFTP_WINDOWSIZE;

static inline int store_block(int block, uchar *src, unsigned int len)
{
	ulong offset = block * tftp_block_size + tftp_block_wrap_offset -
			tftp_block_size;
	ulong newsize = offset + len;
	ulong store_addr = tftp_load_addr + offset;
	void *ptr;

#ifdef CONFIG_LMB
	ulong end_addr = tftp_load_addr + tftp_load_size;

	if (!end_addr)
		end_addr = ULONG_MAX;

	if (store_addr < tftp_load_addr ||
	    store_addr + len > end_addr) {
		puts("\nTFTP error: ");
		puts("trying to overwrite reserved memory...\n");
		return -1;
	}
#endif
	ptr = map_sysmem(store_addr, len);
	memcpy(ptr, src, len);
	unmap_sysmem(ptr);

	if (net_boot_file_size < newsize)
		net_boot_file_size = newsize;

	return 0;
}

/* Clear our state ready for a new transfer */
static void new_transfer(void)
{
	tftp_prev_block = 0;
	tftp_block_wrap = 0;
	tftp_block_wrap_offset = 0;
#ifdef CONFIG_CMD_TFTPPUT
	tftp_put_final_block_sent = 0;
#endif
}

#ifdef CONFIG_CMD_TFTPPUT
/**
 * Load the next block from memory to be sent over tftp.
 *
 * @param block	Block number to send
 * @param dst	Destination buffer for data
 * @param len	Number of bytes in block (this one and every other)
 * Return: number of bytes loaded
 */
static int load_block(unsigned block, uchar *dst, unsigned len)
{
	/* We may want to get the final block from the previous set */
	ulong offset = block * tftp_block_size + tftp_block_wrap_offset -
		       tftp_block_size;
	ulong tosend = len;

	tosend = min(net_boot_file_size - offset, tosend);
	(void)memcpy(dst, (void *)(image_save_addr + offset), tosend);
	debug("%s: block=%u, offset=%lu, len=%u, tosend=%lu\n", __func__,
	      block, offset, len, tosend);
	return tosend;
}
#endif

static void tftp_send(void);
static void tftp_timeout_handler(void);

/**********************************************************************/

static void show_block_marker(void)
{
	ulong pos;

#ifdef CONFIG_TFTP_TSIZE
	if (tftp_tsize) {
		pos = tftp_cur_block * tftp_block_size +
			tftp_block_wrap_offset;
		if (pos > tftp_tsize)
			pos = tftp_tsize;

		while (tftp_tsize_num_hash < pos * 50 / tftp_tsize) {
			putc('#');
			tftp_tsize_num_hash++;
		}
	} else
#endif
	{
		pos = (tftp_cur_block - 1) +
			(tftp_block_wrap * TFTP_SEQUENCE_SIZE);
		if ((pos % 10) == 0)
			putc('#');
		else if (((pos + 1) % (10 * HASHES_PER_LINE)) == 0)
			puts("\n\t ");
	}
}

/**
 * restart the current transfer due to an error
 *
 * @param msg	Message to print for user
 */
static void restart(const char *msg)
{
	printf("\n%s; starting again\n", msg);
	net_start_again();
}

/*
 * Check if the block number has wrapped, and update progress
 *
 * TODO: The egregious use of global variables in this file should be tidied.
 */
static void update_block_number(void)
{
	/*
	 * RFC1350 specifies that the first data packet will
	 * have sequence number 1. If we receive a sequence
	 * number of 0 this means that there was a wrap
	 * around of the (16 bit) counter.
	 */
	if (tftp_cur_block == 0 && tftp_prev_block != 0) {
		tftp_block_wrap++;
		tftp_block_wrap_offset += tftp_block_size * TFTP_SEQUENCE_SIZE;
		timeout_count = 0; /* we've done well, reset the timeout */
	}
	show_block_marker();
}

/* The TFTP get or put is complete */
static void tftp_complete(void)
{
#ifdef CONFIG_TFTP_TSIZE
	/* Print hash marks for the last packet received */
	while (tftp_tsize && tftp_tsize_num_hash < 49) {
		putc('#');
		tftp_tsize_num_hash++;
	}
	puts("  ");
	print_size(tftp_tsize, "");
#endif
	time_start = get_timer(time_start);
	if (time_start > 0) {
		puts("\n\t ");	/* Line up with "Loading: " */
		print_size(net_boot_file_size /
			time_start * 1000, "/s");
	}
	puts("\ndone\n");
	if (IS_ENABLED(CONFIG_CMD_BOOTEFI)) {
		if (!tftp_put_active)
			efi_set_bootdev("Net", "", tftp_filename,
					map_sysmem(tftp_load_addr, 0),
					net_boot_file_size);
	}
	net_set_state(NETLOOP_SUCCESS);
}

static void tftp_send(void)
{
	uchar *pkt;
	uchar *xp;
	int len = 0;
	ushort *s;
	bool err_pkt = false;

	/*
	 *	We will always be sending some sort of packet, so
	 *	cobble together the packet headers now.
	 */
	if (IS_ENABLED(CONFIG_IPV6) && use_ip6)
		pkt = net_tx_packet + net_eth_hdr_size() +
		      IP6_HDR_SIZE + UDP_HDR_SIZE;
	else
		pkt = net_tx_packet + net_eth_hdr_size() + IP_UDP_HDR_SIZE;

	switch (tftp_state) {
	case STATE_SEND_RRQ:
	case STATE_SEND_WRQ:
		xp = pkt;
		s = (ushort *)pkt;
#ifdef CONFIG_CMD_TFTPPUT
		*s++ = htons(tftp_state == STATE_SEND_RRQ ? TFTP_RRQ :
			TFTP_WRQ);
#else
		*s++ = htons(TFTP_RRQ);
#endif
		pkt = (uchar *)s;
		strcpy((char *)pkt, tftp_filename);
		pkt += strlen(tftp_filename) + 1;
		strcpy((char *)pkt, "octet");
		pkt += 5 /*strlen("octet")*/ + 1;
		strcpy((char *)pkt, "timeout");
		pkt += 7 /*strlen("timeout")*/ + 1;
		sprintf((char *)pkt, "%lu", timeout_ms / 1000);
		debug("send option \"timeout %s\"\n", (char *)pkt);
		pkt += strlen((char *)pkt) + 1;
#ifdef CONFIG_TFTP_TSIZE
		pkt += sprintf((char *)pkt, "tsize%c%u%c",
				0, net_boot_file_size, 0);
#endif
		/* try for more effic. blk size */
		pkt += sprintf((char *)pkt, "blksize%c%d%c",
				0, tftp_block_size_option, 0);

		/* try for more effic. window size.
		 * Implemented only for tftp get.
		 * Don't bother sending if it's 1
		 */
		if (tftp_state == STATE_SEND_RRQ && tftp_window_size_option > 1)
			pkt += sprintf((char *)pkt, "windowsize%c%d%c",
					0, tftp_window_size_option, 0);
		len = pkt - xp;
		break;

	case STATE_OACK:

	case STATE_RECV_WRQ:
	case STATE_DATA:
		xp = pkt;
		s = (ushort *)pkt;
		s[0] = htons(TFTP_ACK);
		s[1] = htons(tftp_cur_block);
		pkt = (uchar *)(s + 2);
#ifdef CONFIG_CMD_TFTPPUT
		if (tftp_put_active) {
			int toload = tftp_block_size;
			int loaded = load_block(tftp_cur_block, pkt, toload);

			s[0] = htons(TFTP_DATA);
			pkt += loaded;
			tftp_put_final_block_sent = (loaded < toload);
		}
#endif
		len = pkt - xp;
		break;

	case STATE_TOO_LARGE:
		xp = pkt;
		s = (ushort *)pkt;
		*s++ = htons(TFTP_ERROR);
			*s++ = htons(3);

		pkt = (uchar *)s;
		strcpy((char *)pkt, "File too large");
		pkt += 14 /*strlen("File too large")*/ + 1;
		len = pkt - xp;
		err_pkt = true;
		break;

	case STATE_BAD_MAGIC:
		xp = pkt;
		s = (ushort *)pkt;
		*s++ = htons(TFTP_ERROR);
		*s++ = htons(2);
		pkt = (uchar *)s;
		strcpy((char *)pkt, "File has bad magic");
		pkt += 18 /*strlen("File has bad magic")*/ + 1;
		len = pkt - xp;
		err_pkt = true;
		break;

	case STATE_INVALID_OPTION:
		xp = pkt;
		s = (ushort *)pkt;
		*s++ = htons(TFTP_ERROR);
		*s++ = htons(TFTP_ERR_OPTION_NEGOTIATION);
		pkt = (uchar *)s;
		strcpy((char *)pkt, "Option Negotiation Failed");
		/* strlen("Option Negotiation Failed") + NULL*/
		pkt += 25 + 1;
		len = pkt - xp;
		err_pkt = true;
		break;
	}

	if (IS_ENABLED(CONFIG_IPV6) && use_ip6)
		net_send_udp_packet6(net_server_ethaddr,
				     &tftp_remote_ip6,
				     tftp_remote_port,
				     tftp_our_port, len);
	else
		net_send_udp_packet(net_server_ethaddr, tftp_remote_ip,
				    tftp_remote_port, tftp_our_port, len);

	if (err_pkt)
		net_set_state(NETLOOP_FAIL);
}

#ifdef CONFIG_CMD_TFTPPUT
static void icmp_handler(unsigned type, unsigned code, unsigned dest,
			 struct in_addr sip, unsigned src, uchar *pkt,
			 unsigned len)
{
	if (type == ICMP_NOT_REACH && code == ICMP_NOT_REACH_PORT) {
		/* Oh dear the other end has gone away */
		restart("TFTP server died");
	}
}
#endif

static void tftp_handler(uchar *pkt, unsigned dest, struct in_addr sip,
			 unsigned src, unsigned len)
{
	__be16 proto;
	__be16 *s;
	int i;
	u16 timeout_val_rcvd;

	if (dest != tftp_our_port) {
			return;
	}
	if (tftp_state != STATE_SEND_RRQ && src != tftp_remote_port &&
	    tftp_state != STATE_RECV_WRQ && tftp_state != STATE_SEND_WRQ)
		return;

	if (len < 2)
		return;
	len -= 2;
	/* warning: don't use increment (++) in ntohs() macros!! */
	s = (__be16 *)pkt;
	proto = *s++;
	pkt = (uchar *)s;
	switch (ntohs(proto)) {
	case TFTP_RRQ:
		break;

	case TFTP_ACK:
#ifdef CONFIG_CMD_TFTPPUT
		if (tftp_put_active) {
			if (tftp_put_final_block_sent) {
				tftp_complete();
			} else {
				/*
				 * Move to the next block. We want our block
				 * count to wrap just like the other end!
				 */
				int block = ntohs(*s);
				int ack_ok = (tftp_cur_block == block);

				tftp_prev_block = tftp_cur_block;
				tftp_cur_block = (unsigned short)(block + 1);
				update_block_number();
				if (ack_ok)
					tftp_send(); /* Send next data block */
			}
		}
#endif
		break;

	default:
		break;

#ifdef CONFIG_CMD_TFTPSRV
	case TFTP_WRQ:
		debug("Got WRQ\n");
		tftp_remote_ip = sip;
		tftp_remote_port = src;
		tftp_our_port = 1024 + (get_timer(0) % 3072);
		new_transfer();
		tftp_send(); /* Send ACK(0) */
		break;
#endif

	case TFTP_OACK:
		debug("Got OACK: ");
		for (i = 0; i < len; i++) {
			if (pkt[i] == '\0')
				debug(" ");
			else
				debug("%c", pkt[i]);
		}
		debug("\n");
		tftp_state = STATE_OACK;
		tftp_remote_port = src;
		/*
		 * Check for 'blksize' option.
		 * Careful: "i" is signed, "len" is unsigned, thus
		 * something like "len-8" may give a *huge* number
		 */
		for (i = 0; i+8 < len; i++) {
			if (strcasecmp((char *)pkt + i, "blksize") == 0) {
				tftp_block_size = (unsigned short)
					dectoul((char *)pkt + i + 8, NULL);
				debug("Blocksize oack: %s, %d\n",
				      (char *)pkt + i + 8, tftp_block_size);
				if (tftp_block_size > tftp_block_size_option) {
					printf("Invalid blk size(=%d)\n",
					       tftp_block_size);
					tftp_state = STATE_INVALID_OPTION;
				}
			}
			if (strcasecmp((char *)pkt + i, "timeout") == 0) {
				timeout_val_rcvd = (unsigned short)
					dectoul((char *)pkt + i + 8, NULL);
				debug("Timeout oack: %s, %d\n",
				      (char *)pkt + i + 8, timeout_val_rcvd);
				if (timeout_val_rcvd != (timeout_ms / 1000)) {
					printf("Invalid timeout val(=%d s)\n",
					       timeout_val_rcvd);
					tftp_state = STATE_INVALID_OPTION;
				}
			}
#ifdef CONFIG_TFTP_TSIZE
			if (strcasecmp((char *)pkt + i, "tsize") == 0) {
				tftp_tsize = dectoul((char *)pkt + i + 6,
						     NULL);
				debug("size = %s, %d\n",
				      (char *)pkt + i + 6, tftp_tsize);
			}
#endif
			if (strcasecmp((char *)pkt + i,  "windowsize") == 0) {
				tftp_windowsize =
					dectoul((char *)pkt + i + 11, NULL);
				debug("windowsize = %s, %d\n",
				      (char *)pkt + i + 11, tftp_windowsize);
			}
		}

		tftp_next_ack = tftp_windowsize;

#ifdef CONFIG_CMD_TFTPPUT
		if (tftp_put_active && tftp_state == STATE_OACK) {
			/* Get ready to send the first block */
			tftp_state = STATE_DATA;
			tftp_cur_block++;
		}
#endif
		tftp_send(); /* Send ACK or first data block */
		break;
	case TFTP_DATA:
		if (len < 2)
			return;
		len -= 2;

		if (ntohs(*(__be16 *)pkt) != (ushort)(tftp_cur_block + 1)) {
			debug("Received unexpected block: %d, expected: %d\n",
			      ntohs(*(__be16 *)pkt),
			      (ushort)(tftp_cur_block + 1));
			/*
			 * If one packet is dropped most likely
			 * all other buffers in the window
			 * that will arrive will cause a sending NACK.
			 * This just overwellms the server, let's just send one.
			 */
			if (tftp_last_nack != tftp_cur_block) {
				tftp_send();
				tftp_last_nack = tftp_cur_block;
				tftp_next_ack = (ushort)(tftp_cur_block +
							 tftp_windowsize);
			}
			break;
		}

		tftp_cur_block++;
		tftp_cur_block %= TFTP_SEQUENCE_SIZE;

		if (tftp_state == STATE_SEND_RRQ) {
			debug("Server did not acknowledge any options!\n");
			tftp_next_ack = tftp_windowsize;
		}

		if (tftp_state == STATE_SEND_RRQ || tftp_state == STATE_OACK ||
		    tftp_state == STATE_RECV_WRQ) {
			/* first block received */
			tftp_state = STATE_DATA;
			tftp_remote_port = src;
			new_transfer();

			if (tftp_cur_block != 1) {	/* Assertion */
				puts("\nTFTP error: ");
				printf("First block is not block 1 (%ld)\n",
				       tftp_cur_block);
				puts("Starting again\n\n");
				net_start_again();
				break;
			}
		}

		if (tftp_cur_block == tftp_prev_block) {
			/* Same block again; ignore it. */
			break;
		}

		update_block_number();
		tftp_prev_block = tftp_cur_block;
		timeout_count_max = tftp_timeout_count_max;
		net_set_timeout_handler(timeout_ms, tftp_timeout_handler);

		if (store_block(tftp_cur_block, pkt + 2, len)) {
			eth_halt();
			net_set_state(NETLOOP_FAIL);
			break;
		}

		if (len < tftp_block_size) {
			tftp_send();
			tftp_complete();
			break;
		}

		/*
		 *	Acknowledge the block just received, which will prompt
		 *	the remote for the next one.
		 */
		if (tftp_cur_block == tftp_next_ack) {
			tftp_send();
			tftp_next_ack += tftp_windowsize;
		}
		break;

	case TFTP_ERROR:
		printf("\nTFTP error: '%s' (%d)\n",
		       pkt + 2, ntohs(*(__be16 *)pkt));

		switch (ntohs(*(__be16 *)pkt)) {
		case TFTP_ERR_FILE_NOT_FOUND:
		case TFTP_ERR_ACCESS_DENIED:
			puts("Not retrying...\n");
			eth_halt();
			net_set_state(NETLOOP_FAIL);
			break;
		case TFTP_ERR_UNDEFINED:
		case TFTP_ERR_DISK_FULL:
		case TFTP_ERR_UNEXPECTED_OPCODE:
		case TFTP_ERR_UNKNOWN_TRANSFER_ID:
		case TFTP_ERR_FILE_ALREADY_EXISTS:
		default:
			puts("Starting again\n\n");
			net_start_again();
			break;
		}
		break;
	}
}


static void tftp_timeout_handler(void)
{
	if (++timeout_count > timeout_count_max) {
		restart("Retry count exceeded");
	} else {
		puts("T ");
		net_set_timeout_handler(timeout_ms, tftp_timeout_handler);
		if (tftp_state != STATE_RECV_WRQ)
			tftp_send();
	}
}

/* Initialize tftp_load_addr and tftp_load_size from image_load_addr and lmb */
static int tftp_init_load_addr(void)
{
#ifdef CONFIG_LMB
	struct lmb lmb;
	phys_size_t max_size;

	lmb_init_and_reserve(&lmb, gd->bd, (void *)gd->fdt_blob);

	max_size = lmb_get_free_size(&lmb, image_load_addr);
	if (!max_size)
		return -1;

	tftp_load_size = max_size;
#endif
	tftp_load_addr = image_load_addr;
	return 0;
}

static int saved_tftp_block_size_option;
static void sanitize_tftp_block_size_option(enum proto_t protocol)
{
	int cap, max_defrag;

	switch (protocol) {
	case TFTPGET:
		max_defrag = config_opt_enabled(CONFIG_IP_DEFRAG, CONFIG_NET_MAXDEFRAG, 0);
		if (max_defrag) {
			/* Account for IP, UDP and TFTP headers. */
			cap = max_defrag - (20 + 8 + 4);
			/* RFC2348 sets a hard upper limit. */
			cap = min(cap, 65464);
			break;
		}
		/*
		 * If not CONFIG_IP_DEFRAG, cap at the same value as
		 * for tftp put, namely normal MTU minus protocol
		 * overhead.
		 */
		fallthrough;
	case TFTPPUT:
	default:
		/*
		 * U-Boot does not support IP fragmentation on TX, so
		 * this must be small enough that it fits normal MTU
		 * (and small enough that it fits net_tx_packet which
		 * has room for PKTSIZE_ALIGN bytes).
		 */
		cap = 1468;
	}
	if (tftp_block_size_option > cap) {
		printf("Capping tftp block size option to %d (was %d)\n",
		       cap, tftp_block_size_option);
		saved_tftp_block_size_option = tftp_block_size_option;
		tftp_block_size_option = cap;
	}
}

void tftp_start(enum proto_t protocol)
{
	__maybe_unused char *ep;             /* Environment pointer */

	if (saved_tftp_block_size_option) {
		tftp_block_size_option = saved_tftp_block_size_option;
		saved_tftp_block_size_option = 0;
	}

	if (IS_ENABLED(CONFIG_NET_TFTP_VARS)) {

		/*
		 * Allow the user to choose TFTP blocksize and timeout.
		 * TFTP protocol has a minimal timeout of 1 second.
		 */

		ep = env_get("tftpblocksize");
		if (ep != NULL)
			tftp_block_size_option = simple_strtol(ep, NULL, 10);

		ep = env_get("tftpwindowsize");
		if (ep != NULL)
			tftp_window_size_option = simple_strtol(ep, NULL, 10);

		ep = env_get("tftptimeout");
		if (ep != NULL)
			timeout_ms = simple_strtol(ep, NULL, 10);

		if (timeout_ms < 1000) {
			printf("TFTP timeout (%ld ms) too low, set min = 1000 ms\n",
			       timeout_ms);
			timeout_ms = 1000;
		}

		ep = env_get("tftptimeoutcountmax");
		if (ep != NULL)
			tftp_timeout_count_max = simple_strtol(ep, NULL, 10);

		if (tftp_timeout_count_max < 0) {
			printf("TFTP timeout count max (%d ms) negative, set to 0\n",
			       tftp_timeout_count_max);
			tftp_timeout_count_max = 0;
		}
	}

	sanitize_tftp_block_size_option(protocol);

	debug("TFTP blocksize = %i, TFTP windowsize = %d timeout = %ld ms\n",
	      tftp_block_size_option, tftp_window_size_option, timeout_ms);

	if (IS_ENABLED(CONFIG_IPV6))
		tftp_remote_ip6 = net_server_ip6;

	tftp_remote_ip = net_server_ip;
	if (!net_parse_bootfile(&tftp_remote_ip, tftp_filename, MAX_LEN)) {
		sprintf(default_filename, "%02X%02X%02X%02X.img",
			net_ip.s_addr & 0xFF,
			(net_ip.s_addr >>  8) & 0xFF,
			(net_ip.s_addr >> 16) & 0xFF,
			(net_ip.s_addr >> 24) & 0xFF);

		strncpy(tftp_filename, default_filename, DEFAULT_NAME_LEN);
		tftp_filename[DEFAULT_NAME_LEN - 1] = 0;

		printf("*** Warning: no boot file name; using '%s'\n",
		       tftp_filename);
	}

	if (IS_ENABLED(CONFIG_IPV6)) {
		if (use_ip6) {
			char *s, *e;
			size_t len;

			s = strchr(net_boot_file_name, '[');
			e = strchr(net_boot_file_name, ']');
			len = e - s;
			if (s && e) {
				string_to_ip6(s + 1, len, &tftp_remote_ip6);
				strlcpy(tftp_filename, e + 2, MAX_LEN);
			} else {
				strlcpy(tftp_filename, net_boot_file_name, MAX_LEN);
				tftp_filename[MAX_LEN - 1] = 0;
			}
		}
	}

	printf("Using %s device\n", eth_get_name());

	if (IS_ENABLED(CONFIG_IPV6) && use_ip6) {
		printf("TFTP from server %pI6c; our IP address is %pI6c",
		       &tftp_remote_ip6, &net_ip6);

		if (tftp_block_size_option > TFTP_MTU_BLOCKSIZE6)
			tftp_block_size_option = TFTP_MTU_BLOCKSIZE6;
	} else {
		printf("TFTP %s server %pI4; our IP address is %pI4",
#ifdef CONFIG_CMD_TFTPPUT
	       protocol == TFTPPUT ? "to" : "from",
#else
	       "from",
#endif
	       &tftp_remote_ip, &net_ip);
	}

	/* Check if we need to send across this subnet */
	if (IS_ENABLED(CONFIG_IPV6) && use_ip6) {
		if (!ip6_addr_in_subnet(&net_ip6, &tftp_remote_ip6,
					net_prefix_length))
			printf("; sending through gateway %pI6c",
			       &net_gateway6);
	} else if (net_gateway.s_addr && net_netmask.s_addr) {
		struct in_addr our_net;
		struct in_addr remote_net;

		our_net.s_addr = net_ip.s_addr & net_netmask.s_addr;
		remote_net.s_addr = tftp_remote_ip.s_addr & net_netmask.s_addr;
		if (our_net.s_addr != remote_net.s_addr)
			printf("; sending through gateway %pI4", &net_gateway);
	}
	putc('\n');

	printf("Filename '%s'.", tftp_filename);

	if (net_boot_file_expected_size_in_blocks) {
		printf(" Size is 0x%x Bytes = ",
		       net_boot_file_expected_size_in_blocks << 9);
		print_size(net_boot_file_expected_size_in_blocks << 9, "");
	}

	putc('\n');
#ifdef CONFIG_CMD_TFTPPUT
	tftp_put_active = (protocol == TFTPPUT);
	if (tftp_put_active) {
		printf("Save address: 0x%lx\n", image_save_addr);
		printf("Save size:    0x%lx\n", image_save_size);
		net_boot_file_size = image_save_size;
		puts("Saving: *\b");
		tftp_state = STATE_SEND_WRQ;
		new_transfer();
	} else
#endif
	{
		if (tftp_init_load_addr()) {
			eth_halt();
			net_set_state(NETLOOP_FAIL);
			puts("\nTFTP error: ");
			puts("trying to overwrite reserved memory...\n");
			return;
		}
		printf("Load address: 0x%lx\n", tftp_load_addr);
		puts("Loading: *\b");
		tftp_state = STATE_SEND_RRQ;
	}

	time_start = get_timer(0);
	timeout_count_max = tftp_timeout_count_max;

	net_set_timeout_handler(timeout_ms, tftp_timeout_handler);
	net_set_udp_handler(tftp_handler);
#ifdef CONFIG_CMD_TFTPPUT
	net_set_icmp_handler(icmp_handler);
#endif
	tftp_remote_port = WELL_KNOWN_PORT;
	timeout_count = 0;
	/* Use a pseudo-random port unless a specific port is set */
	tftp_our_port = 1024 + (get_timer(0) % 3072);

#ifdef CONFIG_TFTP_PORT
	ep = env_get("tftpdstp");
	if (ep != NULL)
		tftp_remote_port = simple_strtol(ep, NULL, 10);
	ep = env_get("tftpsrcp");
	if (ep != NULL)
		tftp_our_port = simple_strtol(ep, NULL, 10);
#endif
	tftp_cur_block = 0;
	tftp_windowsize = 1;
	tftp_last_nack = 0;
	/* zero out server ether in case the server ip has changed */
	memset(net_server_ethaddr, 0, 6);
	/* Revert tftp_block_size to dflt */
	tftp_block_size = TFTP_BLOCK_SIZE;
#ifdef CONFIG_TFTP_TSIZE
	tftp_tsize = 0;
	tftp_tsize_num_hash = 0;
#endif

	tftp_send();
}

#ifdef CONFIG_CMD_TFTPSRV
void tftp_start_server(void)
{
	tftp_filename[0] = 0;

	if (tftp_init_load_addr()) {
		eth_halt();
		net_set_state(NETLOOP_FAIL);
		puts("\nTFTP error: trying to overwrite reserved memory...\n");
		return;
	}
	printf("Using %s device\n", eth_get_name());
	printf("Listening for TFTP transfer on %pI4\n", &net_ip);
	printf("Load address: 0x%lx\n", tftp_load_addr);

	puts("Loading: *\b");

	timeout_count_max = tftp_timeout_count_max;
	timeout_count = 0;
	timeout_ms = TIMEOUT;
	net_set_timeout_handler(timeout_ms, tftp_timeout_handler);

	/* Revert tftp_block_size to dflt */
	tftp_block_size = TFTP_BLOCK_SIZE;
	tftp_cur_block = 0;
	tftp_our_port = WELL_KNOWN_PORT;
	tftp_windowsize = 1;
	tftp_next_ack = tftp_windowsize;

#ifdef CONFIG_TFTP_TSIZE
	tftp_tsize = 0;
	tftp_tsize_num_hash = 0;
#endif

	tftp_state = STATE_RECV_WRQ;
	net_set_udp_handler(tftp_handler);

	/* zero out server ether in case the server ip has changed */
	memset(net_server_ethaddr, 0, 6);
}
#endif /* CONFIG_CMD_TFTPSRV */
