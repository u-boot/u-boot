// SPDX-License-Identifier: GPL-2.0

/*
 * (C) Copyright 2023 Linaro Ltd. <maxim.uvarov@linaro.org>
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <bootstage.h>
#include <efi_loader.h>
#include <mapmem.h>

#include "tftp_client.h"
#include "tftp_server.h"
#include <tftp_example.h>

#include <string.h>

#include <net/ulwip.h>

static char *filename;
static ulong daddr;
static ulong size;
static unsigned int progress_print;
#define PROGRESS_PRINT 700 /* about one # per Megabyte */

static void *tftp_open(const char *fname, const char *mode, u8_t is_write)
{
	return NULL;
}

static void tftp_close(void *handle)
{
	log_info("\ndone\n");
	log_info("Bytes transferred = %ld (0x%lx hex)\n", size, size);

	bootstage_mark_name(BOOTSTAGE_KERNELREAD_STOP, "tftp_done");
	if (env_set_hex("filesize", size)) {
		log_err("filesize not updated\n");
		free(filename);
		ulwip_exit(-1);
		return;
	}

	if (IS_ENABLED(CONFIG_CMD_BOOTEFI))
		efi_set_bootdev("Net", "", filename,
				map_sysmem(daddr - size, 0),
				size);

	free(filename);
	ulwip_exit(0);
}

static int tftp_read(void *handle, void *buf, int bytes)
{
	return 0;
}

static int tftp_write(void *handle, struct pbuf *p)
{
	struct pbuf *q;

	for (q = p; q != NULL; q = q->next) {
		void *ptr = map_sysmem(daddr, q->len);
		memcpy(ptr, q->payload, q->len);
		unmap_sysmem(ptr);
		memcpy((void *)daddr, q->payload, q->len);
		daddr += q->len;
		size += q->len;
		if (!(progress_print++ % PROGRESS_PRINT))
			log_info("#");
	}

	return 0;
}

static void tftp_error(void *handle, int err, const char *msg, int size)
{
	char message[100];

	memset(message, 0, sizeof(message));
	memcpy(message, msg, LWIP_MIN(sizeof(message) - 1, (size_t)size));

	log_info("TFTP error: %d (%s)", err, message);
}

static const struct tftp_context tftp = {
	tftp_open,
	tftp_close,
	tftp_read,
	tftp_write,
	tftp_error
};

int ulwip_tftp(ulong addr, char *fname)
{
	void *f = (void *)0x1; /* unused fake file handle*/
	err_t err;
	ip_addr_t srv;
	int ret;
	char *server_ip;

	if (!fname || addr == 0)
		return CMD_RET_FAILURE;

	size = 0;
	daddr = addr;
	server_ip = env_get("serverip");
	if (!server_ip) {
		log_err("error: serverip variable has to be set\n");
		return CMD_RET_FAILURE;
	}

	ret = ipaddr_aton(server_ip, &srv);
	if (!ret) {
		log_err("error: ipaddr_aton\n");
		return CMD_RET_FAILURE;
	}

	log_info("TFTP from server %s; our IP address is %s\n",
		 server_ip, env_get("ipaddr"));
	log_info("Filename '%s'.\n", fname);
	log_info("Load address: 0x%lx\n", daddr);
	log_info("Loading:");

	bootstage_mark_name(BOOTSTAGE_KERNELREAD_START, "tftp_start");

	err = tftp_init_client(&tftp);
	if (!(err == ERR_OK || err == ERR_USE))
		log_err("tftp_init_client err: %d\n", err);

	err = tftp_get(f, &srv, TFTP_PORT, fname, TFTP_MODE_OCTET);
	/* might return different errors, like routing problems */
	if (err != ERR_OK) {
		log_err("tftp_get err=%d\n", err);
		return CMD_RET_FAILURE;
	}

	if (env_set_hex("fileaddr", addr)) {
		log_err("fileaddr not updated\n");
		return CMD_RET_FAILURE;
	}

	filename = strdup(fname);
	if (!filename)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}
