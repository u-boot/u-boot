// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2025 Linaro Ltd. */

#include <console.h>
#include <display_options.h>
#include <env.h>
#include <image.h>
#include <linux/kconfig.h>
#include <lwip/timeouts.h>
#include <lwip/udp.h>
#include <net.h>
#include "../nfs-common.h"
#include <time.h>

static ulong timer_start;

static struct nfs_ctx {
	ip_addr_t nfs_server;
	struct udp_pcb *pcb;
} sess_ctx;

/**************************************************************************
 * RPC_LOOKUP - Lookup RPC Port numbers
 **************************************************************************
 */
void rpc_req(int rpc_prog, int rpc_proc, uint32_t *data, int datalen)
{
	struct pbuf *pb;
	int pktlen;
	int sport;
	err_t err;

	pb = pbuf_alloc(PBUF_TRANSPORT, sizeof(struct rpc_t), PBUF_RAM);
	if (!pb) {
		debug("Failed to allocate pbuf to build RPC packet\n");
		return;
	}

	rpc_req_common(rpc_prog, rpc_proc, data, datalen,
		       pb->payload, &pktlen, &sport);

	pbuf_realloc(pb, (u16_t)pktlen);

	err = udp_sendto(sess_ctx.pcb, pb, &sess_ctx.nfs_server, sport);
	debug_cond((err != ERR_OK), "Failed to send UDP packet err = %d\n", err);
	pbuf_free(pb);
}

void nfs_refresh_timeout(void)
{
	timer_start = get_timer(0);
}

static void nfs_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
		     const ip_addr_t *addr, u16_t port)
{
	struct nfs_ctx *ctx = arg;
	int plen;
	struct rpc_t rpc_pkt;

	if (addr->addr != ctx->nfs_server.addr)
		goto exitfree;

	if (p->tot_len > sizeof(struct rpc_t))
		goto exitfree;

	plen = pbuf_copy_partial(p, &rpc_pkt.u.data[0], sizeof(rpc_pkt), 0);
	nfs_pkt_recv(&rpc_pkt.u.data[0], plen);

exitfree:
	pbuf_free(p);
}

static int nfs_udp_init(struct nfs_ctx *ctx)
{
	ctx->pcb = udp_new();
	if (!ctx->pcb)
		return -ENOMEM;

	ctx->pcb->local_port = nfs_our_port;
	udp_recv(ctx->pcb, nfs_recv, ctx);

	return 0;
}

static int nfs_timeout_check(void)
{
	if (get_timer(timer_start) < nfs_timeout)
		return 0;
	if (++nfs_timeout_count < NFS_RETRY_COUNT) {
		puts("T ");
		timer_start = get_timer(0);
		nfs_send();
		return 0;
	}

	return 1;
}

static int nfs_loop(struct udevice *udev, ulong addr, char *fname,
		    ip_addr_t srvip)
{
	struct netif *netif;
	int ret;

	nfs_download_state = NETLOOP_FAIL;
	net_set_state(NETLOOP_FAIL);

	if (!fname || addr == 0)
		return -1;

	netif = net_lwip_new_netif(udev);
	if (!netif)
		return -1;

	nfs_filename = nfs_basename(fname);
	nfs_path     = nfs_dirname(fname);

	printf("Using %s device\n", eth_get_name());

	printf("File transfer via NFS from server %s; our IP address is %s\n",
	       ip4addr_ntoa(&srvip), env_get("ipaddr"));

	printf("\nFilename '%s/%s'.", nfs_path, nfs_filename);

	if (net_boot_file_expected_size_in_blocks) {
		printf(" Size is 0x%x Bytes = ",
		       net_boot_file_expected_size_in_blocks << 9);
		print_size(net_boot_file_expected_size_in_blocks << 9, "");
	}
	printf("\nLoad address: 0x%lx\nLoading: *\b", addr);
	image_load_addr = addr;

	nfs_timeout_count = 0;
	nfs_state = STATE_PRCLOOKUP_PROG_MOUNT_REQ;

	ret = nfs_udp_init(&sess_ctx);
	if (ret < 0) {
		net_lwip_remove_netif(netif);
		debug("Failed to init network interface, aborting for error = %d\n", ret);
		return ret;
	}

	net_set_state(NETLOOP_CONTINUE);

	sess_ctx.nfs_server.addr = srvip.addr;

	nfs_send();

	timer_start = get_timer(0);
	do {
		net_lwip_rx(udev, netif);
		if (net_state != NETLOOP_CONTINUE)
			break;
		if (ctrlc()) {
			printf("\nAbort\n");
			break;
		}

		if (nfs_timeout_check())
			break;
	} while (true);
	debug("%s: Loop exit at %lu\n", __func__, get_timer(0));

	net_lwip_remove_netif(netif);

	if (net_state == NETLOOP_SUCCESS) {
		ret = 0;
		if (net_boot_file_size > 0) {
			printf("Bytes transferred = %u (%x hex)\n",
			       net_boot_file_size, net_boot_file_size);
			env_set_hex("filesize", net_boot_file_size);
			env_set_hex("fileaddr", image_load_addr);
		}

	} else {
		debug("%s: NFS loop failed\n", __func__);
		ret = -1;
	}

	return ret;
}

int do_nfs(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = CMD_RET_SUCCESS;
	char *arg = NULL;
	char *words[2] = { };
	char *fname = NULL;
	char *server_ip = NULL;
	char *end;
	ip_addr_t srvip;
	ulong laddr;
	ulong addr;
	int i;

	laddr = env_get_ulong("loadaddr", 16, image_load_addr);

	switch (argc) {
	case 1:
		fname = env_get("bootfile");
		break;
	case 2:
		/*
		 * Only one arg - accept two forms:
		 * Just load address, or just boot file name. The latter
		 * form must be written in a format which can not be
		 * mis-interpreted as a valid number.
		 */
		addr = hextoul(argv[1], &end);
		if (end == (argv[1] + strlen(argv[1]))) {
			laddr = addr;
			fname = env_get("bootfile");
		} else {
			arg = strdup(argv[1]);
		}
		break;
	case 3:
		laddr = hextoul(argv[1], NULL);
		arg = strdup(argv[2]);
		break;
	default:
		ret = CMD_RET_USAGE;
		goto out;
	}

	if (!arg)
		arg = net_boot_file_name;

	if (*arg) {
		/* Parse [ip:]fname */
		i = 0;
		while ((*(words + i) = strsep(&arg, ":")))
			i++;

		switch (i) {
		case 2:
			server_ip = words[0];
			fname = words[1];
			break;
		case 1:
			fname = words[0];
			break;
		default:
			break;
		}
	}

	if (!server_ip)
		server_ip = env_get("serverip");
	if (!server_ip) {
		log_err("*** ERROR: 'serverip' not set\n");
		ret = CMD_RET_FAILURE;
		goto out;
	}

	if (!ipaddr_aton(server_ip, &srvip)) {
		log_err("error: ipaddr_aton\n");
		ret = CMD_RET_FAILURE;
		goto out;
	}

	if (!fname) {
		log_err("error: no file name\n");
		ret = CMD_RET_FAILURE;
		goto out;
	}

	if (!laddr) {
		log_err("error: no load address\n");
		ret = CMD_RET_FAILURE;
		goto out;
	}

	if (net_lwip_eth_start() < 0) {
		ret = CMD_RET_FAILURE;
		goto out;
	}

	if (nfs_loop(eth_get_dev(), laddr, fname, srvip) < 0)
		ret = CMD_RET_FAILURE;
out:
	if (arg != net_boot_file_name)
		free(arg);
	return ret;
}
