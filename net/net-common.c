// SPDX-License-Identifier: GPL-2.0
#include <net-common.h>

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
	return wget_with_dns(dst_addr, uri);
}
