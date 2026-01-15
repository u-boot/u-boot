// SPDX-License-Identifier: GPL-2.0+
/*
 * NFS support driver - based on etherboot and U-BOOT's tftp.c
 *
 * Masami Komiya <mkomiya@sonare.it> 2004
 *
 */

/* NOTE: the NFS code is heavily inspired by the NetBSD netboot code (read:
 * large portions are copied verbatim) as distributed in OSKit 0.97.  A few
 * changes were necessary to adapt the code to Etherboot and to fix several
 * inconsistencies.  Also the RPC message preparation is done "by hand" to
 * avoid adding netsprintf() which I find hard to understand and use.  */

/* NOTE 2: Etherboot does not care about things beyond the kernel image, so
 * it loads the kernel image off the boot server (ARP_SERVER) and does not
 * access the client root disk (root-path in dhcpd.conf), which would use
 * ARP_ROOTSERVER.  The root disk is something the operating system we are
 * about to load needs to use.	This is different from the OSKit 0.97 logic.  */

/* NOTE 3: Symlink handling introduced by Anselm M Hoffmeister, 2003-July-14
 * If a symlink is encountered, it is followed as far as possible (recursion
 * possible, maximum 16 steps). There is no clearing of ".."'s inside the
 * path, so please DON'T DO THAT. thx. */

/* NOTE 4: NFSv3 support added by Guillaume GARDET, 2016-June-20.
 * NFSv2 is still used by default. But if server does not support NFSv2, then
 * NFSv3 is used, if available on NFS server. */

/* NOTE 5: NFSv1 support added by Christian Gmeiner, Thomas Rienoessl,
 * September 27, 2018. As of now, NFSv3 is the default choice. If the server
 * does not support NFSv3, we fall back to versions 2 or 1. */

#include <display_options.h>
#include <image.h>
#include <log.h>
#include <net.h>
#include <malloc.h>
#include "nfs.h"
#include "nfs-common.h"
#include <time.h>

/**************************************************************************
RPC_LOOKUP - Lookup RPC Port numbers
**************************************************************************/
void rpc_req(int rpc_prog, int rpc_proc, uint32_t *data, int datalen)
{
	int pktlen;
	int sport;

	rpc_req_common(rpc_prog, rpc_proc, data, datalen,
		       (char *)net_tx_packet + net_eth_hdr_size()
		       + IP_UDP_HDR_SIZE, &pktlen, &sport);

	net_send_udp_packet(net_server_ethaddr, nfs_server_ip, sport,
			    nfs_our_port, pktlen);
}

/**************************************************************************
Interfaces of U-BOOT
**************************************************************************/
static void nfs_timeout_handler(void)
{
	if (++nfs_timeout_count > NFS_RETRY_COUNT) {
		puts("\nRetry count exceeded; starting again\n");
		net_start_again();
	} else {
		puts("T ");
		net_set_timeout_handler(nfs_timeout +
					nfs_timeout * nfs_timeout_count,
					nfs_timeout_handler);
		nfs_send();
	}
}

void nfs_refresh_timeout(void)
{
	net_set_timeout_handler(nfs_timeout, nfs_timeout_handler);
}

static void nfs_handler(uchar *pkt, unsigned dest, struct in_addr sip,
			unsigned src, unsigned len)
{
	debug("%s\n", __func__);

	if (len > sizeof(struct rpc_t))
		return;

	if (dest != nfs_our_port)
		return;

	nfs_pkt_recv(pkt, len);
}

void nfs_start(void)
{
	debug("%s\n", __func__);
	nfs_download_state = NETLOOP_FAIL;

	nfs_server_ip = net_server_ip;
	nfs_path = (char *)nfs_path_buff;

	if (nfs_path == NULL) {
		net_set_state(NETLOOP_FAIL);
		printf("*** ERROR: Fail allocate memory\n");
		return;
	}

	if (!net_parse_bootfile(&nfs_server_ip, nfs_path,
				sizeof(nfs_path_buff))) {
		sprintf(nfs_path, "/nfsroot/%02X%02X%02X%02X.img",
			net_ip.s_addr & 0xFF,
			(net_ip.s_addr >>  8) & 0xFF,
			(net_ip.s_addr >> 16) & 0xFF,
			(net_ip.s_addr >> 24) & 0xFF);

		printf("*** Warning: no boot file name; using '%s'\n",
		       nfs_path);
	}

	nfs_filename = nfs_basename(nfs_path);
	nfs_path     = nfs_dirname(nfs_path);

	printf("Using %s device\n", eth_get_name());

	printf("File transfer via NFS from server %pI4; our IP address is %pI4",
	       &nfs_server_ip, &net_ip);

	/* Check if we need to send across this subnet */
	if (net_gateway.s_addr && net_netmask.s_addr) {
		struct in_addr our_net;
		struct in_addr server_net;

		our_net.s_addr = net_ip.s_addr & net_netmask.s_addr;
		server_net.s_addr = nfs_server_ip.s_addr & net_netmask.s_addr;
		if (our_net.s_addr != server_net.s_addr)
			printf("; sending through gateway %pI4",
			       &net_gateway);
	}
	printf("\nFilename '%s/%s'.", nfs_path, nfs_filename);

	if (net_boot_file_expected_size_in_blocks) {
		printf(" Size is 0x%x Bytes = ",
		       net_boot_file_expected_size_in_blocks << 9);
		print_size(net_boot_file_expected_size_in_blocks << 9, "");
	}
	printf("\nLoad address: 0x%lx\nLoading: *\b", image_load_addr);

	net_set_timeout_handler(nfs_timeout, nfs_timeout_handler);
	net_set_udp_handler(nfs_handler);

	nfs_timeout_count = 0;
	nfs_state = STATE_PRCLOOKUP_PROG_MOUNT_REQ;

	/*nfs_our_port = 4096 + (get_ticks() % 3072);*/
	/*FIX ME !!!*/
	nfs_our_port = 1000;

	/* zero out server ether in case the server ip has changed */
	memset(net_server_ethaddr, 0, 6);

	nfs_send();
}
