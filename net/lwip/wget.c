// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <console.h>
#include <display_options.h>
#include <efi_loader.h>
#include <env.h>
#include <linux/kconfig.h>
#include <lwip/apps/http_client.h>
#include "lwip/altcp_tls.h"
#include <lwip/errno.h>
#include <lwip/timeouts.h>
#include <rng.h>
#include <mapmem.h>
#include <net.h>
#include <time.h>
#include <dm/uclass.h>

#define SERVER_NAME_SIZE 254
#define HTTP_PORT_DEFAULT 80
#define HTTPS_PORT_DEFAULT 443
#define PROGRESS_PRINT_STEP_BYTES (100 * 1024)

enum done_state {
	NOT_DONE = 0,
	SUCCESS = 1,
	FAILURE = 2
};

struct wget_ctx {
	char server_name[SERVER_NAME_SIZE];
	u16 port;
	char *path;
	ulong daddr;
	ulong saved_daddr;
	ulong size;
	ulong prevsize;
	ulong start_time;
	enum done_state done;
};

static void wget_lwip_fill_info(struct pbuf *hdr, u16_t hdr_len, u32_t hdr_cont_len)
{
	if (wget_info->headers) {
		if (hdr_len < MAX_HTTP_HEADERS_SIZE)
			pbuf_copy_partial(hdr, (void *)wget_info->headers, hdr_len, 0);
		else
			hdr_len = 0;
		wget_info->headers[hdr_len] = 0;
	}
	wget_info->hdr_cont_len = (u32)hdr_cont_len;
}

static void wget_lwip_set_file_size(u32_t rx_content_len)
{
	wget_info->file_size = (ulong)rx_content_len;
}

bool wget_validate_uri(char *uri);

int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len,
			  size_t *olen)
{
	struct udevice *dev;
	int ret;

	*olen = 0;

	ret = uclass_get_device(UCLASS_RNG, 0, &dev);
	if (ret) {
		log_err("Failed to get an rng: %d\n", ret);
		return ret;
	}
	ret = dm_rng_read(dev, output, len);
	if (ret)
		return ret;

	*olen = len;

	return 0;
}

