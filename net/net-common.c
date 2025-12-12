// SPDX-License-Identifier: GPL-2.0

#include <dm/uclass.h>
#include <env.h>
#include <net-common.h>
#include <linux/time.h>
#include <rtc.h>

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

#if defined(CONFIG_CMD_DHCP)
int dhcp_run(ulong addr, const char *fname, bool autoload)
{
	char *dhcp_argv[] = {"dhcp", NULL, (char *)fname, NULL};
	struct cmd_tbl cmdtp = {};	/* dummy */
	char file_addr[17];
	int old_autoload;
	int ret, result;

	log_debug("addr=%lx, fname=%s, autoload=%d\n", addr, fname, autoload);
	old_autoload = env_get_yesno("autoload");
	ret = env_set("autoload", autoload ? "y" : "n");
	if (ret)
		return log_msg_ret("en1", -EINVAL);

	if (autoload) {
		sprintf(file_addr, "%lx", addr);
		dhcp_argv[1] = file_addr;
	}

	result = do_dhcp(&cmdtp, 0, !autoload ? 1 : fname ? 3 : 2, dhcp_argv);

	ret = env_set("autoload", old_autoload == -1 ? NULL :
		      old_autoload ? "y" : "n");
	if (ret)
		return log_msg_ret("en2", -EINVAL);

	if (result)
		return log_msg_ret("res", -ENOENT);

	return 0;
}
#endif

#if defined(CONFIG_CMD_TFTPBOOT)
int tftpb_run(ulong addr, const char *fname)
{
	char *tftp_argv[] = {"tftpboot", NULL, (char *)fname, NULL};
	struct cmd_tbl cmdtp = {};      /* dummy */
	char file_addr[17] = {0};

	log_debug("addr=%lx, fname=%s\n", addr, fname);
	sprintf(file_addr, "%lx", addr);
	tftp_argv[1] = file_addr;

	int result = do_tftpb(&cmdtp, 0, fname ? 3 : 2, tftp_argv);

	if (result)
		return log_msg_ret("res", -ENOENT);

	return 0;
}

#endif
