/*
 * Copyright 2006, 2008-2009, 2011 Freescale Semiconductor
 * York Sun (yorksun@freescale.com)
 * Haiying Wang (haiying.wang@freescale.com)
 * Timur Tabi (timur@freescale.com)
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
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
 */

#include <common.h>
#include <command.h>
#include <i2c.h>

/* #define DEBUG */

/*
 * static eeprom: EEPROM layout
 */
static struct __attribute__ ((__packed__)) eeprom {
	u8 id[16];		/* 0x01 - 0x0F Type e.g. 100wG-5111 */
	u8 sn[10];		/* 0x10 - 0x19 Serial Number */
	u8 date[6];		/* 0x1A - 0x1F Build Date */
	u8 mac[6];		/* 0x20 - 0x25 MAC address  */
	u8 reserved[10];/* 0x26 - 0x2f reserved */
	u32 crc;        /* x+1         CRC32 checksum */
} e;

/* Set to 1 if we've read EEPROM into memory */
static int has_been_read;

/**
 * show_eeprom - display the contents of the EEPROM
 */
static void show_eeprom(void)
{
	unsigned int crc;
	char safe_string[16];

#ifdef DEBUG
	int i;
#endif
	u8 *p;

	/* ID */
	strncpy(safe_string, (char *)e.id, sizeof(e.id));
	safe_string[sizeof(e.id)-1] = 0;
	printf("ID: mvBlueLYNX-X%s\n", safe_string);

	/* Serial number */
	strncpy(safe_string, (char *)e.sn, sizeof(e.sn));
	safe_string[sizeof(e.sn)-1] = 0;
	printf("SN: %s\n", safe_string);

	/* Build date, BCD date values, as YYMMDDhhmmss */
	printf("Build date: 20%02x/%02x/%02x %02x:%02x:%02x %s\n",
		e.date[0], e.date[1], e.date[2],
		e.date[3] & 0x7F, e.date[4], e.date[5],
		e.date[3] & 0x80 ? "PM" : "");

	/* Show MAC address  */
	p = e.mac;
	printf("Eth: %02x:%02x:%02x:%02x:%02x:%02x\n",
		p[0], p[1], p[2], p[3],	p[4], p[5]);

	crc = crc32(0, (void *)&e, sizeof(e) - 4);

	if (crc == be32_to_cpu(e.crc))
		printf("CRC: %08x\n", be32_to_cpu(e.crc));
	else
		printf("CRC: %08x (should be %08x)\n", be32_to_cpu(e.crc), crc);

#ifdef DEBUG
	printf("EEPROM dump: (0x%x bytes)\n", sizeof(e));
	for (i = 0; i < sizeof(e); i++) {
		if ((i % 16) == 0)
			printf("%02X: ", i);
		printf("%02X ", ((u8 *)&e)[i]);
		if (((i % 16) == 15) || (i == sizeof(e) - 1))
			printf("\n");
	}
#endif
}

/**
 * read_eeprom - read the EEPROM into memory
 */
static int read_eeprom(void)
{
	int ret;
#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	unsigned int bus;
#endif

	if (has_been_read)
		return 0;

#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SYS_EEPROM_BUS_NUM);
#endif

	ret = eeprom_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0,
		(uchar *)&e, sizeof(e));

#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	i2c_set_bus_num(bus);
#endif

#ifdef DEBUG
	show_eeprom();
#endif

	has_been_read = (ret == 0) ? 1 : 0;

	return ret;
}

/**
 *  update_crc - update the CRC
 *
 *  This function should be called after each update to the EEPROM structure,
 *  to make sure the CRC is always correct.
 */
static void update_crc(void)
{
	u32 crc;

	crc = crc32(0, (void *)&e, sizeof(e) - 4);
	e.crc = cpu_to_be32(crc);
}

/**
 * prog_eeprom - write the EEPROM from memory
 */
static int prog_eeprom(void)
{
	int ret = 0;
#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	unsigned int bus;
#endif

	update_crc();

#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SYS_EEPROM_BUS_NUM);
#endif

	ret = eeprom_write(CONFIG_SYS_I2C_EEPROM_ADDR, 0,
		(uchar *)&e, sizeof(e));

	if (!ret) {
		/* Verify the write by reading back the EEPROM and comparing */
		struct eeprom e2;
#ifdef DEBUG
		printf("%s verifying...\n", __func__);
#endif
		ret = eeprom_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0,
			(uchar *)&e2, sizeof(e2));

		if (!ret && memcmp(&e, &e2, sizeof(e)))
			ret = -1;
	}

#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	i2c_set_bus_num(bus);
#endif

	if (ret) {
		printf("Programming failed.\n");
		has_been_read = 0;
		return -1;
	}

	printf("Programming passed.\n");
	return 0;
}

/**
 * h2i - converts hex character into a number
 *
 * This function takes a hexadecimal character (e.g. '7' or 'C') and returns
 * the integer equivalent.
 */
static inline u8 h2i(char p)
{
	if ((p >= '0') && (p <= '9'))
		return p - '0';

	if ((p >= 'A') && (p <= 'F'))
		return (p - 'A') + 10;

	if ((p >= 'a') && (p <= 'f'))
		return (p - 'a') + 10;

	return 0;
}

