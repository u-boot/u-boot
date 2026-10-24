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
#include <lwip/apps/tftp_server.h>
#include <lwip/etharp.h>
#include <lwip/timeouts.h>
#include <mapmem.h>
#include <net.h>
#include <time.h>

#define PROGRESS_PRINT_STEP_BYTES (10 * 1024)
#define UBOOT_TFTP_TIMEOUT_MS 5000
#define UBOOT_TFTP_MAX_RETRIES (CONFIG_NET_RETRY_COUNT * 2)

enum done_state {
	NOT_DONE = 0,
	SUCCESS,
	FAILURE,
	ABORTED
};

enum tftp_operation {
	TFTP_DOWNLOAD,
	TFTP_UPLOAD,
};

struct tftp_ctx {
	ulong daddr;
	ulong size;
	ulong total_size;
	ulong block_count;
	ulong hash_count;
	ulong start_time;
	enum done_state done;
	bool is_server;
	bool upload;
	bool wrq_accepted;
	u32 listen_retries;
	bool retry;
	char fname[TFTP_MAX_FILENAME_LEN + 1];
};

/*
 * The lwIP TFTP server open callback has no user-data argument. Keep the
 * current server context here so tftp_open() can return it.
 */
static struct tftp_ctx *tftpsrv_active_ctx;

static int tftp_check_ip(struct net_lwip_ctx *net)
{
	if (ip4_addr_isany_val(*netif_ip4_addr(net->netif))) {
		log_err("error: IP address for %s has to be set\n", net->dev->name);
		return -EINVAL;
	}

	return 0;
}

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

	return 0;
}

static ulong tftp_transfer_size(struct tftp_ctx *ctx)
{
	if (ctx->upload)
		return ctx->total_size;
	if (!ctx->is_server)
		return tftp_client_get_tsize();

	return 0;
}

static void tftp_show_progress(struct tftp_ctx *ctx)
{
	ulong tftp_tsize = tftp_transfer_size(ctx);
	ulong pos;

	ctx->block_count++;

	if (tftp_tsize) {
		pos = clamp(ctx->size, 0UL, tftp_tsize);

		while (ctx->hash_count < (u64)pos * 50 / tftp_tsize) {
			putc('#');
			ctx->hash_count++;
		}
	} else {
		if (ctx->block_count % 10 == 0) {
			putc('#');
			if (ctx->block_count % (65 * 10) == 0)
				puts("\n\t ");
		}
	}
}

static void *tftp_open(const char *fname, const char *mode, u8_t is_write)
{
	struct tftp_ctx *ctx = tftpsrv_active_ctx;

	if (!IS_ENABLED(CONFIG_CMD_TFTPSRV) || !ctx || !is_write)
		return NULL;
	/* The command may still be draining its final receive batch. */
	if (ctx->done != NOT_DONE)
		return NULL;

	ctx->wrq_accepted = true;
	ctx->start_time = get_timer(0);
	snprintf(ctx->fname, sizeof(ctx->fname), "%s", fname);
	printf("\nReceiving '%s' mode '%s'\n", fname, mode);
	puts("Loading: ");

	return ctx;
}

static void tftp_close(void *handle)
{
	struct tftp_ctx *ctx = handle;
	ulong tftp_tsize;
	ulong elapsed;

	if (ctx->done == FAILURE || ctx->done == ABORTED) {
		/* Closing after an error or Ctrl-C */
		return;
	}
	ctx->done = SUCCESS;

	tftp_tsize = tftp_transfer_size(ctx);
	if (tftp_tsize) {
		/* Print hash marks for the last packet received */
		while (ctx->hash_count < 50) {
			putc('#');
			ctx->hash_count++;
		}
		puts("  ");
		print_size(tftp_tsize, "");
	}

	elapsed = get_timer(ctx->start_time);
	if (elapsed > 0) {
		puts("\n\t ");	/* Line up with "Loading: " */
		print_size((u64)ctx->size * 1000 / elapsed, "/s");
	}
	puts("\ndone\n");
	printf("Bytes transferred = %lu (%lx hex)\n", ctx->size, ctx->size);
}

