// SPDX-License-Identifier: GPL-2.0

/*
 * (C) Copyright 2023 Maxim Uvarov, maxim.uvarov@linaro.org
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <display_options.h>
#include <memalign.h>
#include <net.h>
#include <image.h>

#include "net/lwip.h"
#include "net/ulwip.h"

static int do_lwip_init(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	if (!ulwip_init())
		return CMD_RET_SUCCESS;
	return CMD_RET_FAILURE;
}

static int do_lwip_list(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	ulwip_list();

	return CMD_RET_SUCCESS;
}

static int do_lwip_down(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	ulwip_up_down(argv[1], 0);

	return CMD_RET_SUCCESS;
}

#if defined(CONFIG_CMD_PING)
int do_lwip_ping(struct cmd_tbl *cmdtp, int flag, int argc,
		 char *const argv[])
{
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	ret = ulwip_init();
	if (ret) {
		log_err("ulwip_init err %d\n", ret);
		return CMD_RET_FAILURE;
	}

	log_info("Using %s device\n", eth_get_name());
	log_info("pinging addr: %s\n", argv[1]);

	if (ulwip_ping(argv[1])) {
		printf("ping init fail\n");
		return CMD_RET_FAILURE;
	}

	return ulwip_loop();
}
#endif /* CONFIG_CMD_PING */

#if defined(CONFIG_CMD_WGET)
int do_lwip_wget(struct cmd_tbl *cmdtp, int flag, int argc,
		 char *const argv[])
{
	char *url;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	url = argv[1];

	ret = ulwip_init();
	if (ret) {
		log_err("ulwip_init err %d\n", ret);
		return CMD_RET_FAILURE;
	}

	ret = ulwip_wget(image_load_addr, url);
	if (ret) {
		log_err("lwip_wget err %d\n", ret);
		return CMD_RET_FAILURE;
	}

	return ulwip_loop();
}
#endif

#if defined(CONFIG_CMD_TFTPBOOT)
int do_lwip_tftp(struct cmd_tbl *cmdtp, int flag, int argc,
		 char *const argv[])
{
	char *filename;
	ulong addr;
	char *end;
	int ret;

	switch (argc) {
	case 1:
		filename = env_get("bootfile");
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
			filename = env_get("bootfile");
		} else {
			filename = argv[1];
		}
		break;
	case 3:
		image_load_addr = hextoul(argv[1], NULL);
		filename = argv[2];
		break;
	default:
		return CMD_RET_USAGE;
	}

	ret = ulwip_init();
	if (ret) {
		log_err("ulwip_init err %d\n", ret);
		return CMD_RET_FAILURE;
	}

	ret = ulwip_tftp(image_load_addr, filename);
	if (ret)
		return ret;

	return ulwip_loop();
}
#endif /* CONFIG_CMD_TFTPBOOT */

#if defined(CONFIG_CMD_DHCP) || defined(CONFIG_CMD_DNS)
static void ulwip_timeout_handler(void)
{
	eth_halt();
	net_set_state(NETLOOP_FAIL);	/* we did not get the reply */
	ulwip_loop_set(0);
}
#endif

#if defined(CONFIG_CMD_DHCP)
int do_lwip_dhcp(void)
{
	int ret;
	char *filename;

	ret = ulwip_init();
	if (ret) {
		log_err("ulwip_init err %d\n", ret);
		return CMD_RET_FAILURE;
	}

	ret = ulwip_dhcp();
	if (ret)
		return CMD_RET_FAILURE;

	net_set_timeout_handler(200000UL, ulwip_timeout_handler);

	ret = ulwip_loop();
	if (ret)
		return CMD_RET_FAILURE;

	if (IS_ENABLED(CONFIG_CMD_TFTPBOOT)) {
		if (!env_get_yesno("autoload"))
			return ret;

		filename = env_get("bootfile");
		if (!filename) {
			log_notice("no bootfile\n");
			return CMD_RET_SUCCESS;
		}

		ret = ulwip_init();
		if (ret) {
			log_err("ulwip_init err %d\n", ret);
			return CMD_RET_FAILURE;
		}

		net_set_timeout_handler(20000UL, ulwip_timeout_handler);
		ulwip_tftp(image_load_addr, filename);

		ret =  ulwip_loop();
	}

	return ret;
}

static int _do_lwip_dhcp(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	return do_lwip_dhcp();
}
#endif /* CONFIG_CMD_DHCP */

#if defined(CONFIG_CMD_DNS)
int do_lwip_dns(struct cmd_tbl *cmdtp, int flag, int argc,
		char *const argv[])
{
	int ret;
	char *name;
	char *varname;

	if (argc == 1)
		return CMD_RET_USAGE;

	name = argv[1];

	if (argc == 3)
		varname = argv[2];
	else
		varname = NULL;

	ret = ulwip_init();
	if (ret) {
		log_err("ulwip_init err %d\n", ret);
		return CMD_RET_FAILURE;
	}

	ret = ulwip_dns(name, varname);
	if (ret == 0)
		return CMD_RET_SUCCESS;
	if (ret != -EINPROGRESS)
		return CMD_RET_FAILURE;

	net_set_timeout_handler(1000UL, ulwip_timeout_handler);

	return ulwip_loop();
}
#endif /* CONFIG_CMD_DNS */

static struct cmd_tbl cmds[] = {
	U_BOOT_CMD_MKENT(init, 1, 0, do_lwip_init,
			 "initialize lwip stack", ""),
	U_BOOT_CMD_MKENT(list, 1, 0, do_lwip_list,
			 "list lwip ifaces", ""),
	U_BOOT_CMD_MKENT(down, 2, 0, do_lwip_down,
			 "down lwip ifaces", ""),
#if defined(CONFIG_CMD_PING)
	U_BOOT_CMD_MKENT(ping, 2, 0, do_lwip_ping,
			 "send ICMP ECHO_REQUEST to network host",
			 "pingAddress"),
#endif
#if defined(CONFIG_CMD_WGET)
	U_BOOT_CMD_MKENT(wget, 2, 0, do_lwip_wget, "", ""),
#endif
#if defined(CONFIG_CMD_TFTPBOOT)
	U_BOOT_CMD_MKENT(tftp, 3, 0, do_lwip_tftp,
			 "boot image via network using TFTP protocol\n",
			 "[loadAddress] [[hostIPaddr:]bootfilename]"),
#endif
#if defined(CONFIG_CMD_DHCP)
	U_BOOT_CMD_MKENT(dhcp, 1, 0, _do_lwip_dhcp,
			 "boot image via network using DHCP/TFTP protocol",
			 ""),
#endif
#if defined(CONFIG_CMD_DNS)
	U_BOOT_CMD_MKENT(dns, 3, 0, do_lwip_dns,
			 "lookup dns name [and store address at variable]",
			 ""),
#endif
};

static int do_ops(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	struct cmd_tbl *cp;

	cp = find_cmd_tbl(argv[1], cmds, ARRAY_SIZE(cmds));

	argc--;
	argv++;

	if (cp == NULL || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	lwip, 4, 1, do_ops,
	"LWIP sub system",
	"init - init LWIP\n"
	"ping addr - pingAddress\n"
	"wget http://IPadress/url/\n"
	"tftp [loadAddress] [[hostIPaddr:]bootfilename]\n"
	"dhcp - boot image via network using DHCP/TFTP protocol\n"
	);
