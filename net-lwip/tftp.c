// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <console.h>
#include <image.h>
#include <linux/delay.h>
#include <lwip/apps/tftp_client.h>
#include <lwip/timeouts.h>
#include <net-lwip.h>
#include <time.h>

static ulong daddr;
static ulong size;
static ulong prevsize;
#define PROGRESS_PRINT_STEP_BYTES (100 * 1024)
static ulong start_time;
static enum done_state {
	NOT_DONE = 0,
	SUCCESS = 1,
	FAILURE = 2
} done;

static void *tftp_open(const char *fname, const char *mode, u8_t is_write)
{
	return NULL;
}

static void tftp_close(void *handle)
{
	ulong elapsed;

	if (done == FAILURE) {
		/* Closing after an error */
		return;
	}

	elapsed = get_timer(start_time);
	done = SUCCESS;
	printf("\nBytes transferred = %lu (%lx hex)\n", size, size);

	if (env_set_hex("filesize", size)) {
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
	struct pbuf *q;

	for (q = p; q != NULL; q = q->next) {
		memcpy((void *)daddr, q->payload, q->len);
		daddr += q->len;
		size += q->len;
		if (size - prevsize > PROGRESS_PRINT_STEP_BYTES) {
			printf("#");
			prevsize = size;
		}
	}

	return 0;
}

static void tftp_error(void *handle, int err, const char *msg, int size)
{
	char message[100];

	done = FAILURE;
	memset(message, 0, sizeof(message));
	memcpy(message, msg, LWIP_MIN(sizeof(message) - 1, (size_t)size));

	log_info("\nTFTP error: %d (%s)\n", err, message);
}

static const struct tftp_context tftp_context = {
	tftp_open,
	tftp_close,
	tftp_read,
	tftp_write,
	tftp_error
};

static int tftp_run(ulong addr, char *fname, ip_addr_t srvip)
{
	void *f = (void *)0x1; /* unused fake file handle*/
	err_t err;

	if (!fname || addr == 0)
		return -1;

	done = NOT_DONE;
	size = 0;
	prevsize = 0;
	daddr = addr;

	log_info("TFTP from server %s; our IP address is %s\n",
		 ip4addr_ntoa(&srvip), env_get("ipaddr"));
	log_info("Filename '%s'.\n", fname);
	log_info("Load address: 0x%lx\n", daddr);
	log_info("Loading: ");

	err = tftp_init_client(&tftp_context);
	if (!(err == ERR_OK || err == ERR_USE))
		log_err("tftp_init_client err: %d\n", err);

	start_time = get_timer(0);
	err = tftp_get(f, &srvip, TFTP_PORT, fname, TFTP_MODE_OCTET);
	/* might return different errors, like routing problems */
	if (err != ERR_OK) {
		log_err("tftp_get err=%d\n", err);
		return -1;
	}

	while (!done) {
		eth_rx();
		sys_check_timeouts();
		if (ctrlc())
			break;
	}

	tftp_cleanup();

	if (done == SUCCESS) {
		if (env_set_hex("fileaddr", addr)) {
			log_err("fileaddr not updated\n");
			return -1;
		}
		return 0;
	}

	return -1;
}

int do_tftpb(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char *fname;
	char *server_ip;
	ip_addr_t srvip;
	ulong addr;
	char *end;
	char *col;

	image_load_addr = env_get_ulong("loadaddr", 16, image_load_addr);

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
			image_load_addr = addr;
			fname = env_get("bootfile");
		} else {
			fname = argv[1];
		}
		break;
	case 3:
		image_load_addr = hextoul(argv[1], NULL);
		fname = argv[2];
		break;
	default:
		return CMD_RET_USAGE;
	}

	col = strchr(fname, ':');
	if (col) {
		server_ip = fname;
		*col = '\0';
		fname = col + 1;
	} else {
		server_ip = env_get("serverip");
		if (!server_ip) {
			log_err("error: serverip variable has to be set\n");
			return CMD_RET_FAILURE;
		}
	}
	if (!ipaddr_aton(server_ip, &srvip)) {
		log_err("error: ipaddr_aton\n");
		return CMD_RET_FAILURE;
	}

	if (!fname) {
		log_err("error: no file name\n");
		return CMD_RET_FAILURE;
	}

	if (!image_load_addr) {
		log_err("error: no load address\n");
		return CMD_RET_FAILURE;
	}

	if (tftp_run(image_load_addr, fname, srvip) < 0)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}