/* Publish completion only after teardown and the fallible environment writes. */
static int tftp_complete(struct tftp_ctx *ctx, ulong addr, const char *fname)
{
	if (ctx->done != SUCCESS)
		return ctx->done == FAILURE && ctx->retry ? -EAGAIN : -EIO;
	if (!ctx->upload && env_set_hex("fileaddr", addr)) {
		log_err("fileaddr not updated\n");
		return -EIO;
	}
	if (env_set_hex("filesize", ctx->size)) {
		log_err("filesize not updated\n");
		return -EIO;
	}
	net_boot_file_size = ctx->size;
	if (!ctx->upload)
		efi_set_bootdev("Net", "", fname, map_sysmem(addr, 0), ctx->size);

	return 0;
}

static int tftp_read(void *handle, void *buf, int bytes)
{
	struct tftp_ctx *ctx = handle;
	ulong remaining;
	ulong len;
	void *ptr;

	if (bytes <= 0 || ctx->size >= ctx->total_size) {
		len = 0;
	} else {
		remaining = ctx->total_size - ctx->size;
		len = min_t(ulong, bytes, remaining);

		ptr = map_sysmem(ctx->daddr, len);
		memcpy(buf, ptr, len);
		unmap_sysmem(ptr);

		ctx->daddr += len;
		ctx->size += len;
	}

	tftp_show_progress(ctx);

	return len;
}

static int tftp_write(void *handle, struct pbuf *p)
{
	struct tftp_ctx *ctx = handle;
	struct pbuf *q;

	for (q = p; q; q = q->next) {
		if (store_block(ctx, q->payload, q->len) < 0) {
			ctx->done = FAILURE;
			return -1;
		}
	}
	tftp_show_progress(ctx);

	return 0;
}

