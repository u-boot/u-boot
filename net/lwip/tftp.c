// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <console.h>
#include <display_options.h>
#include <dm/device.h>
#include <efi_loader.h>
#include <env.h>
#include <image.h>
#include <linux/delay.h>
#include <linux/kconfig.h>
#include <lwip/apps/tftp_client.h>
#include <lwip/timeouts.h>
#include <mapmem.h>
#include <net.h>
#include <time.h>

#define PROGRESS_PRINT_STEP_BYTES (10 * 1024)
/* Max time to wait for first data packet from server */
#define NO_RSP_TIMEOUT_MS 10000

enum done_state {
	NOT_DONE = 0,
	SUCCESS,
	FAILURE,
	ABORTED
};

struct tftp_ctx {
	ulong daddr;
	ulong size;
	ulong block_count;
	ulong start_time;
	enum done_state done;
};

/**
 * store_block() - copy received data
 *
 * This function is called by the receive callback to copy a block of data
 * into its final location (ctx->daddr). Before doing so, it checks if the copy
 * is allowed.
 *
 * @ctx: the context for the current transfer
 * @src: the data received from the TCP stack
 * @len: the length of the data
 */
static int store_block(struct tftp_ctx *ctx, void *src, u16_t len)
{
	ulong store_addr = ctx->daddr;
	void *ptr;

	if (CONFIG_IS_ENABLED(LMB)) {
		if (store_addr + len < store_addr ||
		    lmb_read_check(store_addr, len)) {
			puts("\nTFTP error: ");
			puts("trying to overwrite reserved memory...\n");
			return -1;
		}
	}

	ptr = map_sysmem(store_addr, len);
	memcpy(ptr, src, len);
	unmap_sysmem(ptr);

	ctx->daddr += len;
	ctx->size += len;
	ctx->block_count++;
	if (ctx->block_count % 10 == 0) {
		putc('#');
		if (ctx->block_count % (65 * 10) == 0)
			puts("\n\t ");
	}

	return 0;
}

static void *tftp_open(const char *fname, const char *mode, u8_t is_write)
{
	return NULL;
}

static void tftp_close(void *handle)
{
	struct tftp_ctx *ctx = handle;
	ulong elapsed;

	if (ctx->done == FAILURE || ctx->done == ABORTED) {
		/* Closing after an error or Ctrl-C */
		return;
	}
	ctx->done = SUCCESS;

	elapsed = get_timer(ctx->start_time);
	if (elapsed > 0) {
		puts("\n\t ");	/* Line up with "Loading: " */
		print_size(ctx->size / elapsed * 1000, "/s");
	}
	puts("\ndone\n");
	printf("Bytes transferred = %lu (%lx hex)\n", ctx->size, ctx->size);

	if (env_set_hex("filesize", ctx->size)) {
		log_err("filesize not updated\n");
		return;
	}
}

static int tftp_read(void *handle, void *buf, int bytes)
{
	return 0;
}

static int tftp_write(void *handle, struct pbuf *p)
{
	struct tftp_ctx *ctx = handle;
	struct pbuf *q;

	for (q = p; q; q = q->next)
		if (store_block(ctx, q->payload, q->len) < 0)
			return -1;

	return 0;
}

static void tftp_error(void *handle, int err, const char *msg, int size)
{
	struct tftp_ctx *ctx = handle;
	char message[100];

	ctx->done = FAILURE;
	memset(message, 0, sizeof(message));
	memcpy(message, msg, LWIP_MIN(sizeof(message) - 1, (size_t)size));

	printf("\nTFTP error: %d (%s)\n", err, message);
}

static const struct tftp_context tftp_context = {
	tftp_open,
	tftp_close,
	tftp_read,
	tftp_write,
	tftp_error
};

static void no_response(void *arg)
{
	struct tftp_ctx *ctx = (struct tftp_ctx *)arg;

	if (ctx->size)
		return;

	printf("Timeout!\n");
	ctx->done = FAILURE;
}

static int tftp_loop(struct udevice *udev, ulong addr, char *fname,
		     ip_addr_t srvip, uint16_t srvport)
{
	int blksize = CONFIG_TFTP_BLOCKSIZE;
	struct netif *netif;
	struct tftp_ctx ctx;
	const char *ep;
	err_t err;

	if (!fname || addr == 0)
		return -1;

	if (!srvport)
		srvport = TFTP_PORT;

	netif = net_lwip_new_netif(udev);
	if (!netif)
		return -1;

	ctx.done = NOT_DONE;
	ctx.size = 0;
	ctx.block_count = 0;
	ctx.daddr = addr;

	printf("Using %s device\n", udev->name);
	printf("TFTP from server %s; our IP address is %s\n",
	       ip4addr_ntoa(&srvip), env_get("ipaddr"));
	printf("Filename '%s'.\n", fname);
	printf("Load address: 0x%lx\n", ctx.daddr);
	printf("Loading: ");

	err = tftp_init_client(&tftp_context);
	if (!(err == ERR_OK || err == ERR_USE))
		log_err("tftp_init_client err: %d\n", err);

	ep = env_get("tftpblocksize");
	if (ep)
		blksize = simple_strtol(ep, NULL, 10);
	tftp_client_set_blksize(blksize);

	ctx.start_time = get_timer(0);
	err = tftp_get(&ctx, &srvip, srvport, fname, TFTP_MODE_OCTET);
	/* might return different errors, like routing problems */
	if (err != ERR_OK) {
		printf("tftp_get() error %d\n", err);
		net_lwip_remove_netif(netif);
		return -1;
	}

	sys_timeout(NO_RSP_TIMEOUT_MS, no_response, &ctx);
	while (!ctx.done) {
		net_lwip_rx(udev, netif);
		if (ctrlc()) {
			printf("\nAbort\n");
			ctx.done = ABORTED;
			break;
		}
	}
	sys_untimeout(no_response, (void *)&ctx);

	tftp_cleanup();

	net_lwip_remove_netif(netif);

	if (ctx.done == SUCCESS) {
		if (env_set_hex("fileaddr", addr)) {
			log_err("fileaddr not updated\n");
			return -1;
		}
		efi_set_bootdev("Net", "", fname, map_sysmem(addr, 0),
				ctx.size);
		return 0;
	}

	return -1;
}

int do_tftpb(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = CMD_RET_SUCCESS;
	char *arg = NULL;
	char *words[3] = { };
	char *fname = NULL;
	char *server_ip = NULL;
	char *server_port = NULL;
	char *end;
	ip_addr_t srvip;
	u16 port = TFTP_PORT;
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

	if (arg) {
		/* Parse [ip:[port:]]fname */
		i = 0;
		while ((*(words + i) = strsep(&arg, ":")))
			i++;

		switch (i) {
		case 3:
			server_ip = words[0];
			server_port = words[1];
			fname = words[2];
			break;
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
		server_ip = env_get("tftpserverip");
	if (!server_ip)
		server_ip = env_get("serverip");
	if (!server_ip) {
		log_err("error: tftpserverip/serverip has to be set\n");
		ret = CMD_RET_FAILURE;
		goto out;
	}

	if (server_port)
		port = dectoul(server_port, NULL);

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

	if (tftp_loop(eth_get_dev(), laddr, fname, srvip, port) < 0)
		ret = CMD_RET_FAILURE;
out:
	free(arg);
	return ret;
}
