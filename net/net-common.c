// SPDX-License-Identifier: GPL-2.0

#include <env.h>
#include <limits.h>
#include <malloc.h>
#include <net-common.h>
#include <rtc.h>
#include <dm/uclass.h>
#include <linux/time.h>

/* Network loop state */
enum net_loop_state net_state;
/* Our IP addr (0 = unknown) */
struct in_addr	net_ip;
/* Boot File name */
char net_boot_file_name[1024];
/* The actual transferred size of the bootfile (in bytes) */
u32 net_boot_file_size;
/* Boot file size in blocks as reported by the DHCP server */
u32 net_boot_file_expected_size_in_blocks;
uchar *net_rx_packets[PKTBUFSRX];

void copy_filename(char *dst, const char *src, int size)
{
	if (src && *src && (*src == '"')) {
		++src;
		--size;
	}

	while ((--size > 0) && src && *src && (*src != '"'))
		*dst++ = *src++;
	*dst = '\0';
}

struct wget_http_info default_wget_info = {
	.method = WGET_HTTP_METHOD_GET,
	.set_bootdev = true,
};

struct wget_http_info *wget_info;

int wget_request(ulong dst_addr, char *uri, struct wget_http_info *info)
{
	wget_info = info ? info : &default_wget_info;
	return wget_do_request(dst_addr, uri);
}

void net_sntp_set_rtc(u32 seconds)
{
	struct rtc_time tm;
	struct udevice *dev;
	int ret;

	rtc_to_tm(seconds, &tm);

	ret = uclass_get_device(UCLASS_RTC, 0, &dev);
	if (ret)
		printf("SNTP: cannot find RTC: err=%d\n", ret);
	else
		dm_rtc_set(dev, &tm);

	printf("Date: %4d-%02d-%02d Time: %2d:%02d:%02d\n",
	       tm.tm_year, tm.tm_mon, tm.tm_mday,
	       tm.tm_hour, tm.tm_min, tm.tm_sec);
}

#if defined(CONFIG_CMD_DHCP) || defined(CONFIG_CMD_TFTPBOOT)
/* Internal downloads return data to their caller; only commands may boot it. */
static int netboot_run(int (*command)(struct cmd_tbl *, int, int, char *const []),
		       int argc, char *const argv[])
{
	struct cmd_tbl cmdtp = {};
	const char *value;
	char *saved;
	int ret, restore;

	/* An already-disabled setting may be protected against writes. */
	if (!env_get_autostart())
		return command(&cmdtp, 0, argc, argv);

	value = env_get("autostart");
	saved = strdup(value);
	if (!saved)
		return CMD_RET_FAILURE;
	if (env_set("autostart", "no")) {
		free(saved);
		return CMD_RET_FAILURE;
	}
	ret = command(&cmdtp, 0, argc, argv);
	restore = env_set("autostart", saved);
	free(saved);

	return ret ?: (restore ? CMD_RET_FAILURE : CMD_RET_SUCCESS);
}
#endif /* defined(CONFIG_CMD_DHCP) || defined(CONFIG_CMD_TFTPBOOT) */

#if defined(CONFIG_CMD_DHCP)
int dhcp_run(ulong addr, const char *fname, bool autoload)
{
	char *dhcp_argv[] = {"dhcp", NULL, (char *)fname, NULL};
	char file_addr[17];
	const char *autoload_env = env_get("autoload");
	char *saved_autoload = NULL;
	int ret, result;

	log_debug("addr=%lx, fname=%s, autoload=%d\n", addr, fname, autoload);
	if (autoload_env) {
		saved_autoload = strdup(autoload_env);
		if (!saved_autoload)
			return -ENOMEM;
	}
	ret = env_set("autoload", autoload ? "y" : "n");
	if (ret) {
		free(saved_autoload);
		return log_msg_ret("en1", -EINVAL);
	}

	if (autoload) {
		sprintf(file_addr, "%lx", addr);
		dhcp_argv[1] = file_addr;
	}

	result = netboot_run(do_dhcp, !autoload ? 1 : fname ? 3 : 2, dhcp_argv);

	ret = env_set("autoload", saved_autoload);
	free(saved_autoload);
	if (ret)
		return log_msg_ret("en2", -EINVAL);

	if (result)
		return log_msg_ret("res", -ENOENT);

	return 0;
}
#endif

#if defined(CONFIG_CMD_TFTPBOOT)
static const struct tftp_request_timeout *tftp_request_timeout;

const struct tftp_request_timeout *tftp_get_request_timeout(void)
{
	return tftp_request_timeout;
}

int tftpb_run_argv(int argc, char *const argv[])
{
	return netboot_run(do_tftpb, argc, argv);
}

int tftpb_run(ulong addr, const char *fname)
{
	char *tftp_argv[] = {"tftpboot", NULL, (char *)fname, NULL};
	char file_addr[17] = {0};

	log_debug("addr=%lx, fname=%s\n", addr, fname);
	sprintf(file_addr, "%lx", addr);
	tftp_argv[1] = file_addr;

	int result = tftpb_run_argv(fname ? 3 : 2, tftp_argv);

	if (result)
		return log_msg_ret("res", -ENOENT);

	return 0;
}

int tftpb_run_timeout(ulong addr, const char *fname, ulong timeout_ms,
		      int max_retries)
{
	struct tftp_request_timeout timeout = { timeout_ms, max_retries };
	const struct tftp_request_timeout *saved = tftp_request_timeout;
	int ret;

	if (!timeout_ms || timeout_ms > U32_MAX / 4 || max_retries < 0)
		return -EINVAL;

	tftp_request_timeout = &timeout;
	ret = tftpb_run(addr, fname);
	tftp_request_timeout = saved;

	return ret;
}

#endif
