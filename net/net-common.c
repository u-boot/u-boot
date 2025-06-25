// SPDX-License-Identifier: GPL-2.0

#include <dm/uclass.h>
#include <net-common.h>
#include <linux/time.h>
#include <rtc.h>

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