static void tftp_error(void *handle, int err, const char *msg, int size)
{
	struct tftp_ctx *ctx = handle;
	char message[100];

	ctx->done = FAILURE;
	/* Match the legacy client's server-error and retry-exhaustion policy.
	 * Local validation and memory-write failures must not be retried.
	 */
	ctx->retry = err == TFTP_ERROR_TIMEOUT ||
		(err >= 0 && err != TFTP_ERROR_FILE_NOT_FOUND &&
		 err != TFTP_ERROR_ACCESS_VIOLATION);
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

static int tftp_get_blocksize(enum tftp_operation operation, u16 *blksize)
{
	const char *ep = NULL;
	ulong value = CONFIG_TFTP_BLOCKSIZE;
	ulong max_blksize = 1468;

	/* Keep uploads within the Ethernet MTU, like legacy. Downloads may
	 * use the pbuf budget sized for full-MTU fragments in lwipopts.h.
	 * CONFIG_TFTP_BLOCKSIZE is the default, not the receive limit.
	 */
	if (operation == TFTP_DOWNLOAD && IP_REASSEMBLY)
		max_blksize = min_t(ulong, 65464,
				    IP_REASS_MAX_PBUFS * IP_FRAG_MTU_USABLE - UDP_HDR_SIZE - 4);
	if (IS_ENABLED(CONFIG_NET_TFTP_VARS))
		ep = env_get("tftpblocksize");

	if (ep && strict_strtoul(ep, 10, &value)) {
		log_err("error: invalid TFTP block size\n");
		return -EINVAL;
	}
	if (value < 8) {
		log_err("error: TFTP block size must be at least 8 bytes\n");
		return -EINVAL;
	}
	if (value > max_blksize) {
		printf("Capping TFTP block size to %lu (was %lu)\n",
		       max_blksize, value);
		value = max_blksize;
	}

	*blksize = value;

	return 0;
}

/* Resolve the next hop before starting an internal caller's short RRQ timer. */
static int tftp_probe_arp(struct netif *netif, const ip4_addr_t *server)
{
	const ip4_addr_t *target = server;
	const ip4_addr_t *resolved;
	struct eth_addr *ethaddr;
	ulong start;
	int tries;

	if (ip4_addr_isbroadcast(server, netif) || ip4_addr_ismulticast(server))
		return 0;
	if (!ip4_addr_net_eq(server, netif_ip4_addr(netif), netif_ip4_netmask(netif)) &&
	    !ip4_addr_islinklocal(server)) {
		target = netif_ip4_gw(netif);
		/* Leave a missing route to the TFTP client's normal error handling. */
		if (ip4_addr_isany_val(*target))
			return 0;
	}
	if (etharp_find_addr(netif, target, &ethaddr, &resolved) >= 0)
		return 0;

	/* Match net/arp.c's bounded retry policy, including the initial request. */
	for (tries = 0; tries < max(1, CONFIG_NET_RETRY_COUNT - 1); tries++) {
		start = get_timer(0);
		/* Do not queue an RRQ or create a pending entry with its own timer. */
		etharp_request(netif, target);
		do {
			net_lwip_poll();
			if (ctrlc()) {
				printf("\nAbort\n");
				return -EINTR;
			}
			if (etharp_find_addr(netif, target, &ethaddr, &resolved) >= 0)
				return 0;
		} while (get_timer(start) <= CONFIG_ARP_TIMEOUT);
	}

	printf("\nARP Retry count exceeded\n");
	return -EAGAIN;
}

static int tftp_loop(struct net_lwip_ctx *net,
		     enum tftp_operation operation,
		     ulong addr, ulong size, char *fname, ip_addr_t srvip,
		     u16 srvport, u16 srcport, u32 timeout_msecs,
		     u32 max_retries)
{
	const struct tftp_request_timeout *request = tftp_get_request_timeout();
	struct tftp_ctx ctx;
	bool initialized = false;
	const char *window_env = NULL;
	ulong windowsize = CONFIG_TFTP_WINDOWSIZE;
	u16 blksize;
	err_t err;

	if (!fname || !*fname)
		return -EINVAL;
	if (operation == TFTP_UPLOAD &&
	    (size > U32_MAX || addr + size < addr))
		return -E2BIG;
	if (tftp_get_blocksize(operation, &blksize))
		return -1;
	if (IS_ENABLED(CONFIG_NET_TFTP_VARS))
		window_env = env_get("tftpwindowsize");
	if (operation == TFTP_DOWNLOAD &&
	    ((window_env && strict_strtoul(window_env, 10, &windowsize)) ||
	     !windowsize || windowsize > U16_MAX)) {
		log_err("error: TFTP window size must be between 1 and 65535\n");
		return -EINVAL;
	}

	if (!srvport)
		srvport = TFTP_PORT;

	memset(&ctx, 0, sizeof(ctx));
	ctx.done = NOT_DONE;
	ctx.daddr = addr;
	ctx.total_size = size;
	ctx.upload = operation == TFTP_UPLOAD;
	net_boot_file_size = 0;

	printf("Using %s device\n", net->dev->name);
	printf("TFTP %s server %s; our IP address is %pI4\n",
	       ctx.upload ? "to" : "from", ipaddr_ntoa(&srvip),
	       &netif_ip4_addr(net->netif)->addr);
	printf("Filename '%s'.\n", fname);
	if (ctx.upload) {
		printf("Save address: 0x%lx\n", ctx.daddr);
		printf("Save size:    0x%lx\n", ctx.total_size);
		printf("Saving: ");
	} else {
		printf("Load address: 0x%lx\n", ctx.daddr);
		printf("Loading: ");
	}

	if (operation == TFTP_DOWNLOAD && request) {
		int ret = tftp_probe_arp(net->netif, ip_2_ip4(&srvip));

		if (ret)
			return ret;
	}

	err = tftp_init_client(&tftp_context);
	if (err != ERR_OK) {
		log_err("tftp_init_client err: %d\n", err);
		return -1;
	}
	initialized = true;

	if (srcport) {
		err = tftp_client_bind(srcport);
		if (err != ERR_OK) {
			log_err("tftp_client_bind err: %d\n", err);
			goto out_cleanup;
		}
	}

	err = tftp_set_timeout(timeout_msecs, max_retries);
	if (err != ERR_OK) {
		log_err("tftp_set_timeout err: %d\n", err);
		goto out_cleanup;
	}
	if (operation == TFTP_DOWNLOAD && request) {
		err = tftp_client_set_request_timeout(request->msecs, request->max_retries);
		if (err != ERR_OK)
			goto out_cleanup;
	}

	tftp_client_set_blksize(blksize);
	if (operation == TFTP_DOWNLOAD)
		tftp_client_set_windowsize(windowsize);

	ctx.start_time = get_timer(0);
	if (ctx.upload) {
		tftp_client_set_tsize(size);
		err = tftp_put(&ctx, &srvip, srvport, fname,
			       TFTP_MODE_OCTET);
	} else {
		err = tftp_get(&ctx, &srvip, srvport, fname,
			       TFTP_MODE_OCTET);
	}
	/* might return different errors, like routing problems */
	if (err != ERR_OK) {
		printf("tftp_%s() error %d\n", ctx.upload ? "put" : "get",
		       err);
		goto out_cleanup;
	}

	while (!ctx.done) {
		net_lwip_poll();
		if (ctrlc()) {
			printf("\nAbort\n");
			ctx.done = ABORTED;
			break;
		}
	}

out_cleanup:
	if (initialized)
		tftp_cleanup();

	return tftp_complete(&ctx, addr, fname);
}

static void no_request(void *arg)
{
	struct tftp_ctx *ctx = (struct tftp_ctx *)arg;

	if (ctx->wrq_accepted)
		return;

	if (ctx->listen_retries < UBOOT_TFTP_MAX_RETRIES) {
		ctx->listen_retries++;
		sys_timeout(UBOOT_TFTP_TIMEOUT_MS, no_request, ctx);
		return;
	}

	printf("Timeout!\n");
	ctx->done = FAILURE;
	ctx->retry = true;
}

static int tftpsrv_loop(struct net_lwip_ctx *net, ulong addr)
{
	struct tftp_ctx ctx;
	int ret = -1;
	err_t err;

	if (tftp_check_ip(net))
		return -1;

	memset(&ctx, 0, sizeof(ctx));
	ctx.done = NOT_DONE;
	ctx.daddr = addr;
	ctx.is_server = true;
	net_boot_file_size = 0;

	printf("Using %s device\n", net->dev->name);
	printf("Listening for TFTP transfer on %pI4\n",
	       &netif_ip4_addr(net->netif)->addr);
	printf("Load address: 0x%lx\n", ctx.daddr);

	tftpsrv_active_ctx = &ctx;
	err = tftp_init_server(&tftp_context);
	if (err != ERR_OK) {
		log_err("tftp_init_server err: %d\n", err);
		goto out;
	}

	/* Legacy tftpsrv uses compiled defaults, not client env overrides. */
	err = tftp_set_timeout(UBOOT_TFTP_TIMEOUT_MS, UBOOT_TFTP_MAX_RETRIES);
	if (err != ERR_OK)
		goto out_cleanup;

	ctx.start_time = get_timer(0);
	sys_timeout(UBOOT_TFTP_TIMEOUT_MS, no_request, &ctx);
	while (!ctx.done) {
		net_lwip_poll();
		if (ctrlc()) {
			printf("\nAbort\n");
			ctx.done = ABORTED;
			break;
		}
	}
out_cleanup:
	sys_untimeout(no_request, &ctx);

	tftp_cleanup();

	ret = tftp_complete(&ctx, addr, ctx.fname);

out:
	tftpsrv_active_ctx = NULL;

	return ret;
}

int do_tftpsrv(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct net_lwip_ctx net = {};
	int ret = CMD_RET_SUCCESS;
	char *end;
	ulong laddr;
	ulong addr;

	net_boot_file_size = 0;
	if (!IS_ENABLED(CONFIG_CMD_TFTPSRV))
		return CMD_RET_FAILURE;

	laddr = env_get_ulong("loadaddr", 16, image_load_addr);

	switch (argc) {
	case 1:
		break;
	case 2:
		addr = hextoul(argv[1], &end);
		if (end == argv[1] || *end) {
			ret = CMD_RET_USAGE;
			goto out;
		}
		laddr = addr;
		break;
	default:
		ret = CMD_RET_USAGE;
		goto out;
	}

	if (net_lwip_start(&net, NET_LWIP_ADDR_ENV_STRICT)) {
		ret = CMD_RET_FAILURE;
		goto out;
	}

	net_try_count = 1;
	net_restart_wrap = 0;
	for (;;) {
		ret = tftpsrv_loop(&net, laddr);
		if (ret != -EAGAIN)
			break;
		puts("Starting again\n\n");
		ret = net_lwip_restart(&net);
		if (ret)
			break;
	}
	if (!ret)
		image_load_addr = laddr;
	ret = ret ? CMD_RET_FAILURE : CMD_RET_SUCCESS;

out:
	net_lwip_stop(&net);
	if (ret || !net_boot_file_size)
		return ret;
	return bootm_maybe_autostart(cmdtp, argv[0]);
}

static void tftp_default_filename(struct net_lwip_ctx *net, char *fname,
				  size_t size)
{
	const ip4_addr_t *addr = netif_ip4_addr(net->netif);

	snprintf(fname, size, "%02X%02X%02X%02X.img",
		 ip4_addr1(addr), ip4_addr2(addr), ip4_addr3(addr), ip4_addr4(addr));
	printf("*** Warning: no boot file name; using '%s'\n", fname);
}

static int do_tftp(int argc, char *const argv[], enum tftp_operation operation)
{
	struct net_lwip_ctx local = {};
	struct net_lwip_ctx *net = &local;
	u32 timeout_msecs = UBOOT_TFTP_TIMEOUT_MS;
	u32 max_retries = UBOOT_TFTP_MAX_RETRIES;
	int ret = CMD_RET_SUCCESS;
	char *arg = NULL;
	char *arg_copy = NULL;
	char *fname = NULL;
	char *server_ip = NULL;
	char *server_port = NULL;
	char default_fname[sizeof("FFFFFFFF.img")];
	char *end;
	ip_addr_t srvip;
	u16 srcport = 0;
	u16 port = TFTP_PORT;
	ulong laddr;
	ulong size = 0;
	ulong addr;
	const char *ep;
	ulong value;

	net_boot_file_size = 0;
	laddr = env_get_ulong("loadaddr", 16, image_load_addr);

	if (operation == TFTP_UPLOAD) {
		if (argc < 3 || argc > 4 ||
		    strict_strtoul(argv[1], 16, &laddr) ||
		    strict_strtoul(argv[2], 16, &size)) {
			ret = CMD_RET_USAGE;
			goto out;
		}
		if (argc == 4)
			arg = argv[3];
	} else {
		switch (argc) {
		case 1:
			arg = env_get("bootfile");
			break;
		case 2:
			/* Accept either a load address or a file name. */
			addr = hextoul(argv[1], &end);
			if (end == argv[1] + strlen(argv[1])) {
				laddr = addr;
				arg = env_get("bootfile");
			} else {
				arg = argv[1];
			}
			break;
		case 3:
			if (strict_strtoul(argv[1], 16, &laddr)) {
				ret = CMD_RET_USAGE;
				goto out;
			}
			arg = argv[2];
			break;
		default:
			ret = CMD_RET_USAGE;
			goto out;
		}
	}

	if (arg && *arg) {
		arg_copy = strdup(arg);
		if (!arg_copy) {
			ret = CMD_RET_FAILURE;
			goto out;
		}
		fname = arg_copy;
		end = strchr(fname, ':');
		if (end) {
			*end = '\0';
			/* Like legacy, only an IP address introduces a server.
			 * Retain the lwIP empty-server and numeric-port extensions,
			 * but leave other colons in the filename untouched.
			 */
			if (!*fname || ipaddr_aton(fname, &srvip)) {
				server_ip = fname;
				fname = end + 1;
				end = strchr(fname, ':');
				if (end) {
					*end = '\0';
					if (strspn(fname, "0123456789") == strlen(fname)) {
						server_port = fname;
						fname = end + 1;
					} else {
						*end = ':';
					}
				}
			} else {
				*end = ':';
			}
		}
	}

	if (!server_ip || !*server_ip)
		server_ip = env_get("tftpserverip");
	if (!server_ip || !*server_ip)
		server_ip = env_get("serverip");
	if (!server_ip || !*server_ip) {
		log_err("error: tftpserverip/serverip has to be set\n");
		ret = CMD_RET_FAILURE;
		goto out;
	}

	if (!server_port || !*server_port)
		server_port = env_get("tftpdstp");
	if (server_port && *server_port) {
		if (strict_strtoul(server_port, 10, &value) || value > U16_MAX) {
			log_err("error: invalid TFTP destination port\n");
			ret = CMD_RET_FAILURE;
			goto out;
		}
		port = value;
	}

	ep = env_get("tftpsrcp");
	if (ep && *ep) {
		if (strict_strtoul(ep, 10, &value) || value > U16_MAX) {
			log_err("error: invalid TFTP source port\n");
			ret = CMD_RET_FAILURE;
			goto out;
		}
		srcport = value;
	}

	if (IS_ENABLED(CONFIG_NET_TFTP_VARS)) {
		ep = env_get("tftptimeout");
		if (ep && *ep) {
			if (strict_strtoul(ep, 10, &value) || value > U32_MAX / 4) {
				log_err("error: invalid TFTP timeout\n");
				ret = CMD_RET_FAILURE;
				goto out;
			}
			timeout_msecs = value;
		}
		if (timeout_msecs < 1000) {
			printf("TFTP timeout (%u ms) too low, set min = 1000 ms\n",
			       timeout_msecs);
			timeout_msecs = 1000;
		}

		ep = env_get("tftptimeoutcountmax");
		if (ep && *ep) {
			if (strict_strtoul(ep, 10, &value) || value > U32_MAX) {
				log_err("error: invalid TFTP timeout count max\n");
				ret = CMD_RET_FAILURE;
				goto out;
			}
			max_retries = value;
		}
	}

	if (!ipaddr_aton(server_ip, &srvip)) {
		log_err("error: ipaddr_aton\n");
		ret = CMD_RET_FAILURE;
		goto out;
	}

	if (fname && strlen(fname) > TFTP_MAX_FILENAME_LEN) {
		log_err("error: TFTP file name is too long\n");
		ret = CMD_RET_FAILURE;
		goto out;
	}

	if (operation == TFTP_UPLOAD &&
	    (size > U32_MAX || laddr + size < laddr)) {
		log_err("error: TFTP upload range is too large\n");
		ret = CMD_RET_FAILURE;
		goto out;
	}

	if (net_lwip_start(net, NET_LWIP_ADDR_ENV_STRICT)) {
		ret = CMD_RET_FAILURE;
		goto out;
	}
	net_try_count = 1;
	net_restart_wrap = 0;

	for (;;) {
		ret = tftp_check_ip(net);
		if (ret)
			break;
		/* Rotation may select an interface with a different IP address. */
		if (!fname || !*fname)
			tftp_default_filename(net, default_fname, sizeof(default_fname));
		ret = tftp_loop(net, operation, laddr, size,
				fname && *fname ? fname : default_fname, srvip,
				port, srcport, timeout_msecs, max_retries);
		if (ret != -EAGAIN)
			break;
		/* The failed transfer's PCB and timers are already gone. Never
		 * change interfaces underneath another shared-runtime client.
		 */
		puts("Starting again\n\n");
		ret = net_lwip_restart(net);
		if (ret)
			break;
	}
	if (!ret && operation == TFTP_DOWNLOAD)
		image_load_addr = laddr;
	ret = ret ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
out:
	net_lwip_stop(net);
	free(arg_copy);
	return ret;
}

int do_tftpb(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;

	ret = do_tftp(argc, argv, TFTP_DOWNLOAD);
	if (ret || !net_boot_file_size)
		return ret;
	/* The transfer's callbacks and runtime attachment are already gone. */
	return bootm_maybe_autostart(cmdtp, argv[0]);
}

int do_tftpput(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	return do_tftp(argc, argv, TFTP_UPLOAD);
}