/**
 * set_date - stores the build date into the EEPROM
 *
 * This function takes a pointer to a string in the format "YYMMDDhhmmss"
 * (2-digit year, 2-digit month, etc), converts it to a 6-byte BCD string,
 * and stores it in the build date field of the EEPROM local copy.
 */
static void set_date(const char *string)
{
	unsigned int i;

	if (strlen(string) != 12) {
		printf("Usage: mac date YYMMDDhhmmss\n");
		return;
	}

	for (i = 0; i < 6; i++)
		e.date[i] = h2i(string[2 * i]) << 4 | h2i(string[2 * i + 1]);

	update_crc();
}

/**
 * set_mac_address - stores a MAC address into the EEPROM
 *
 * This function takes a pointer to MAC address string
 * (i.e."XX:XX:XX:XX:XX:XX", where "XX" is a two-digit hex number) and
 * stores it in the MAC address field in the EEPROM local copy.
 */
static void set_mac_address(const char *string)
{
	char *p = (char *) string;
	unsigned int i;

	for (i = 0; *p && (i < 6); i++) {
		e.mac[i] = simple_strtoul(p, &p, 16);
		if (*p == ':')
			p++;
	}

	update_crc();
}

int do_mac(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char cmd;

	if (argc == 1) {
		show_eeprom();
		return 0;
	}

	cmd = argv[1][0];

	if (cmd == 'r') {
#ifdef DEBUG
		printf("%s read\n", __func__);
#endif
		read_eeprom();
		return 0;
	}

	if (argc == 2) {
		switch (cmd) {
		case 's':	/* save */
#ifdef DEBUG
			printf("%s save\n", __func__);
#endif
			prog_eeprom();
			break;
		default:
			return cmd_usage(cmdtp);
		}

		return 0;
	}

	/* We know we have at least one parameter  */

	switch (cmd) {
	case 'n':	/* serial number */
#ifdef DEBUG
		printf("%s serial number\n", __func__);
#endif
		memset(e.sn, 0, sizeof(e.sn));
		strncpy((char *)e.sn, argv[2], sizeof(e.sn) - 1);
		update_crc();
		break;
	case 'd':	/* date BCD format YYMMDDhhmmss */
		set_date(argv[2]);
		break;
	case 'e':	/* errata */
		printf("mac errata not implemented\n");
		break;
	case 'i':	/* id */
		memset(e.id, 0, sizeof(e.id));
		strncpy((char *)e.id, argv[2], sizeof(e.id) - 1);
		update_crc();
		break;
	case 'p':	/* ports */
		printf("mac ports not implemented (always 1 port)\n");
		break;
	case '0' ... '9':
		/* we only have "mac 0" but any digit can be used here */
		set_mac_address(argv[2]);
		break;
	case 'h':	/* help */
	default:
		return cmd_usage(cmdtp);
	}

	return 0;
}

int mac_read_from_eeprom(void)
{
	u32 crc, crc_offset = offsetof(struct eeprom, crc);
	u32 *crcp; /* Pointer to the CRC in the data read from the EEPROM */

	if (read_eeprom()) {
		printf("EEPROM Read failed.\n");
		return -1;
	}

	crc = crc32(0, (void *)&e, crc_offset);
	crcp = (void *)&e + crc_offset;
	if (crc != be32_to_cpu(*crcp)) {
		printf("EEPROM CRC mismatch (%08x != %08x)\n", crc,
			be32_to_cpu(e.crc));
		return -1;
	}

	if (memcmp(&e.mac, "\0\0\0\0\0\0", 6) &&
		memcmp(&e.mac, "\xFF\xFF\xFF\xFF\xFF\xFF", 6)) {
		char ethaddr[9];

		sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
			e.mac[0],
			e.mac[1],
			e.mac[2],
			e.mac[3],
			e.mac[4],
			e.mac[5]);
		/* Only initialize environment variables that are blank
		 * (i.e. have not yet been set)
		 */
		if (!getenv("ethaddr"))
			setenv("ethaddr", ethaddr);
	}

	if (memcmp(&e.sn, "\0\0\0\0\0\0\0\0\0\0", 10) &&
		memcmp(&e.sn, "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 10)) {
		char serial_num[12];

		strncpy(serial_num, (char *)e.sn, sizeof(e.sn) - 1);
		/* Only initialize environment variables that are blank
		 * (i.e. have not yet been set)
		 */
		if (!getenv("serial#"))
			setenv("serial#", serial_num);
	}

	/* TODO should I calculate CRC here? */
	return 0;
}

#ifdef CONFIG_SERIAL_TAG
void get_board_serial(struct tag_serialnr *serialnr)
{
	char *serial = getenv("serial#");

	if (serial && (strlen(serial) > 3)) {
		/* use the numerical part of the serial number LXnnnnnn */
		serialnr->high = 0;
		serialnr->low = simple_strtoul(serial + 2, NULL, 10);
	} else {
		serialnr->high = 0;
		serialnr->low = 0;
	}
}
#endif
