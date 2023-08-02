// SPDX-License-Identifier: GPL-2.0

/*
 * (C) Copyright 2023 Linaro Ltd. <maxim.uvarov@linaro.org>
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <vsprintf.h>

#include "http_client.h"
#include <net/ulwip.h>

static ulong daddr;
static httpc_connection_t settings;

#define SERVER_NAME_SIZE 200
#define HTTP_PORT_DEFAULT 80

static err_t httpc_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *pbuf,
			err_t unused_err)
{
	struct pbuf *buf;

	if (!pbuf)
		return ERR_BUF;

	for (buf = pbuf; buf != NULL; buf = buf->next) {
		memcpy((void *)daddr, buf->payload, buf->len);
		log_debug("downloaded chunk size %d, to addr 0x%lx\n",
			  buf->len, daddr);
		daddr += buf->len;
	}

	altcp_recved(pcb, pbuf->tot_len);
	pbuf_free(pbuf);
	return ERR_OK;
}

static void httpc_result(void *arg, httpc_result_t httpc_result, u32_t rx_content_len,
			 u32_t srv_res, err_t err)
{
	if (httpc_result == HTTPC_RESULT_OK) {
		log_info("\n%d bytes successfully downloaded.\n", rx_content_len);
		env_set_hex("filesize", rx_content_len);
		ulwip_exit(0);
	} else {
		log_err("\nhttp eroror: %d\n", httpc_result);
		ulwip_exit(-1);
	}
}

/* http://hostname/url */
static int parse_url(char *url, char *host, u16 *port)
{
	char *p, *pp;

	p = strstr(url, "http://");
	if (!p)
		return -ENOENT;

	p += strlen("http://");

	/* parse hostname */
	pp = strchr(p, '/');
	if (!pp)
		return -ENOENT;

	if (p + SERVER_NAME_SIZE <= pp)
		return -ENOENT;

	memcpy(host, p, pp - p);
	host[pp - p + 1] = '\0';
	*port = HTTP_PORT_DEFAULT;

	return 0;
}

int ulwip_wget(ulong addr, char *url)
{
	err_t err;
	u16 port;
	char server_name[SERVER_NAME_SIZE];
	httpc_state_t *connection;

	daddr = addr;

	err = parse_url(url, server_name, &port);
	if (err)
		return -ENOENT;

	log_info("downloading %s to addr 0x%lx\n", url, addr);
	memset(&settings, 0, sizeof(settings));
	settings.result_fn = httpc_result;
	err = httpc_get_file_dns(server_name, port, url, &settings,
				 httpc_recv, NULL,  &connection);
	if (err != ERR_OK)
		return -EPERM;

	if (env_set_hex("fileaddr", addr))
		return -EACCES;

	return 0;
}
