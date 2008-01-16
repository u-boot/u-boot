/*
 * Copyright 2006 Freescale Semiconductor
 * York Sun (yorksun@freescale.com)
 * Haiying Wang (haiying.wang@freescale.com)
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
#include <linux/ctype.h>

typedef struct {
	unsigned char id[4];		/* 0x0000 - 0x0003 */
	unsigned char sn[12];		/* 0x0004 - 0x000F */
	unsigned char errata[5];	/* 0x0010 - 0x0014 */
	unsigned char date[7];		/* 0x0015 - 0x001a */
	unsigned char res_1[37];	/* 0x001b - 0x003f */
	unsigned char tab_size;		/* 0x0040 */
	unsigned char tab_flag;		/* 0x0041 */
	unsigned char mac[8][6];	/* 0x0042 - 0x0071 */
	unsigned char res_2[126];	/* 0x0072 - 0x00ef */
	unsigned int crc;		/* 0x00f0 - 0x00f3 crc32 checksum */
} EEPROM_data;

static EEPROM_data mac_data;

int mac_show(void)
{
	int i;
	unsigned char ethaddr[8][18];

	printf("ID %c%c%c%c\n",
	       mac_data.id[0],
	       mac_data.id[1],
	       mac_data.id[2],
	       mac_data.id[3]);
	printf("Errata %c%c%c%c%c\n",
	       mac_data.errata[0],
	       mac_data.errata[1],
	       mac_data.errata[2],
	       mac_data.errata[3],
	       mac_data.errata[4]);
	printf("Date %c%c%c%c%c%c%c\n",
	       mac_data.date[0],
	       mac_data.date[1],
	       mac_data.date[2],
	       mac_data.date[3],
	       mac_data.date[4],
	       mac_data.date[5],
	       mac_data.date[6]);
	for (i = 0; i < 8; i++) {
		sprintf((char *)ethaddr[i],
			"%02x:%02x:%02x:%02x:%02x:%02x",
			mac_data.mac[i][0],
			mac_data.mac[i][1],
			mac_data.mac[i][2],
			mac_data.mac[i][3],
			mac_data.mac[i][4],
			mac_data.mac[i][5]);
		printf("MAC %d %s\n", i, ethaddr[i]);
	}

	setenv("ethaddr",  (char *)ethaddr[0]);
	setenv("eth1addr", (char *)ethaddr[1]);
	setenv("eth2addr", (char *)ethaddr[2]);
	setenv("eth3addr", (char *)ethaddr[3]);

	return 0;
}

int mac_read(void)
{
	int ret, length;
	unsigned int crc = 0;
	unsigned char dev = ID_EEPROM_ADDR, *data;

	length = sizeof(EEPROM_data);
	ret = i2c_read(dev, 0, 1, (unsigned char *)(&mac_data), length);
	if (ret) {
		printf("Read failed.\n");
		return -1;
	}

	data = (unsigned char *)(&mac_data);
	printf("Check CRC on reading ...");
	crc = crc32(crc, data, length - 4);
	if (crc != mac_data.crc) {
		printf("CRC checksum is invalid, in EEPROM CRC is %x, calculated CRC is %x\n",
		     mac_data.crc, crc);
		return -1;
	} else {
		printf("CRC OK\n");
		mac_show();
	}
	return 0;
}

int mac_prog(void)
{
	int ret, i, length;
	unsigned int crc = 0;
	unsigned char dev = ID_EEPROM_ADDR, *ptr;
	unsigned char *eeprom_data = (unsigned char *)(&mac_data);

	for (i = 0; i < sizeof(mac_data.res_1); i++)
		mac_data.res_1[i] = 0;
	for (i = 0; i < sizeof(mac_data.res_2); i++)
		mac_data.res_2[i] = 0;
	length = sizeof(EEPROM_data);
	crc = crc32(crc, eeprom_data, length - 4);
	mac_data.crc = crc;
	for (i = 0, ptr = eeprom_data; i < length; i += 8, ptr += 8) {
		ret =
		    i2c_write(dev, i, 1, ptr,
			      (length - i) < 8 ? (length - i) : 8);
		udelay(5000);	/* 5ms write cycle timing */
		if (ret)
			break;
	}
	if (ret) {
		printf("Programming failed.\n");
		return -1;
	} else {
		printf("Programming %d bytes. Reading back ...\n", length);
		mac_read();
	}
	return 0;
}

int do_mac(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int i;
	char cmd = 's';
	unsigned long long mac_val;

	if (i2c_probe(ID_EEPROM_ADDR) != 0)
		return -1;

	if (argc > 1) {
		cmd = argv[1][0];
		switch (cmd) {
		case 'r':	/* display */
			mac_read();
			break;
		case 's':	/* save */
			mac_prog();
			break;
		case 'i':	/* id */
			for (i = 0; i < 4; i++) {
				mac_data.id[i] = argv[2][i];
			}
			break;
		case 'n':	/* serial number */
			for (i = 0; i < 12; i++) {
				mac_data.sn[i] = argv[2][i];
			}
			break;
		case 'e':	/* errata */
			for (i = 0; i < 5; i++) {
				mac_data.errata[i] = argv[2][i];
			}
			break;
		case 'd':	/* date */
			for (i = 0; i < 7; i++) {
				mac_data.date[i] = argv[2][i];
			}
			break;
		case 'p':	/* number of ports */
			mac_data.tab_size =
			    (unsigned char)simple_strtoul(argv[2], NULL, 16);
			break;
		case '0':	/* mac 0 */
		case '1':	/* mac 1 */
		case '2':	/* mac 2 */
		case '3':	/* mac 3 */
		case '4':	/* mac 4 */
		case '5':	/* mac 5 */
		case '6':	/* mac 6 */
		case '7':	/* mac 7 */
			mac_val = simple_strtoull(argv[2], NULL, 16);
			for (i = 0; i < 6; i++) {
				mac_data.mac[cmd - '0'][i] =
				    *((unsigned char *)
				      (((unsigned int)(&mac_val)) + i + 2));
			}
			break;
		case 'h':	/* help */
		default:
			printf("Usage:\n%s\n", cmdtp->usage);
			break;
		}
	} else {
		mac_show();
	}
	return 0;
}

int mac_read_from_eeprom(void)
{
	int length, i;
	unsigned char dev = ID_EEPROM_ADDR;
	unsigned char *data;
	unsigned char ethaddr[4][18];
	unsigned char enetvar[32];
	unsigned int crc = 0;

	length = sizeof(EEPROM_data);
	if (i2c_read(dev, 0, 1, (unsigned char *)(&mac_data), length)) {
		printf("Read failed.\n");
		return -1;
	}

	data = (unsigned char *)(&mac_data);
	crc = crc32(crc, data, length - 4);
	if (crc != mac_data.crc) {
		return -1;
	} else {
		for (i = 0; i < 4; i++) {
			if (memcmp(&mac_data.mac[i], "\0\0\0\0\0\0", 6)) {
				sprintf((char *)ethaddr[i],
					"%02x:%02x:%02x:%02x:%02x:%02x",
					mac_data.mac[i][0],
					mac_data.mac[i][1],
					mac_data.mac[i][2],
					mac_data.mac[i][3],
					mac_data.mac[i][4],
					mac_data.mac[i][5]);
				sprintf((char *)enetvar,
					i ? "eth%daddr" : "ethaddr",
					i);
				setenv((char *)enetvar, (char *)ethaddr[i]);
			}
		}
	}
	return 0;
}
