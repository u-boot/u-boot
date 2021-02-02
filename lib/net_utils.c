// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic network code. Moved from net.c
 *
 * Copyright 1994 - 2000 Neil Russell.
 * Copyright 2000 Roland Borde
 * Copyright 2000 Paolo Scaffardi
 * Copyright 2000-2002 Wolfgang Denk, wd@denx.de
 * Copyright 2009 Dirk Behme, dirk.behme@googlemail.com
 */

#include <common.h>
#include <net.h>

struct in_addr string_to_ip(const char *s)
{
	struct in_addr addr;
	char *e;
	int i;

	addr.s_addr = 0;
	if (s == NULL)
		return addr;

	for (addr.s_addr = 0, i = 0; i < 4; ++i) {
		ulong val = s ? simple_strtoul(s, &e, 10) : 0;
		if (val > 255) {
			addr.s_addr = 0;
			return addr;
		}
		if (i != 3 && *e != '.') {
			addr.s_addr = 0;
			return addr;
		}
		addr.s_addr <<= 8;
		addr.s_addr |= (val & 0xFF);
		if (s) {
			s = (*e) ? e+1 : e;
		}
	}

	addr.s_addr = htonl(addr.s_addr);
	return addr;
}

void string_to_enetaddr(const char *addr, uint8_t *enetaddr)
{
	char *end;
	int i;

	if (!enetaddr)
		return;

	for (i = 0; i < 6; ++i) {
		enetaddr[i] = addr ? simple_strtoul(addr, &end, 16) : 0;
		if (addr)
			addr = (*end) ? end + 1 : end;
	}
}

uint compute_ip_checksum(const void *vptr, uint nbytes)
{
	int sum, oddbyte;
	const unsigned short *ptr = vptr;

	sum = 0;
	while (nbytes > 1) {
		sum += *ptr++;
		nbytes -= 2;
	}
	if (nbytes == 1) {
		oddbyte = 0;
		((u8 *)&oddbyte)[0] = *(u8 *)ptr;
		((u8 *)&oddbyte)[1] = 0;
		sum += oddbyte;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	sum = ~sum & 0xffff;

	return sum;
}

uint add_ip_checksums(uint offset, uint sum, uint new)
{
	ulong checksum;

	sum = ~sum & 0xffff;
	new = ~new & 0xffff;
	if (offset & 1) {
		/*
		 * byte-swap the sum if it came from an odd offset; since the
		 * computation is endian-independent this works.
		 */
		new = ((new >> 8) & 0xff) | ((new << 8) & 0xff00);
	}
	checksum = sum + new;
	if (checksum > 0xffff)
		checksum -= 0xffff;

	return (~checksum) & 0xffff;
}

int ip_checksum_ok(const void *addr, uint nbytes)
{
	return !(compute_ip_checksum(addr, nbytes) & 0xfffe);
}
