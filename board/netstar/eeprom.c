/*
 * (C) Copyright 2005
 * Ladislav Michl, 2N Telekomunikace, michl@2n.cz
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Some code shamelessly stolen back from Robin Getz.
 */

#include <common.h>
#include <exports.h>
#include <timestamp.h>
#include <net.h>
#include "../drivers/net/smc91111.h"

static struct eth_device dev = {
	.iobase = CONFIG_SMC91111_BASE
};

static u16 read_eeprom_reg(u16 reg)
{
	int timeout;

	SMC_SELECT_BANK(&dev, 2);
	SMC_outw(&dev, reg, PTR_REG);

	SMC_SELECT_BANK(&dev, 1);
	SMC_outw(&dev, SMC_inw(&dev, CTL_REG) | CTL_EEPROM_SELECT |
		CTL_RELOAD, CTL_REG);

	timeout = 100;

	while ((SMC_inw(&dev, CTL_REG) & CTL_RELOAD) && --timeout)
		udelay(100);
	if (timeout == 0) {
		printf("Timeout reading register %02x\n", reg);
		return 0;
	}

	return SMC_inw(&dev, GP_REG);
}

static int write_eeprom_reg(u16 value, u16 reg)
{
	int timeout;

	SMC_SELECT_BANK(&dev, 2);
	SMC_outw(&dev, reg, PTR_REG);

	SMC_SELECT_BANK(&dev, 1);

	SMC_outw(&dev, value, GP_REG);
	SMC_outw(&dev, SMC_inw(&dev, CTL_REG) | CTL_EEPROM_SELECT |
		CTL_STORE, CTL_REG);

	timeout = 100;

	while ((SMC_inw(&dev, CTL_REG) & CTL_STORE) && --timeout)
		udelay(100);
	if (timeout == 0) {
		printf("Timeout writing register %02x\n", reg);
		return 0;
	}

	return 1;
}

static int write_data(u16 *buf, int len)
{
	u16 reg = 0x23;

	while (len--)
		write_eeprom_reg(*buf++, reg++);

	return 0;
}

static int verify_macaddr(char *s)
{
	u16 reg;
	int i, err = 0;

	puts("HWaddr: ");
	for (i = 0; i < 3; i++) {
		reg = read_eeprom_reg(0x20 + i);
		printf("%02x:%02x%c", reg & 0xff, reg >> 8, i != 2 ? ':' : '\n');
		if (s)
			err |= reg != ((u16 *)s)[i];
	}

	return err ? 0 : 1;
}

static int set_mac(char *s)
{
	int i;
	char *e, eaddr[6];

	/* turn string into mac value */
	for (i = 0; i < 6; i++) {
		eaddr[i] = simple_strtoul(s, &e, 16);
		s = (*e) ? e+1 : e;
	}

	for (i = 0; i < 3; i++)
		write_eeprom_reg(*(((u16 *)eaddr) + i), 0x20 + i);

	return 0;
}

static int parse_element(char *s, unsigned char *buf, int len)
{
	int cnt;
	char *p, num[3];
	unsigned char id;

	id = simple_strtoul(s, &p, 16);
	if (*p++ != ':')
		return -1;
	cnt = 2;
	num[2] = 0;
	for (; *p; p += 2) {
		if (p[1] == 0)
			return -2;
		if (cnt + 3 > len)
			return -3;
		num[0] = p[0];
		num[1] = p[1];
		buf[cnt++] = simple_strtoul(num, NULL, 16);
	}
	buf[0] = id;
	buf[1] = cnt - 2;

	return cnt;
}

int eeprom(int argc, char *argv[])
{
	int i, len, ret;
	unsigned char buf[58], *p;

	app_startup(argv);
	i = get_version();
	if (i != XF_VERSION) {
		printf("Using ABI version %d, but U-Boot provides %d\n",
			XF_VERSION, i);
		return 1;
	}

	if ((SMC_inw(&dev, BANK_SELECT) & 0xFF00) != 0x3300) {
		puts("SMSC91111 not found\n");
		return 2;
	}

	/* Called without parameters - print MAC address */
	if (argc < 2) {
		verify_macaddr(NULL);
		return 0;
	}

	/* Print help message */
	if (argv[1][1] == 'h') {
		puts("NetStar EEPROM writer\n"
			"Built: " U_BOOT_DATE " at " U_BOOT_TIME "\n"
			"Usage:\n\t<mac_address> [<element_1>] [<...>]\n");
		return 0;
	}

	/* Try to parse information elements */
	len = sizeof(buf);
	p = buf;
	for (i = 2; i < argc; i++) {
		ret = parse_element(argv[i], p, len);
		switch (ret) {
		case -1:
			printf("Element %d: malformed\n", i - 1);
			return 3;
		case -2:
			printf("Element %d: odd character count\n", i - 1);
			return 3;
		case -3:
			puts("Out of EEPROM memory\n");
			return 3;
		default:
			p += ret;
			len -= ret;
		}
	}

	/* First argument (MAC) is mandatory */
	set_mac(argv[1]);
	if (verify_macaddr(argv[1])) {
		puts("*** HWaddr does not match! ***\n");
		return 4;
	}

	while (len--)
		*p++ = 0;

	write_data((u16 *)buf, sizeof(buf) >> 1);

	return 0;
}