static int parse_url(char *url, char *host, u16 *port, char **path,
		     bool *is_https)
{
	char *p, *pp;
	long lport;
	size_t prefix_len = 0;

	if (!wget_validate_uri(url)) {
		log_err("Invalid URL. Use http(s)://\n");
		return -EINVAL;
	}

	*is_https = false;
	*port = HTTP_PORT_DEFAULT;
	prefix_len = strlen("http://");
	p = strstr(url, "http://");
	if (!p) {
		p = strstr(url, "https://");
		prefix_len = strlen("https://");
		*port = HTTPS_PORT_DEFAULT;
		*is_https = true;
	}

	p += prefix_len;

	/* Parse hostname */
	pp = strchr(p, ':');
	if (!pp)
		pp = strchr(p, '/');
	if (!pp)
		return -EINVAL;

	if (p + SERVER_NAME_SIZE <= pp)
		return -EINVAL;

	memcpy(host, p, pp - p);
	host[pp - p] = '\0';

	if (*pp == ':') {
		/* Parse port number */
		p = pp + 1;
		lport = simple_strtol(p, &pp, 10);
		if (pp && *pp != '/')
			return -EINVAL;
		if (lport > 65535)
			return -EINVAL;
		*port = (u16)lport;
	}

	if (*pp != '/')
		return -EINVAL;
	*path = pp;

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
static int store_block(struct wget_ctx *ctx, void *src, u16_t len)
{
	ulong store_addr = ctx->daddr;
	uchar *ptr;

	/* Avoid overflow */
	if (wget_info->buffer_size && wget_info->buffer_size < ctx->size + len)
		return -1;

	if (CONFIG_IS_ENABLED(LMB) && wget_info->set_bootdev) {
		if (store_addr + len < store_addr ||
		    lmb_read_check(store_addr, len)) {
			if (!wget_info->silent) {
				printf("\nwget error: ");
				printf("trying to overwrite reserved memory\n");
			}
			return -1;
		}
	}

	ptr = map_sysmem(store_addr, len);
	memcpy(ptr, src, len);
	unmap_sysmem(ptr);

	ctx->daddr += len;
	ctx->size += len;
	if (ctx->size - ctx->prevsize > PROGRESS_PRINT_STEP_BYTES) {
		if (!wget_info->silent)
			printf("#");
		ctx->prevsize = ctx->size;
	}

	return 0;
}

static err_t httpc_recv_cb(void *arg, struct altcp_pcb *pcb, struct pbuf *pbuf,
			   err_t err)
{
	struct wget_ctx *ctx = arg;
	struct pbuf *buf;
	err_t ret;

	if (!pbuf)
		return ERR_BUF;

	if (!ctx->start_time)
		ctx->start_time = get_timer(0);

	for (buf = pbuf; buf; buf = buf->next) {
		if (store_block(ctx, buf->payload, buf->len) < 0) {
			altcp_abort(pcb);
			ret = ERR_BUF;
			goto out;
		}
	}
	altcp_recved(pcb, pbuf->tot_len);
	ret = ERR_OK;
out:
	pbuf_free(pbuf);
	return ret;
}

static void httpc_result_cb(void *arg, httpc_result_t httpc_result,
			    u32_t rx_content_len, u32_t srv_res, err_t err)
{
	struct wget_ctx *ctx = arg;
	ulong elapsed;

	wget_info->status_code = (u32)srv_res;

	if (err == ERR_BUF) {
		ctx->done = FAILURE;
		return;
	}

	if (httpc_result != HTTPC_RESULT_OK) {
		log_err("\nHTTP client error %d\n", httpc_result);
		ctx->done = FAILURE;
		return;
	}
	if (srv_res != 200) {
		log_err("\nHTTP server error %d\n", srv_res);
		ctx->done = FAILURE;
		return;
	}

	elapsed = get_timer(ctx->start_time);
	if (!elapsed)
		elapsed = 1;
	if (!wget_info->silent) {
		if (rx_content_len > PROGRESS_PRINT_STEP_BYTES)
			printf("\n");
		printf("%u bytes transferred in %lu ms (", rx_content_len,
		       elapsed);
		print_size(rx_content_len / elapsed * 1000, "/s)\n");
		printf("Bytes transferred = %lu (%lx hex)\n", ctx->size,
		       ctx->size);
	}
	if (wget_info->set_bootdev)
		efi_set_bootdev("Http", ctx->server_name, ctx->path, map_sysmem(ctx->saved_daddr, 0),
				rx_content_len);
	wget_lwip_set_file_size(rx_content_len);
	if (env_set_hex("filesize", rx_content_len) ||
	    env_set_hex("fileaddr", ctx->saved_daddr)) {
		log_err("Could not set filesize or fileaddr\n");
		ctx->done = FAILURE;
		return;
	}

	ctx->done = SUCCESS;
}

static err_t httpc_headers_done_cb(httpc_state_t *connection, void *arg, struct pbuf *hdr,
				   u16_t hdr_len, u32_t content_len)
{
	wget_lwip_fill_info(hdr, hdr_len, content_len);

	if (wget_info->check_buffer_size && (ulong)content_len > wget_info->buffer_size)
		return ERR_BUF;

	return ERR_OK;
}


#if CONFIG_IS_ENABLED(WGET_CACERT)
#endif

int wget_do_request(ulong dst_addr, char *uri)
{
#if CONFIG_IS_ENABLED(WGET_HTTPS)
	altcp_allocator_t tls_allocator;
#endif
	httpc_connection_t conn;
	httpc_state_t *state;
	struct udevice *udev;
	struct netif *netif;
	struct wget_ctx ctx;
	char *path;
	bool is_https;

	ctx.daddr = dst_addr;
	ctx.saved_daddr = dst_addr;
	ctx.done = NOT_DONE;
	ctx.size = 0;
	ctx.prevsize = 0;
	ctx.start_time = 0;

	if (parse_url(uri, ctx.server_name, &ctx.port, &path, &is_https))
		return CMD_RET_USAGE;

	if (net_lwip_eth_start() < 0)
		return CMD_RET_FAILURE;

	if (!wget_info)
		wget_info = &default_wget_info;

	udev = eth_get_dev();

	netif = net_lwip_new_netif(udev);
	if (!netif)
		return -1;

	/* if URL with hostname init dns */
	if (!ipaddr_aton(ctx.server_name, NULL) && net_lwip_dns_init())
		return CMD_RET_FAILURE;

	memset(&conn, 0, sizeof(conn));
#if CONFIG_IS_ENABLED(WGET_HTTPS)
	if (is_https) {
		char *ca;
		size_t ca_sz;

#if CONFIG_IS_ENABLED(WGET_CACERT) || CONFIG_IS_ENABLED(WGET_BUILTIN_CACERT)
#if CONFIG_IS_ENABLED(WGET_BUILTIN_CACERT)
		if (!cacert_initialized)
			set_cacert_builtin();
#endif
		ca = cacert;
		ca_sz = cacert_size;

		if (cacert_auth_mode == AUTH_REQUIRED) {
			if (!ca || !ca_sz) {
				if (!wget_info->silent)
					printf("Error: cacert authentication "
					       "mode is 'required' but no CA "
					       "certificates given\n");
				return CMD_RET_FAILURE;
		       }
		} else if (cacert_auth_mode == AUTH_NONE) {
			ca = NULL;
			ca_sz = 0;
		} else if (cacert_auth_mode == AUTH_OPTIONAL) {
			/*
			 * Nothing to do, this is the default behavior of
			 * altcp_tls to check server certificates against CA
			 * certificates when the latter are provided and proceed
			 * with no verification if not.
			 */
		}
#endif
		if (!ca && !wget_info->silent) {
			printf("WARNING: no CA certificates, ");
			printf("HTTPS connections not authenticated\n");
		}
		tls_allocator.alloc = &altcp_tls_alloc;
		tls_allocator.arg =
			altcp_tls_create_config_client(ca, ca_sz,
						       ctx.server_name);

		if (!tls_allocator.arg) {
			log_err("error: Cannot create a TLS connection\n");
			net_lwip_remove_netif(netif);
			return -1;
		}

		conn.altcp_allocator = &tls_allocator;
	}
#endif

	conn.result_fn = httpc_result_cb;
	conn.headers_done_fn = httpc_headers_done_cb;
	ctx.path = path;
	if (httpc_get_file_dns(ctx.server_name, ctx.port, path, &conn, httpc_recv_cb,
			       &ctx, &state)) {
		net_lwip_remove_netif(netif);
		return CMD_RET_FAILURE;
	}

	errno = 0;

	while (!ctx.done) {
		net_lwip_rx(udev, netif);
		if (ctrlc())
			break;
	}

	net_lwip_remove_netif(netif);

	if (ctx.done == SUCCESS)
		return 0;

	if (errno == EPERM && !wget_info->silent)
		printf("Certificate verification failed\n");

	return -1;
}

/**
 * wget_validate_uri() - validate the uri for wget
 *
 * @uri:	uri string
 *
 * This function follows the current U-Boot wget implementation.
 * scheme: only "http:" is supported
 * authority:
 *   - user information: not supported
 *   - host: supported
 *   - port: not supported(always use the default port)
 *
 * Uri is expected to be correctly percent encoded.
 * This is the minimum check, control codes(0x1-0x19, 0x7F, except '\0')
 * and space character(0x20) are not allowed.
 *
 * TODO: stricter uri conformance check
 *
 * Return:	true on success, false on failure
 */
bool wget_validate_uri(char *uri)
{
	char c;
	bool ret = true;
	char *str_copy, *s, *authority;
	size_t prefix_len = 0;

	for (c = 0x1; c < 0x21; c++) {
		if (strchr(uri, c)) {
			log_err("invalid character is used\n");
			return false;
		}
	}

	if (strchr(uri, 0x7f)) {
		log_err("invalid character is used\n");
		return false;
	}

	if (!strncmp(uri, "http://", strlen("http://"))) {
		prefix_len = strlen("http://");
	} else if (CONFIG_IS_ENABLED(WGET_HTTPS)) {
		if (!strncmp(uri, "https://", strlen("https://"))) {
			prefix_len = strlen("https://");
		} else {
			log_err("only http(s):// is supported\n");
			return false;
		}
	} else {
		log_err("only http:// is supported\n");
		return false;
	}

	str_copy = strdup(uri);
	if (!str_copy)
		return false;

	s = str_copy + strlen("http://");
	authority = strsep(&s, "/");
	if (!s) {
		log_err("invalid uri, no file path\n");
		ret = false;
		goto out;
	}
	s = strchr(authority, '@');
	if (s) {
		log_err("user information is not supported\n");
		ret = false;
		goto out;
	}

out:
	free(str_copy);

	return ret;
}
